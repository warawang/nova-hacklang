<?hh
use nova\util\ClassLoader;
use nova\util\Config;
use nova\util\Error;

function init(string $rootPath) {
  //Timezone
  date_default_timezone_set("Asia/Seoul");

  //function
  require_once "/wwwdata/nova/classes/func/stuff.hh";
  require_once "/wwwdata/nova/classes/func/language.hh";

  //class
  require_once "/wwwdata/nova/classes/util/Config.hh";
  require_once "/wwwdata/nova/classes/util/Error.hh";
  require_once "/wwwdata/nova/classes/util/exception/ClassFileNotFoundException.hh";
  require_once "/wwwdata/nova/classes/util/ClassLoader.hh";

  // config init
  Config::getInstance()->init($rootPath);

  //error handling
  set_exception_handler('Error::exceptionHandler');
  set_error_handler('Error::errorHandler', E_ALL);

  //autoloader
  spl_autoload_register('nova\util\ClassLoader::callback');

  //config load
  //위의 autoloader 가 설정된 후 호출되어야 함
  Config::getInstance()->load();

  //타임아웃 해제
  set_time_limit(0);
}

if(!array_key_exists("HTTP_HOST",$_SERVER) || stripos($_SERVER['HTTP_HOST'],"pcg.gg") !== false) {
  init("/wwwdata/pcg");
} else if(stripos($_SERVER['HTTP_HOST'],"iu.mayb.so") !== false) {
  init("/wwwdata/iu");
}
