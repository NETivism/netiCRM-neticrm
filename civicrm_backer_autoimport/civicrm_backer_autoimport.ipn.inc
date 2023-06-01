<?php

function civicrm_backer_autoimport_ipn($payment_processor_id) {
  $input = file_get_contents('php://input');
  $saved = 0;
  if (!empty($input)) {
    civicrm_initialize();
    $config = CRM_Core_Config::singleton();
    if (!empty($payment_processor_id) && is_numeric($payment_processor_id)) {
      $payment = CRM_Core_BAO_PaymentProcessor::getPayment($payment_processor_id, 'live');
      $processor = CRM_Core_Payment::singleton('live', $payment);
      $check_result = $processor->checkSignature($input);
      $delivery = $_SERVER['HTTP_X_BACKME_DELIVERY'];
      if (!empty($delivery) && $check_result) {
        $tmpfile = rtrim($config->uploadDir, '/').'/backer-founder-ipn'.date('_YmdHis_').$delivery.'.verified';
      }
      else {
        $tmpfile = tempnam($config->uploadDir, 'backer-founder-ipn'.date('_YmdHis_'));
      }

      // first, save to civicrm upload path to preserve original input data
      $saved = file_put_contents($tmpfile, $input);

      // second, try to process contribution and contact record
      if ($check_result) {
        $result = array();
        $processor->processContribution($input, $result);
      }
    }
    CRM_Utils_System::civiBeforeShutdown();
  }
  if ($saved) {
    echo 'OK';
  }
  else {
    echo '0';
  }
}