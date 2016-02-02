<?hh //strict
  namespace nova\util;

  use nova\util\Connection;

  class Modernizr {
    private static Map<string,mixed> $browserInfo = Map{};

    public static function test(string $prop) :  bool {
      $ua = strtolower(Connection::getUserAgent());

      if($prop == "videoautoplay") {
        //PC이면 TRUE
        if(
          stristr($ua, "darwin") //mac
          || stristr($ua, "win") //window
        ) return true;


        //모바일이면 특정 브라우저만
        if(
          stristr($ua, "iphone") //ios
          || stristr($ua, "ipad") //ios
          || stristr($ua, "ipod") //ios
          || stristr($ua, "android") //android
        ) {
          if(
            stristr($ua, "firefox")
            || stristr($ua, "opera")
            || stristr($ua, "naver")
          ) return true;
        }
        return false;
      } else if($prop == "video") {
        if(preg_match('/(?i)msie [5-8]/',$ua)) {
          return false;
        } else {
          return true;
        }

        return false;
      }


      return false;
    }

  }
