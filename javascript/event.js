/**
 * 페이지의 모든 컨텐츠를 스크롤로 확인하였을 경우 호출되는 이벤트를 생성한다.
 * @param  {[type]} window [description]
 * @return {[type]}        [description]
 */
$(window).bind("scroll.allread", function(){
  //전체 목록 열람 여부 체크
  //st = document.body.scrollTop ? document.body.scrollTop : document.documentElement.scrollTop;
  var st = $(this).scrollTop();
  //if(st > $(document).height()-$(window).height()-(Math.round($(window).height()/3))) {
  if(st > $(document).height()-$(window).height()-700) {
    $(document).trigger("allread");
  }
});

/**
 * 문서 로딩이 완료된 후 스크롤 위치가 페이지 최 하단에 있을 경우를 대비하여 scroll 이벤트를 강제 발생시켜
 * allread 이벤트 발생 여부 판단.
 * @param  {[type]} document [description]
 * @return {[type]}          [description]
 */
$(document).bind("ready.allread", function() {
  $(window).trigger("scroll.allread");
});

/**
 * 윈도우 리사이즈가 종료될 경우 한번만 호출되는 이벤트를 생성한다
 * @param  {[type]} window [description]
 * @return {[type]}        [description]
 */
var resizeHandle;
$(window).bind("resize.resizeEnd", function() {
  if(!isundef(resizeHandle)) {
    clearTimeout(resizeHandle);
  }

  resizeHandle = setTimeout(function() {
    $(window).trigger("resizeEnd");
  },250);
});
