<?hh //strict
  namespace nova\db;

  use nova\util\Config;
  use nova\db\DBC;
  use PDO;
  use PDOException;

  class DB {
    private static Map<string,DBC> $instance = Map{};

    protected function __construct() {}

    /**
     * 싱글톤 인스턴스
     * @return {Config}
     */
    public static function getInstance(string $target = "master") : DBC {
      if(self::$instance->get($target) === null) {
        Config::getInstance()->getDBHost($target)->shuffle();
        $host = Config::getInstance()->getDBHost($target)->get(0);
        $dsn = "mysql:host=".$host.";dbname=";

        try {
					self::$instance[$target] = new DBC($dsn, Config::getInstance()->getDBUsername(), Config::getInstance()->getDBPW());
					self::$instance[$target]->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
          $res = self::$instance[$target]->query("SET names utf8mb4");
				} catch (PDOException $e) {
          throw new PDOException("DB Open Fail.");
				}

      }
      return self::$instance[$target];
    }
  }
