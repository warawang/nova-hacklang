// placeholder.
// IE 7,8,9 PLACEHOLDER 지원
var placeholder = placeholder || function() {

  var init = function() {
    if(Modernizr.input.placeholder) return;

    $(document).bind("add.placeholder", function() {
      setElements();
    });
    setElements();

    if(isundef($(document).data("placeholder-init", true))) {
      $(document).on("focus", "input[placeholder], textarea[placeholder]", function(e) {
        var $e = $(e.target);
        if($e.val() == $e.attr('placeholder')){
          $e.val('');
        }
        $e.removeClass("placeholder");
      });

      $(document).on("blur", "input[placeholder], textarea[placeholder]", function(e) {
        var $e = $(e.target);
        if($e.val() === '' || $e.val() == $e.attr('placeholder')){
            $e.val($e.attr('placeholder'));
            $e.addClass("placeholder");
        }
      });
    }
  };

  var setElements = function() {
    $('input[placeholder], textarea[placeholder]').each(function(){
      var $element = $(this);
      if($element.val() === '') {
        $element.val($element.attr('placeholder'));
        $element.addClass("placeholder");

      }
    });
  };

  return {
    init : init
  };
} ();

$(document).bind("ready.placeholder", function() {
  placeholder.init();
});

var form = form || function () {
  var init = function($f) {
    $f.on("focus focusout", 'input[type="text"], textarea', function(event) {
      var $e = $(event.target);
      var $item = $e.parents(".item");

      $item.toggleClass("selected");
    });


    // 같은 ID와 Label 이 복수개 존재할 경우 대응.
    // input[type="text"] 와 textarea 만 지원
    $f.on("click", "label", function(event) {
      var $e = $(event.target);
      var $item = $e.closest(".item");

      if($item.length>0) {
        var $field = $item.find('input[type="text"], textarea');
        if($field.length>0) {
          $field.focus();
        }

        event.preventDefault();
      }
    });

    //핸들 이벤트
    $f.on("click", ".handle", function(event) {
      var $el = $(this);

      if($el.hasClass("unfold")) {
        // unfold 버튼일 경우
        var $item = $el.parent().find("."+$el.attr("data-target-item"));
        $item.removeClass("hidden");
        $item.find("input, textarea").focus();
        $el.addClass("hidden");
      }


      event.preventDefault();
    });

    //최대 입력가능 길이가 지정되어 있을경우 정보 업데이트
    $f.find(".item input[type='text'], .item textarea").each(function(i, el) {
      var $el = $(el);
      if(!isundef($el.attr("data-max-length"))) {
        $el.closest(".item").find(".istatus .counter .max").html($el.attr("data-max-length"));
      }
    });

    $f.on("change keydown keyup focus focusout paste", 'input[type="text"], textarea', function(event) {
      var $e = $(event.target);
      var $item = $e.closest(".item");
      $item.find(".counter .now").html($e.val().length);

      //validation
      var maxLength = $e.attr("data-max-length");
      if(typeof maxLength === undefined) return;

      if(maxLength < $e.val().length) {
        $item.addClass("error over-length");
        $item.find(".istatus .msg").html($item.attr("data-msg-over-length"));
      } else {
        $item.removeClass("error over-length");
      }

      //textarea
      if($e.is("textarea")) {
        //nobr
        if($e.attr("data-nobr") == "true") {
          if(event.keyCode == 13) {
            event.preventDefault();
          }

          //복붙으로 시도시
          if(event.type == "paste") {
            setTimeout(function() {
              $e.val($e.val().replace(/(?:\r\n|\r|\n)/g,""));
              $e.trigger("change");
            },100);
          }
        }

        //autosize
        if($e.attr("data-autosize") == "true") {
          $e.css("height","auto").css("height",$e.prop("scrollHeight")-parseInt($e.css("paddingTop"))-parseInt($e.css("paddingBottom")));
        }
      }
    });
  };

  return {
    init : init
  };
} ();
