<?hh //strict

  namespace nova\util;

  use nova\util\exception\ConfigLoadException;
  use nova\util\exception\ConfigParsingException;
  use nova\util\Col;
  use nova\util\Connection;

  class Config {
    private static ?Config $instance;

    private bool $isLoaded = false;
    private string $rootPath = "";
    private Map<string,mixed> $configMap = Map{};

    protected function __construct() {}

    /**
     * 싱글톤 인스턴스
     * @return {Config}
     */
    public static function getInstance() : Config {
      if(self::$instance === null) {
        self::$instance = new Config();
      }
      return self::$instance;
    }

    public function load(bool $reload = false) : void {
      if($reload == false && $this->isLoaded()) return;

      $configPath = $this->getRootPath()."/conf/";
      if($this->isWorkServer()) {
        $configPath .= "nova-work.json";        
      } else {
        $configPath .= "nova.json";
      }

      $jsonString = @file_get_contents($configPath);
      if($jsonString === false) {
        throw new ConfigLoadException("config.json open fail.");
      }

      $arr = json_decode($jsonString,true);
      if($arr === null) {
        throw new ConfigParsingException("config.json contents error.");
      }

      $this->configMap = Col::toMap($arr);
      $this->isLoaded = true;
    }

    public function isLoaded() : bool {
      return $this->isLoaded;
    }

    public function isWorkServer() : bool {
      return stripos(gethostname(),'work')!==false ? true : false;
    }

    public function init(string $root) : void {
      $this->rootPath = $root;
    }

    public function getExtra(string $filename) : ?array<mixed> {
      $configPath = $this->getRootPath()."/conf/".$filename;
      $jsonString = @file_get_contents($configPath);
      if($jsonString === false) {
        return NULL;
      }

      $arr = json_decode($jsonString,true);
      if($arr === null) {
        throw new ConfigParsingException("failed to parse extra config file : ".$configPath);
      }

      return $arr;
    }

    public function getRootPath() : string {
      return $this->rootPath;
    }

    public function getServiceId() : string {
      // UNSAFE
      return $this->configMap['service']['id'];
    }

    public function getServiceName() : string {
      // UNSAFE
      return $this->configMap['service']['name'];
    }

    public function getServiceDesc() : string {
      // UNSAFE
      return $this->configMap['service']['desc'];
    }

    public function getServiceTitle() : string {
      // UNSAFE
      return $this->configMap['service']['title'];
    }

    public function getServiceFaceImage() : string {
      // UNSAFE
      return $this->configMap['service']['faceImage'];
    }

    public function getServiceKeywords() : string {
      // UNSAFE
      return $this->configMap['service']['keywords'];
    }

    public function getServiceMasterEmail() : string {
      // UNSAFE
      return $this->configMap['service']['masterEmail'];
    }

    public function getStaticURL() : string {
      return Connection::getProtocol()."://".$this->getDomain("static");
    }

    public function getDomain(string $target, bool $includeProtocol = false) : string {
      // UNSAFE
      return ($includeProtocol ? Connection::getProtocol()."://" : "") . $this->configMap['domain'][$target];
    }

    public function getAWSKey() : string {
      //UNSAFE
      return $this->configMap["aws"]["credentials"]["key"];
    }

    public function getAWSSecret() : string {
      //UNSAFE
      return $this->configMap["aws"]["credentials"]["secret"];
    }

    public function getAWSRegion() : string {
      //UNSAFE
      return $this->configMap["aws"]["region"];
    }

    public function getS3Bucket(string $target = "stg") : string {
      //UNSAFE
      return $this->configMap['s3']['bucket'][$target];
    }

    public function getS3Option(string $target = "stg") : array<string, mixed> {
      $option =  Array(
        "region"            => $this->getAWSRegion(),
        "version"           => "latest",
        'credentials' => [
            'key'    => $this->getAWSKey(),
            'secret' => $this->getAWSSecret(),
        ]
      );
      return $option;
    }

    public function getS3Host(string $target = "stg") : string {
      //UNSAFE
      return $this->configMap['s3']['host'][$target];
    }

    public function getStaticVersion(string $target) : int {
      //UNSAFE
      return (int)$this->configMap['html'][$target]['staticVersion'];
    }

    public function getHtmlInfo(string $target) : Map<string, mixed> {
      //UNSAFE
      return $this->configMap['html'][$target];
    }

    public function getDBHost(string $target) : Vector<string> {
      //UNSAFE
      return $this->configMap['db']['host'][$target];
    }

    public function getDBUsername() : string {
      //UNSAFE
      return $this->configMap['db']['username'];
    }

    public function getDBPW() : string {
      //UNSAFE
      return $this->configMap['db']['pw'];
    }

    public function getDBPrefix() : string {
      //UNSAFE
      return $this->configMap['db']['prefix'];
    }

    public function getMCHost(string $target = "default") : string {
      //UNSAFE
      return $this->configMap["mc"]["host"][$target];
    }

    public function getMCPrefix() : string {
      //UNSAFE
      return $this->configMap["mc"]["prefix"];
    }

    public function getMcRev(string $key) : int {
      //UNSAFE
      $rev = $this->configMap["mc"]["rev"];
      if(!$rev->contains($key)) return 0;

      return $rev[$key];
    }

    public function getCryptKey() : string {
      //UNSAFE
      return $this->configMap['crypt']['key'];
    }

    public function getFFMpegPath() : string {
      //UNSAFE
      return $this->configMap['ffmpeg']['path'];
    }

    public function getFacebook(string $target) : string {
      //UNSAFE
      return $this->configMap['facebook'][$target];
    }

    public function getTwitter(string $target) : string {
      //UNSAFE
      return $this->configMap['twitter'][$target];
    }

    // https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes
    public function getDefaultLanguage() : string {
      //UNSAFE
      return $this->configMap["language"]["default"];
    }

    public function getSyndicationUrl(string $target) : string {
      //UNSAFE
      return $this->configMap["syndication"][$target]["url"];
    }

    public function getSyndicationKey(string $target) : string {
      //UNSAFE
      return $this->configMap["syndication"][$target]["key"];
    }

    public function getImaggaKey() : string {
      //UNSAFE
      return $this->configMap["imagga"]["key"];
    }


  }
