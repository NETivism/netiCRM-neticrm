<?php

function _neticrm_update_update_6142(){

  civicrm_initialize();

  $sql = "SELECT name FROM civicrm_payment_processor_type WHERE name = 'Mobile'";
  $exist = CRM_Core_DAO::singleValueQuery($sql);
  if(empty($exist)){
    $sql = "INSERT INTO civicrm_payment_processor_type (name, title, is_active, is_default, user_name_label, password_label, signature_label, subject_label, class_name, billing_mode) VALUES ('Mobile', '".ts('Mobile Payment')."', 1, 0, '".ts('Merchant ID')."', '".ts('Payment Processor Provider')."', '".ts('Account')."', '".ts('Password')."',  'Payment_Mobile', 4);";
    CRM_Core_DAO::executeQuery($sql);
  }
}