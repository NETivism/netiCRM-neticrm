<?php

function _neticrm_update_update_7281() {
  $paymentProcessors = [
    'SPGATEWAY' => '藍新信用卡',
    'SPGATEWAYX' => '藍新非信用卡',
    'Neweb' => '舊藍新',
  ];
  foreach ($paymentProcessors as $name => $title) {
    $params = [
      1 => [$name, 'String'],
      2 => [$title, 'String'],
    ];
    $sql = "SELECT id FROM civicrm_payment_processor_type WHERE name = %1";
    if (CRM_Core_DAO::singleValueQuery($sql, $params)) {
      CRM_Core_DAO::executeQuery("UPDATE civicrm_payment_processor_type SET title = %2 WHERE name = %1", $params);
    }
  }
}
