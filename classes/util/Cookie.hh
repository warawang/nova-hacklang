<?hh //strict

  namespace nova\util;

  use nova\util\Config;

  class Cookie {
    public static function set(string $key, string $value, int $expire = 0, string $domain = "") : void {
      setcookie($key, $value, $expire ? time()+$expire : 0, "/", $domain ? $domain : ".".Config::getInstance()->getDomain("www"));

      //UNSAFE
      $_COOKIE[$key] = $value;
    }

    public static function get(string $key) : ?string {
      //UNSAFE
      return strlen($_COOKIE[$key])>0 ? $_COOKIE[$key] : NULL;
    }

    public static function del(string $key, string $domain = "") : void {
      self::set($key, "", -3600, $domain);

      //UNSAFE
      unset($_COOKIE[$key]);
    }
  }
