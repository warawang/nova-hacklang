<?hh //strict
  namespace nova\cache;

  use Memcached;
  use nova\util\Config;
  use nova\cache\exception\MCException;

  class MC {
    private static Map<string,MC> $instance = Map{};
    private Memcached $handle;

    /**
     * 싱글톤 인스턴스
     * @return {Config}
     */
    public static function getInstance(string $target = "default") : MC {
      if(self::$instance->get($target) === NULL) {
        self::$instance[$target] = new MC($target);
      }

      return self::$instance[$target];
    }

    protected function __construct(string $target) {
      $this->handle = new Memcached();
      $this->handle->addServer(Config::getInstance()->getMCHost(),11211, 5);
    }

    public function getHandle() : Memcached {
      return $this->handle;
    }

    public function getRev(string $key) : int {
      $rkey = "";
      if(($pos = strripos($key,"/"))!== false) {
        $rkey = substr($key,0,$pos);
      } else {
        $rkey = $key;
      }

      return Config::getInstance()->getMcRev($rkey);
    }

    public function makeupKey(mixed $key) : mixed {
      if(is_string($key)) {
        return Config::getInstance()->getMCPrefix().$key."/".$this->getRev($key);
      } else if(is_array($key)) {
        foreach($key as $k => $v) {
          $key[$k] = $this->makeupKey($v);
        }
      }
      return $key;
    }

    public function set(string $key,mixed $val,int $ttl = 0) : bool {
			$key = $this->makeupKey($key);

      if($res = $this->handle->set($key, $val, $ttl)) {
        return true;
      }

      return false;
		}

    public function get(string $key) : mixed {
			$key = $this->makeupKey($key);
      $res = $this->handle->get($key);
      //상태코드 참고 : http://php.net/manual/en/memcached.getresultcode.php
      return $this->handle->getResultCode() === Memcached::RES_NOTFOUND ? NULL : $res;
		}

    /**
     * 캐시를 삭제한다
     * @param  {[type]} mixed $key          문자열로 넘기면 해당키만, 배열로 넘기면 전체 키를 삭제
     * @return {[type]} TRUE : 성공, FALSE : 실패
     */
    public function del(mixed $key) : bool {
      $key = $this->makeupKey($key);

      if(is_string($key)) {
        $res = $this->handle->delete($key);
        return $this->handle->getResultCode() === Memcached::RES_NOTFOUND ? false : true;
      } else if(is_array($key)) {
        foreach($key as $v) {
          $res = $this->handle->delete($v);
        }
        return true;
      } else {
        return false;
      }
		}
  }
