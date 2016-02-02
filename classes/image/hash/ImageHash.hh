<?hh //strict
/**
 * This code was based on https://github.com/jenssegers/imagehash
 */

namespace nova\image\hash;

use nova\image\Image;
use nova\image\hash\ImageHashType;
use nova\image\hash\HasherInterface;
use nova\image\hash\DifferenceHasher;
use nova\image\hash\PerceptualHasher;

class ImageHash {
  private HasherInterface $hasher;

  public function __construct(ImageHashType $type = ImageHashType::Difference) {
    if($type == ImageHashType::Difference) {
      $this->hasher = new DifferenceHasher();
    } else { //ImageHashType::Average
      $this->hasher = new AverageHasher();
    }
  }

  public function hash(resource $im) : mixed {
    $hash = $this->hasher->hash($im);
    return is_int($hash) ? dechex($hash) : $hash;
  }

  public function hashFile(string $filePath) : mixed {
    $image = new Image();
    $image->open($filePath);
    $im = $image->getResource();
    return $this->hash($im);
  }

  public function compare(string $hash1, string $hash2) : int {
    return (int)gmp_hamdist("0x$hash1", "0x$hash2");
  }

  public function compareFile(string $filePath1, string $filePath2) : int {
    $hash1 = $this->hashFile($filePath1);
    $hash2 = $this->hashFile($filePath2);

    return (int)gmp_hamdist("0x".(string)$hash1, "0x".(string)$hash2);
  }
}
