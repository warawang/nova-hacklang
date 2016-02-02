<?hh //strict
  namespace nova\syndication;

  abstract class SyndicationReader {
    abstract public function getContentList(int $groupUid) : array<mixed>;
  }
