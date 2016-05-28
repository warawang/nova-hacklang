<?hh //strict
  namespace nova\image;

  use nova\image\Image;
  use nova\util\Config;
  use nova\util\Connection;
  use nova\io\FileUtils;
  use nova\io\FileManager;
  use nova\image\exception\ImageFileException;
  use nova\image\ImageSaveResultItem;
  use nova\image\hash\ImageHash;

  use nova\image\ImageListItem;

  use Aws\S3\S3Client;

  abstract class ImageManager extends FileManager {
    abstract public function getMaxSize() : array<int>;

    public function getS3Bucket() : string {
      return Config::getInstance()->getS3Bucket("stg");
    }

    public function getImageFormat() : int {
      return IMAGETYPE_JPEG;
    }

    public function getImageExtension() : string {
      return "jpg";
    }

    public function getMIMEType() : string {
      return "image/jpeg";
    }

    public function getImageQuality() : int {
      return 65;
    }

    public function isEnableAGif() : bool {
      return true;
    }

    public function getImageList() : Map<string,ImageListItem> {
      list($width, $height) = $this->getMaxSize();
      return Map {
        "org" => ImageListItem::create($width, $height, ImageResizeType::FitAndCut),
        "video" => ImageListItem::createOptional() //Anigif는 원본 그대로 저장
      };
    }

    public function getURL(mixed $key, string $type) : string {
      if(!$this->getImageList()->containsKey($type)) throw new \OutOfBoundsException("$type is not defined.");
      return Connection::getProtocol()."://".Config::getInstance()->getS3Host("stg")."/".$this->getKey($key, $type);
    }

    public function getKey(mixed $key, string $type) : string {
      if(!$this->getImageList()->containsKey($type)) throw new \OutOfBoundsException("$type is not defined.");
      $ext = $type == "video" ? "gif" : $this->getImageExtension();
      return $this->getCategoryName().$this->getDivideFolder($key)."/".$type.".".$ext;
    }

    public function getPath(mixed $key, string $type) : string {
      return "/".$this->getKey($key, $type);
    }

    public function save(mixed $key, string $sourcePath, bool $trim = true) : ImageSaveResult {
      $options = Config::getInstance()->getS3Option();
      $resultItems = Map {};

      // UNSAFE
      $s3 = new S3Client($options);

      $orgImage = new Image();
      if(!$orgImage->open($sourcePath)) {
        throw new ImageFileException("Failed to open image : " . $sourcePath);
      }

      //공백삭제
      if($trim) $orgImage = $orgImage->trim();

      //TopPiece를 생성한다 ( 세로로 긴 형태의 편집 이미지에 대응)
      $orgImage->createTopPiece();      
      $topPiece = $orgImage->getTopPiece();

      foreach($this->getImageList() as $type => $typeInfo) {
        if(!$typeInfo->indispensable) continue; //필수로 생성해야 하는 이미지만 생성, 이 외 옵셔널 이미지는 상속 클래스에서 별도 생성.
        $tmpImage = $orgImage->resize($typeInfo->width, $typeInfo->height, $typeInfo->resizeType);

        $tmpPath = FileUtils::getTempFilePath();
        if(!$tmpImage->save($this->getImageFormat(), $tmpPath, $this->getImageQuality())) {
          throw new ImageFileException("Failed to temp image create.");
        }

        $result = $s3->putObject([
          "ACL" => "public-read",
          "Bucket" => $this->getS3Bucket(),
          "Key"    => $this->getKey($key, $type),
          "SourceFile"  => $tmpPath,
          "ContentType" => $this->getMIMEType(),
          "CacheControl" => "public, max-age=315360000",
          "Expires" => gmdate("D, d M Y H:i:s T", strtotime("+10 years"))
        ]);

        $resultItems[$type] = ImageSaveResultItem::create($tmpImage->getWidth(), $tmpImage->getHeight(), $tmpImage->getSizeType(), $result["ObjectURL"]);
        unlink($tmpPath);
      }

      //hash 데이터 추출
      $imageHash = new ImageHash();

      return shape("width"     => $orgImage->getWidth(),
                   "height"    => $orgImage->getHeight(),
                   "topPieceY" => $topPiece === NULL ? 0 : $topPiece->getHeight(),
                   "hash"      => $imageHash->hash($topPiece === NULL ? $orgImage->getResource() : $topPiece->getResource()),
                   "items"     => $resultItems);
    }
  }
