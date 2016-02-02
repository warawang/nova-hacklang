<?hh //strict
  namespace nova\view {
    type HtmlMetaElement = Pair<string, Vector<HtmlMetaAttr>>; //tag name, attrs
    type HtmlMetaAttr = shape("name" => string, "value" => string, "isUnique" => bool);

    enum ViewRequestType : int {
      Json = 0;
      Layout = 1;
    }
  }
