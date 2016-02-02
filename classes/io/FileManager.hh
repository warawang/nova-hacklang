<?hh //strict
  namespace nova\io;

  abstract class FileManager {
    abstract public function getCategoryName() : string;    
    abstract public function getS3Bucket() : string;

    public function getDivideFolder(mixed $key) : string {
      if(is_int($key)) {
        $fullKey = sprintf("%05d",$key);
      } else if(is_string($key)) {
        $fullKey = sprintf("%05s",$key);
      } else {
        throw new \InvalidArgumentException("Key must be int or string.");
      }

      $folder = "/";
      for($i=0;$i<5;$i++) {
        $folder .= $fullKey[$i] ."/";
      }

      return $folder.$fullKey;
    }
  }
