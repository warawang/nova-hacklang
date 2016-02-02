<?hh //strict
  namespace nova\controller;

  use nova\controller\exception\ActNotFoundException;
  use nova\controller\RequestParam;
  use nova\view\ViewRequestFactory;
  use Map;

  abstract class Controller {
    abstract protected function getActSet() : ImmSet<string>;
    abstract protected function getDefaultAct() : ?string;
    abstract protected function presentation(mixed $result) : void;

    private RequestParam $requestParam;

    public function __construct() {
      $this->requestParam = new RequestParam();
    }

    public function getRequestParam() : RequestParam {
      return $this->requestParam;
    }

    public function getActMethod(string $act) : string {
      if($this->getActSet()->contains($act) === false ) {
        throw new ActNotFoundException("$act is not allow Act");
      }

      return $act;
    }

    public function go() : void {
      $act = $this->getRequestParam()->get()->string("act");
      if($act === NULL) {
        //지정된 액트가 없을 경우 기본 액트 확인
        $act = $this->getDefaultAct();
      }

      if($act === NULL) {
        //기본으로 지정된 액트도 없을 경우 에러페이지 출력.
        $this->presentation(ViewRequestFactory::createError());
        return;
      }

      //허용된 액트가 아닐 경우
      $method = $this->getActMethod((string)$act);

      //UNSAFE
      $viewRequest = $this->$method();

      $this->presentation($viewRequest);
    }
  }
