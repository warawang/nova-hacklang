<?hh //strict
namespace nova\view;

use nova\util\Connection;
use nova\util\Config;

class ViewUtils {
  public static function getViewFolder(?string $folderName = NULL) : string {
    if($folderName === NULL) {
      $pathInfo = pathinfo(Connection::getScriptFileName());
      $path = str_replace("/public_html/","/view/",$pathInfo["dirname"]);
    } else {
      $path = str_replace("/public_html/","/view/",Connection::getDocumentRoot())."/$folderName";
    }
    
    return $path;
  }

  public static function getLayoutFolder(string $layoutName) : string {
    $path = str_replace("/public_html/","/layout/",Connection::getDocumentRoot());
    return $path."/".$layoutName;
  }
}
