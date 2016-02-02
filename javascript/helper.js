/**
 * 텍스트 랩핑
 * @param  {[type]} str [description]
 * @return {[type]}     [description]
 */
function __t(str) {
  return str;
}

/**
 * UNIX 타임 스탬프
 * @return {[type]} [description]
 */
function time() {
  return Math.floor(+new Date()/1000);
}

/**
 * 이 변수가 undefined 인지 확인한다
 * @param  {[type]} val [description]
 * @return {[type]}     [description]
 */
function isundef(val) {
  if(val === undefined || typeof val === undefined) return true;
  else return false;
}

/**
 * 쿠키 컨트롤. 값이 없을 경우 null  을 리턴한다.
 * @return {[type]} [description]
 */
var cookie = cookie || function() {
  var get = function(key) {
    var start = document.cookie.indexOf(key + "=");
  	var len = start + key.length + 1;
  	if ((!start) && (key != document.cookie.substring(0, key.length))) return null;
  	if (start == -1) return null;

  	var end = document.cookie.indexOf(";", len);
  	if (end == -1) end = document.cookie.length;

  	return window.unescape(document.cookie.substring(len, end));
  };

  var set = function(key, value, expires, path, domain, secure) {
    var today = new Date();
  	today.setTime(today.getTime());

  	if(expires) expires = expires * 1000;
  	var expires_date = new Date(today.getTime() + (expires));

  	document.cookie = key + "=" +window.escape(value) +
  	( ( expires ) ? ";expires=" + expires_date.toGMTString() : "" ) +
  	( ( path ) ? ";path=" + path : "" ) +
  	( ( domain ) ? ";domain=" + domain : "" ) +
  	( ( secure ) ? ";secure" : "" );
  };

  var del = function(key) {
    set(key,null);
  };

  return {
    get : get,
    set : set,
    del : del
  };

} ();
