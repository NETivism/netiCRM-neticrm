/*
  Masked Input plugin for jQuery
  Copyright (c) 2007-2013 Josh Bush (digitalbush.com)
  Licensed under the MIT license (http://digitalbush.com/projects/masked-input-plugin/#license)
  Version: 1.3.1
*/
(function($) {
  function getPasteEvent() {
    var el = document.createElement('input'),
        name = 'onpaste';
    el.setAttribute(name, '');
    return (typeof el[name] === 'function')?'paste':'input';             
}

var pasteEventName = getPasteEvent() + ".amask",
  ua = navigator.userAgent,
  iPhone = /iphone/i.test(ua),
  android=/android/i.test(ua),
  caretTimeoutId;

$.amask = {
  //Predefined character definitions
  definitions: {
    '9': "[0-9]",
    'a': "[A-Za-z]",
    '*': "[A-Za-z0-9]"
  },
  dataName: "arawMaskFn",
  placeholder: '_',
};

$.fn.extend({
  //Helper Function for Caret positioning
  caret: function(begin, end) {
    var range;

    if (this.length === 0 || this.is(":hidden")) {
      return;
    }

    if (typeof begin == 'number') {
      end = (typeof end === 'number') ? end : begin;
      return this.each(function() {
        if (this.setSelectionRange) {
          this.setSelectionRange(begin, end);
        } else if (this.createTextRange) {
          range = this.createTextRange();
          range.collapse(true);
          range.moveEnd('character', end);
          range.moveStart('character', begin);
          range.select();
        }
      });
    } else {
      if (this[0].setSelectionRange) {
        begin = this[0].selectionStart;
        end = this[0].selectionEnd;
      } else if (document.selection && document.selection.createRange) {
        range = document.selection.createRange();
        begin = 0 - range.duplicate().moveStart('character', -100000);
        end = begin + range.text.length;
      }
      return { begin: begin, end: end };
    }
  },
  unmask: function() {
    return this.trigger("unmask");
  },
  amask: function(amask, settings) {
    var input,
      defs,
      tests,
      partialPosition,
      firstNonMaskPos,
      len,
      keyIsPress,
      isIME = false,
      imeValue;

    if (!amask && this.length > 0) {
      input = $(this[0]);
      return input.data($.amask.dataName)();
    }
    settings = $.extend({
      placeholder: $.amask.placeholder, // Load default placeholder
      completed: null
    }, settings);


    defs = $.amask.definitions;
    tests = [];
    partialPosition = len = amask.length;
    firstNonMaskPos = null;

    $.each(amask.split(""), function(i, c) {
      if (c == '?') {
        len--;
        partialPosition = i;
      } else if (defs[c]) {
        tests.push(new RegExp(defs[c]));
        if (firstNonMaskPos === null) {
          firstNonMaskPos = tests.length - 1;
        }
      } else {
        tests.push(null);
      }
    });

    return this.trigger("unmask").each(function() {
      var input = $(this),
        buffer = $.map(
        amask.split(""),
        function(c, i) {
          if (c != '?') {
            return defs[c] ? settings.placeholder : c;
          }
        }),
        focusText = input.val();

      function getKeyCode(str){
        var regex = new RegExp('\['+settings.placeholder +'\]*$');
        str = str.replace(regex, '');
        return str.charCodeAt(str.length - 1);
      }

      function seekNext(pos) {
        while (++pos < len && !tests[pos]);
        return pos;
      }

      function seekPrev(pos) {
        while (--pos >= 0 && !tests[pos]);
        return pos;
      }

      function shiftL(begin,end) {
        var i,
          j;

        if (begin<0) {
          return;
        }

        for (i = begin, j = seekNext(end); i < len; i++) {
          if (tests[i]) {
            if (j < len && tests[i].test(buffer[j])) {
              buffer[i] = buffer[j];
              buffer[j] = settings.placeholder;
            } else {
              break;
            }

            j = seekNext(j);
          }
        }
        writeBuffer();
        input.caret(Math.max(firstNonMaskPos, begin));
      }

      function shiftR(pos) {
        var i,
          c,
          j,
          t;

        for (i = pos, c = settings.placeholder; i < len; i++) {
          if (tests[i]) {
            j = seekNext(i);
            t = buffer[i];
            buffer[i] = c;
            if (j < len && tests[j].test(t)) {
              c = t;
            } else {
              break;
            }
          }
        }
      }

      function compositionStart(e) {
        isIME = true;
        // handling notify message of ime start
        var error = input.next('.error');
        if (!error.length) {
          var notifyLabel = '<label for="'+input.attr('id')+'" generated="true" class="error">'+Drupal.settings.jvalidate.imeNotify+'</label>';
          input.after(notifyLabel);
        }
        input.attr('title', Drupal.settings.jvalidate.imeNotify);
        imeValue = input.val();
      }

      function keydownEvent(e) {
        var k = e.which,
          pos,
          begin,
          end,
          keyIsPress = false;

        //backspace, delete, and escape get special treatment
        if (isIME) {
          e.preventDefault();
          return;
        }
        else if (k === 8 || k === 46 || (iPhone && k === 127)) {
          pos = input.caret();
          begin = pos.begin;
          end = pos.end;

          if (end - begin === 0) {
            begin=k!==46?seekPrev(begin):(end=seekNext(begin-1));
            end=k===46?seekNext(end):end;
          }
          clearBuffer(begin, end);
          shiftL(begin, end - 1);

          e.preventDefault();
        }
        else if (k == 27) {//escape
          input.val(focusText);
          input.caret(0, checkVal());
          e.preventDefault();
        }
      }

      function keypressEvent(e) {
        var k = e.which,
          pos,
          p,
          c,
          next;
        keyIsPress = true;

        if (isIME) {
          e.preventDefault();
          return;
        }
        else if (e.ctrlKey || e.altKey || e.metaKey || k < 32) {//Ignore
          e.preventDefault();
        }
        else if (k) {
          pos = input.caret();
          if (pos.end - pos.begin !== 0){
            clearBuffer(pos.begin, pos.end);
            shiftL(pos.begin, pos.end-1);
          }

          p = seekNext(pos.begin - 1);
          if (p < len) {
            c = String.fromCharCode(k);
            if (tests[p].test(c)) {
              shiftR(p);

              buffer[p] = c;
              writeBuffer();
              next = seekNext(p);

              if(android){
                setTimeout(function(){input.caret(next);}, 1);
              }else{
                input.caret(next);
              }

              if (settings.completed && next >= len) {
                settings.completed.call(input);
              }
            }
          }
          e.preventDefault();
        }
      }

      function keyupEvent(e) {
        var k = e.which,
          pos,
          p,
          c,
          next;
        if (isIME) {
          e.preventDefault();
          return;
        }
        if (keyIsPress) {
          keyIsPress = false;
          return;
        }

        if ((e.ctrlKey || e.altKey || e.metaKey || k < 32 || isIME) && !android) {//Ignore
          return;
        }
        else if (k) {
          pos = input.caret();
          pos.end--;
          pos.begin--;
          if (pos.end - pos.begin !== 0){
            clearBuffer(pos.begin, pos.end);
            shiftL(pos.begin, pos.end-1);
          }

          p = seekNext(pos.begin - 1);
          if (p < len) {
            if (android && (k === 229 || k === 0) ) {
              k = getKeyCode(input.val());
              c = String.fromCharCode(k);
            }
            else{
              c = String.fromCharCode(k-48);
            }

            if (tests[p].test(c)) {
              shiftR(p);

              buffer[p] = c;
              writeBuffer();
              next = seekNext(p);
              if(android){
                setTimeout(function(){input.caret(next);},0);
              }else{
                input.caret(next);
              }

              if (settings.completed && next >= len) {
                settings.completed.call(input);
              }
            }
          }
          e.preventDefault();
        }
        keyIsPress = false;
      }

      function compositionEnd(e) {
        isIME = false;
        setTimeout(function() { input.val(imeValue); }, 50);
        input.attr('title', '');
      }

      function clearBuffer(start, end) {
        var i;
        for (i = start; i < end && i < len; i++) {
          if (tests[i]) {
            buffer[i] = settings.placeholder;
          }
        }
      }

      function writeBuffer() { input.val(buffer.join('')); }

      function checkVal(allow) {
        //try to place characters where they belong
        var test = input.val(),
          lastMatch = -1,
          i,
          c;

        for (i = 0, pos = 0; i < len; i++) {
          if (tests[i]) {
            buffer[i] = settings.placeholder;
            while (pos++ < test.length) {
              c = test.charAt(pos - 1);
              if (tests[i].test(c)) {
                buffer[i] = c;
                lastMatch = i;
                break;
              }
            }
            if (pos > test.length) {
              break;
            }
          } else if (buffer[i] === test.charAt(pos) && i !== partialPosition) {
            pos++;
            lastMatch = i;
          }
        }
        if (allow) {
          writeBuffer();
        } else if (lastMatch + 1 < partialPosition) {
          input.val("");
          clearBuffer(0, len);
        } else {
          writeBuffer();
          input.val(input.val().substring(0, lastMatch + 1));
        }
        return (partialPosition ? i : firstNonMaskPos);
      }

      input.data($.amask.dataName,function(){
        return $.map(buffer, function(c, i) {
          return tests[i]&&c!=settings.placeholder ? c : null;
        }).join('');
      });

      if (!input.attr("readonly"))
        input
        .one("unmask", function() {
          input
            .unbind(".amask")
            .removeData($.amask.dataName);
        })
        .bind("focus.amask", function() {
          clearTimeout(caretTimeoutId);
          var pos,
            moveCaret;

          focusText = input.val();
          pos = checkVal();
          
          caretTimeoutId = setTimeout(function(){
            writeBuffer();
            if (pos == amask.length) {
              input.caret(0, pos);
            } else {
              input.caret(pos);
            }
          }, 10);
        })
        .bind("blur.amask", function() {
          checkVal();
          if (input.val() != focusText)
            input.change();
        })
        .on("compositionstart", compositionStart)
        .on("compositionend", compositionEnd)
        .bind("keydown.amask", keydownEvent)
        .bind("keypress.amask", keypressEvent)
        .bind("keyup.amask", keyupEvent)
        .bind(pasteEventName, function() {
          setTimeout(function() { 
            var pos=checkVal(true);
            input.caret(pos); 
            if (settings.completed && pos == input.val().length)
              settings.completed.call(input);
          }, 0);
        });
      checkVal(); //Perform initial check for existing values
    });
  }
});


})(jQuery);
