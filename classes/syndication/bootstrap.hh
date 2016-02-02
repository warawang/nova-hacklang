<?hh //strict
  namespace nova\syndication {
    enum SyndicationLogType : string {
      Update = "U";
      Delete = "D";
      Insert = "I";
    }
  }
