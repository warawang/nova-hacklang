<?hh //strict
  namespace nova\view;

  class JsonViewRequest extends ViewRequest {
    protected Map<string, mixed> $data;

    public function __construct(Map<string, mixed> $data) {
      $this->data = $data;
    }

    public function getData() : Map<string, mixed> {
      return $this->data;
    }

    public static function getError(int $code = 1000, string $msg = __t("일시적인 오류가 발생했습니다.")) : JsonViewRequest {
      return new JsonViewRequest(Map {
        "error" => Array(
          "code" => $code,
          "msg" => $msg
        )
      });
    }

  }
