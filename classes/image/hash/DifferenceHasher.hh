<?hh //strict
/**
 * This code was based on https://github.com/jenssegers/imagehash
 */

  namespace nova\image\hash;

  class DifferenceHasher implements HasherInterface {
    const SIZE = 8;

    public function hash(resource $im) : mixed {
      // For this implementation we create a 8x9 image.
      $width = static::SIZE + 1;
      $heigth = static::SIZE;
      // Resize the image.
      $resized = imagecreatetruecolor($width, $heigth);
      imagecopyresampled($resized, $im, 0, 0, 0, 0, $width, $heigth, imagesx($im), imagesy($im));
      $hash = 0;
      $one = 1;
      for ($y = 0; $y < $heigth; $y++)
      {
          // Get the pixel value for the leftmost pixel.
          $rgb = imagecolorsforindex($resized, imagecolorat($resized, 0, $y));
          $left = floor(($rgb['red'] + $rgb['green'] + $rgb['blue']) / 3);
          for ($x = 1; $x < $width; $x++)
          {
              // Get the pixel value for each pixel starting from position 1.
              $rgb = imagecolorsforindex($resized, imagecolorat($resized, $x, $y));
              $right = floor(($rgb['red'] + $rgb['green'] + $rgb['blue']) / 3);
              // Each hash bit is set based on whether the left pixel is brighter than the right pixel.
              // http://www.hackerfactor.com/blog/index.php?/archives/529-Kind-of-Like-That.html
              if ($left > $right) $hash |= $one;

              // Prepare the next loop.
              $left = $right;
              $one = $one << 1;
          }
      }
      // Free up memory.
      imagedestroy($resized);      
      return $hash;
    }
  }
