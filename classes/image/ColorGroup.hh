<?hh //strict
  namespace nova\image;

  class ColorGroup {
    public int $width;
    public int $height;
    public Map<string, int> $map;

    public function __construct(int $width, int $height, Map<string, int> $map) {
			$this->width = $width;
      $this->height = $height;
      $this->map = $map;
		}

    public function getMap() : Map<string, int> {
      return $this->map;
    }

    public function getWidth() : int {
      return $this->width;
    }

    public function getHeight() :  int {
      return $this->height;
    }

    public function getPixelCount() : int {
      return $this->width * $this->height;
    }

    public function diff(ColorGroup $target) : float {
      $merged = new Map($this->map);
			foreach($target->getMap() as $color => $count) {
				if($merged->contains($color)) {
					// Group1 에서도 발견된 그룹이면
					$merged[$color] = abs($merged[$color]-$count);
				} else {
					$merged[$color] = $count;
				}
			}

			$diffSum = 0;
			foreach($merged as $count) $diffSum += $count;

			//두 비교그룹 중 컨텐츠 영역이 큰 것을 찾는다.
			$maxContentSize = max($this->getPixelCount(), $target->getPixelCount());

			//두 그룹 다 컨텐츠 영역이 없다면 차이가 없는것으로 판단 ( getColorGroup의 trimIgnoreColor 옵션으로 인해 발생하는 이슈. )
			if($maxContentSize <= 0) return 0.0;

			//두 컬러그룹의 크기 차이가 있다면 차이만큼 diff 를 증가시킨다
			$diffSum += abs(($this->getPixelCount()) - ($target->getPixelCount()));

			return round((($diffSum/$maxContentSize)*100)/2,1);
    }
  }
