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

  if (!empty($post['Period'])) {
    $paymentProcessor = CRM_Core_BAO_PaymentProcessor::getPayment($pid, $isTest ? 'test': 'live');
    $decryptParams = _civicrm_spgateway_post_decode(_civicrm_spgateway_recur_decrypt($post['Period'], $paymentProcessor));
    CRM_Core_Error::debug_var('spgateway_neweb_transfer_decrypt_params', $decryptParams);
    $rid = $decryptParams['MerchantOrderNo'];
  }

  if (!empty($decryptParams) && $decryptParams['RespondCode'] == '00' && $decryptParams['Status'] == 'SUCCESS') {
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
    CRM_Core_Error::debug_var('spgateway_neweb_transfer_ipn_post', $ipn_get);

    // create recurring record
    $result = new stdClass();
    $result->_ipn_result = $ipn_result;
    $result->_post = $ipn_post;
    $result->_get = $ipn_get;
    module_load_include('inc', 'civicrm_spgateway', 'civicrm_spgateway.ipn');
    $result->_response = civicrm_spgateway_ipn('Credit', $ipn_post, $ipn_get, FALSE);
    CRM_Core_Error::debug_var('spgateway_neweb_transfer_ipn_response', $result->_response);
    return json_encode($result);
  }

}