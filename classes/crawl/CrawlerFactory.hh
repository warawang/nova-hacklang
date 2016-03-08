<?hh //strict
  namespace nova\crawl;

  use nova\crawl\Crawler;
  use nova\crawl\StoreManager;

  class CrawlerFactory {
    private Map<int, string> $sourceList;
    public function __construct(private string $namespace, array<mixed, mixed> $sourceList, private string $storeManagerClass) : void {
      $this->sourceList = Map {};
      foreach($sourceList as $k=>$v) $this->sourceList->set((int)$k, (string)$v);
    }

    public function create(int $sourceUid) : Crawler {
      $storeManagerReflector = new \ReflectionClass($this->namespace."\\".$this->storeManagerClass);
      $storeManager = $storeManagerReflector->newInstanceArgs([$sourceUid]);

      invariant($storeManager instanceof StoreManager, $this->storeManagerClass." must be Crawler class instance.");

      $crawlerClassName = $this->namespace."\\".$this->sourceList->get($sourceUid);
      $crawlerReflector = new \ReflectionClass($crawlerClassName);
      $crawler = $crawlerReflector->newInstanceArgs([$storeManager]);

      invariant($crawler instanceof Crawler, "{$crawlerClassName} must be Crawler class instance.");

      return $crawler;
    }

    public function getSourceList() : Map<int, string> {
      return $this->sourceList;
    }

  }
