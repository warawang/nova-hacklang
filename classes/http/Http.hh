<?hh //strict
  namespace nova\http;

  use nova\io\FileUtils;
  use nova\http\exception\HttpURLAccessException;
  use nova\http\exception\HttpDownloadException;

  class Http {
    private string $userAgent = "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/45.0.2454.93 Safari/537.36";
    private int $timeout = 120;
    private ?string $authorization = NULL;
    private mixed $postData;
    private ?string $referer = NULL;
    private bool $enableAutoReferer = true;
    private ?string $contentType = NULL;

    private function buildCurlSession(string $url) : resource {
      $ch = curl_init();
      $urlinfo = parse_url($url);
      $httpHeader = array();

      curl_setopt($ch, CURLOPT_URL, $url);
      curl_setopt($ch, CURLOPT_HEADER, false);
      curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
      curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
      curl_setopt($ch, CURLOPT_USERAGENT, $this->userAgent);
      curl_setopt($ch, CURLOPT_TIMEOUT, $this->timeout);
      curl_setopt($ch, CURLOPT_REFERER, $this->referer === NULL ? ($this->enableAutoReferer ? $urlinfo["scheme"]."://".$urlinfo["host"] : "") : $this->referer);

      if($this->postData !== NULL) {
        curl_setopt($ch, CURLOPT_POST, true);

        //Key:Value 값 형식일 때        
        if(is_array($this->postData) || $this->postData instanceof Map) {
          $postDataQuery = http_build_query($this->postData);
          curl_setopt($ch, CURLOPT_POSTFIELDS, $postDataQuery);
        } else { //Json 등의 형식일 때.
          curl_setopt($ch, CURLOPT_BINARYTRANSFER, true);
          curl_setopt($ch, CURLOPT_POSTFIELDS, $this->postData);
          $httpHeader[] = "Content-Length : ".strlen($this->postData);
        }
      }

      //인증 정보
      if($this->authorization !== NULL) {
        $httpHeader[] = "authorization: ".$this->authorization;
      }

      // 컨텐츠 타입
      if($this->contentType !== NULL) {
        $httpHeader[] = "Content-Type: ".$this->contentType;
      }

      // 설정된 헤더가 있을 경우
      if(arcnt($httpHeader) > 0) {
        curl_setopt($ch, CURLOPT_HTTPHEADER, $httpHeader);
      }

      return $ch;
    }

    public function get(string $url) : string {
      $ch = $this->buildCurlSession($url);

      $res = curl_exec($ch);

      $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
      if(!$this->isCodeOk($httpCode)) {
        throw new HttpURLAccessException("Failed to URL open : {$url} / ".htmlspecialchars($res));
      }
      curl_close($ch);

      if($res === false) {
        throw new HttpURLAccessException("Failed to URL open : {$url}");
      }

      return $res;
    }

    public function getJson(string $url) : array<mixed,mixed> {
      $jsonString = $this->get($url);

      if(($json = json_decode($jsonString, true)) === NULL) {
        throw new HttpURLAccessException("Failed to decode json string : {$url}");
      }

      return $json;
    }

    public function download(string $url, ?string $destPath = NULL) : string {
      if($destPath === NULL) $destPath = FileUtils::getTempFilePath();
      $ch = $this->buildCurlSession($url);

      if(($res = curl_exec($ch)) === false) {
        curl_close($ch);
        throw new HttpURLAccessException("Failed to URL open : {$url}");
      }
      //상태 확인
      $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
      curl_close($ch);

      if(!$this->isCodeOk($httpCode)) {
        throw new HttpURLAccessException("Failed to URL open : {$url}");
      }

      $out = fopen($destPath, "w");
      fwrite($out, $res);
      fclose($out);

      return $destPath;
    }

    /**
     * URL이 가르키는 파일의 용량을 확인한다.
     * @param  {[type]} string $url          [description]
     * @return {int}  성공시 용량, 실패시 -1
     */
    public function getFileSize(string $url) : int {
      $ch = curl_init();

      curl_setopt($ch, CURLOPT_URL, $url);
      curl_setopt($ch, CURLOPT_NOBODY, true);
      curl_setopt($ch, CURLOPT_HEADER, true);
      curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
      curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
      curl_setopt($ch, CURLOPT_USERAGENT, $this->userAgent);
      curl_setopt($ch, CURLOPT_TIMEOUT, $this->timeout);

      if(($data = curl_exec($ch)) === false) {
        curl_close($ch);
        throw new HttpURLAccessException("Failed to URL open : {$url}");
      }

      curl_close($ch);

      $result = -1;

      if($data) {
        $contentLength = -1;
        $status = 0;

        $matches = array();
        if(preg_match( "/^HTTP\/1\.[01] (\d\d\d)/", $data, $matches)) {
         $status = (int)$matches[1];
        }

        if(preg_match( "/Content-Length: (\d+)/", $data, $matches)) {
         $contentLength = (int)$matches[1];
        }

        // http://en.wikipedia.org/wiki/List_of_HTTP_status_codes
        if(!$this->isCodeOk($status)) {
          throw new HttpURLAccessException("Failed to URL open : {$url}");
        }

        $result = $contentLength;
      }

      return $result;
    }

    /**
     * URL 의 컨텐츠 타입을 확인한다
     */
    public function getContentType(string $url) : ?string {
      $ch = curl_init();

      curl_setopt($ch, CURLOPT_URL, $url);
      curl_setopt($ch, CURLOPT_NOBODY, true);
      curl_setopt($ch, CURLOPT_HEADER, true);
      curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
      curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
      curl_setopt($ch, CURLOPT_USERAGENT, $this->userAgent);
      curl_setopt($ch, CURLOPT_TIMEOUT, $this->timeout);

      if(($data = curl_exec($ch)) === false) {
        curl_close($ch);
        throw new HttpURLAccessException("Failed to URL open : {$url}");
      }

      curl_close($ch);

      $contentType = NULL;

      if($data) {
        $status = 0;

        $matches = array();
        if(preg_match( "/^HTTP\/1\.[01] (\d\d\d)/", $data, $matches)) {
         $status = (int)$matches[1];
        }

        if(preg_match( "/Content-Type: (.+)?/", $data, $matches)) {
         $contentType = $matches[1];
        }

        // http://en.wikipedia.org/wiki/List_of_HTTP_status_codes
        if(!$this->isCodeOk($status)) {
          throw new HttpURLAccessException("Failed to URL open : {$url}");
        }

        //$result = $contentLength;
      }

      return trim($contentType);
    }

    // http://en.wikipedia.org/wiki/List_of_HTTP_status_codes
    public function isCodeOk(int $httpCode) : bool {
      if($httpCode == 200 || ($httpCode > 300 && $httpCode <= 308)) return true;
      else return false;
    }

    public function setContentType(string $type) : void {
      $this->contentType = $type;
    }

    public function setAutoReferer(bool $enable) : void {
      $this->enableAutoReferer = $enable;
    }

    public function setReferer(string $referer) : void {
      $this->referer = $referer;
    }

    public function getReferer() : ?string {
      return $this->referer;
    }

    public function setPostData(mixed $postData) : void {
      $this->postData = $postData;
    }

    public function getPostData() : mixed {
      return $this->postData;
    }

    public function setAuthorization(string $authorization) : void {
      $this->authorization = $authorization;
    }

    public function getUserAgent() : string {
      return $this->userAgent;
    }

    public function setUsetAgent(string $usetAgent) : void {
      $this->userAgent = $usetAgent;
    }

    public function setTimeout(int $timeout) : void {
      $this->timeout = $timeout;
    }

  }
