<?hh //strict
  namespace nova\crawl;

  use nova\crawl\CrawlContent;
  use nova\crawl\ContentSaver;
  use nova\http\Http;


  abstract class Crawler {
    private ?Http $http;
    public function __construct(private StoreManager $storeManager) {
    }

    public function http() : Http {
      if($this->http === NULL) {
        $this->http = new Http();
      }
      return $this->http;
    }

    public function getStoreManager() : StoreManager {
      return $this->storeManager;
    }

    abstract public function getList(int $page = 1) : Vector<string>;
    abstract public function getContent(string $contentUid) : ?Map<string, mixed>;

    public function run(int $toPage = 1) : void {
      for($page = $toPage; $page >= 1; $page--) {
        echo("-------- {$page} --------\n");
        $list = $this->getList($page);
        foreach($list as $contentUid) {
          if($this->storeManager->has($contentUid)) {
            echo("has : $contentUid\n");
            continue;
          }

          $content = $this->getContent($contentUid);
          if($content === NULL) {
            echo("failed to get a content : $contentUid\n");
            continue;
          }

          if($content["is_skip"] === true) {
            echo("skip : $contentUid\n");
            continue;
          }

          $crawlUid = $this->storeManager->add($content);
          echo("added : $contentUid => $crawlUid\n");
        }
      }
    }
  }
