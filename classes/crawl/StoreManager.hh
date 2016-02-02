<?hh //strict
  /**
   * 크롤링 raw 데이터를 저장하기 위한 함수들을 impl 한다.
   * raw 데이터를 가공하여 서비스에 활용하는 클래스는 별도 정의한다.
   */
  namespace nova\crawl;

  abstract class StoreManager {
    public function __construct(private int $sourceUid) {}

    public function getSourceUid() : int {
      return $this->sourceUid;
    }

    abstract public function add(Map<string, mixed> $content) : int;
    abstract public function set(string $contentUid, Map<string, mixed> $content) : void;
    abstract public function has(string $contentUid) : bool;
    abstract public function get(string $contentUid) : ?array<string, mixed>;
  }
