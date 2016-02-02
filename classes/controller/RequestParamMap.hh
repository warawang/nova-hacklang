<?hh //strict
  namespace nova\controller;

  class RequestParamMap {
    private Map<string,mixed> $map;
    public function __construct(RequestType $requestType) {
      //UNSAFE
      $this->map = new Map($requestType === RequestType::Get ? $_GET : $_POST);
    }

    public function int(string $name) : ?int {
      if(!$this->map->contains($name)) { return null; }
      $param = $this->map->get($name);
      if (!is_numeric($param)) { return null; }

      return (int)$param;
    }

    public function string(string $name) : ?string {
      if(!$this->map->contains($name)) { return null; }
      $param = $this->map->get($name);
      if (!is_string($param)) { return null; }

      return $param;
    }

    public function getArray(string $name) : ?array<int,mixed> {
      if(!$this->map->contains($name)) { return null; }
      $param = $this->map->get($name);
      if (!is_array($param)) { return null; }

      return $param;
    }

  }
