<?hh //strict
  namespace nova\view;

  use nova\view\HtmlMetaElement;
  use nova\view\HtmlMetaPair;
  use nova\util\Config;
  use nova\util\Connection;

  class HtmlMeta {
    private Map<string, HtmlMetaElement> $list;

    private function __construct() {
      $this->list = Map {};
    }

    public static function create(?string $title = NULL, ?string $desc = NULL) : HtmlMeta {
      $m = new HtmlMeta();
      $m->setWithName("title", $title !== NULL ? $title : Config::getInstance()->getServiceTitle());

      $m->setWithName("description", $desc !== NULL ? $desc : Config::getInstance()->getServiceKeywords());
      $m->setWithName("generator", Config::getInstance()->getServiceName());
      $m->setWithName("distribution", Config::getInstance()->getServiceName());
      $m->setWithName("keywords", Config::getInstance()->getServiceKeywords());

      //페이스북 OG
      $m->setWithProperty("og:title", $title !== NULL ? $title : Config::getInstance()->getServiceTitle());
      $m->setWithProperty("og:description", $desc !== NULL ? $desc : Config::getInstance()->getServiceDesc());
      $m->setWithProperty("og:type", "website");
      $m->setWithProperty("og:url", Connection::getProtocol()."://".Config::getInstance()->getDomain("www"));
      $m->setWithProperty("og:image", Connection::getProtocol()."://".Config::getInstance()->getS3Host("stg").Config::getInstance()->getServiceFaceImage());
      $m->setWithProperty("og:site_name", Config::getInstance()->getServiceName());
      $m->setWithProperty("fb:app_id", Config::getInstance()->getFacebook("appID"));

      //트위터 CARD
      $m->setWithName("twitter:card", "summary_large_image");
      $m->setWithName("twitter:site", Config::getInstance()->getTwitter("username"));
      $m->setWithName("twitter:site:id", Config::getInstance()->getTwitter("username"));
      $m->setWithName("twitter:creator", Config::getInstance()->getTwitter("username"));
      $m->setWithName("twitter:title", $title !== NULL ? $title : Config::getInstance()->getServiceTitle());
      $m->setWithName("twitter:description", $desc !== NULL ? $desc : Config::getInstance()->getServiceDesc());
      $m->setWithName("twitter:image", Connection::getProtocol()."://".Config::getInstance()->getS3Host("stg").Config::getInstance()->getServiceFaceImage());

      return $m;
    }

    /**
     * 셋 헬퍼
     */
    public function setTitle(string $title) : void {
      $this->setWithName("title", $title);
      $this->setWithProperty("og:title", $title);
      $this->setWithName("twitter:title", $title);    
    }

    public function setDescription(string $title) : void {
      $this->setWithName("description", $title);
      $this->setWithProperty("og:description", $title);
      $this->setWithName("twitter:description", $title);
    }

    public function setImage(string $title) : void {
      $this->setWithName("image", $title);
      $this->setWithProperty("og:image", $title);
      $this->setWithName("twitter:image", $title);
    }

    public function set(HtmlMetaElement $el) : void {
      $this->list->set($this->makeElementKey($el), $el);
    }

    public function makeElementKey(HtmlMetaElement $element) : string {
      $key = (string)$element->get(0) . ":";
      $attrs = $element->get(1);
      invariant($attrs instanceof Vector, "Attribute list not found.");

      $keyCount = 0;
      foreach($attrs as $attr) {
        if($attr["isUnique"] === true) {
          $keyCount ++;
          $key .= $attr["name"]."=".$attr["value"].",";
        }
      }

      //지정된 키 어트리뷰트가 없으면 에러 발생
      invariant($keyCount > 0, "Key attribute list not found.");
      $key = substr($key,0,-1);

      return $key;
    }

    public function createAttr(string $name, string $value, bool $isUnique = false) : HtmlMetaAttr {
      return shape("name" => $name, "value" => $value, "isUnique" => $isUnique);
    }

    public function getContent(string $tag, string $name) : string {
      $element = $this->list->get("meta:{$tag}={$name}");
      invariant($element instanceof Pair,"Element must be Pair.");

      $attrs = $element->get(1);
      invariant($attrs instanceof Vector,"Attribute list must be vector.");

      foreach($attrs as $attr) {
        if($attr["name"] == "content") {
          return $attr["value"];
        }
      }

      throw new \OutOfBoundsException("{$tag}={$name} attribute not found.");
    }

    public function setWithName(string $name, string $content) : void {
      $this->set(Pair {
        "meta",
        Vector {
          $this->createAttr("name", $name, true),
          $this->createAttr("content", $content)
        }
      });
    }

    public function getContentWithName(string $name) : string {
      return $this->getContent("name", $name);
    }

    public function setWithProperty(string $name, string $content) : void {
      $this->set(Pair {
        "meta",
        Vector {
          $this->createAttr("property", $name, true),
          $this->createAttr("content", $content)
        }
      });
    }

    public function getContentWithProperty(string $name) : string {
      return $this->getContent("property", $name);
    }

    public function setLink(string $rel, string $href) : void {
      $this->set(Pair {
        "link",
        Vector {
          $this->createAttr("rel", $rel, true),
          $this->createAttr("href", $href)
        }
      });
    }

    public function setHreflang(string $hreflang, string $href) : void {
      $this->set(Pair {
        "link",
        Vector {
          $this->createAttr("rel", "alternate", true),
          $this->createAttr("hreflang", $hreflang, true),
          $this->createAttr("href", $href)
        }
      });
    }

    public function setCanonicalURL(string $url, bool $isRelative = true) : string {
      if($isRelative) {
        $url = Connection::getProtocol()."://".Config::getInstance()->getDomain(Connection::getHost()).$url;
      }
      $this->setLink("canonical", $url);
      $this->setHreflang(Config::getInstance()->getDefaultLanguage(), $url);

      return $url;

      // TODO : support language 별 canonialURL 지설정
    }

    public function toHtml() : string {
      $str = "";
      foreach($this->list as $element) {
        if(!($element instanceof Pair)) continue;
        $str .= "<".(string)$element->get(0)." ";
        $attrs = $element->get(1);
        if(!($attrs instanceof Vector)) continue;

        foreach($attrs as $attr) {
          //$str .= $attr->get(0)->get(0)."=\"".htmlspecialchars($attr->get(0)->get(1))."\" ";
          $str .= $attr["name"]."=\"".$attr["value"]."\" ";
        }
        $str = substr($str, 0, -1);
        $str .= ">\n";
      }
      return $str;
    }

  }
