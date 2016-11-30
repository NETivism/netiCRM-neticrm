(function ($) {
$(document).ready(function(){
  var showEle = function(obj) {
    var section = obj.closest('.crm-section');
    var label = section.find('.label label');
    var marker = label.find('.crm-marker');
    if (!marker.length) {
      label.append('<span class="crm-marker">*</span>');
    }
    section.show();
    obj.rules('add', {required: true});
  }
  var hideEle = function(obj) {
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

  var triggerDeviceType = function() {
    var $taxReceiptDeviceType = $('input[name=taxReceiptDeviceType]:checked');
    if ($taxReceiptDeviceType.length) {
      switch ($taxReceiptDeviceType.val()) {
        case '0':
        case '1':
          showEle($('input[name=taxReceiptDeviceNumber]'));
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
          hideEle($('input[name=taxReceiptDonate]'));
          showEle($('input[name=taxReceiptDeviceType]'));
          break;
        case 'donate':
          showEle($('input[name=taxReceiptDonate]'));
          triggerDeviceType();
          hideEle($('input[name=taxReceiptDeviceType]'));
          break;
      }
    }
    else {
      hideEle($('input[name=taxReceiptDonate]'));
      hideEle($('input[name=taxReceiptDeviceType]'));
      hideEle($('input[name=taxReceiptDeviceNumber]'));
    }
    triggerDeviceType();
  }
  var triggerReceiptSerial = function() {
    var checked = $('#serialCheckbox').prop('checked');
    if (checked || $('input[name=taxReceiptSerial]').val() != '') {
      showEle($('input[name=taxReceiptSerial]'));
      $('#serialCheckbox').prop('checked', 'checked');
    }
    else {
      hideEle($('input[name=taxReceiptSerial]'));
      $('#serialCheckbox').prop('checked', false);
    }
  }
  $('input[name=taxReceiptType]').click(function(){
    triggerReceiptType();
  });
  $('input[name=taxReceiptDeviceType]').click(function(){
    triggerDeviceType();
  });
  triggerReceiptType();

  // something really initial actions
  var serialCheckbox = mdFormElement('checkbox', '我要填寫統一編號', {'id': 'serialCheckbox'});
  $('.taxReceiptSerial-section .content').append('<div class="description">同意由組織進行折讓單處理的訊息說明</div>');
  $('.taxReceiptSerial-section').before('<div class="crm-section crm-secation-serialCheckbox"><div class="content">' + serialCheckbox + '</div></div>');
  $('#serialCheckbox').click(function(){
    triggerReceiptSerial();
  });
  triggerReceiptSerial();
});
}(jQuery));
