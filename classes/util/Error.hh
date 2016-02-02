<?hh
  //TODO : 하기 클레스 Namespace 지정 필요
  final class WarningException              extends ErrorException {}
  final class ParseException                extends ErrorException {}
  final class NoticeException               extends ErrorException {}
  final class CoreErrorException            extends ErrorException {}
  final class CoreWarningException          extends ErrorException {}
  final class CompileErrorException         extends ErrorException {}
  final class CompileWarningException       extends ErrorException {}
  final class UserErrorException            extends ErrorException {}
  final class UserWarningException          extends ErrorException {}
  final class UserNoticeException           extends ErrorException {}
  final class StrictException               extends ErrorException {}
  final class RecoverableErrorException     extends ErrorException {}
  final class DeprecatedException           extends ErrorException {}
  final class UserDeprecatedException       extends ErrorException {}

  use nova\util\Config;
  use nova\util\Connection;

  class Error {
    public static function exceptionHandler($e) {
      $str =  "[".get_class($e)."] ". $e->getMessage(). " at line {$e->getLine()} in {$e->getFile()}\n";
      $trace = $e->getTrace();
      if(count($trace)>=1) {
        $str .= "- Trace \n";
        $step = 0;
        foreach($e->getTrace() as $v) {
          if(isset($v["file"])) $str .= "- #{$step} : at line {$v['line']} ({$v['function']}) in {$v['file']}";
          else $str .= "- #{$step}";
          if(isset($v['class'])) {
            $str .= " / {$v['class']}";
          }
          $str .= "\n";
          $step ++;
        }
      }
      self::fire($str);
    }

    public static function getErrorName($errno) {
      switch($errno)
      {
        case E_ERROR:             return "E_ERROR";
        case E_WARNING:           return "E_WARNING";
        case E_PARSE:             return "E_PARSE";
        case E_NOTICE:            return "E_NOTICE";
        case E_CORE_ERROR:        return "E_CORE_ERROR";
        case E_CORE_WARNING:      return "E_CORE_WARNING";
        case E_COMPILE_ERROR:     return "E_COMPILE_ERROR";
        case E_COMPILE_WARNING:   return "E_COMPILE_WARNING";
        case E_USER_ERROR:        return "E_USER_ERROR";
        case E_USER_WARNING:      return "E_USER_WARNING";
        case E_USER_NOTICE:       return "E_USER_NOTICE";
        case E_STRICT:            return "E_STRICT";
        case E_RECOVERABLE_ERROR: return "E_RECOVERABLE_ERROR";
        case E_DEPRECATED:        return "E_DEPRECATED";
        case E_USER_DEPRECATED:   return "E_USER_DEPRECATED";
        default:                  return "E_FALTAL_ERROR";
      }
    }


    public static function errorHandler(
      $errno,
      $errmsg,
      $errfile,
      $errline,
      $errcontext = array(),
      $errtrace = array(),
    ): bool {
      //skip notice
      if($errno === E_NOTICE) return false;

      // if error has been supressed with an @
      if (error_reporting() === 0) return false;      

      self::fire("[".self::getErrorName($errno)."] {$errmsg} at line {$errline} in {$errfile}");

      /*
      switch($errno)
      {
        case E_ERROR:               throw new ErrorException            ($errmsg, 0, $errno, $errfile, $errline);
        case E_WARNING:             throw new WarningException          ($errmsg, 0, $errno, $errfile, $errline);
        case E_PARSE:               throw new ParseException            ($errmsg, 0, $errno, $errfile, $errline);
        case E_NOTICE:              throw new NoticeException           ($errmsg, 0, $errno, $errfile, $errline);
        case E_CORE_ERROR:          throw new CoreErrorException        ($errmsg, 0, $errno, $errfile, $errline);
        case E_CORE_WARNING:        throw new CoreWarningException      ($errmsg, 0, $errno, $errfile, $errline);
        case E_COMPILE_ERROR:       throw new CompileErrorException     ($errmsg, 0, $errno, $errfile, $errline);
        case E_COMPILE_WARNING:     throw new CoreWarningException      ($errmsg, 0, $errno, $errfile, $errline);
        case E_USER_ERROR:          throw new UserErrorException        ($errmsg, 0, $errno, $errfile, $errline);
        case E_USER_WARNING:        throw new UserWarningException      ($errmsg, 0, $errno, $errfile, $errline);
        case E_USER_NOTICE:         throw new UserNoticeException       ($errmsg, 0, $errno, $errfile, $errline);
        case E_STRICT:              throw new StrictException           ($errmsg, 0, $errno, $errfile, $errline);
        case E_RECOVERABLE_ERROR:   throw new RecoverableErrorException ($errmsg, 0, $errno, $errfile, $errline);
        case E_DEPRECATED:          throw new DeprecatedException       ($errmsg, 0, $errno, $errfile, $errline);
        case E_USER_DEPRECATED:     throw new UserDeprecatedException   ($errmsg, 0, $errno, $errfile, $errline);
      }
      */

      return false;
    }

    /**
     * CLI 이거나 워킹 서버일 경우에는 에러메세지 출력, 그 외에는 에러페이지 출력.
     * @param  {[type]} $str [description]
     * @return {[type]}      [description]
     *
     */
    private static function fire($str) : void {
      if(Config::getInstance()->isWorkServer()) {
        echo(nl2br($str));
      } else {
        // TODO 에러 로깅

        if(Connection::isCli()) {
          echo(nl2br($str));
        } else {
          // TODO ErrorViewController 를 만들어서 대응
          header("HTTP/1.1 500 Internal Server Error");
        }
        exit;
      }
    }
  }
