<?hh //strict
  namespace nova\image;

  use nova\io\Vo;

  class ImageSaveResultItem extends Vo {
    public int $width;
    public int $height;
    public ImageSizeType $sizeType;
    public string $url;

    private function __construct(int $width, int $height, ImageSizeType $sizeType, string $url) {
      $this->width = $width;
      $this->height = $height;
      $this->sizeType = $sizeType;
      $this->url = $url;
    }

    public static function create(int $width, int $height, ImageSizeType $sizeType, string $url) : ImageSaveResultItem {
      return new ImageSaveResultItem($width, $height, $sizeType, $url);
    }
  }
