(function($) {
  var init = function() {
    $(".crm-section[class*='state_province']").each(function() {
      let $container = $(this),
          $selectFields = $container.find(".crm-form-select"),
          $postcodeField = $container.find(".crm-form-post-code");

      $selectFields.each(function() {
        let $field = $(this),
            $select = $field.find(".form-select"),
            label = "";

        if (!$field.hasClass("crm-floating-label")) {
          $select.find("option[value]:first-child").html("");
          $field.addClass("crm-floating-label");

          if (!$field.children(".elem-label").length) {
            let selectName = $select.attr("name");

            if (selectName.indexOf("state_province") != -1) {
              label = '縣市';
            }

            if (selectName.indexOf("city") != -1) {
              label = '鄉鎮市區';
            }

            $field.append("<label class='elem-label' for='" + selectName  +"'>" + label + "</label>");
            $select.find("option[value]:first-child").html("");

            if ($select.val() != "" || $select.find("option[selected]").length) {
              $select.addClass("is-selected-val");
            }
            else {
              $select.removeClass("is-selected-val");
            }

            $select.change(function() {
              $(this).find("option[value]:first-child").html("");

              if ($(this).val() != "" || $(this).find("option[selected]").length) {
                $(this).addClass("is-selected-val");
              }
              else {
                $(this).removeClass("is-selected-val");
              }
            });
          }
        }
      });

      if ($postcodeField.length) {
        let $input = $postcodeField.find(".form-text");

        if (!$postcodeField.hasClass("crm-floating-label")) {
          $postcodeField.addClass("crm-floating-label");
        }

        if ($postcodeField.next(".elem-label").length) {
          $postcodeField.next(".elem-label").appendTo($postcodeField);
        }

        let phText = $input.attr("data-label") !== "undefined" ? $input.attr("data-label") : "郵遞區號";
        $input.attr("placeholder", phText);
      }
    });
  }

  $(document).ready(function(){
    setTimeout(init, 2000);
  });
}(cj));