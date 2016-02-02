<?hh //strict
namespace nova\collection;

class ArrayUtils {


}

/*

function array2sql($ar)
{
  if(!sizeof($ar)) return false;

  $ret_str = "";
  $ret_str .= "(";
  foreach($ar AS $vv)
  {
    $ret_str .= "'$vv',";
  }
  $ret_str = substr($ret_str,0,-1);
  $ret_str .= ")";

  return $ret_str;
}

function array2xml($ar)
{
  foreach($ar AS $k=>$v)
  {
    if(is_int($k)) $elname = "item";
    else $elname = $k;
    if(is_array($v)) $result .= "<$elname>\n".array2xml($v)."</$elname>\n";
    else $result .= "<$elname>$v</$elname>\n";
  }
  return $result;
}

function array2phpstr($ar,$sub=0)
{
  if(!sizeof($ar)) return null;

  $str = "Array(";
  $step_cnt=0;
  foreach($ar as $v)
  {
    $step_cnt++;
    if(is_array($v))
    {
      $str .= array2phpstr($v,1);
    }
    else
    {
      $v = addslashes($v);
      $str .= "\"$v\",";
    }
  }
  if($step_cnt) $str = substr($str,0,-1);

  if($sub) $str .= "),\n";
  else
  {
    if($step_cnt) $str = substr($str,0,-1);
    $str .= ");\n";
  }

  return $str;
}

//전체 배열중 fields에 요청된 값만 남겨둔다
//fields: id,name
function array_key_filter(&$arr,$fields)
{
  if($fields=="*") return;

  $fields_arr = explode(",",$fields);
  foreach($fields_arr AS $k=>$v) $fields_arr[$k] = trim($fields_arr[$k]);
  foreach($arr AS $k=>$v)
  {
    if(!in_array($k,$fields_arr))
    {
      unset($arr[$k]);
    }
  }
}

function array_trim($arr)
{
  foreach($arr AS $k=>$v)
  {
    if(is_array($v))
    {
      array_trim($v);
    }
    else
    {
      $arr[$k] = trim($v);
    }
  }
    return $arr;
}

//값이없는 배열을 제거한다.
function array_null_clear($ar)
{
  $result = Array();
  $new_pos = 0;
  foreach($ar AS $k=>$v)
  {
    if($v==null||$v=="") continue;
    $result[$new_pos]=$v;
    $new_pos++;
  }
  return $result;
}

//size 변수로 지정된 수를 넘는 내용을 삭제한다.
function array_cut($ar,$size=20)
{

  if(sizeof($ar)<=$size)
  {
    return $ar;
  }
  else
  {
    $result_ar = Array();
    foreach($ar as $v)
    {
      $now++;
      if($now>$size) break;

      array_push($result_ar,$v);
    }

    return $result_ar;
  }
}
*/
