<?hh //strict
  namespace nova\controller;

  class RequestParam {
    private RequestParamMap $get;
    private RequestParamMap $post;

    public function __construct() {
      $this->get = new RequestParamMap(RequestType::Get);
      $this->post = new RequestParamMap(RequestType::Post);
    }

    public function get() : RequestParamMap {
      return $this->get;
    }

    public function post() : RequestParamMap {
      return $this->post;
    }

    public function isGET(): bool {
      //UNSAFE
      return $_SERVER['REQUEST_METHOD'] === 'GET';
    }


  }
