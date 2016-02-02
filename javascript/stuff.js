/**
 * 문자열이 URL인지 판단한다
 * @return {[bool]} true : URL / false : NoURL
 */
String.prototype.isURL = function() {
  var exp = /(ftp|http|https):\/\/(\w+:{0,1}\w*@)?(\S+)(:[0-9]+)?(\/|\/([\w#!:.?+=&%@!\-\/]))?/i;
  if(exp.test(this)) return true;
  return false;
};

var Video = function(){};
Video.prototype.download = function(url, oncomplete, onprogress) {
  var thisVideo = this;
  thisVideo.completedPercentage = 0;

  //이미 로딩된 비디오라면
  if(!isundef(imageBlobMap[url])) {
    if(!isundef(onprogress)) {
      onprogress(100);
    }

    if(!isundef(oncomplete)) {
      oncomplete(imageBlobMap[url]);
    }

    return;
  }

  if(Modernizr.cors && Modernizr.bloburls && Modernizr.xhrresponsetypeblob) { //IE10 이상
    var xmlHTTP = new XMLHttpRequest();
    xmlHTTP.onload = function() {
      if(this.status == 200) {
        var blob = new Blob([this.response], {"type" : "video/mp4"});
        if(!isundef(oncomplete)) {
          imageBlobMap[url] = window.URL.createObjectURL(blob);
          oncomplete(imageBlobMap[url]);
        }
      } else {
        if(!isundef(oncomplete)) {
          oncomplete();
        }
      }
    };
    xmlHTTP.onprogress = function(e) {
      thisVideo.completedPercentage = parseInt((e.loaded / e.total) * 100);
      if(!isundef(onprogress)) {
        onprogress(thisVideo.completedPercentage);
      }
    };
    xmlHTTP.onloadstart = function() {
      thisVideo.completedPercentage = 0;
    };

    xmlHTTP.onerror = function() {
      if(!isundef(oncomplete)) {
        oncomplete();
      }
    };
    xmlHTTP.open('GET', url, true);
    xmlHTTP.responseType = 'blob';
    xmlHTTP.setRequestHeader('Access-Control-Allow-Origin', '*');
    xmlHTTP.send();
  } else { // Not support.
    onprogress(100);
    oncomplete(url);
  }
};

Video.prototype.completedPercentage = 0;
Video.prototype.intervalHandler = 0;
Video.prototype.clearDownloadInterval = function() {
  clearInterval(this.intervalHandler);
  this.intervalHandler = 0;
};



/**
 * 이미지를 다운로드한다
 * 이미지 프리로딩을 위해 주로 사용하며, CORS 및 bloburls가 지원되지 않는 브라우저의 경우
 * Fake progress 정보를 제공한다.
 *
 * @param  {[type]} url [description]
 * @return {[type]}     [description]
 */
var imageBlobMap = {};
Image.prototype.download = function(url, oncomplete, onprogress) {
  var thisImg = this;
  thisImg.completedPercentage = 0;

  //이미 로딩된 이미지라면.
  if(!isundef(imageBlobMap[url])) {
    if(!isundef(onprogress)) {
      onprogress(100);
    }

    if(!isundef(oncomplete)) {
      oncomplete(imageBlobMap[url]);
    }

    return;
  }

  if(Modernizr.cors && Modernizr.bloburls && Modernizr.xhrresponsetypearraybuffer) { //IE10 이상
    var xmlHTTP = new XMLHttpRequest();
    xmlHTTP.onload = function() {
      if(this.status == 200) {
        var blob = new Blob([this.response]);
        if(!isundef(oncomplete)) {
          imageBlobMap[url] = window.URL.createObjectURL(blob);
          oncomplete(imageBlobMap[url]);
        }
      } else {
        if(!isundef(oncomplete)) {
          oncomplete();
        }
      }
    };
    xmlHTTP.onprogress = function(e) {
      thisImg.completedPercentage = parseInt((e.loaded / e.total) * 100);
      if(!isundef(onprogress)) {
        onprogress(thisImg.completedPercentage);
      }
    };
    xmlHTTP.onloadstart = function() {
      thisImg.completedPercentage = 0;
    };

    xmlHTTP.onerror = function() {
      if(!isundef(oncomplete)) {
        oncomplete();
      }
    };
    xmlHTTP.open('GET', url, true);
    xmlHTTP.responseType = 'arraybuffer';
    xmlHTTP.setRequestHeader('Access-Control-Allow-Origin', '*');
    xmlHTTP.send();
  } else { // Fake.
    this.onload = function() {
      thisImg.clearDownloadInterval();
      thisImg.completedPercentage = 100;
      onprogress(thisImg.completedPercentage);

      if(!isundef(oncomplete)) {
        oncomplete(url);
      }
    };
    this.onerror = function() {
      thisImg.clearDownloadInterval();
      thisImg.completedPercentage = 100;
      onprogress(thisImg.completedPercentage);

      if(!isundef(oncomplete)) {
        oncomplete();
      }
    };

    if(this.intervalHandler !== 0) {
      thisImg.clearDownloadInterval();
    }

    this.intervalHandler = setInterval(function() {
      thisImg.completedPercentage += Math.floor((Math.random() * 10) + 3);
      if(thisImg.completedPercentage>80) {
        thisImg.clearDownloadInterval();
      }

      if(!isundef(onprogress)) {
        onprogress(thisImg.completedPercentage);
      }
    },500);

    this.src = url;
  }
};

Image.prototype.completedPercentage = 0;
Image.prototype.intervalHandler = 0;
Image.prototype.clearDownloadInterval = function() {
  clearInterval(this.intervalHandler);
  this.intervalHandler = 0;
};

/**
 * TODO
 * target 이 both이면 양쪽 다 적용
 */

jQuery.fn.blindDown = function(duration, callback) {
  $(this).blind("show","height",duration, callback);
};

jQuery.fn.blindUp = function(duration, callback) {
  $(this).blind("hide","height",duration, callback);
};

jQuery.fn.blind = function(mode, target, durarion, callback){
  var $e, fromSize, toSize, pro;
  return this.each(function(i,e){
    $e = $(e);

    //기본 속도
    if(!isundef(durarion)) durarion = 400;

    //설정된 사이즈가 없으면, 실제 표시될 사이즈를 계산하여 적용
    if(mode === "show") {
      if(e.style[target].length<=0 || e.style[target] === "auto") {
        toSize = $e.autoSize(target);
      } else {
        toSize = $e.css(target);
      }
      $e.show().css(target,"0px");
    } else { //hide
      fromSize = $e.css(target);
      toSize = "0px";
    }

    pro = {};
    if(target == "height") pro.height = toSize;
    else pro.width = toSize;

    $e.animate(pro, durarion, function() {
      if(mode === "hide") {
        $e.hide().css(target,fromSize);
      }
      if($.isFunction(callback)) callback();
    });
  });
};

/**
 * Height, Width 가 지정되지 않은 엘리멘트의 표시될 사이즈를 계산한다.
 *
 * TODO
 * target 없이 요청하면 width, height를 배열로 리턴하도록 처리
 */
jQuery.fn.autoSize = function(target) {
  var $e, org, ret;
  $e = $(this);
  org = $e.css(target);
  $e.css(target,"auto");
  ret = $e.css(target);
  $e.css(target,org);
  return ret;
};
