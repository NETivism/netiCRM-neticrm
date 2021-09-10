(function($){
  $(document).ready(function(){
    var lang = drupalSettings.path.currentLanguage;
    var skiptwcheck = typeof(drupalSettings.skiptwcheck) == 'undefined' ? 0 : 1;
    var path = location.pathname;
    var documentURL = new URLSearchParams(document.URL);
    var action = documentURL.get('action') ? 'update' : 'add';
    var admin = path.match('civicrm/contact/add') ? 1 : 0;
    var is_event = path.match('civicrm/event/register') ? 1 : 0;

    if(admin){
      $("form input.form-submit").addClass('cancel');
    }
    if(is_event){
      $("form input.form-submit[name$='next_skip']").addClass('cancel');
      $("form input.form-submit[name$='back']").addClass('cancel');
    }

    $("#crm-container form, form#user-profile-form, form#user-register-form").each(function(){
      $(".crm-section .label .crm-marker").each(function(){
        if($(this).text() == "*") {
          var inputs = $(this).parents(".crm-section:first").find(":input:first:visible:not([type=checkbox])");
          inputs.addClass("required");

          var select = $(this).parents(".crm-section:first").find("select:visible");
          select.addClass("required");

          var checkboxes = $(this).parents(".crm-section:first").find(":input[type=checkbox]:visible:not(.ignore-required)");
          checkboxes.parents("div.content:first").addClass("ckbox");

          var advselect = $(this).parents(".crm-section:first").find(".advmultiselect select[multiple]");
          advselect.removeClass("required");

          var file = $(this).parents(".crm-section:first").find(":input[type='file']");
          file.each(function(i, e) {
            var $e = $(e);
            // For custom fields
            if( $e.closest('.crm-section').next().find('img').length) {
              $e.removeClass("required");
            }
            if( $e.closest('.crm-section').next().find('a').length) {
              $e.removeClass("required");
            }
            // for contact image
            if ($e.closest('.crm-section').find('.crm-contact_image-block img').length) {
              $e.removeClass("required");
            }
          });
        }
      });
      if($(this).attr("id")){
        var formid = $(this).attr("id");
        $("#"+formid).validate({
          errorPlacement: function (error, element) {
            if(admin){
              error.css({"color":"#E55","padding-left":"10px","display":"block"});
              error.appendTo($(element).parent());
            }
            else if (element.is(":radio")) {
              var $c = element.closest('.content');
              $c.find("label.error").remove();
              error.css({"color":"#E55","padding-left":"10px","display":"block"});
              $c.prepend(error);
            }
            else if (element.is(":checkbox")) {
              var $c = element.closest('.content');
              $c.find("label.error").remove();
              error.css({"color":"#E55","padding-left":"10px","display":"block"});
              $c.prepend(error);
            }
            else {
              error.css({"color":"#E55","padding-left":"10px"});
              error.insertAfter(element);
            }
          }
        });
        $("#"+formid+" input[name*=email]").each(function(){
          $(this).rules("add", {required:false,email:true});
        });

        // add further validate when dynamic adding new element
        if(admin){
          $("#addEmail,#addPhone").click(function(){
            setTimeout(function(){
              $("#"+formid+" input[name*=email]:not(#email_1_email)").each(function(){
                $(this).rules("add", {required:false,email:true});
              });
              $("#"+formid+" input[name$='[phone]']:not(#phone_1_phone)").each(function(){
                $.amask.phone_add_validate(this, admin);
              });
            },1200);
          });
        }
        // only validate required when not in contact adding
        else{
          $("#"+formid+" input.required:visible:not([type=checkbox]):not(.ignore-required)").each(function(){
            $(this).rules("add", {required:true });
          });

          var $ckbox = $("#"+formid+" div.ckbox");
          $ckbox.each(function(){
            $(this).find("input:not(.ignore-required)").each(function(){
              $(this).rules("add", 'ckbox');
            });
          });
          $ckbox.find("input:checkbox").click(function(){
            var $p = $(this).parents("div.ckbox:first");
            $p.find("label.error").remove();
            $(this).valid();
          });
          $("#"+formid+" input[name*=url]").each(function(){
            $(this).rules("add", {required:false,url:true});
          });
        }

        // only add validate when language is chinese.
        if(lang == 'zh-hant' && !skiptwcheck){
          // twid
          $("#"+formid+" input[name*=legal_identifier]").each(function(){
            $.amask.id_add_validate(this);
          });

          // phone
          if(admin){
            $("#"+formid+" input[name$='[phone]']").each(function(){
              $.amask.phone_add_validate(this, admin);
            });
          }
          else{
            $("#"+formid+" input[name*=phone]").each(function(){
              $.amask.phone_add_validate(this, admin);
            });
          }
        }
      }
    });
  });
})(jQuery);
