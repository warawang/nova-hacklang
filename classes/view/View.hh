<?hh //strict
  namespace nova\view;

  use nova\view\ViewRequest;
  use nova\view\Layout;
  use nova\util\Connection;
  use nova\util\Config;

  class View {
    private ViewRequest $request;

    public function __construct(ViewRequest $request) {
      $this->request = $request;
    }

    public function render() : string {
      //Client Cache를 사용하지 않을 경우 관련 헤더 송출
      if(!$this->request->isUseClientCache()) {
        header("Pragma: no-cache");
        header("Cache-Control: no-cache, must-revalidate");
      }

      if($this->request instanceof JsonViewRequest) {
        header('Content-Type: application/json');
        return json_encode($this->request->getData());
      } else if($this->request instanceof XmlViewRequest) {
        header('Content-Type: application/xml');
        $viewPath = ViewUtils::getViewFolder()."/".$this->request->getViewName().".xml";

        //View에서 사용할 값을 지역 변수로 지정
        $d = $this->request->getData();

        ob_start();
        // UNSAFE
        eval("?>".file_get_contents($viewPath)."<?");
				$result = ob_get_clean();

        return $result;
      } else if($this->request instanceof LayoutViewRequest) {
        $viewPath = ViewUtils::getViewFolder()."/".$this->request->getViewName().".html";

        //View에서 사용할 값을 지역 변수로 지정
        $d = $this->request->getData();
        $layout = new Layout($this->request->getLayoutName(), $this->request->getHtmlMeta(), $d);

        ob_start();
        // UNSAFE
        eval("?>".file_get_contents($viewPath)."<?");
				$result = ob_get_clean();

        return $result;
      } else if($this->request instanceof SingleViewRequest) {
        $viewPath = ViewUtils::getViewFolder()."/".$this->request->getViewName().".html";

        //View에서 사용할 값을 지역 변수로 지정
        $d = $this->request->getData();

        ob_start();
        // UNSAFE
        eval("?>".file_get_contents($viewPath)."<?");
				$result = ob_get_clean();

        return $result;
      } else if($this->request instanceof PieceViewRequest) {
        $viewPath = ViewUtils::getLayoutFolder("piece")."/".$this->request->getViewName().".html";

        //View에서 사용할 값을 지역 변수로 지정
        $d = $this->request->getData();

        ob_start();
        // UNSAFE
        eval("?>".file_get_contents($viewPath)."<?");
				$result = ob_get_clean();

        return $result;
      } else if($this->request instanceof ErrorViewRequest) {
        header("HTTP/1.1 500 Internal Server Error");
        $viewPath = ViewUtils::getViewFolder("error")."/".$this->request->getViewName().".html";

        //View에서 사용할 값을 지역 변수로 지정
        $d = $this->request->getData();

        ob_start();
        // UNSAFE
				eval("?>".file_get_contents($viewPath)."<?");
				$result = ob_get_clean();
        return "";
      } else {
        return "";
      }
    }

    public function draw() : void {
      echo($this->render());
    }
  }
