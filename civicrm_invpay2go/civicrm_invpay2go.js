(function ($) {
$(document).ready(function(){
  var showEle = function(obj) {
    if (obj.length) {
      var section = obj.closest('.crm-section');
      var label = section.find('.label label');
      var marker = label.find('.crm-marker');
      if (!marker.length) {
        label.append('<span class="crm-marker">*</span>');
      }
      section.show();
      obj.rules('add', {required: true});
    }
  }
  var hideEle = function(obj) {
    if (obj.length) {
      var section = obj.closest('.crm-section');
      section.hide();
      if(obj.prop('type') === 'radio') {
        obj.prop('checked', false);
      }
      else {
        obj.val('');
      }
      obj.rules('remove');
    }
  }

  var triggerDeviceType = function() {
    var $taxReceiptDeviceType = $('input[name=taxReceiptDeviceType]:checked');
    if ($taxReceiptDeviceType.length) {
      switch ($taxReceiptDeviceType.val()) {
        case '0':
          showEle($('input[name=taxReceiptDeviceNumber]'));
          $('input[name=taxReceiptDeviceNumber]').prop('placeholder', '請輸入手機號碼');
          break;
        case '1':
          showEle($('input[name=taxReceiptDeviceNumber]'));
          $('input[name=taxReceiptDeviceNumber]').prop('placeholder', '請輸入身分證字號');
          break;
        case '2':
          hideEle($('input[name=taxReceiptDeviceNumber]'));
          break;
      }
    }
    else {
      hideEle($('input[name=taxReceiptDeviceNumber]'));
    }
  }
  var triggerReceiptType = function(){
    var $taxReceiptType = $('input[name=taxReceiptType]:checked');
    if ($taxReceiptType.length) {
      switch ($taxReceiptType.val()) {
        case 'elec':
          hideEle($('[name=taxReceiptDonate]'));
          hideEle($('input[name=taxReceiptSerial]'));
          showEle($('input[name=taxReceiptDeviceType]'));
          break;
        case 'donate':
          hideEle($('input[name=taxReceiptSerial]'));
          hideEle($('input[name=taxReceiptDeviceType]'));
          showEle($('[name=taxReceiptDonate]'));
          triggerDeviceType();
          break;
        case 'company':
          hideEle($('input[name=taxReceiptDeviceType]'));
          hideEle($('[name=taxReceiptDonate]'));
          showEle($('[name=taxReceiptSerial]'));
          break;
      }
    }
    else {
      hideEle($('[name=taxReceiptDonate]'));
      hideEle($('input[name=taxReceiptDeviceType]'));
      hideEle($('input[name=taxReceiptDeviceNumber]'));
      hideEle($('input[name=taxReceiptSerial]'));
    }
    triggerDeviceType();
  }
  $('input[name=taxReceiptType]').click(function(){
    triggerReceiptType();
  });
  $('input[name=taxReceiptDeviceType]').click(function(){
    triggerDeviceType();
  });
  
  // init all
  $('.taxReceiptSerial-section .content').append('<div class="description">同意由組織進行折讓單處理的訊息說明</div>');
  triggerReceiptType();
});
}(jQuery));
