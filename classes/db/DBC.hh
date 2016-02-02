<?hh //strict
  namespace nova\db;

  use nova\collection\MapUtils;

  class DBC extends \PDO {
    private string $blacklist = 'UNION|LOAD_FILE|OUTFILE|DUMPFILE|ESCAPED|TERMINATED|CASCADE|INFILE|X509|TRIGGER|REVOKE';
    private bool $lastSelectUseCache = false;

    public function __construct(string $dsn, string $username, string $password) {
      parent::__construct($dsn, $username, $password);
    }

    public function query(string $sql, mixed $param = NULL) : \PDOStatement {
      if($this->isBlacklistQuery($sql)) {
        throw new \PDOException("This query Blocked.");
      }

      $sth = $this->prepare($sql);

      if($param instanceof Vector || $param instanceof Map) {
        //UNSAFE
        foreach($param as $k=>$v) {
          $sth->bindValue(is_int($k) ? $k+1 : ":".$k, $v,gettype($v)=="integer" ? \PDO::PARAM_INT : \PDO::PARAM_STR);
        }
      }

      $sth->setFetchMode(\PDO::FETCH_ASSOC);
      $sth->execute();
      $this->lastSelectUseCache = false;

      return $sth;
    }

    public function isBlacklistQuery(string $sql) : bool {
			return preg_match("/".$this->blacklist."/i", $sql) ? true : false;
		}

    public function isSelectQuery(string $sql) : bool {
      return preg_match("/^SELECT/i", trim($sql)) ? true : false;
    }

    public function lastSelectUseCache() : bool {
      return $this->lastSelectUseCache;
    }

    public function insert(string $sql, Vector<mixed> $param = Vector{}) : int {
      $sth = $this->query($sql,$param);
      return (int)$this->lastInsertId();
    }

    public function getList(string $sql, mixed $param = Vector{}, ?string $cacheKey = NULL, int $ttl = 0) : array<array<string, mixed>> {
      if($cacheKey !== NULL && ($res = \nova\cache\MC::getInstance()->get($cacheKey)) !== NULL) {
        invariant(is_array($res), "Expected a array");
        $this->lastSelectUseCache = true;
        return $res;
      }

      $res = $this->query($sql, $param)->fetchAll();

      if($cacheKey !== NULL) {
        \nova\cache\MC::getInstance()->set($cacheKey, $res, $ttl);
      }

      return $res;
    }

    public function getRow(string $sql, mixed $param = Vector{}, ?string $cacheKey = NULL, int $ttl = 0) : ?array<string,mixed> {
      if($cacheKey !== NULL && ($res = \nova\cache\MC::getInstance()->get($cacheKey)) !== NULL) {        
        invariant(is_array($res), "Expected a array");
        $this->lastSelectUseCache = true;
        return $res;
      }

      $res = $this->query($sql, $param)->fetch(\PDO::FETCH_ASSOC);

      if($cacheKey !== NULL && $res !== false) {
        \nova\cache\MC::getInstance()->set($cacheKey, $res, $ttl);
      }

      return $res === false ? null : $res;
    }

    //$pk 의 row을 $info에 담긴 정보로 업데이트 한다.
		public function update(string $tableName, Map<string, mixed> $info, string $pkName, mixed $pk, mixed $cacheKeys = NULL) : int {
      if($info->count() <= 0) return 0;

			//DB쿼리
			$set = "";
			$bind = Vector {};
			foreach($info as $k => $v) {
				$increment = substr($k,-1);
				if($increment=="+") {
					$k = substr($k,0,-1);
					$set .= "`$k`=`$k`{$increment}".(int)$v.",";
				} else {
					$set .= "`$k`=?,";
					$bind->add($v);
				}
			}
			$set = substr($set,0,-1);
      $bind->add($pk);

			$res = $this->query("UPDATE {$tableName} SET {$set} WHERE {$pkName}=?",$bind);
      if($cacheKeys !== NULL) \nova\cache\MC::getInstance()->del($cacheKeys);
			return $res->rowCount();
		}

		public function inserts(string $frontSQL, Vector<Vector<mixed>> $queue, int $length = 500) : void {
			$sql = "";
			$qstep = 1;;
			$qcount = $queue->count();
			foreach($queue as $v) {
				$qstep++;
				$sql .= "(".implode(",",$v)."),";
				if(strlen($sql)>1000 || $qcount < $qstep) {
					$sql = substr($sql,0,-1);
					$this->query($frontSQL.$sql);
					$sql = "";
				}
			}
		}

    /**
     * 레코드를 추가하거나, 이미 동일한 Unique 타입의 키(like. PK)를 가진 레코드가 있으면 해당 레코드의 정보를 업데이트 한다.
     * [주의] 업데이트인 경우 PK 가 리턴되지 않는다.
     * @param  {[type]} string      $tableName    [description]
     * @param  {[type]} Map<string, mixed>        $info         [description]
     * @return {[type]}             [description]
     */
    public function unique(string $tableName, Map<string, mixed> $info) : mixed {
      if($info->count() <= 0) return 0;

      $fields = [];
      $values = [];
      $update = [];
      $params = Map {};

      foreach($info as $k => $v) {
        $needIncrement = substr($k,-1) === "+" ? true : false;

        $fieldName = $needIncrement ? substr($k,0,-1) : $k;
        $fields[] = "`{$fieldName}`";
        $values[] = ":".$fieldName;
        $update[] = $needIncrement ? "`{$fieldName}`=`{$fieldName}`+:{$fieldName}" : "`{$fieldName}`=:{$fieldName}";
        $params[$fieldName] = $v;
      }

      $res = $this->query("INSERT INTO {$tableName}(".implode(",",$fields).") VALUES(".implode(",",$values).")
        ON DUPLICATE KEY UPDATE ".implode(",",$update),$params);



      return (int)$this->lastInsertId();
    }

  }
