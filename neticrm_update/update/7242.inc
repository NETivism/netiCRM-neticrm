<?php

function _neticrm_update_update_7242(){

  civicrm_initialize();

  $sql = "SELECT name FROM civicrm_payment_processor_type WHERE name = 'Mobile";
  $exist = CRM_Core_DAO::singleValueQuery($sql);
  if(empty($exist)){
    $sql = "INSERT INTO civicrm_payment_processor_type (name, title, is_active, is_default, user_name_label, password_label, signature_label, subject_label, class_name, billing_mode) VALUES ('Mobile', '".ts('Mobile Payment')."', 1, 0, '".ts('Merchant ID')."', '".ts('Payment Processor Provider')."', '".ts('Account')."', '".ts('Password')."',  'Payment_Mobile', 4);";
    CRM_Core_DAO::executeQuery($sql);
  }

  $payment_instrument = CRM_Core_OptionGroup::values('payment_instrument', TRUE);
  $payment_instrument = array_keys($payment_instrument);

  $definition = array(
    'ApplePay' => 'Apple Pay',
    'GooglePay' => 'Google Pay',
  );

  $values = array();
  $group_params = array('name' => 'payment_instrument');
  $params = array(
    'is_active' => 1,
  );
  foreach($definition as $name => $label){
    if(!in_array($label, $payment_instrument)){
      $params = array();
      $params['label'] = $label;
      $params['name'] = $name;
      $params['is_active'] = 1;
      $params['is_reserved'] = 1;
      if($values[$label]){
        // skipped, won't update.
      }
      else{
        $action = CRM_Core_Action::ADD;
        $option_value_id = 0;
        CRM_Core_OptionValue::addOptionValue($params, $group_params, $action, $option_value_id);
      }
    }
  }

}