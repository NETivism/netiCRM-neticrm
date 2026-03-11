<?php

function _neticrm_update_update_x328() {
  civicrm_initialize();
  $group_params = ['name' => 'payment_instrument'];
  $params = [
    'name' => 'ApplePayFront',
    'label' => ts('Apple Pay Front'),
    'is_active' => 1,
    'is_reserved' => 1,
    'weight' => 19,
  ];
  $dao = CRM_Core_DAO::executeQuery("SELECT id, value FROM civicrm_option_value WHERE name = 'ApplePayFront'");
  $dao->fetch();
  if ($dao->id) {
    $params['value'] = $dao->value;
    $action = CRM_Core_Action::UPDATE;
    $option_value_id = $dao->id;
  }
  else {
    $action = CRM_Core_Action::ADD;
    $option_value_id = 0;
  }
  CRM_Core_OptionValue::addOptionValue($params, $group_params, $action, $option_value_id);
  echo "Success add Apple Pay Front payment instrument";
}
