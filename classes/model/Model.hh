<?hh //strict
  namespace nova\model;

  use nova\model\exception\ModelException;

  class Model {

    /**
     * 메소드 이름과, 파라미터를 기준으로 메소드 결과값을 캐싱할 캐시 키를 생성합니다.
     * 파라미터 없이 호출할 경우 이 메소드를 호출한 메소드의 이름과 파라미터를 추적하여 생성합니다.
     * @param  {[type]} ?string $target       = NULL [description]
     * @return {[type]}         [description]
     */
    public function getCacheKeyFromMethod(?string $targetMethod = NULL, ?array<mixed> $args = NULL, ?string $namespace = NULL) : string {
      $caller = debug_backtrace(DEBUG_BACKTRACE_PROVIDE_OBJECT,2)[1];

      $key = "method/";

      //네임 스페이스
      $key .= $namespace === NULL ? str_replace("\\","/",$caller["class"])."/" : $namespace."/";

      //메소드 이름
      $key .= $targetMethod === NULL ? $caller["function"]."/" : $targetMethod."/";

      //파라미터
      $key .= implode("-",$args === NULL ? $caller["args"] : $args);

      return $key;
    }
  }
/*

array(7) {
  'file' =>
  string(49) "/wwwdata/popcorngak/public_html/www/work/popcorn.hh"
  'line' =>
  int(48)
  'function' =>
  string(9) "getPopcorn"
  'class' =>
  string(21) "service\\popcorn\\Popcorn"
  'object' =>
  class service\popcorn\Popcorn#25 (0) {
  }
  'type' =>
  string(2) "->"
  'args' =>
  array(1) {
    [0] =>
    int(1)
  }
}


 */
