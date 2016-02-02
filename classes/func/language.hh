<?hh //strict

  function __t(string $str,...) : string {
    $argsCount = func_num_args();
    if($argsCount>1) {
      $args = func_get_args();
      for($i=1; $i<$argsCount; $i++) {
        $str = str_replace("%{$i}",$args[$i],$str);
      }
    }
    return $str;
  }
