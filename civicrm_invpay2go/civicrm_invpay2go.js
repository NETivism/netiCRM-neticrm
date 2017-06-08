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
      switch ($taxReceiptDeviceType.val()) {
        case '0':
          showEle($('input[data-invpay2go=taxReceiptDeviceNumber]'));
          $('input[data-invpay2go=taxReceiptDeviceNumber]').prop('placeholder', '請輸入手機號碼');
          break;
        case '1':
          showEle($('input[data-invpay2go=taxReceiptDeviceNumber]'));
          $('input[data-invpay2go=taxReceiptDeviceNumber]').prop('placeholder', '請輸入身分證字號');
          break;
        case '2':
          hideEle($('input[data-invpay2go=taxReceiptDeviceNumber]'));
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
          hideEle($('input[data-invpay2go=taxReceiptSerial]'));
          showEle($('input[data-invpay2go=taxReceiptDeviceType]'));
          break;
        case 'donate':
          hideEle($('input[data-invpay2go=taxReceiptSerial]'));
          hideEle($('input[data-invpay2go=taxReceiptDeviceType]'));
          showEle($('[data-invpay2go=taxReceiptDonate]'));
          triggerDeviceType();
          break;
        case 'company':
          hideEle($('input[data-invpay2go=taxReceiptDeviceType]'));
          hideEle($('[data-invpay2go=taxReceiptDonate]'));
          showEle($('[data-invpay2go=taxReceiptSerial]'));
          break;
      }
    }
    else {
      hideEle($('[data-invpay2go=taxReceiptDonate]'));
      hideEle($('input[data-invpay2go=taxReceiptDeviceType]'));
      hideEle($('input[data-invpay2go=taxReceiptDeviceNumber]'));
      hideEle($('input[data-invpay2go=taxReceiptSerial]'));
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
    }
  }, 500);
});
}(cj));
