var ui = ui || function() {
  /**
   * wrapWidth 안에 width x height 크기의 엘리멘트가 잘 표시될 수 있는 크기를 계산한다.
   * 엘리멘트의 가로/세로 비율이 유지된다.
   *
   * @param  {[type]} width     [description]
   * @param  {[type]} height    [description]
   * @param  {[type]} wrapWidth [description]
   * @param  {bool}   fitWrap   wrapWidth보다 엘리멘트의 가로 크기가 작을 경우 wrapWidth 넓이로 강제 확대한다.
   * @param  {[type]} maxRatio  가로 넓이 대비 세로 길이의 비율이 maxRatio 이상이면 세로 길이를 width*maxRatio로 변경한다
   */
  var fitSize = function(width, height, wrapWidth, fitWrap, maxRatio) {
    var destWidth;
    var destHeight;
    var zoom;
    var ratio;

    if(width >= wrapWidth) {
      destWidth = wrapWidth;
      zoom = wrapWidth/width;
    } else { // width < wrapWidth
      if(fitWrap) {
        destWidth = wrapWidth;
        zoom = wrapWidth/width;
      } else {
        destWidth = width;
        zoom = 1;
      }
    }
    destHeight = Math.round(height*zoom);
    ratio = destHeight/destWidth;

    if(!isundef(maxRatio)) {
      if(ratio>maxRatio) {
        destHeight = destWidth*maxRatio;
      }
    }

    return {
      width : destWidth,
      height : destHeight,
      ratio : ratio
    };
  };

  return {
    fitSize : fitSize
  };
} ();
