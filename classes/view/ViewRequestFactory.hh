<?hh //strict
  namespace nova\view;

  use nova\view\ViewRequest;
  use nova\view\JsonViewRequest;
  use nova\view\LayoutViewRequest;
  use nova\view\SingleViewRequest;
  use nova\view\ErrorViewRequest;
  use nova\view\HtmlMeta;

  class ViewRequestFactory {
    public static function createJson(Map<string,mixed> $data) : JsonViewRequest {
      return new JsonViewRequest($data);
    }

    public static function createXml(string $viewName, ?Map<string, mixed> $data = null) : ViewRequest {
      return new XmlViewRequest($viewName, $data);
    }

    public static function createLayout(string $layoutName, string $viewName, ?Map<string, mixed> $data = null, ?HtmlMeta $htmlMeta = null) : ViewRequest {
      return new LayoutViewRequest($layoutName, $viewName, $data, $htmlMeta);
    }

    public static function createSingle(string $viewName, ?Map<string, mixed> $data = null) : ViewRequest {
      return new SingleViewRequest($viewName, $data);
    }

    public static function createPiece(string $pieceName, ?Map<string, mixed> $data = null) : ViewRequest {
      return new PieceViewRequest($pieceName, $data);
    }

    public static function createError(int $errorCode = 500, Map<string, mixed> $data = Map {}) : ViewRequest {
      return new ErrorViewRequest($errorCode, $data);
    }
  }
