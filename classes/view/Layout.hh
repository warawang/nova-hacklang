<?hh //strict
  namespace nova\view;

  use nova\util\Config;
  use nova\util\Connection;
  use nova\view\HtmlMeta;

  class Layout {
    protected string $layoutName;
    protected HtmlMeta $htmlMeta;
    protected Vector<string> $jsList = Vector{};
    protected Vector<string> $cssList = Vector{};
      protected Map<string, mixed> $data;

    public function __construct(string $layoutName = "default",HtmlMeta $htmlMeta = HtmlMeta::create(), ?Map<string, mixed> $data = null) {
      $this->layoutName = $layoutName;
      $this->htmlMeta = $htmlMeta;
      $this->data = $data === null ? Map {} : $data;

      $conHost = Connection::getHost();

      //JS
      $baseJsList = Config::getInstance()->getHtmlInfo($conHost)->get('js');
      invariant($baseJsList instanceof Vector, "Base js list not found in config.json");
      foreach($baseJsList as $k=>$v) {
        $this->addJs($v);
      }

      //CSS
      $baseCssList = Config::getInstance()->getHtmlInfo($conHost)->get('css');
      invariant($baseCssList instanceof Vector, "Base css list not found in config.json");
      foreach($baseCssList as $k=>$v) {
        $this->addCss($v);
      }
    }

    public function addResource(string $target,string $path) : void {
      if($target=="js") $list = $this->jsList;
      else $list = $this->cssList;

      $info = new Map(parse_url($path));
      if($info->get('host') === null) {
        $list->add(Connection::getProtocol().'://'.Config::getInstance()->getDomain("static").$info['path'].
                    "?".Config::getInstance()->getStaticVersion(Connection::getHost()));
      } else {
        $list->add(Connection::getProtocol().'://'.$info['host'].$info['path']);
      }
    }
    public function addJs(string $path) : void {
      $this->addResource("js", $path);
    }

    public function addCss(string $path) : void {
      $this->addResource("css", $path);
    }

    public function getLayoutName() : string {
      return $this->layoutName;
    }

    public function header() : string {
      return $this->render("header");
    }

    public function footer() : string {
      return $this->render("footer");
    }

    public function sidebar() : string {
      return $this->render("sidebar");
    }

    public function piece(string $target) : string {
      return $this->render($target, "piece");
    }

    public function render(string $target, ?string $layoutName = NULL) : string {
      $filePath = ViewUtils::getLayoutFolder($layoutName === NULL ? $this->layoutName : $layoutName) . "/" . $target .".html";

      //필요한 값들을 전역변수로 지정
      $jsList = $this->jsList;
      $cssList = $this->cssList;
      $htmlMeta = $this->htmlMeta;
      $d = $this->data;
      $layout = $this;
      $config = Config::getInstance();

      ob_start();
      // UNSAFE
      eval("?>".file_get_contents($filePath)."<?");
      $result = ob_get_clean();

      return $result;
    }

    public function draw(string $target) : void {
      echo($this->render($target));
    }



  }
