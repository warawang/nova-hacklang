<?hh //strict
  namespace nova\util;

  class StringUtils {
    public static function isURL(string $url) : bool {
      $result = array();
      if(preg_match('/(ftp|http|https):\/\/(\w+:{0,1}\w*@)?(\S+)(:[0-9]+)?(\/|\/([\w#!:.?+=&%@!\-\/]))?/i', $url, $result)>0) return true;
      return false;
    }

    public static function url2name(string $url) : ?string {
      if(($host = parse_url($url, PHP_URL_HOST)) == false) return NULL;
      $parts = explode(".",$host);
      $len = count($parts);
      if($len<2) return NULL;
      return $parts[$len-2].".".$parts[$len-1];
    }

    public static function stripBeginnginWord(string $title) : string {
      $title = trim($title);
      $title = preg_replace('/\((.*?)\)/i', "", $title); //(블라)
      $title = preg_replace('/\[(.*?)\]/i', "", $title); // [블라]
      $title = preg_replace('/^(.*?)\)/i', "", $title); // 블라)
      $title = preg_replace('/^(.*?)\]/i', "", $title); // 블라]
      $title = trim($title);

      return $title;
    }

  }
