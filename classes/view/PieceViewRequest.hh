<?hh //strict
  namespace nova\view;

  use nova\view\HtmlMeta;

  class PieceViewRequest extends ViewRequest {
    protected string $pieceName;
    protected Map<string, mixed> $data;

    public function __construct(string $pieceName, ?Map<string, mixed> $data = null) {
      $this->pieceName = $pieceName;
      $this->data = $data === null ? Map {} : $data;
    }

    public function getViewName() : string {
      return $this->pieceName;
    }

    public function getData() : Map<string, mixed> {
      return $this->data;
    }
  }
