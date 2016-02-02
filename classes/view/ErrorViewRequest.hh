<?hh //strict
  namespace nova\view;

  class ErrorViewRequest extends ViewRequest {
    protected Map<string, mixed> $data;
    protected int $errorCode;

    public function __construct(int $errorCode = 500, Map<string, mixed> $data = Map {}) {
      $this->data = $data;
      $this->errorCode = $errorCode;
    }

    public function getData() : Map<string, mixed> {
      return $this->data;
    }

    public function getViewName() : string {
      return (string)$this->errorCode;
    }

    public function getErrorCode() : int {
      return $this->errorCode;
    }
  }
