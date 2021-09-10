(function($){
  $.amask.definitions['~']='[1-9]';
  $.amask.definitions['o']='[0]';
  $.amask.definitions['z']='[9]';
  $.amask.definitions['A']='[A-Z]';
  $.amask.definitions['#']='[0-9#]';

  $.amask.phone_add_validate = function(obj, admin){
    var mobile_mask = function(obj){
      var fid = $(obj).attr("id");
      $("span[rel="+fid+"]").remove();
      $(obj).css("max-width", "280px")

      // check format first, if correct, apply mask
      var validatedDefault = false;
      // refs #31241, this regex /^09-*\d{2}-*?\d{6}_*$/ is tested when switch between phone type
      if ($(obj).val() && ($(obj).val().match(/^09-*\d{2}-*?\d{6}_*$/) || $(obj).val().match(/^[_]*-[_]*$/) )) {
        var validatedDefault = true;
      }
      if (!$(obj).val() || validatedDefault) {
        if (drupalSettings.jvalidate.phoneValidator) {
          $(obj).rules("add", drupalSettings.jvalidate.phoneValidator);
        }
        else{
          $(obj).rules("add", "twphone");
        }
        if (drupalSettings.jvalidate.mobileMask) {
          $(obj).amask(drupalSettings.jvalidate.mobileMask);
        }
        else{
          $(obj).amask("oz99-999999");
        }
      }
    }
    var phone_mask = function(obj, force){
      $(obj).css("max-width", "280px")
      var validatedDefault = false;
      if ($(obj).val() && ( $(obj).val().match(/^0\d{1}-?\d+#?\d*$/) || $(obj).val().match(/^[_]*-[_]*$/) )) {
        var validatedDefault = true;
      }
      if (!$(obj).val() || validatedDefault || force) {
        if (drupalSettings.jvalidate.phoneValidator) {
          $(obj).rules("add", drupalSettings.jvalidate.phoneValidator);
        }
        else {
          $(obj).rules("add", "twphone");
        }

        if (drupalSettings.jvalidate.phoneMask) {
          $(obj).amask(drupalSettings.jvalidate.phoneMask);
        }
        else{
          $(obj).amask("o~-9999999?##########");
        }
      }
      // add phone ext box.
      var fid = $(obj).attr("id");
      $("span[rel="+fid+"]").remove();
      $('<span href="#" class="extend" rel="'+fid+'"> +'+drupalSettings.jvalidate.ext+'</span>').insertAfter(obj);
      $("span[rel='"+fid+"']").css({cursor:"pointer",color:"green"});
      $("span[rel='"+fid+"']").click(function(){
        var ext = prompt(drupalSettings.jvalidate.extprompt);
        if(ext != null && ext != ""){
          var f = '#'+$(this).attr("rel");
          var v = $(f).val().replace(/#.*/, '');
          $(f).val(v+'#'+ext);
        }
      });
    }

    var mobile = false;
    var phone = false;
    if(admin){
      var $p = $(obj).parents("tr:first");
      var type_id = Number($p.find("select[name*='phone_type_id']").val());
      if(type_id == 2){
        mobile = true;
      }
      if(type_id == 1 || type_id == 3){
        phone = true;
      }
    }
    else{
      var n = $(obj).attr('name');
      var re = /phone-(\w+)-(\d+)/g;
      var match = re.exec(n);
      if(match != null){
        var idx = match.length - 1;
        if(match[idx] == '2'){
          mobile = true;
        }
        if(match[idx] == '1' || match[idx] == '3'){
          phone = true;
        }
      }
    }
    if(mobile){
      mobile_mask(obj);
    }
    else if(phone){
      phone_mask(obj);
    }

    if (admin) {
      var refreshMask = function(obj, type_id, force) {
        $(obj).next('.error').remove();
        $(obj).unbind(".amask");
        $(obj).unbind("input");

        if(type_id == 2){
          mobile_mask(obj);
        }
        else if(type_id == 1 || type_id == 3){
          phone_mask(obj, force);
        }
        else{
          $(obj).rules('remove');
          $(obj).unmask();
          var fid = $(obj).attr("id");
          $("span[rel="+fid+"]").remove();
        }
      }

      // phone type change
      $(".contact_information-section").on("change", "select[name*='phone_type_id']", function(){
        var type_id = Number($(this).val());
        $(this).closest('tr').find("input[name$='[phone]']").each(function(){
          refreshMask(this, type_id);
        });
      });
      
      // phone input paste
      $(".contact_information-section").on("focus", "input[name$='[phone]']", function(){
        var type_id = Number($(this).closest('tr').find("select[name*='phone_type_id']").val());
        refreshMask(this, type_id);
      });
    }
    else {
      $("select[name*='phone_type_id']").on("change", function(){
        var type_id = Number($(this).val());
        $(this).parents('tr:first').find("input[name$='[phone]']").each(function(){
          $(this).next('.error').remove();
          if(type_id == 2){
            mobile_mask(this);
          }
          else if(type_id == 1 || type_id == 3){
            phone_mask(this);
          }else{
            $(this).rules('remove');
            $(this).unmask();
            var fid = $(this).attr("id");
            $("span[rel="+fid+"]").remove();
          }
        });
      });
    }
  }

  $.amask.id_add_validate = function(obj){
    var validatedDefault = false;
    if ($(obj).val() && $(obj).val().match(/^[a-zA-Z]\w\d{8}$/)) {
      var validatedDefault = true;
    }
    if(!$(obj).val() || validatedDefault){
      $(obj).rules("add", "twid");
      $(obj).amask("a*99999999", {completed:function(){ obj.value = obj.value.toUpperCase(); }});
    }

    // add id validate remove rule.
    var fid = $(obj).attr("id");
    $("span[rel="+fid+"]").remove();
    $('<span href="#" class="valid-id" rel="'+fid+'"> '+drupalSettings.jvalidate.notw+'</span>').insertAfter(obj);
    $("span[rel='"+fid+"']").css({cursor:"pointer",color:"green"});
    $("span[rel='"+fid+"']").click(function(){
      var notw = prompt(drupalSettings.jvalidate.notwprompt);
      if(notw != null && notw != ""){
        $(obj).rules("remove", "twid");
        $(obj).unmask();
        $(obj).val(notw);
        $(obj).removeClass('error');
        $(obj).parent().find('.error').hide();
        $(obj).click(function(){
          $(obj).rules("add", "twid");
          $(obj).amask("a*99999999", {completed:function(){ obj.value = obj.value.toUpperCase(); }});
        })
      }
    });
  }
})(jQuery);