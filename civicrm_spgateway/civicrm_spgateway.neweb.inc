<?php
function civicrm_spgateway_neweb_transfer($post = NULL, $get = NULL, $print = TRUE, $isTest = FALSE) {
  civicrm_initialize();
  // detect variables
  $post = !empty($post) ? $post : $_POST;
  CRM_Core_Error::debug_var('spgateway_neweb_transfer_post', $post);
  $ids = array();
  $pid = variable_get('civicrm_spgateway_neweb_pid', 0);
  if (empty($pid)) {
    $pid = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_payment_processor WHERE payment_processor_type = 'SPGATEWAY'");
    if ($isTest) {
      $pid += 1;
    }
  }

  if (empty($pid)) {
    watchdog('civicrm_spgateway.neweb', "There are no SPGATEWAY payment processor.");
    drupal_not_found();
  }

  if (!empty($post['Period'])) {
    if (is_array($pid)) {
      $pids = $pid;
      foreach ($pids as $pid) {
        $paymentProcessor = CRM_Core_BAO_PaymentProcessor::getPayment($pid, $isTest ? 'test': 'live');
        $decryptParams = _civicrm_spgateway_post_decode(_civicrm_spgateway_recur_decrypt($post['Period'], $paymentProcessor));
        CRM_Core_Error::debug_var('spgateway_neweb_transfer_decrypt_params', $decryptParams);
        if (!empty($decryptParams['MerchantOrderNo'])) {
          $rid = $decryptParams['MerchantOrderNo'];
          break;
        }
      }
      if (empty($pid)) {
        watchdog('civicrm_spgateway.neweb', "You set multiple SPGATEWAY payment processors, but none of them are decryptable.");
        echo "Error. Please check log.";
        drupal_not_found();
      }
    }
    else {
      $paymentProcessor = CRM_Core_BAO_PaymentProcessor::getPayment($pid, $isTest ? 'test': 'live');
      $decryptParams = _civicrm_spgateway_post_decode(_civicrm_spgateway_recur_decrypt($post['Period'], $paymentProcessor));
      CRM_Core_Error::debug_var('spgateway_neweb_transfer_decrypt_params', $decryptParams);
      $rid = $decryptParams['MerchantOrderNo'];
      CRM_Core_Error::debug_var('spgateway_neweb_transfer_rid', $rid);
    }
  }
  else {
    watchdog('civicrm_spgateway.neweb', "\$post['Period'] not exist.");
    drupal_not_found();
  }

  if (!empty($decryptParams) && !empty($rid)) {
    // complex part to simulate spgateway ipn
    $ipn_result = $ipn_get = $ipn_post = array();

    // prepare post, complex logic because recurring have different variable names
    $ipn_result['Result'] = $decryptParams;
    $queryParams = array(1 => array($rid, 'Positive'));
    $ipn_result['Result']['MerchantOrderNo'] = CRM_Core_DAO::singleValueQuery("SELECT trxn_id FROM civicrm_contribution WHERE contribution_recur_id = %1 ORDER BY id ASC LIMIT 1", $queryParams);
    $ipn_result['Status'] = $decryptParams['Status'];
    $ipn_result['Result']['OrderNo'] = 'r_'.$ipn_result['Result']['OrderNo'];
    CRM_Core_Error::debug_var('spgateway_neweb_transfer_ipn_result', $ipn_result);
    $ipn_result = json_encode($ipn_result);
    $ipn_post = array('Period' => _civicrm_spgateway_recur_encrypt($ipn_result, $paymentProcessor));
    CRM_Core_Error::debug_var('spgateway_neweb_transfer_ipn_post', $ipn_post);

    // prepare get
    $firstCid = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_contribution WHERE contribution_recur_id = %1 ORDER BY id ASC LIMIT 1", $queryParams);
    $ids = CRM_Contribute_BAO_Contribution::buildIds($firstCid);
    $query = CRM_Contribute_BAO_Contribution::makeNotifyUrl($ids, NULL, TRUE);
    $query .= '&ppid='.$pid;
    parse_str($query, $ipn_get);
    CRM_Core_Error::debug_var('spgateway_neweb_transfer_ipn_get', $ipn_get);

    // create recurring record
    $result = new stdClass();
    $result->_ipn_result = $ipn_result;
    $result->_post = $ipn_post;
    $result->_get = $ipn_get;
    module_load_include('inc', 'civicrm_spgateway', 'civicrm_spgateway.ipn');
    $result->_response = civicrm_spgateway_ipn('Credit', $ipn_post, $ipn_get, FALSE);
    // If correct, it must return '1|OK'.
    if (!empty($result->_response)) {
      $query = "UPDATE civicrm_contribution SET payment_processor_id = %1 WHERE contribution_recur_id = %2 ORDER BY id DESC LIMIT 1";
      $params = array(
        1 => array($pid, 'Positive'),
        2 => array($ids['contributionRecurID'], 'Positive'),
      );
      CRM_Core_DAO::executeQuery($query, $params);
    }
    $recur_processor_id = CRM_Core_DAO::getFieldValue("CRM_Contribute_DAO_ContributionRecur", $rid, "processor_id");
    if ($recur_processor_id != $pid) {
      CRM_Core_DAO::setFieldValue("CRM_Contribute_DAO_ContributionRecur", $rid, "processor_id", $pid);
    }
    CRM_Core_Error::debug_var('spgateway_neweb_transfer_ipn_response', $result->_response);
    echo $result->_response;
  }
  else {
    $msg = 'Error. Decrypt error or Recurring ID is not found.';
    watchdog('civicrm_spgateway.neweb', $msg);
    echo $msg;
  }
}