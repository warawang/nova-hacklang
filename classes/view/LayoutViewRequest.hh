<?hh //strict
  namespace nova\view;

  use nova\view\HtmlMeta;

  class LayoutViewRequest extends ViewRequest {
    protected string $layoutName;
    protected string $viewName;
    protected Map<string, mixed> $data;
    protected HtmlMeta $htmlMeta;

    public function __construct(string $layoutName, string $viewName, ?Map<string, mixed> $data = null, ?HtmlMeta $htmlMeta = null) {
      $this->layoutName = $layoutName;
      $this->viewName = $viewName;
      $this->data = $data === null ? Map {} : $data;
      $this->htmlMeta = $htmlMeta === null ? HtmlMeta::create() : $htmlMeta;
    }

    public function setTitleAndDesc(string $title, string $desc) : void {
      $this->htmlMeta->setTitle($title);
      $this->htmlMeta->setDescription($desc);      
    }

    public function getLayoutName() : string {
      return $this->layoutName;
    }

    public function getViewName() : string {
      return $this->viewName;
    }

    public function getData() : Map<string, mixed> {
      return $this->data;
    }

    public function setHtmlMeta(HtmlMeta $htmlMeta) : void {
      $this->htmlMeta = $htmlMeta;
    }

    public function getHtmlMeta() : HtmlMeta {
      return $this->htmlMeta;
    }
  }
