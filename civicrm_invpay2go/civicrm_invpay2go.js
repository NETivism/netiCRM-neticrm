(function ($) {
$(document).ready(function(){
  var showEle = function(obj) {
    if (obj.length) {
      var section = obj.closest('.crm-section');
      if (!section.length) {
        section = obj.closest('tr.custom_field-row');
      }
      var label = section.find('.label label');
      var marker = label.find('.crm-marker');
      if (!marker.length) {
        label.append('<span class="crm-marker">*</span>');
      }
      if (obj.prop('tagName').toLowerCase() == 'select')  {
        var choosed = 0;
        obj.find('option').each(function() {
          if (!$(this).val()) {
            $(this).hide();
          }
          else if(!choosed) {
            $(obj).val($(this).val());
            choosed = 1;
          }
        });
      }
      section.show();
      if (typeof obj.rules !== 'undefined') {
        obj.rules('add', {required: true});
      }
    }
  }
  var hideEle = function(obj) {
    if (obj.length) {
      var section = obj.closest('.crm-section');
      if (!section.length) {
        section = obj.closest('tr.custom_field-row');
      }
      section.hide();
      if(obj.prop('type') === 'radio') {
        obj.prop('checked', false);
      }
      else if (obj.prop('tagName').toLowerCase() == 'select')  {
        obj.find('option').show();
        $(obj).val('');
      }
      else {
        // obj.val('');
        // prevent clear user input data
      }
      if (typeof obj.rules !== 'undefined') {
        obj.rules('remove');
      }
    }
  }

  var triggerDeviceType = function() {
    var $taxReceiptDeviceType = $('input[data-invpay2go=taxReceiptDeviceType]:checked');
    if ($taxReceiptDeviceType.length) {
      var $desc = $('input[data-invpay2go=taxReceiptDeviceNumber]').closest('.crm-section').find('.description');
      switch ($taxReceiptDeviceType.val()) {
        case '0':
          showEle($('input[data-invpay2go=taxReceiptDeviceNumber]'));
          $('input[data-invpay2go=taxReceiptDeviceNumber]').prop('placeholder', '/1234567');
          $desc.html('總長度為8碼字元，第一碼必為『/』，在此<a href="https://www.einvoice.nat.gov.tw/APMEMBERVAN/GeneralCarrier/generalCarrier" target="_blank">申請條碼</a>');
          break;
        case '1':
          showEle($('input[data-invpay2go=taxReceiptDeviceNumber]'));
          $('input[data-invpay2go=taxReceiptDeviceNumber]').prop('placeholder', 'AA00000000000000');
          $desc.html('總長度為16碼字元，前兩碼為大寫英文，後14碼為數字0-9');
          break;
        case '2':
          hideEle($('input[data-invpay2go=taxReceiptDeviceNumber]'));
          $desc.html('');
          break;
      }
    }
    else {
      hideEle($('input[data-invpay2go=taxReceiptDeviceNumber]'));
    }
  }
  var triggerReceiptType = function(){
    var $taxReceiptType = $('input[data-invpay2go=taxReceiptType]:checked');
    if ($taxReceiptType.length) {
      switch ($taxReceiptType.val()) {
        case 'elec':
          hideEle($('[data-invpay2go=taxReceiptDonate]'));
          hideEle($('[data-invpay2go=taxReceiptAgree]'));
          hideEle($('input[data-invpay2go=taxReceiptSerial]'));
          hideEle($('input[data-invpay2go=taxReceiptTitle]'));
          showEle($('input[data-invpay2go=taxReceiptDeviceType]'));
          break;
        case 'donate':
          hideEle($('input[data-invpay2go=taxReceiptSerial]'));
          hideEle($('input[data-invpay2go=taxReceiptTitle]'));
          hideEle($('input[data-invpay2go=taxReceiptDeviceType]'));
          hideEle($('[data-invpay2go=taxReceiptAgree]'));
          showEle($('[data-invpay2go=taxReceiptDonate]'));
          triggerDeviceType();
          break;
        case 'company':
          hideEle($('input[data-invpay2go=taxReceiptDeviceType]'));
          hideEle($('[data-invpay2go=taxReceiptDonate]'));
          showEle($('[data-invpay2go=taxReceiptSerial]'));
          showEle($('[data-invpay2go=taxReceiptAgree]'));
          showEle($('[data-invpay2go=taxReceiptTitle]'));
          break;
      }
    }
    else {
      hideEle($('[data-invpay2go=taxReceiptDonate]'));
      hideEle($('[data-invpay2go=taxReceiptAgree]'));
      hideEle($('input[data-invpay2go=taxReceiptDeviceType]'));
      hideEle($('input[data-invpay2go=taxReceiptDeviceNumber]'));
      hideEle($('input[data-invpay2go=taxReceiptSerial]'));
      hideEle($('input[data-invpay2go=taxReceiptTitle]'));
    }
    triggerDeviceType();
  }
  $(document).on('click', 'input[data-invpay2go=taxReceiptType]', function(){
    triggerReceiptType();
  });
  $(document).on('click', 'input[data-invpay2go=taxReceiptDeviceType]', function(){
    triggerDeviceType();
  });
  
  // init all
  var checkExist = setInterval(function() {
    if ($('input[data-invpay2go=taxReceiptType]').length) {
      triggerReceiptType();
      clearInterval(checkExist);
      $('input[data-invpay2go=taxReceiptDeviceNumber]').closest('.crm-form-elem').find('.elem-label').append(' <span>(<a href="https://www.einvoice.nat.gov.tw/APMEMBERVAN/GeneralCarrier/generalCarrier" target="_blank">申請</a>)</span>');
      $('input[data-invpay2go=taxReceiptDeviceNumber]').blur(function(){
        $(this).val($(this).val().toUpperCase());
        if ($('input[data-invpay2go=taxReceiptDeviceType]:checked').val() == "0") {
          $(this).prop("pattern", '^\/[0-9A-z+-.]{7}$');
        }
        if ($('input[data-invpay2go=taxReceiptDeviceType]:checked').val() == "1") {
          $(this).prop("pattern", '^[A-Z]{2}[0-9]{14}$');
        }
        if ($(this).prop("pattern") && !this.checkValidity()) {
          var form = $(this).closest('form')[0];
          setTimeout(function(){
            form.reportValidity();
          }, 500);
        }
      });
    }
  }, 500);
  if ($("#customData").length) {
    $(document).ajaxComplete(function(event, xhr, settings) {
      if ( settings.url.indexOf('type=Contribution&subType=')) {
        triggerReceiptType();
        $('input[data-invpay2go=taxReceiptDeviceNumber]').closest('.crm-form-elem').find('.elem-label').append(' <span>(<a href="https://www.einvoice.nat.gov.tw/APMEMBERVAN/GeneralCarrier/generalCarrier" target="_blank">申請</a>)</span>');
      }
    });
  }
});
}(cj));
