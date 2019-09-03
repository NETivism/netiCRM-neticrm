(function($){
  $.validator.addMethod("ckbox", function(value, element, param) {
    var p = $(element).parents('div.ckbox');
    var s = p.find("input:checkbox:checked").length;
    if(s > 0){
      return true;
    }
    else {
      return false;
    }
  },$.validator.messages.required);

  $.validator.addMethod("twid", function(value, element, param){
    if(!value || (value == "__________" && !$(element).hasClass('required'))) return true;
    if(validTWID(value) || validResidentID(value)) return true;
    return false;
  }, "請輸入正確的身分證字號或居留證號!");

  function validTWID(value) {
    var tab = "ABCDEFGHJKLMNPQRSTUVXYWZIO";
    var A1 = new Array (1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3 );
    var A2 = new Array (0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5 );
    var Mx = new Array (9,8,7,6,5,4,3,2,1,1);

    if ( value.length != 10 ){
      return false;
    }
    var i = tab.indexOf( value.charAt(0) );
    if ( i == -1 ){
      return false;
    }
    var sum = A1[i] + A2[i]*9;

    for( i=1; i<10; i++ ){
      var v = parseInt( value.charAt(i) );
      if ( isNaN(v) ){
        return false;
      }
      sum = sum + v * Mx[i];
    }
    if ( sum % 10 != 0 ){
      return false;
    }
    return true;
  }

  function validResidentID(value) {
    if (value == '') return true;
    value = value.toUpperCase();
    var tab = "ABCDEFGHJKLMNPQRSTUVXYWZIO";
    var c = (tab.indexOf(value.substr(0, 1)) + 10) + '' + (tab.indexOf(value.substr(1, 1)) % 10) + value.substr(2, 8);
    var checkCode = parseInt(c.substr(0, 1));
    for (var i = 1; i <= 9; i++) {
      checkCode += (parseInt(c.substr(i, 1)) * (10 - i)) % 10;
    }
    checkCode += parseInt(c.substr(10, 1));
    if (checkCode % 10 == 0) {
      return true;
    }
    return false;
  }

  $.validator.addMethod("twphone", function(value, element, param) {
    if(((/^0_-_/.test(value)) || (/^09__-_/.test(value))) && !$(element).hasClass('required')){
      return true;
    }
    var tel = /^0[1-9]-[0-9]{7,8}[#_]*[0-9_]*$/;
    var mobile = /^09[0-9]{2}-[0-9]{3}[0-9]{3}$/;
    return this.optional(element) || (tel.test(value)) || (mobile.test(value));
  }, "電話號碼格式有誤！（市話:03-1111111 行動:0911-111111）");
})(jQuery);
