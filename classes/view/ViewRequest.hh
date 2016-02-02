<?hh //strict
  namespace nova\view;

  use nova\view\HtmlMeta;

  abstract class ViewRequest {
    private bool $useClientCache = false;
    abstract public function getData() : Map<string, mixed>;

    public function getLayoutName() : string {
      return "";
    }

    public function getViewName() : string {
      return "";
    }

    public function getHtmlMeta() : HtmlMeta {
      return HtmlMeta::create();
    }

    public function setUseClientCache(bool $disable) : void {
      $this->useClientCache = $disable;
    }

    public function isUseClientCache() : bool {
      return $this->useClientCache;
    }
  }
