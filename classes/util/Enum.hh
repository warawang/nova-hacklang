<?hh //strict
  namespace nova\util;
  
  abstract class Enum {
      private static Map<string,array<string,mixed>> $constCache = Map {};

      /*public static function __callStatic(string $name, string $args) : void {
      }*/

      public static function getConstants() : array<string,mixed> {
        $calledClass = get_called_class();

        if(!self::$constCache->containsKey($calledClass)) {
          $reflect = new \ReflectionClass($calledClass);
          self::$constCache[$calledClass] = $reflect->getConstants();
        }

        return self::$constCache[$calledClass];
      }

      public static function containsName(string $name) : bool {
          $constants = self::getConstants();

          if(!is_array($constants)) return false;
          return array_key_exists($name, $constants);
      }

      public static function contains(mixed $value) : bool {
          $values = self::getConstants();
          return in_array($value, $values, true);
      }

      public static function value2name(mixed $value) : string {
        $constants = self::getConstants();
        if(!is_array($constants)) throw new \OutOfBoundsException();

        foreach($constants as $k=>$v) {
          if($v == $value) return $k;
        }

        throw new \OutOfBoundsException();
      }

      public static function name2value(mixed $name) : mixed {
        $constants = self::getConstants();
        if(!is_array($constants) || $constants[$name] === null) {
          throw new \OutOfBoundsException();
        }

        return $constants[$name];
      }

  }
