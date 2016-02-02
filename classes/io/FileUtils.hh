<?hh //strict

  namespace nova\io;

  use nova\util\Config;

  class FileUtils {

    public static function getTempFilePath(string $prefix = Config::getInstance()->getServiceId()."_tmp_") : string {
      return tempnam("/tmp",$prefix);
    }

    public static function unlink(mixed $target) : void {
      if(is_string($target)) {
        unlink($target);
      } else if(is_array($target)) {
        foreach($target as $file) {
          unlink($file);
        }
      } else if($target instanceof Vector) {
        foreach($target as $file) {
          unlink($file);
        }
      }
    }

  }
