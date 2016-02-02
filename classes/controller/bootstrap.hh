<?hh //strict
  namespace nova\controller {
    enum RequestType : int {
      Get = 0;
      Post = 1;
    }

    enum RequestParamType : int {
      String = 0;
      Integer = 1;
    }
  }
