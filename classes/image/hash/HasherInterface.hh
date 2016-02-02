<?hh //strict
  namespace nova\image\hash;

  interface HasherInterface {
    public function hash(resource $im) : mixed;
  }
