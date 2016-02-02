<?hh //strict
/**
 * This code was based on https://github.com/jenssegers/imagehash
 */

  namespace nova\image\hash;

  class AverageHasher implements HasherInterface {
    const SIZE = 8;

    public function hash(resource $im) : mixed {
      // Resize the image.
      $resized = imagecreatetruecolor(static::SIZE, static::SIZE);
      imagecopyresampled($resized, $im, 0, 0, 0, 0, static::SIZE, static::SIZE, imagesx($im), imagesy($im));

      // Create an array of greyscale pixel values.
      $pixels = [];
      for ($y = 0; $y < static::SIZE; $y++) {
        for ($x = 0; $x < static::SIZE; $x++) {
          $rgb = imagecolorsforindex($resized, imagecolorat($resized, $x, $y));
          //print_r($rgb);
          $pixels[] = floor(($rgb['red'] + $rgb['green'] + $rgb['blue']) / 3);
        }
      }
      // Free up memory.
      imagedestroy($resized);
      // Get the average pixel value.
      $average = floor(array_sum($pixels) / count($pixels));
      //var_dump($average);exit;
      // Each hash bit is set based on whether the current pixels value is above or below the average.
      $hash = 0;
      $one = 1;
      foreach ($pixels as $pixel) {
        if ($pixel > $average) $hash |= $one;
        $one = $one << 1;
      }


      return $hash;
    }
  }
