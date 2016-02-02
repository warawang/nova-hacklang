<?hh //strict
  namespace nova\model;

  class ArrayListResult {
    public function __construct(
      private array<mixed> $list,
      private bool $isLast
      ) {}

    public function isLast() : bool {
      return $this->isLast;
    }

    public function getList() : array<mixed> {
      return $this->list;
    }

    public function getCount() : int {
      return arcnt($this->list);
    }
  }
