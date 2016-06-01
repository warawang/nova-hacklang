<?hh //strict
	namespace nova\image;

	use nova\image\ColorGroup;
	use nova\image\exception\ImageException;
	use nova\image\exception\ImageRotateException;
	use nova\image\exception\ImageCopyException;
	use nova\image\exception\ImageResourceException;
	use nova\image\exception\ImageTrimException;
	use nova\io\FileUtils;

	class Image {

		// 이미지의 형태를 파악할 때 사용되는 기준 ( 가로 / 세로 )
		const HORIZONTAL_WIDE_RATIO = 2.8; // 이와 같거나 크면 가로로 긴 이미지로 판단
		const VERTICAL_LONG_RATIO = 0.55; // 이와 같거나 작으면 세로로 긴 이미지로 판단

		const COLOR_BLANK_DEVIATION = 7.5; //특정 영역을 구성하는 필셀별 색생의 표준 편차가 여기에 정의된 숫자 이하이면 공백으로 본다.
		const DIVIDED_DIFF = 75; //각 라인에 사용된 색상 그룹의 유사성이 이 이상이면 분절된 지점으로 판단한다.
    const TOPPIECE_LIMIT = 1024; //DEFAULT_TOPPIECE 까지 분절점이 확인되지 않으면 찾지 못한 것으로 본다

		// Properties
		private ?resource $im = NULL;
		private ?array<mixed,mixed> $exif = NULL;
		private int $imageType = 0;
		private ?Vector<int> $imageSize;
		private Map<string, ColorGroup> $colorGroupCache = Map{};
		private ?ResizeResult $resizeResult;
		private ?Image $topPiece;
		private bool $failCreateTopPiece = false; // topPiece 생성을 시도하였으나, 실패한 내역이 있을 경우 true. 오버헤드를 방지하기 위함
		private bool $isTopPiece = false;

		public function __construct(?resource $im = NULL) {
			if($im !== NULL) $this->im = $im;
		}

		public function __destruct() {
			$this->clear();
		}

		public function newImage(?resource $im) : Image {
			//현재 이미지로 부터 새로운 이미지 클래스를 생설할 떄, 영속성을 보상해야 하는 정보를 주입한다.
			$newImage = new Image($im);
			$newImage->setIsTopPiece($this->isTopPiece());
			return $newImage;
		}

		public function isLoaded() : bool {
			if($this->im == false || $this->im === NULL) return false;

			return true;
		}

		public function getResource() : resource {
			if($this->im == false || $this->im === NULL) {
				throw new ImageResourceException("Image resource not found.");
			}

			return $this->im;
		}

		public function getImageType() : int {
			return $this->imageType;
		}

		//특정 이미지를 열어 정보를 반환한다.
		public function open(string $path) : bool {
			if(!is_file($path)) {
				return false;
			}

			//이미지 타입을 알 수 없는 경우
			if(!$type = exif_imagetype($path)) {
				return false;
			}
			$this->imageType = $type;

			switch($type)
			{
				case IMAGETYPE_JPEG:
					$this->im = @imagecreatefromjpeg($path);

					//exif 정보 로딩
					$this->exif = @exif_read_data($path, "COMPUTED");
				break;

				case IMAGETYPE_GIF:
					$this->im = @imagecreatefromgif($path);
				break;

				case IMAGETYPE_PNG:
					$this->im = @imagecreatefrompng($path);
				break;

				case IMAGETYPE_WBMP:
					$this->im = @imagecreatefromwbmp($path);
				break;

				case IMAGETYPE_BMP:
					$this->im = @$this->imagecreatefrombmp($path);
				break;

				default:
					return false;
				break;
			}

			// imagecreatefromwXXX 에서 정상적으로 리소스가 리턴되었으나, 사용할 수 없는 이미지인 경우가 있음.
			if(imagesx($this->im) <=0 || imagesy($this->im) <=0)  {
				$this->im = NULL;
			}

			if(!$this->isLoaded()) return false;

			return true;
		}

		public function clear() : void {
			if($this->isLoaded()) {
				@imagedestroy($this->im);
				$this->im = null;
			}
		}

		//Save as
		public function savePNG(string $path) : bool {
			if(!$this->isLoaded()) return false;
			return imagepng($this->im, $path);
		}

		public function saveJPEG(string $path, int $quality=95) : bool {
      //print_r($this->getSize());
			if(!$this->isLoaded()) return false;
			return imagejpeg($this->im,$path,$quality);
		}

		public function save(int $imageType, string $path, int $quality=95) : bool {
			if($imageType == IMAGETYPE_PNG) {
				return $this->savePNG($path);
			} else if($imageType == IMAGETYPE_JPEG) {
				return $this->saveJPEG($path, $quality);
			} else {
				return false;
			}
		}

		public function rotate(int $angle) : Image
		{
			if(!$im_out = imagerotate($this->im, $angle, -1)) {
				throw new ImageRotateException("Failed to image rotate.");
			}

			return $this->newImage($im_out);
		}

		public function mirror() : Image {
			list($width,$height) = $this->getSize();

			$im_out = imagecreatetruecolor($width, $height);
			if(!imagecopyresampled($im_out, $this->im, 0, 0, ($width-1), 0, $width, $height, 0-$width, $height)) {
				throw new ImageRotateException("Failed to image copy.");
			}

			return $this->newImage($im_out);
		}

		public function crop(int $x, int $y, int $width, int $height) : Image {
			if(!$this->isLoaded()) {
				throw new ImageResourceException("Image resource not found.");
			}

			$im_out = imagecreatetruecolor($width, $height);
			imagealphablending($im_out, false);
			if(!imagecopyresampled($im_out, $this->im, 0, 0, $x, $y, $width, $height, $width, $height)) {
				imagedestroy($im_out);
				throw new ImageRotateException("Failed to image copy.");
			}
			imagesavealpha($im_out, true);

			$croped = $this->newImage($im_out);
			return $croped;
		}

		public function rotateByExif() : Image {
			if(!$this->isLoaded()) {
				throw new ImageResourceException("Image resource not found.");
			}
			if($this->exif === NULL) return $this;
			if($this->exif['Orientation'] === NULL) return $this;

			switch((int)$this->exif['Orientation'])
			{
				case 1:
					return $this;
				break;

				case 2:
					return $this->mirror();
				break;

				case 3:
					return $this->rotate(180);
				break;

				case 4:
					return $this->rotate(180)->mirror();
				break;

				case 5:
					return $this->rotate(-90)->mirror();
				break;

				case 6:
					return $this->rotate(-90);
				break;

				case 7:
					return $this->rotate(90)->mirror();
				break;

				case 8:
					return $this->rotate(90);
				break;

				default:
					return $this;
				break;
			}

			return $this;
		}

		public function getWidth() : int {
			if(!$this->isLoaded()) {
				throw new ImageResourceException("Image resource not found.");
			}
			return imagesx($this->im);

		}

		public function getHeight() : int {
			if(!$this->isLoaded()) {
				throw new ImageResourceException("Image resource not found.");
			}
			return imagesy($this->im);

		}

		public function getSize() : Vector<int> {
			if(!$this->isLoaded()) {
				throw new ImageResourceException("Image resource not found.");
			}

			if($this->imageSize === NULL) {
				$this->imageSize = Vector {
					imagesx($this->im),
					imagesy($this->im)
				};
			}

			return $this->imageSize;
		}

		/*
		destWidth,destHeight 안에 이미지의 모든 영역이 뿌려질 수 있는 축소 비율을 리턴한다
		*/
		public function getFitRatio(int $srcWidth, int $srcHeight, int $destWidth, int $destHeight) : float {
			$fitRatioX = $destWidth/$srcWidth;
			$fitRatioY = $destHeight/$srcHeight;

			return (float)min($fitRatioX,$fitRatioY);
		}

		public function getFillRatio(int $srcWidth, int $srcHeight, int $destWidth, int $destHeight) : float {
			$fitRatioX = $destWidth/$srcWidth;
			$fitRatioY = $destHeight/$srcHeight;

			return (float)max($fitRatioX,$fitRatioY);
		}

		public function getSizeType() : ImageSizeType {
			list($width,$height) = $this->getSize();

			if(abs($width-$height) < max($width,$height)*0.25) {
				return ImageSizeType::Square;
			} else {
				if($width>$height) {
					if($width/$height >= self::HORIZONTAL_WIDE_RATIO) return ImageSizeType::HorizontalWide;
					else return ImageSizeType::Horizontal;
				} else {
					if($width/$height <= self::VERTICAL_LONG_RATIO) return ImageSizeType::VerticalLong;
					else return ImageSizeType::Vertical;
				}
			}
		}

		public function getRecommResizeType() : ImageResizeType {
			$sizeType = $this->getSizeType();
			switch($sizeType) {
				case ImageSizeType::Square:
					return ImageResizeType::VerticalFace;
				break;

				case ImageSizeType::Vertical:
					if($this->isTopPiece() || $this->failCreateTopPiece) return ImageResizeType::VerticalFace;
					else return ImageResizeType::VerticalTopPiece;
				break;

				case ImageSizeType::VerticalLong:
					if($this->isTopPiece() || $this->failCreateTopPiece) return ImageResizeType::VerticalTop;
					else return ImageResizeType::VerticalTopPiece;
				break;

				case ImageSizeType::Horizontal:
					return ImageResizeType::VerticalMiddle;
				break;

				case ImageSizeType::HorizontalWide:
					return ImageResizeType::HorizontalLeft;
				break;

			}
		}

		/*
			https://docs.google.com/document/d/1XH_cDXnxf_OdI0ZsIdH-dKWk_3WCs8Y3QHYt6lHvobI/edit
		*/
		public function resize(int $destWidth, int $destHeight, ImageResizeType $type = ImageResizeType::Normal, bool $keepTransparent = false) : Image {
			if(!$this->isLoaded()) {
				throw new ImageResourceException("Image resource not found.");
			}

			list($orgWidth, $orgHeight) = $this->getSize();

			$srcX = 0;
			$srcY = 0;
			$srcWidth = 0;
			$srcHeight = 0;
			$outX = 0;
			$outY = 0;

			switch($type) {
				//요청받은 가로 넓이 기준 리사이징
				//원본의 가로 넓이가 요청한 것 보다 작을 경우, 요청 넗이만큼 확장된다.
				//별도로 요청된 높이가 있을 경우, 상단을 기준으로 크롭. 요청된 높이가 없을 경우 가로 변경 비율을 반영하여 모두 포함된다.
				case ImageResizeType::Normal:
					//출력될 이미지크기 계산
					$outWidth = $destWidth;
					$outHeight = round($orgHeight*($destWidth/$orgWidth));
					/*
					$outWidth = $destWidth > $orgWidth ? $orgWidth : $destWidth;
					$outHeight = round($orgHeight*($outWidth/$orgWidth));
					 */

					//원본에서 소스로 사용할 영역 지정
					$srcX = 0;
					$srcY = 0;
					$srcWidth = $orgWidth;
					if($destHeight > 0 && $outHeight>$destHeight) {
						$srcHeight = round($orgHeight*($destHeight/$outHeight));
						$outHeight = $destHeight;
					} else {
						$srcHeight = $orgHeight;
					}
				break;

				case ImageResizeType::Fit:
					$outWidth = $destWidth;
					$outHeight = $destHeight;

					$fitRatio = $this->getFitRatio($orgWidth,$orgHeight,$outWidth,$outHeight);
					$fitWidth = $orgWidth*$fitRatio;
					$fitHeight = $orgHeight*$fitRatio;

					$overRatioWidth = ($fitWidth-$outWidth)/$fitWidth;
					$overRatioHeight = ($fitHeight-$outHeight)/$fitHeight;

					$srcX = round($orgWidth*($overRatioWidth/2));
					$srcY = round($orgHeight*($overRatioHeight/2));
					$srcWidth = $orgWidth-($srcX*2);
					$srcHeight = $orgHeight-($srcY*2);
				break;

				case ImageResizeType::FitAndCut:
					$fitRatio = $this->getFitRatio($orgWidth,$orgHeight,$destWidth,$destHeight);
					if($fitRatio>=1) return $this;
					$outWidth = $orgWidth*$fitRatio;
					$outHeight = $orgHeight*$fitRatio;

					$srcX = 0;
					$srcY = 0;
					$srcWidth = $orgWidth;
					$srcHeight = $orgHeight;
				break;

				case ImageResizeType::VerticalMiddle:
					//요청 받은 크기에 맞춘다
					$outWidth = $destWidth;
					$outHeight = $destHeight;

					//요청받은 넓이에 이미지를 맞추고 보정을 위한 비율 계산
					$fromRatio = $orgWidth/$outWidth;

					$srcX = 0;
					$srcY = round(($orgHeight-($destHeight*$fromRatio))/2);
					$srcWidth = $orgWidth;
					$srcHeight = $orgHeight-($srcY*2);
				break;

				case ImageResizeType::HorizontalLeft:
					//출력 이미지 크기를 요청 받은 크기에 맞춘다
					$outWidth = $destWidth;
					$outHeight = $destHeight;

					//요청받은 높이에 이미지를 맞추고 보정을 위한 비율 계산
					$fromRatio = $orgHeight/$outHeight;

					$srcWidth = $orgWidth-($orgWidth-round($destWidth*$fromRatio));
					$srcHeight = $orgHeight;
					$srcX = 0;
					$srcY = 0;
				break;

				case ImageResizeType::VerticalTop:
					//요청 받은 크기에 맞춘다
					$outWidth = $destWidth;
					$outHeight = $destHeight;

					//요청받은 넓이에 이미지를 맞추고 보정을 위한 비율 계산
					$fromRatio = $orgWidth/$outWidth;
					$toRatio = $outWidth/$orgWidth;

					$srcX = 0;
					$srcY = 0;
					$srcWidth = $orgWidth;
					$srcHeight = (int)round($orgHeight-($orgHeight-($destHeight*$fromRatio)));

					// 섬네일 영역이 컨텐츠 보다 클 경우 센터정렬
					$realSrcHeight = ($orgHeight*$toRatio)-($srcY*$toRatio);
					if($destHeight>$realSrcHeight) $outY = round(($destHeight-$realSrcHeight)/2);

				break;

				case ImageResizeType::VerticalFace:
					//요청 받은 크기에 맞춘다
					$outWidth = $destWidth;
					$outHeight = $destHeight;

					//요청받은 넓이에 이미지를 맞추고 보정을 위한 비율 계산
					$fromRatio = $orgWidth/$outWidth;
					$toRatio = $outWidth/$orgWidth;

					$srcX = 0;
					$srcY = $destHeight>$orgHeight*$toRatio ? 0 : min(round($orgHeight/10),abs($destHeight-($orgHeight*$toRatio))*$fromRatio/2);

					$srcWidth = $orgWidth;
					$srcHeight = round($orgHeight-($orgHeight-($outHeight*$fromRatio)));

					// 섬네일 영역이 컨텐츠 보다 클 경우 센터정렬
					// TODO 이와 같은 상황이면 상단에 잘리는 부분이 없도록 하자.
					$realSrcHeight = ($orgHeight*$toRatio)-($srcY*$toRatio);
					if($destHeight>$realSrcHeight) $outY = round(($destHeight-$realSrcHeight)/2);


				break;

				case ImageResizeType::VerticalTopPiece:
					// 리사이징의 대상이 되는 이미지가 topPiece로 추출된 이미지가 아니고,
					// topPiece 추출을 시도하였으나 실패했던 기록이 없고(오버헤드 방지 / failCreateTopPiece)
					// topPiece가 정상적으로 생성되었다면
					// 요청받은 사이즈로 리사이징하여 리턴.
					if(!$this->isTopPiece() && $this->createTopPiece() && $this->topPiece !== NULL) {
						return $this->topPiece->trim()->resize($destWidth,$destHeight, $this->getRecommResizeType(), $keepTransparent);
					}

					return $this->resize($destWidth,$destHeight, $this->getRecommResizeType(), $keepTransparent);
				break;

				case ImageResizeType::Auto:
					return $this->resize($destWidth,$destHeight, $this->getRecommResizeType(), $keepTransparent);
				break;
			}

			$im = imagecreatetruecolor($outWidth, $outHeight);
			if(!$keepTransparent) {
				//$whiteColor = imagecolorallocate($im, 255, 255, 255);
				$blackColor = imagecolorallocate($im, 0, 0, 0);
				imagefill($im, 0, 0, $blackColor);
			} else {
				imagealphablending($im, false);
				imagesavealpha($im,true);
			}

			if(!imagecopyresampled($im, $this->im, $outX, $outY, $srcX, $srcY, $outWidth, $outHeight, $srcWidth, $srcHeight)) {
				imagedestroy($im);
				throw new ImageCopyException("Failed to image copy.");
			}

			$result = $this->newImage($im);
			$result->setResizeResult(shape(
				"type" => $type,
				"cropX" => $srcX,
				"cropY" => $srcY,
				"cropWidth" => $srcWidth,
				"cropHeight" => $srcHeight,
				"isTopPiece" => $this->isTopPiece()
			));

			return $result;
		}

		public function setResizeResult(ResizeResult $result) : void {
			$this->resizeResult = $result;
		}

		public function getResizeResult() : ?ResizeResult {
			return $this->resizeResult;
		}

		public function imagecreatefrombmp(string $p_sFile) : ?resource {
			$file = fopen($p_sFile,"rb");
			$read = fread($file,10);
			while(!feof($file)&&($read!=""))
			{
				$read .= fread($file,1024);
			}

			$temp = unpack("H*",$read);
			$hex = $temp[1];
			$header = substr($hex,0,108);

			if(substr($header,0,4)=="424d") {
				$header_parts = str_split($header,2);
				$width = hexdec($header_parts[19].$header_parts[18]);
				$height = hexdec($header_parts[23].$header_parts[22]);
			} else return NULL;

			$x = 0;
			$y = 1;

			$image = imagecreatetruecolor($width,$height);
			$body = substr($hex,108);

			$body_size = (strlen($body)/2);
			$header_size = ($width*$height);

			$usePadding = ($body_size>($header_size*3)+4);

			for($i=0;$i<$body_size;$i+=3)
			{
				if($x>=$width)
				{
					if ($usePadding)
					{
						$i += $width%4;
					}

					$x = 0;
					$y++;

					if ($y>$height)  break;
				}

				$i_pos = $i*2;
				$r = hexdec($body[$i_pos+4].$body[$i_pos+5]);
				$g = hexdec($body[$i_pos+2].$body[$i_pos+3]);
				$b = hexdec($body[$i_pos].$body[$i_pos+1]);

				$color = imagecolorallocate($image,$r,$g,$b);
				imagesetpixel($image,$x,$height-$y,$color);

				$x++;
			}

			return $image;
		}

		/**
		 * 지정 영역에 사용된 컬러와 사용 횟수를 계산한다.
		 *
		 * @param  {[type]} int $x            [description]
		 * @param  {[type]} int $y            [description]
		 * @param  {[type]} int $width        [description]
		 * @param  {[type]} int $height       [description]
		 * @return {[type]}     [description]
		 */
		public function getColorMap(int $x, int $y, int $width, int $height) : Map<int, int> {
			$colorMap = Map {};

				for($i = 0; $i < $width; $i++) {
	 				for($ii = 0; $ii < $height; $ii++) {
					$color = imagecolorat($this->im,$x+$i,$y+$ii);
					invariant(is_int($color), "failed to get color. X:".($x+$i).", Y:".($y+$ii));

					if($colorMap->containsKey($color)) {
						$colorMap[$color]++;
					} else {
						$colorMap[$color] = 1;
					}
 				}
 			}

			return $colorMap;
		}

		/**
		 * 지징된 영역이 모두 ignore color 로 채워져있는지 확인한다.
		 *
		 * @param  {[type]}  int $x            [description]
		 * @param  {[type]}  int $y            [description]
		 * @param  {[type]}  int $width        [description]
		 * @param  {[type]}  int $height       [description]
		 * @return {Boolean}     [description]
		 */
		public function isFillIgnoreColor(int $destX, int $destY, int $destWidth, int $destHeight) : bool {
			for($x = 0; $x < $destWidth; $x++) {
				for($y = 0; $y < $destHeight; $y++) {
					if(!$this->isIgnoreColor($this->getRgbaAt($destX+$x, $destY+$y))) return false;
				}
			}

			return true;
		}

		/**
		 * 지정된 영역 외곽의 ignore 컬러를 제외한 컨텐츠 영역의 좌표를 리턴한다.
		 *
		 * @param  {[type]} int $x            [description]
		 * @param  {[type]} int $y            [description]
		 * @param  {[type]} int $width        [description]
		 * @param  {[type]} int $height       [description]
		 * @return {[type]}     [description]
		 */
		public function trimCoordiWithIgnoreColor(int $destX, int $destY, int $destWidth, int $destHeight) : Coordinate {
			$trimL = 0;
			$trimT = 0;
			$trimR = 0;
			$trimB = 0;

			/***
			L ( 왼쪽 공백 )
			***/
			for($x = 0; $x < $destWidth; $x++) {
				if(!$this->isFillIgnoreColor($x+$destX, $destY, 1, $destHeight)) break;
				$trimL = $x+1;
			}

			// 정말 비워져 있는지 확인한다 ( 트림 영역이 2 이상일 때만 검사, 최저 2개는 되어야 색상 표준편차를 통한 공백 여부 확인 가능 )
			if($trimL > 1 && $this->isOverDeviation($this->getColorDeviation($destX, $destY, $trimL, $destHeight), self::COLOR_BLANK_DEVIATION)) $trimL = 0;

			// 지정된 영역이 모두 공백이면
			if($trimL >= $destWidth) return shape("x" => $destX, "y" => $destY, "width" => 0, "height" => 0);

			/***
			T ( 상단 공백 )
			***/
			for($y = 0; $y < $destHeight; $y++) {
				if(!$this->isFillIgnoreColor($destX, $y+$destY, $destWidth, 1)) break;
				$trimT = $y+1;
			}

			// 정말 비워져 있는지 확인한다 ( 트림 영역이 2 이상일 때만 검사, 최저 2개는 되어야 색상 표준편차를 통한 공백 여부 확인 가능 )
			if($trimT > 1 && $this->isOverDeviation($this->getColorDeviation($destX, $destY, $destWidth, $trimT), self::COLOR_BLANK_DEVIATION)) $trimT = 0;

			//지정된 영역이 모두 공백이면
			if($trimT >= $destHeight) return shape("x" => $destX, "y" => $destY, "width" => 0, "height" => 0);

			/***
			R ( 우측 공백 )
			***/
			for($x = 1; $x < $destWidth; $x++) {
				if(!$this->isFillIgnoreColor($destWidth-$x, $destY, 1, $destHeight)) break;
				$trimR = $x;
			}

			// 정말 비워져 있는지 확인한다 ( 트림 영역이 2 이상일 때만 검사, 최저 2개는 되어야 색상 표준편차를 통한 공백 여부 확인 가능 )
			if($trimR > 1 && $this->isOverDeviation($this->getColorDeviation($destWidth-$trimR, $destY, $trimR, $destHeight), self::COLOR_BLANK_DEVIATION)) $trimR = 0;

			//지정된 영역이 모두 공백이면
			if($trimR >= $destWidth) return shape("x" => $destX, "y" => $destY, "width" => 0, "height" => 0);

			/***
			B ( 하단 공백 )
			***/
			for($y = 1; $y < $destHeight; $y++) {
				if(!$this->isFillIgnoreColor($destX, $destHeight-$y, $destWidth, 1)) break;
				$trimB = $y;
			}

			// 정말 비워져 있는지 확인한다 ( 트림 영역이 2 이상일 때만 검사, 최저 2개는 되어야 색상 표준편차를 통한 공백 여부 확인 가능 )
			if($trimB > 1 && $this->isOverDeviation($this->getColorDeviation($destX, $destHeight-$trimB, $destWidth, $trimB), self::COLOR_BLANK_DEVIATION)) $trimB = 0;

			$trimWidth = $destWidth-$trimL-$trimR;
			$trimHeight = $destHeight-$trimT-$trimB;

			return shape("x" => $trimL, "y" => $trimT, "width" => $trimWidth, "height" => $trimHeight);
		}


		/**
		 * 지정 영역에 사용된 컬러의 RGB 코드를 그룹으로 나누어 그룹핑 하고
		 * 각 색상 그룹 별 사용 횟수를 리턴한다.
		 *
		 * @param  {[type]} int $x            [description]
		 * @param  {[type]} int $y            [description]
		 * @param  {[type]} int $width        [description]
		 * @param  {[type]} int $height       [description]
		 * @return {[type]}	int $groupCount	몇개의 그룹으로 나누어 계산할 지 정한다.  기본은 50개 그룹
		 */
		public function getColorGroup(int $destX, int $destY, int $destWidth, int $destHeight, float $groupCount = 25.5, bool $trimIgnoreColor = true) : ColorGroup {
			$colorMap = Map {};
			$cacheKey = $destX."x".$destY."-".$destWidth."x".$destHeight."-".$groupCount."-".($trimIgnoreColor ? 1 : 0);
			if($this->colorGroupCache->containsKey($cacheKey)) return $this->colorGroupCache[$cacheKey];

			if($trimIgnoreColor) {
				$trimCoordi = $this->trimCoordiWithIgnoreColor($destX,$destY,$destWidth,$destHeight);
				$destX += $trimCoordi["x"];
				$destY += $trimCoordi["y"];

				if($trimCoordi["width"] < $destWidth*0.3) $destWidth = 0; // 전체 여역의 70% 이상이 트림되었다면 분석 하지 않는다
				else $destWidth = $trimCoordi["width"];

				if($trimCoordi["height"] < $destHeight*0.3) $destHeight = 0; // 전체 영역의 70% 이상이 트림되었다면 분석 하지 않는다
				else $destHeight = $trimCoordi["height"];
			}

			//16777215 = white

			for($i = 0; $i < $destWidth; $i++) {
				for($ii = 0; $ii < $destHeight; $ii++) {
					$color = $this->getRgbaAt($destX+$i,$destY+$ii);

					$color["r"] = floor($color["r"]/$groupCount)*$groupCount;
					$color["g"] = floor($color["g"]/$groupCount)*$groupCount;
					$color["b"] = floor($color["b"]/$groupCount)*$groupCount;
					$color["a"] = floor($color["a"]/$groupCount)*$groupCount;

					$key = $color["r"].":".$color["g"].":".$color["b"].":".$color["a"];

					if($colorMap->containsKey($key)) {
						$colorMap[$key]++;
					} else {
						$colorMap[$key] = 1;
					}
				}
			}

			//$colorGroupCache[$]
			$this->colorGroupCache[$cacheKey] = new ColorGroup($destWidth, $destHeight, $colorMap);
			return $this->colorGroupCache[$cacheKey];
		}



		/**
		 * 지정된 영역에 분포된 색상들의 RGB Sum(R+G+B) 표준편차를 구한다.
		 * @param  {[type]} int $x            [description]
		 * @param  {[type]} int $y            [description]
		 * @param  {[type]} int $width        [description]
		 * @param  {[type]} int $height       [description]
		 * @return {[type]}     [description]
		 */

		 public function getColorDeviation(int $x, int $y, int $width, int $height) : ColorDeviation {
 			if(!$this->isLoaded()) {
 				throw new ImageResourceException("Image resource not found.");
 			}

			//print_r(func_get_args());

			$list = Map {"r"=>Vector {}, "g"=>Vector {}, "b"=>Vector {}, "a"=>Vector {}};
			$sum = Map {"r"=>0, "g"=>0, "b"=>0, "a"=>0};

 			for($i = 0; $i < $width; $i++) {
 				for($ii = 0; $ii < $height; $ii++) {
 					$color = $this->getRgbaAt($x+$i,$y+$ii);
					foreach($sum as $colorCode => $value) {
						$colorCode = (string)$colorCode;
						// UNSAFE
						$list[$colorCode]->add($color[$colorCode]);
						$sum[$colorCode] += $color[$colorCode];
					}
 				}
 			}

 			//표준편차 계산
 			$totalPixel = $width * $height;
			$avg = Map {"r"=>0, "g"=>0, "b"=>0, "a"=>0};

			foreach($avg as $colorCode => $value) {
				$avg[$colorCode] = $sum[$colorCode] / $totalPixel;
			}


			$dSum = Map {"r"=>0, "g"=>0, "b"=>0, "a"=>0};
			foreach($dSum as $colorCode => $value) {
				foreach($list[$colorCode] as $at) {
					$dSum[$colorCode] += pow($avg[$colorCode]-$at,2);
	 				//$deviationSum += pow($avg-$at,2);
	 			}
			}

 			return shape(
				"r" => sqrt($dSum["r"]/$totalPixel),
				"g" => sqrt($dSum["g"]/$totalPixel),
				"b" => sqrt($dSum["b"]/$totalPixel),
				"a" => sqrt($dSum["a"]/$totalPixel));
 		}


		/**
		 * 이미지 분절 지점 확인시 무시할 흰색계열/검색은계열 색상인지 확인한다.
		 * @param  {[type]}  RGB $color        [description]
		 * @return {Boolean}     [description]
		 */
		public function isWhitishColor(Rgba $color) : bool {
			if($color["r"] >= 230
				 && $color["g"] >= 230
				 && $color["b"] >= 230) return true;
		 	else return false;
		}

		public function isBlackishColor(Rgba $color) : bool {
			return false; //2015.12.20 검은색 ignore 컬러는 사용안함

			if($color["r"] <= 25
 				 && $color["g"] <= 25
 				 && $color["b"] <= 25 ) return true;
		 	else return false;
		}

		public function isIgnoreColor(Rgba $color) : bool {
			return $this->isWhitishColor($color) || $this->isBlackishColor($color);
		}


		/**
		 * 두 색상의 R+G+B 값의 차이를 구한다.
		 * @param  {[type]} $color1 [description]
		 * @param  {[type]} $color2 [description]
		 * @return {[type]}         [description]
		 */
		public function getColorDiff(Rgba $color1, Rgba $color2) : int {
			return abs($color1["r"]-$color2["r"]) +
			abs($color1["g"]-$color2["g"]) +
			abs($color1["b"]-$color2["b"]) +
			abs($color1["a"]-$color1["a"]);
		}

		public function color2rgba(mixed $color) : Rgba {
			invariant(is_int($color), "failed to get color");
			return shape("r" => ($color >> 16) & 0xFF, "g" => ($color >> 8) & 0xFF, "b" => $color & 0xFF, "a" => ($color & 0x7F000000) >> 24);
		}

		public function getRgbaSum(Rgba $color) : int {
			return $color["r"] + $color["g"] + $color["b"];
		}

		public function getRgbaSumAt(int $x, int $y) : int {
			if(!$this->isLoaded()) {
				throw new ImageResourceException("Image resource not found.");
			}

			$color = imagecolorat($this->im, $x, $y);
			invariant(is_int($color), "failed to get color. X:{$x}, Y:{$y}");

			return $this->getRgbaSum($this->color2rgba($color));
		}

		public function getRgbaAt(int $x, int $y) : Rgba {
			if(!$this->isLoaded()) {
				throw new ImageResourceException("Image resource not found.");
			}
			$color = imagecolorat($this->im, $x, $y);
			invariant(is_int($color), "failed to get color. X:{$x}, Y:{$y}");

			return $this->color2rgba($color);
		}

		private function isOverDeviation(ColorDeviation $deviation, float $standard) : bool {
			return ($deviation["r"] > $standard
							|| $deviation["g"] > $standard
							|| $deviation["b"] > $standard
							|| $deviation["a"] > $standard) ? true : false;
		}


		/**
		 * 공백으로 추정되는 이미지 상/하/좌/우 영역을 크롭한다.
		 * @return {[type]} [description]
		 */
		public function trim() : Image {
			if(!$this->isLoaded()) {
				throw new ImageResourceException("Image resource not found.");
			}

			list($width,$height) = $this->getSize();

      /**
       * 임시처리, 너무 큰 이미지는 스킵.
       * @param  {[type]} $width*$height>5000000 [description]
       * @return {[type]}                        [description]
       */
      if($width*$height>5000000) return $this;

			$blankL = 0;
			$blankT = 0;
			$blankR = 0;
			$blankB = 0;

			//L ( 왼쪽 공백 )
			for($x=0; $x<$width; $x++) {
				if($this->isOverDeviation($this->getColorDeviation($x, 0, 1, $height), self::COLOR_BLANK_DEVIATION)) break;
				$blankL = $x+1;
			}

			//T ( 상단 공백 )
			for($y=0; $y<$height; $y++) {
				if($this->isOverDeviation($this->getColorDeviation(0, $y, $width, 1), self::COLOR_BLANK_DEVIATION)) break;
				$blankT = $y+1;
			}

			//W ( 우측 공백 )
			for($x=1; $x<$width; $x++) {
				if($this->isOverDeviation($this->getColorDeviation($width-$x, 0, 1, $height), self::COLOR_BLANK_DEVIATION)) break;
				$blankR = $x;
			}

			//H ( 하단 공백 )
			for($y=1; $y<$height; $y++) {
				if($this->isOverDeviation($this->getColorDeviation(0, $height-$y, $width, 1), self::COLOR_BLANK_DEVIATION)) break;
				$blankB = $y;
			}

			$cropWidth = $width-$blankL-$blankR;
			$cropHeight = $height-$blankT-$blankB;
			if($cropWidth <= 0 || $cropHeight <= 0) {
				throw new ImageTrimException("contents not found.");
			}

			return $this->crop($blankL, $blankT, $cropWidth, $cropHeight);
		}

		/**
		 * $y 좌표 $y-1 좌표의 컬러 차이가 도드라지는 영역의 비율을 구한다.
		 * 좌, 우측의 흰색,검은색 공백은 분석 대상에서 제외된다
		 *
		 * @param  {[type]} int $y            [description]
		 * @param  {[type]} int $width        [description]
		 * @return {[type]}     [description]
		 */
/*
		public function getVerticalDiff(int $y, int $width) : float {
			$calPixel = 0; //실제 분절 여부를 확인한 필셀, 지정 좌표의 색상이 흰색 계열의 색상이면 분석 대상에서 제외한다.
			$dividedPixel = 0;

			//비교대상 두개의 픽색이 모두 흰색/검은색 계열이면 분석 대상에서 제외.
			//외곽에 희색 계열의 여백이 있고, 각각의 이미지들이 여백위에 배치된 경우에 대응
			$rtrimWidth = $width;
			for($x = 1; $x < $width; $x++) {
				$sourceColor = $this->getRgbaAt($width-$x, $y);
				$targetColor = $this->getRgbaAt($width-$x, $y-1);

				if(
					!$this->isIgnoreColor($sourceColor)
					|| !$this->isIgnoreColor($targetColor)
					) {
						// 컨텐츠영역이 확인되면 루프 종료
						$rtrimWidth = $x;
					}
			}

			$ltrimEnded = false;
			for($x = 0; $x < $rtrimWidth; $x++) {
				$sourceColor = $this->getRgbaAt($x, $y);
				$targetColor = $this->getRgbaAt($x, $y-1);

				if(!$ltrimEnded) {
					if(
						!$this->isIgnoreColor($sourceColor)
						|| !$this->isIgnoreColor($targetColor)
						) {
						$ltrimEnded = true;
					} else {
						continue;
					}
				}

				$calPixel++;
				$diff = $this->getColorDiff($sourceColor, $targetColor);

				if($diff >= self::15) {
					$dividedPixel++;
				}
			}

			if($calPixel<=0) return -1.0; //만약 두줄이 모두 흰색/검은색 계열이었다면 공백으로 보고 -1 리턴, 공백 계산과 병행하여 사용할 경우 계산량을 줄이기 위해 사용

			//전체 영역의 33.3% 이하가 ignore color 로 구성되어 있으면 결과값을 실뢰할 수 없음
			if($dividedPixel<$width/3) return 0.0;

			return round(($dividedPixel/$calPixel)*100,1);
		}
		*/


		private function getMaxDiffPosition(Vector<SlicePosition> $list) : SlicePosition {
			invariant($list->count()>0, "SlicePosition list empty.");

			$useIndex = 0;
			$maxDiff = 0;
			foreach($list as $k => $v) {
				if($v["diff"] > $maxDiff) {
					$maxDiff = $v["diff"];
					$useIndex = $k;
				}
			}

			return $list[$useIndex];
		}

		private function getPieceTop(Vector<SlicePosition> $list, int $index) : int {
			return $list->get($index-1) === NULL ? $list[$index]["y"] : $list[$index]["y"]-$list[$index-1]["y"];
		}

		private function getPieceHeight(Vector<SlicePosition> $list, int $index) : int {
			list($width,$height) = $this->getSize();
			return ($list->get($index+1) === NULL ? $height : $list[$index+1]["y"]) - $list[$index]["y"];
		}

		/**
		 * 전체 분절 지점 목록 중, 3px 미만의 간격으로 분절 지점이 반복될 경우
		 * 불절 비율이 가장 큰 지점으로 통합한다.
		 * ( 그라데이션,모자이크 패턴 등의 영역을 처리하기 위해 사용 )
		 *
		 * @param  {[type]} Vector<SlicePosition> $list         [description]
		 * @return {[type]}                       [description]
		 */
		private function groupingSlicePosition(Vector<SlicePosition> $list) : Vector<SlicePosition> {
			$result = Vector {};
			$group = Vector {};

			//그룹을 찾는다
			for($i = 0; $i < $list->count() ; $i++) {
				if($this->getPieceHeight($list, $i)<=3) {
					$group[] = $list[$i];
				} else {
					if($group->count()>0) {
						//이미 발견된 그룹이 있으면, 그릅 중 가장 Diff 가 큰 지점을 목록에 추가.
						$group[] = $list[$i];
						$result[] = $this->getMaxDiffPosition($group);
						$group->clear();
					} else {
						$result[] = $list[$i];
					}
				}
			}

			return $result;
		}

		/**
		 * 너무 잘게 조각난($minHeight) 컨텐츠는 위로 붇인다.
		 *
		 * @param  {[type]} Vector<SlicePosition> $list         [description]
		 * @param  {[type]} int                   $minHeight    =             200 [description]
		 * @return {[type]}                       [description]
		 */
		private function mergeSmallPiece(Vector<SlicePosition> $list, int $minHeight = 200) : Vector<SlicePosition> {
			$result = Vector {};

			//두번째 분절점부터는 하단 컨텐츠 영역의 크기가 $minHeight 이상인지 체크하며 미만이면 삭제
			for($i = 0; $i < $list->count(); $i++) {
				$pieceHeight = $this->getPieceHeight($list, $i);
				if($pieceHeight >= $minHeight) {
					$result[] = $list[$i];
				}
			}

			//첫 분절점을 기준으로 상단위 컨텐츠가 $minHeight 미만이면 분절점 삭제
			if($result->count() > 0 && $this->getPieceTop($result, 0) < $minHeight) {
				$result->removeKey(0);
			}

			return $result;
		}

		/**
		 * 분절 지점을 기준으로 이미지를 조각낼 위치를 찾는다.
		 *
		 * < 기준 >
		 * 지정 영역에 사용된 컬러의 RGB 코드를 그룹으로 나누어 그룹핑 하고
		 * 주로 사용된 색상 그룹의 차이를 통해 이미지 절단 지점을 계산한다.
		 *
		 * 라인별 색상차가 크지 않더라도 넓게 분포해 있으면 이미지가 분절된 것으로 본다
 		 * 사람이 분절을 그렇게 인지하니까.
 		 *
 		 * < 프로세스 >
 		 * 1. 분절 영역이 전체의 70이상이면 분절 지점으로 에상한다.
 		 * 2. 모자이크, 그라데이션 등이 결과를 오염시킬 수 있으므로,
 		 * 		3px 미만의 간격으로 분절된 지점이 연속 발견되면, 연속된 분절 지점 중 가장 분절비율이 큰 지점으로 통합한다.
 		 * 		( groupingSlicePosition 참고)
 		 * 3. 컨텐츠가 너무 작게 조각난 경우, 작은 조각을 위로 붇여나간다.
 		 * 		( mergeSmallPiece )
		 * @return {[type]} [description]
		 */
		public function getAutoSlicePosition(): Vector<SlicePosition> {
			list($width,$height) = $this->getSize();

			$sliceList = Vector {};
			for($y = 1; $y < $height; $y++) {
				$colorGroup1 = $this->getColorGroup(0, $y, $width, 1,);
		    $colorGroup2 = $this->getColorGroup(0, $y-1, $width, 1);
				$res = $colorGroup1->diff($colorGroup2);

				if($res >= self::DIVIDED_DIFF) {
					$piece = shape("y" => $y, "diff" => $res, "debug" => $colorGroup1->getWidth()."-".$colorGroup2->getWidth());
					$sliceList[] = $piece;
				}
			}

			if($sliceList->count() > 0) {
				$sliceList = $this->groupingSlicePosition($sliceList);
				$sliceList = $this->mergeSmallPiece($sliceList);
			}

			return $sliceList;
		}


		/**
		 * 세로로 긴 편집 이미지일 경우 최 상단의 이미지 조각을 추출하여
		 * 저장한다.
		 *
		 * @return {[type]}
		 * 추출할 수 없거나 추출이 불필요한 ImageSizeType 이면 false를 리턴한다.
		 * topPiece가 생성/저장 되었다면 true를 리턴한다
		 */
		public function createTopPiece() : bool {

			if($this->failCreateTopPiece) return false; //이미 실패한 기록이 있으면
			if($this->topPiece !== NULL) return true; //이미 생성되어 있다면 패스!
			if($this->getSizeType() !== ImageSizeType::Vertical && $this->getSizeType() !== ImageSizeType::VerticalLong ) {
				$this->failCreateTopPiece = true;
				return false;
			};

			//우선 생성에 실패한 것으로 마킹 후, 하기에서 조건에 만족되면 성공으로 변경
			$this->failCreateTopPiece = true;

			list($width, $height) = $this->getSize();
			$topPieceY = $this->getTopPiecePosition();

			// 추출된 조각이 없으면.
			if($topPieceY === NULL) return false;

			$pieceRatio = $width/$topPieceY;

			if($pieceRatio > self::VERTICAL_LONG_RATIO && $pieceRatio < self::HORIZONTAL_WIDE_RATIO ) {
				$this->topPiece = $this->crop(0, 0, $width, $topPieceY);
				if($this->topPiece !== NULL) {
					$this->topPiece->setIsTopPiece(true);
					return true;
				}
			}

			return false;
		}

		public function getTopPiece() : ?Image {
			return $this->topPiece;
		}

		public function setIsTopPiece(bool $is) : void {
			$this->isTopPiece = $is;
		}

		public function isTopPiece() : bool {
			return $this->isTopPiece;
		}

		/**
		 * 가장 상단의 분절 이미지를 크롭하여 리턴한다.
		 * 이미지의 가로 비율이 minWidthRatio 이상이고 maxWidthRatio
		 *
		 * @param  {[type]} Image $org           [description]
		 * @param  {[type]} float $minWidthRatio [description]
		 * @param  {[type]} float $maxWidthRatio [description]
		 * @return {[type]}       [description]
		 */
		public function getTopPiecePosition() : ?int {
			list($width, $height) = $this->getSize();

			for($y = 1; $y < $height; $y++) {
				$colorGroup1 = $this->getColorGroup(0, $y, $width, 1,);
				$colorGroup2 = $this->getColorGroup(0, $y-1, $width, 1);
				$res = $colorGroup1->diff($colorGroup2);

				/**
				 * DIVIDED_DIFF이상의 색상 차이가 있고, Y 좌표가 5px 이상일 경우 분절점으로 판단
				 * 5px 조건은 이미지 외곽에 테투리가 있는 경우에 대응하기 위함
				 * https://docs.google.com/document/d/1XH_cDXnxf_OdI0ZsIdH-dKWk_3WCs8Y3QHYt6lHvobI/edit#bookmark=id.dslgewobm41h				 *
				*/
				if($res >= self::DIVIDED_DIFF && $y > 5) {
					return $y;
				}
        if($y >= self::TOPPIECE_LIMIT) {
          return NULL;
        }
			}

			return NULL;
		}

		public function slice(Vector<SlicePosition> $list, int $imageType, ?string $prefix = NULL, int $quality=95, bool $trim = true) : Vector<string> {
			list($width, $height) = $this->getSize();

			// 폴더 생성
			$savedFileList = Vector {};
			if($prefix === NULL) $prefix = FileUtils::getTempFilePath();

			foreach($list as $index => $row) {
				if($index === 0) {
					//첫번째 분절점은 분절점 기준 상위 영역도 크롭
					$piece = $this->crop(0, 0, $width, $row["y"]);
					try {
						if($trim) $piece = $piece->trim();
					} catch (ImageTrimException $e) {
						//컨텐츠가 없는 경우
						continue;
					}

					$destFilename = $prefix."_".$index;
					if($piece->save($imageType, $destFilename)) {
						$savedFileList[] = $destFilename;
					}
					$piece->clear();
				}

				$piece = $this->crop(0, $row["y"], $width, $this->getPieceHeight($list, $index));
				try {
					if($trim) $piece = $piece->trim();
				} catch (ImageTrimException $e) {
					continue;
				}

				$destFilename = $prefix."_".($index+1);
				if($piece->save($imageType, $destFilename)) {
					$savedFileList[] = $destFilename;
				}
				$piece->clear();
			}

			return $savedFileList;
		}

		/******************************************************************************************
		 * Static Method
		******************************************************************************************/

		public static function isANIGIF(string $filename) : bool {
			if(!($fh = @fopen($filename, 'rb'))) return false;
			$count = 0;

			while(!feof($fh) && $count < 2)
			{
			  $chunk = fread($fh, 1024 * 100); //read 100kb at a time
				$matches = array();
				$count += preg_match_all('#\x00\x21\xF9\x04.{4}\x00\x2C#s', $chunk, $matches);
			 }
			fclose($fh);

			return $count > 1;
		}

}
