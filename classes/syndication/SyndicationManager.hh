<?hh //strict
  namespace nova\syndication;

  use nova\util\Config;

  /**
   * TODO
   * 신디케이션 채널이 2개이상 생기면 폴리싱 하자.
   * 수신부, 핑 발송부n개 로 나누면 될 듯.
   */

  class SyndicationManager {
    const GROUP_LOG_LIMIT = 100;
    /**
     * Syndication Log 를 생성한다.
     * @param {[type]} int                $contentType [description]
     * @param {[type]} int                $contentUid  [description]
     * @param {[type]} SyndicationLogType $type        [description]
     */
    public function addLog(int $contentType, int $contentUid, SyndicationLogType $type) : int {
      $logUid = \nova\db\DB::getInstance()->unique(Config::getInstance()->getDBPrefix()."syndication.`log`", Map {
        "content_type" => $contentType,
        "content_uid" => $contentUid,
        "type" => $type,
        "create_at" => time()
      });

      return (int)$logUid;
    }

    /**
     * Group이 지정되지 않은 로그 중, 가장 큰 log_uid 값을 반환한다
     * @return {[type]} 그룹이 지정되지 않은 로그가 없을 경우 NULL을 반환한다.
     */
    public function getNoGroupLogUid() : ?int {
      $res = \nova\db\DB::getInstance()->getRow("
      SELECT MAX(log_uid) as log_uid FROM (
        SELECT log_uid FROM ".Config::getInstance()->getDBPrefix()."syndication.`log` WHERE group_uid=? ORDER BY log_uid ASC LIMIT ?
      ) as T1
      ", Vector {
        0,
        self::GROUP_LOG_LIMIT
      });

      if(!is_array($res) || $res["log_uid"] === NULL) return NULL;
      return (int)$res["log_uid"];
    }

    /**
     * $maxLogId 이하의 log 들의 groupID를  $groupId 로 변경한다
     * @param {[type]} int $groupdID [description]
     * @param {[type]} int $maxLogId [description]
     */
    public function makeGroup(int $maxLogUid) : int {
      $groupUid = \nova\db\DB::getInstance()->insert("INSERT INTO ".Config::getInstance()->getDBPrefix()."syndication.`group`(create_at) VALUES(?)", Vector {
        time()
      });

      $res = \nova\db\DB::getInstance()->query("UPDATE ".Config::getInstance()->getDBPrefix()."syndication.`log` SET group_uid=? WHERE group_uid=? AND log_uid<=?", Vector {
        $groupUid,
        0,
        $maxLogUid
      });

      return $groupUid;
    }

    /**
     * 그룹의 정보를 얻는다.
     * @param  {[type]} int $groupUid     [description]
     * @return {[type]}     [description]
     */
    public function getGroup(int $groupUid) : ?array<string, mixed> {
      return \nova\db\DB::getInstance()->getRow("SELECT group_uid, create_at FROM ".Config::getInstance()->getDBPrefix()."syndication.`group` WHERE group_uid=?", Vector {
        $groupUid
      });
    }

    /**
     * 신디케이션 채널 추가시 개선 필요.
     * @param  {[type]} string $pingUrl      [description]
     * @return {[type]}        [description]
     */
    public function sendPing(string $pingUrl) : string {
      $pingAuthHeader = "Authorization: Bearer ".Config::getInstance()->getSyndicationKey("naver");
      $pingOpt = array(
      CURLOPT_URL => Config::getInstance()->getSyndicationUrl("naver"),
      CURLOPT_POST => true,
      CURLOPT_POSTFIELDS => "ping_url=" . urlencode($pingUrl),
      CURLOPT_RETURNTRANSFER => true,
      CURLOPT_CONNECTTIMEOUT => 10,
      CURLOPT_TIMEOUT => 10,
      CURLOPT_HTTPHEADER => array("Host: apis.naver.com", "Pragma: no-cache", "Accept: */*", $pingAuthHeader)
      );
      $ping = curl_init();
      curl_setopt_array($ping, $pingOpt);
      $result = curl_exec($ping);
      curl_close($ping);

      return $result;
    }
  }
