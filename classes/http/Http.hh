<?hh //strict
  namespace nova\http;

  use nova\io\FileUtils;
  use nova\http\exception\HttpURLAccessException;
  use nova\http\exception\HttpDownloadException;

  class Http {
    private string $userAgent = "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/45.0.2454.93 Safari/537.36";
    private int $timeout = 120;

    private function getCurlSession(string $url, ?array<string,mixed> $postData = NULL, ?string $authorization = NULL) : resource {
      $ch = curl_init();
      $urlinfo = parse_url($url);
      $httpHeader = array();

      curl_setopt($ch, CURLOPT_URL, $url);
      curl_setopt($ch, CURLOPT_HEADER, false);
      curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
      curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
      curl_setopt($ch, CURLOPT_USERAGENT, $this->userAgent);
      curl_setopt($ch, CURLOPT_TIMEOUT, $this->timeout);
      curl_setopt($ch, CURLOPT_REFERER, $urlinfo["scheme"]."://".$urlinfo["host"]);

      if($postData !== NULL) {
        $postDataQuery = http_build_query($postData);

        curl_setopt($ch, CURLOPT_POST, true);
        curl_setopt($ch, CURLOPT_POSTFIELDS, $postDataQuery);
      }

      if($authorization !== NULL) {
        $httpHeader[] = "authorization: ".$authorization;
      }

      // 설정된 헤더가 있을 경우
      if(arcnt($httpHeader) > 0) {
        curl_setopt($ch, CURLOPT_HTTPHEADER, $httpHeader);
      }

      return $ch;
    }

    public function get(string $url, ?array<string,mixed> $postData = NULL, ?string $authorization = NULL) : string {
      $ch = $this->getCurlSession($url, $postData, $authorization);

      $res = curl_exec($ch);

      $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
      if(!$this->isCodeOk($httpCode)) {
        throw new HttpURLAccessException("Failed to URL open : {$url} / ".curl_error($ch));
      }
      curl_close($ch);

      if($res === false) {
        throw new HttpURLAccessException("Failed to URL open : {$url}");
      }

      return $res;
    }

    public function getJson(string $url, ?array<string, mixed> $postData = NULL, ?string $authorization = NULL) : array<mixed,mixed> {
      $jsonString = $this->get($url, $postData, $authorization);

      if(($json = json_decode($jsonString, true)) === NULL) {
        throw new HttpURLAccessException("Failed to decode json string : {$url}");
      }

      return $json;
    }

    public function download(string $url, ?string $destPath = NULL, ?array<string,mixed> $postData = NULL, ?string $authorization = NULL) : string {
      if($destPath === NULL) $destPath = FileUtils::getTempFilePath();
      $ch = $this->getCurlSession($url, $postData, $authorization);

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
