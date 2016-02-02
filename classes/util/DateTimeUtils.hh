<?hh //strict
  namespace nova\util;

  class DateTimeUtils {
    public static function getFancyTime(int $unixTimeStamp, int $nowUnixTimeStamp = 0) : string {
      if($nowUnixTimeStamp === 0) $nowUnixTimeStamp = time();
      $gap = $nowUnixTimeStamp - $unixTimeStamp;

      if($gap < 600) return __t("방금");
      else if($gap < 3600) return __t("%1분 전",floor($gap/60));
      else if($gap < 86400) return __t("%1시간 전",floor($gap/3600));
      else if($gap < 172800) return __t("어제");
      else return date("y-m-d H:i",$unixTimeStamp);
    }

    public static function getDetailTime(int $unixTimeStamp) : string {
      return date(__t("Y년 m월 d일, H시 i분 s초"),$unixTimeStamp);
    }
  }
