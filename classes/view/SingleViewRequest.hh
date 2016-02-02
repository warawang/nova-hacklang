<?hh //strict
  namespace nova\view;

  use nova\view\HtmlMeta;

  class SingleViewRequest extends ViewRequest {
    protected string $viewName;
    protected Map<string, mixed> $data;

    public function __construct(string $viewName, ?Map<string, mixed> $data = null) {
      $this->viewName = $viewName;
      $this->data = $data === null ? Map {} : $data;
    }

    public function getViewName() : string {
      return $this->viewName;
    }

    public function getData() : Map<string, mixed> {
      return $this->data;
    }
  }
