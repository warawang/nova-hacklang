<?hh //strict
  namespace nova\util;

  class Connection {
    public static function isCli() : bool {
      // UNSAFE
      return !hasKey($_SERVER,"HTTP_HOST");
    }

    public static function getHost() : string {
      $pathInfo = pathinfo(static::getDocumentRoot());
      return $pathInfo['basename'];
    }

    public static function getServerValue(string $target) : ?string {
      // UNSAFE
      return hasKey($_SERVER,$target) ? $_SERVER[$target] : NULL;
    }

    public static function getRef() : ?string {
      // UNSAFE
      return strlen($_SERVER["HTTP_REFERER"])> 1 ? $_SERVER["HTTP_REFERER"] : NULL;
    }

    public static function getIP() : string {
      // UNSAFE
      return $_SERVER["REMOTE_ADDR"];
    }

    public static function getDocumentRoot() : ?string {
      return static::getServerValue("DOCUMENT_ROOT");
    }

    public static function getUserAgent() : ?string {
      return static::getServerValue("HTTP_USER_AGENT");
    }

    public static function getScriptFileName() : ?string {
      return static::getServerValue("SCRIPT_FILENAME");
    }

    public static function getProtocol() : string {
      if(static::getServerValue("HTTPS")=='on') $__protocol = 'https';
  	  else $__protocol = 'http';
      return $__protocol;
    }

    public static function getCssBrowserSelector(?string $ua=null) : string {
      $ua = $ua !== null ? strtolower($ua) : strtolower(static::getUserAgent());

  		$g = 'gecko';
  		$w = 'webkit';
  		$s = 'safari';
  		$b = array();

      $array = array();

      // browser
  		if(!preg_match('/opera|webtv/i', $ua) && preg_match('/msie\s(\d)/', $ua, $array)) {
  				$b[] = 'ie ie' . $array[1];
  		}	else if(strstr($ua, 'firefox/2')) {
  				$b[] = $g . ' ff2';
  		}	else if(strstr($ua, 'firefox/3.5')) {
  				$b[] = $g . ' ff3 ff3_5';
  		}	else if(strstr($ua, 'firefox/3')) {
  				$b[] = $g . ' ff3';
  		} else if(strstr($ua, 'gecko/')) {
  				$b[] = $g;
  		} else if(preg_match('/opera(\s|\/)(\d+)/', $ua, $array)) {
  				$b[] = 'opera opera' . $array[2];
  		} else if(strstr($ua, 'konqueror')) {
  				$b[] = 'konqueror';
  		} else if(strstr($ua, 'chrome')) {
  				$b[] = $w . ' ' . $s . ' chrome';
  		} else if(strstr($ua, 'iron')) {
  				$b[] = $w . ' ' . $s . ' iron';
  		} else if(strstr($ua, 'applewebkit/')) {
  				$b[] = (preg_match('/version\/(\d+)/i', $ua, $array)) ? $w . ' ' . $s . ' ' . $s . $array[1] : $w . ' ' . $s;
  		} else if(strstr($ua, 'mozilla/')) {
  				$b[] = $g;
  		}

  		// platform
  		if(strstr($ua, 'j2me')) {
  				$b[] = 'mobile';
  		} else if(strstr($ua, 'iphone')) {
  				$b[] = 'iphone';
      } else if(strstr($ua, 'ipad')) {
  				$b[] = 'ipad';
  		} else if(strstr($ua, 'ipod')) {
  				$b[] = 'ipod';
  		} else if(strstr($ua, 'mac')) {
  				$b[] = 'mac';
  		} else if(strstr($ua, 'darwin')) {
  				$b[] = 'mac';
  		} else if(strstr($ua, 'webtv')) {
  				$b[] = 'webtv';
  		} else if(strstr($ua, 'win')) {
  				$b[] = 'win';
  		} else if(strstr($ua, 'freebsd')) {
  				$b[] = 'freebsd';
  		} else if(strstr($ua, 'x11') || strstr($ua, 'linux')) {
  				$b[] = 'linux';
  		}

  		return implode(' ', $b);
    }
  }
