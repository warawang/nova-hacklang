<?hh //strict
  namespace nova\image;

  use nova\io\Vo;
  use nova\image\ImageResizeType;

  class ImageListItem extends Vo {
    public int $width;
    public int $height;
    public ImageResizeType $resizeType;
    public bool $indispensable;

    private function __construct(int $width, int $height, ImageResizeType $resizeType, bool $indispensable) {
      $this->width = $width;
      $this->height = $height;
      $this->resizeType = $resizeType;
      $this->indispensable = $indispensable;
    }

    public static function create(int $width, int $height, ImageResizeType $resizeType, bool $indispensable = true) : ImageListItem {
      return new ImageListItem($width, $height, $resizeType, $indispensable);
    }

    public static function createOptional(int $width = 0, int $height = 0 , ImageResizeType $resizeType = ImageResizeType::Normal) : ImageListItem {
      return static::create($width, $height, ImageResizeType::Normal, false);
    }
  }
