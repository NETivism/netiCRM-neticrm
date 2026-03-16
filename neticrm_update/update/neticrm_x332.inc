<?php

function _neticrm_update_update_x332() {
  civicrm_initialize();

  $removedTypes = [
    'AuthNet_AIM',
    'ClickAndPledge',
    'Elavon',
    'FirstData',
    'Google_Checkout',
    'Moneris',
    'PayJunction',
    'PayflowPro',
    'Payment_Express',
    'Realex',
    'eWAY',
  ];

  $inList = implode("','", $removedTypes);

  CRM_Core_DAO::executeQuery(
    "UPDATE civicrm_payment_processor_type SET is_active = 0 WHERE name IN ('{$inList}')"
  );

  CRM_Core_DAO::executeQuery(
    "UPDATE civicrm_payment_processor SET is_active = 0 WHERE payment_processor_type IN ('{$inList}')"
  );
}
