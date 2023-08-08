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

  if ($_SERVER['REQUEST_METHOD'] == "GET") {
    if (isset($_GET['id']) && is_numeric($_GET['id'])) {
      $params = array( 1 => array($_GET['id'], 'Integer')) ;
      $donation_query = "SELECT SUM(total_amount)  FROM civicrm_contribution
      WHERE contribution_page_id = %1
      AND contribution_status_id = '1'
      AND payment_processor_id NOT IN (SELECT id FROM civicrm_payment_processor
                                    WHERE payment_processor_type = 'Backer'
                                    AND is_test = '0')
      AND is_test = '0'";

      $people_query = "SELECT count(contact_id)  FROM civicrm_contribution
      WHERE contribution_page_id = %1
      AND contribution_status_id = '1'
      AND payment_processor_id NOT IN (SELECT id FROM civicrm_payment_processor
                                    WHERE payment_processor_type = 'Backer'
                                    AND is_test = '0')
      AND is_test = '0'";
      $donationAmountSum = CRM_Core_DAO::singleValueQuery($donation_query, $params);
      $donationPeopleSum = CRM_Core_DAO::singleValueQuery($people_query, $params);

      if (!empty($donationAmountSum) && !empty($donationPeopleSum)) {
        $data = array(
          'people' => $donationPeopleSum,
          'amount' => $donationAmountSum,
        );
        echo json_encode($data);
        CRM_Utils_System::civiExit();
      }
      else {
        $error = array(
          'message' => 'An error occurred during processing.',
        );
        http_response_code(400);
        echo json_encode($error);
        CRM_Utils_System::civiExit();
      }
    }
    else
    {
      $error = array(
        'message' => 'Contribution page id is empty',
      );
      http_response_code(400);
      echo json_encode($error);
      CRM_Utils_System::civiExit();
    }
  }
}