<?hh
  function arcnt(mixed $ar) : int {
    return is_array($ar) ? count($ar) : 0;
  }
  
  function str_domino(...$strs) : ?string {
    foreach($strs as $v) {
      if($v !== NULL && strlen($v)>0) return $v;
    }

    return NULL;
  }

  function implode_key(string $delimiter, array<string,mixed> $arr, int $limit=-1) : string {
    $result = "";
    $cnt = 0;
    foreach($arr as $k=>$v) {
      $cnt++;
      if($limit != -1 && $cnt>$limit) break;
      $result .= $k.$delimiter;
    }
    if(strlen($result)>=1) $result = substr($result,0,-1);
    return $result;
  }

  //Shortcut
  function hasKey(mixed $target, mixed $key) : bool {
    return \nova\util\ArrayUtils::hasKey($target, $key);
  }

  function str_empty(string $str) : bool {
    return strlen($str) <= 0;
  }

  function str_xmlcdata(string $str, bool $includeTag = true) : string {
    if($includeTag) return "<![CDATA[".str_replace("]]>","] ]>",$str)."]]>";
    else return str_replace("]]>","] ]>",$str);
  }

  /**
   * http 에러 발생
   * @param  {[type]} int $errorCode    = 500 [description]
   * @return {[type]}     [description]
   *
   * TODO 에러 코드별 헤더 메시지 변경
   */
  function httpError(int $errorCode = 500) : void {
    header("HTTP/1.1 500 Internal Server Error");
    exit;
  }
