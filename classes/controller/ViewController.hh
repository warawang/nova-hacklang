<?hh //strict
  namespace nova\controller;
  /**
   * 결과물을 출력하는것이 최종 목표인 controller
   */

  use nova\view\View;
  use nova\view\ViewRequest;
  use nova\view\JsonViewRequest;
  use nova\view\LayoutViewRequest;

  abstract class ViewController extends BaseController {

    protected function presentation(mixed $result) : void {
      if($result instanceof ViewRequest) {
        $view = new View($result);
        $view->draw();

      } else {
        // string or stuff...
        echo($result);
      }
    }
  }
