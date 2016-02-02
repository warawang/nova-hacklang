<?hh //strict
  namespace nova\util;

  class Col {

    public static function every<Tk, Tv>(\Indexish<Tk, Tv> $collection, (function(Tk, Tv): bool) $callback): bool {
      foreach ($collection as $key => $value) {
          if (!call_user_func_array($callback, [$key, $value])) {
              return false;
          }
      }

      return true;
    }

    public static function isNumeric<Tk, Tv>(\Indexish<Tk, Tv> $collection): bool {
        return static::every($collection, ($key, $value) ==> is_numeric($value) );
    }

    public static function toMap<Tk, Tv, Tr>(Tr $resource): Map<Tk, Tv> {
      invariant($resource instanceof \Indexish, 'Resource must be traversable');

      $map = Map{};
      foreach ($resource as $key => $value) {
        if ($value instanceof Vector || (is_array($value) && $value && Col::isNumeric(array_keys($value)))) {
          $map[$key] = static::toVector($value);
        } else if ($value instanceof \Indexish){
          $map[$key] = static::toMap($value);
        } else {
          $map[$key] = $value;
        }
      }

      // UNSAFE
      return $map;
    }

    public static function toVector<Tv, Tr>(Tr $resource): Vector<Tv> {
        invariant($resource instanceof \Indexish, 'Resource must be traversable');

        $vector = Vector {};

        foreach ($resource as $value) {
            if ($value instanceof Map || (is_array($value) && !Col::isNumeric(array_keys($value)))) {
                $vector[] = static::toMap($value);
            } else if ($value instanceof \Indexish) {
                $vector[] = static::toVector($value);
            } else {
                $vector[] = $value;
            }
        }

        // UNSAFE
        return $vector;
    }
  }
