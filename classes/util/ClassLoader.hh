<?hh
  namespace nova\util;

  use nova\util\Config;
  use nova\util\exception\ClassFileNotFoundException;

  class ClassLoader {
    public static function callback(string $target) : void {
      //namespace to path
      $path = str_replace("\\","/",$target);
      $dirs = explode("/",$path);
      $dirsLength = count($dirs);
      $className = $dirs[$dirsLength-1];

      //자체 라이브러리
      if(in_array($dirs[0],Array("nova","service","controller"))) {
        /**
         * 각 네임스페이스 2depth 폴더에는 bootstrap.hh 파일이 존재할 수 있으며
         * 이 파일에는 하위 클래스가 공통으로 사용는 정보가 정의되어 있다.
         */
        if(count($dirs)>=2 && strlen($dirs[1])>=1) {
          if($dirs[0] === "nova") $bsPath = Config::getInstance()->getRootPath()."/nova/classes/".$dirs[1]."/bootstrap.hh";
          else $bsPath = Config::getInstance()->getRootPath()."/classes/".$dirs[0]."/".$dirs[1]."/bootstrap.hh";

          if(is_file($bsPath)) {
            if(!in_array($bsPath,get_included_files())) {
              // UNSAFE
              require_once $bsPath;
              if(class_exists($target,false)) {
                //요구하던 클래스가 bootstrap 을 통해 정이 되었을 경우 별도의 클래스 파일 로딩을 시도하지 않는다.
                return;
              }
            }
          }
        }


        // 클래스 파일 로딩
        if($dirs[0] === "nova") $filePath = Config::getInstance()->getRootPath()."/nova/classes/".implode("/",array_slice($dirs, 1)).".hh";
        else $filePath = Config::getInstance()->getRootPath()."/classes/".$path.".hh";

        if(is_file($filePath)) {
          require_once $filePath;
        }

        if(!class_exists($target,false) && !interface_exists($target, false) && !trait_exists($target, false)) {
          throw new ClassFileNotFoundException("[{$className}] Class File Not Found ({$filePath})");
        }
      } else {
        //3rd party PHP Extension

        //bootstrap
        require_once Config::getInstance()->getRootPath()."/classes/vender/bootstrap.hh";

        // Class file loading
        $filePath = Config::getInstance()->getRootPath()."/classes/vender/".$path.".php";
        if(is_file($filePath)) {
          require_once $filePath;
        }

        if(!class_exists($target,false) && !interface_exists($target, false) && !trait_exists($target, false)) {
          throw new ClassFileNotFoundException("[{$className}] Class File Not Found ({$filePath})");
        }
      }
    }
  }
