(function($){
  $(function(){
    if (Drupal.settings.backer) {
      var url = Drupal.settings.backer.url;
      var label = Drupal.settings.backer.label;
      $('.progress-block .progress-wrapper').after("<div class='progress-extends-backer'>本專案與<a target='_blank' href='"+url+"'>"+label+"</a>共同募款</div>");
    }
  });
})(jQuery)