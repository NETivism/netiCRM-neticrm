<?php

define('ALLPAY_EXPIRE_DAY', 7);

/**
 * Checkout functions
 *
 * We place the code into drupal inorder to seperate the civicrm role.
 * Called by Civicrm doTransferCheckout of Class Payment_ALLPAY
 * 
 * @param $vars 
 * @param $component 
 * @param $payment_processor 
 */
function civicrm_allpay_do_transfer_checkout(&$vars, &$component, &$payment_processor, $is_test){
  // once they enter here, we will check SESSION
  // to see what instrument for newweb
  $instrument_id = $vars['civicrm_instrument_id'];
  $instrument_name = civicrm_instrument_by_id($instrument_id, 'name');
  $allpay_instruments = _civicrm_allpay_instrument('code');
  $instrument_code = $allpay_instruments[$instrument_name];
  $form_key = $component == 'event' ? 'CRM_Event_Controller_Registration_'.$vars['qfKey'] : 'CRM_Contribute_Controller_Contribution_'.$vars['qfKey'];

  // The first, we insert every contribution into record. After this, we'll use update for the record.
  $record = array('cid' => $vars['contributionID']);
  drupal_write_record("civicrm_contribution_allpay", $record);

  $_SESSION['allpay']['submitted'] = TRUE;
  $_SESSION['allpay']['instrument'] = $instrument_code;

  if($instrument_code == 'Credit' || $instrument_code == 'WebATM'){
    $is_pay_later = FALSE;
  }
  else{
    $is_pay_later = TRUE;

    // Set participant status to 'Pending from pay later', Accupied the seat.
    if($vars['participantID']){
      $pstatus = CRM_Event_PseudoConstant::participantStatus();
      if($new_pstatus = array_search('Pending from pay later', $pstatus)){
        CRM_Core_DAO::setFieldValue('CRM_Event_DAO_Participant', $vars['participantID'], 'status_id', $new_pstatus, 'id');
        $cancelled_pstatus = array_search('Cancelled', $pstatus);
        $sql = 'SELECT id FROM civicrm_participant WHERE registered_by_id = %1 AND status_id != %2';
        $params = array(
          1 => array($vars['participantID'], 'Integer'),
          2 => array($cancelled_pstatus, 'Integer'),
        );
        $dao = CRM_Core_DAO::executeQuery($sql, $params);
        while($dao->fetch()){
          CRM_Core_DAO::setFieldValue('CRM_Event_DAO_Participant', $dao->id, 'status_id', $new_pstatus, 'id');
        }
      }
    }
  }

  // now process contribution to save some default value
  $contrib_params = array( 'id' => $vars['contributionID'] );
  $contrib_values = $contrib_ids = array();
  CRM_Contribute_BAO_Contribution::getValues($contrib_params, $contrib_values, $contrib_ids);
  if($vars['civicrm_instrument_id']){
    $contrib_values['payment_instrument_id'] = $vars['civicrm_instrument_id'];
  }
  $contrib_values['is_pay_later'] = $is_pay_later;
  $contrib_values['trxn_id'] = _civicrm_allpay_trxn_id($is_test, $vars['contributionID']);
  $contribution =& CRM_Contribute_BAO_Contribution::create($contrib_values, $contrib_ids);

  // Inject in quickform sessions
  // Special hacking for display trxn_id after thank you page.
  $_SESSION['CiviCRM'][$form_key]['params']['trxn_id'] = $contribution->trxn_id;
  $_SESSION['CiviCRM'][$form_key]['params']['is_pay_later'] = $is_pay_later;
  $vars['trxn_id'] = $contribution->trxn_id;

  $arguments = _civicrm_allpay_order($vars, $component, $payment_processor, $instrument_code, $form_key);
  _civicrm_allpay_checkmacvalue($arguments, $payment_processor);
  $alter = array(
    'module' => 'civicrm_allpay',
    'billing_mode' => $payment_processor['billing_mode'],
    'params' => $arguments,
  );
  drupal_alter('civicrm_checkout_params', $alter);
  print _civicrm_allpay_form_redirect($alter['params'], $payment_processor);
  // move things to CiviCRM cache as needed
  CRM_Utils_System::civiExit();
}

function _civicrm_allpay_order(&$vars, $component, &$payment_processor, $instrument_code, $form_key){
  global $base_url, $base_path, $language;

  // url 
  $notify_url = _civicrm_allpay_notify_url($vars, 'allpay/ipn/'.$instrument_code, $component);
  $civi_base_url = CRM_Utils_System::currentPath();
  $thankyou_url = url($civi_base_url, array( 'query' => array( "_qf_ThankYou_display" => "1" , "qfKey" => $vars['qfKey'], ), 'absolute' => true));

  // parameter
  if($component == 'event' && !empty($_SESSION['CiviCRM'][$form_key])){
    $values =& $_SESSION['CiviCRM'][$form_key]['values']['event'];
  }
  else{
    $values =& $_SESSION['CiviCRM'][$form_key]['values'];
  }

  // building vars
  $amount = $vars['currencyID'] == 'TWD' && strstr($vars['amount'], '.') ? substr($vars['amount'], 0, strpos($vars['amount'],'.')) : $vars['amount'];

  $args = array(
    'MerchantID' => $payment_processor['user_name'],
    'MerchantTradeNo' => $vars['trxn_id'],
    'MerchantTradeDate' => date('Y/m/d H:i:s'),
    'PaymentType' => 'aio',
    'TotalAmount' => $amount,
    'TradeDesc' => preg_replace('~[^\p{L}\p{N}]++~u', ' ', $vars['description']),
    'ItemName' => preg_replace('~[^\p{L}\p{N}]++~u', ' ', $vars['item_name']),
    'ReturnURL' => $notify_url,
    'ChoosePayment' => $instrument_code,
    #'CheckMacValue' => '', // add in civicrm_allpay_checkmacvalue
    'ClientBackURL' => $thankyou_url,
    'ItemURL' => '',
    'Remark' => '',
    'ChooseSubPayment' => '',
    'OrderResultURL' => $thankyou_url,
    'NeedExtraPaidInfo' => 'Y',
    'DeviceSource' => '',
  );

  // max 7 days of expire
  $baseTime = time() + 86400; // because not include today
  if (!empty($vars['payment_expired_timestamp'])) {
    $hours = ($vars['payment_expired_timestamp'] - $baseTime) / 3600;
  }
  else {
    $hours = (CRM_Core_Payment::calcExpirationDate(0) - $baseTime) / 3600;
  }
  if ($hours < 24) {
    $hours = 24;
  }

  switch($instrument_code){
    case 'ATM':
      $args['ExpireDate'] = ceil($hours/24) > 60 ? 60 : ceil($hours/24);
    case 'BARCODE':
      $args['StoreExpireDate'] = ceil($hours/24) > 7 ? 7 : ceil($hours/24);
    case 'CVS':
      if($instrument_code == 'CVS' && !empty($hours)) {
        // hour before 24hr
        $end_of_day_hr = 24 - (int)date('H');
        $end_of_day_min = (int)date('i') + 1;
        $args['StoreExpireDate'] = ceil($hours/24) > 7 ? 7 : ceil($hours/24);
        $args['StoreExpireDate'] = $args['StoreExpireDate']*24*60 + $end_of_day_hr*60 - $end_of_day_min;
      }
      $args['Desc_1'] = '';
      $args['Desc_2'] = '';
      $args['Desc_3'] = '';
      $args['Desc_4'] = '';

      #ATM / CVS / BARCODE
      $args['PaymentInfoURL'] = url('allpay/record/'.$vars['contributionID'], array('absolute' => true));
      break;
    case 'Alipay':
      $params = array(
        'version' => 3,
        'id' => $vars['contactID'],
        'return.sort_name' => 1,
        'return.phone' => 1,
      );
      $result = civicrm_api('contact', 'get', $params);
      if(!empty($result['count'])){
        $phone = $result['values'][$result['id']]['phone'];
        $name = $result['values'][$result['id']]['sort_name'];
      }
      $args['AlipayItemName'] = $vars['item_name'];
      $args['AlipayItemCounts'] = 1;
      $args['AlipayItemPrice'] = $amount;
      $args['Email'] = $vars['email-5'];
      $args['PhoneNo'] = $phone;
      $args['UserName'] = $name;
      break;
    /*
    case 'Tenpay':
      $args['ExpireTime'] = '';
      break;
     */
    case 'WebATM':
      break;
    case 'Credit':
      if($vars['is_recur']){
        $args['PeriodAmount'] = $amount;
        $period = strtoupper($vars['frequency_unit'][0]);
        $args['PeriodType'] = $vars['frequency_unit'] == 'week' ? 'D' : $period;

        if($vars['frequency_unit'] == 'month'){
          $frequency_interval = $vars['frequency_interval'] > 12 ? 12 : $vars['frequency_interval'];
        }
        elseif($vars['frequency_unit'] == 'week'){
          $frequency_interval = (7 * $vars['frequency_interval']) > 365 ? 365 : ($vars['frequency_interval'] * 7);
        }
        elseif($vars['frequency_unit'] == 'day'){
          $frequency_interval = $vars['frequency_interval'] > 365 ? 365 : $vars['frequency_interval'];
        }
        elseif($vars['frequency_unit'] == 'year'){
          $frequency_interval = 1;
        }
        if(empty($frequency_interval)){
          $frequency_interval = 1;
        }
        $args['Frequency'] = $frequency_interval;
        if($vars['frequency_unit'] == 'year'){
          $args['ExecTimes'] = empty($vars['installments']) ? 9 : $vars['installments'];
        }else{
          $args['ExecTimes'] = empty($vars['installments']) ? 99 : $vars['installments']; // support endless
        }
        $args['PeriodReturnURL'] = $notify_url.'&is_recur=1';
      }
      if($language->language == 'en'){
        $args['Language'] = 'ENG';
      }
      # Recurring
      break;
  }
  // Use hook_civicrm_alterPaymentProcessorParams
  $mode = $payment_processor['is_test'] ? 'test' : 'live';
  $paymentClass = CRM_Core_Payment::singleton($mode, $payment_processor, CRM_Core_DAO::$_nullObject);
  CRM_Utils_Hook::alterPaymentProcessorParams($paymentClass, $vars, $args);
  return $args ;
}

function _civicrm_allpay_form_redirect($redirect_vars, $payment_processor){
  header('Pragma: no-cache');
  header('Cache-Control: no-store, no-cache, must-revalidate');
  header('Expires: 0');

  $o = "";

  $js = 'document.forms.redirect.submit();';
  $o .= '<form action="'.$payment_processor['url_site'].'" name="redirect" method="post" id="redirect-form">';
  foreach($redirect_vars as $k=>$p){
    if($k[0] != '#'){
      $o .= '<input type="hidden" name="'.$k.'" value="'.$p.'" />';
    }
  }
  $o .= '</form>';
  return '
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"> 
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en" dir="ltr"> 
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
</head>
<body>
  '.$o.'
  <script type="text/javascript">
  '.$js.'
  </script>
</body>
<html>
';
}

function _civicrm_allpay_notify_url(&$vars, $path, $component){
  $query = array();
  $query["contact_id"] = $vars['contactID'];
  $query["cid"] = $vars['contributionID'];
  $query["module"] = $component;

  if ( $component == 'event' ) {
    $query["eid"] = $vars['eventID'];
    $query["pid"] = $vars['participantID'];
  }
  else {
    if ( !empty($vars['membershipID']) ) {
      $query["mid"] = $vars['membershipID'];
    }
    if ( !empty($vars['related_contact']) ){
      $query["rid"] = $vars['related_contact'];
      if ( !empty($vars['onbehalf_dupe_alert']) ){
        $query["onbehalf_dupe_alert"] = $vars['onbehalf_dupe_alert'];
      }
    }
  }

  // if recurring donations, add a few more items
  if ( !empty( $vars['is_recur']) ) {
     if ($vars['contributionRecurID']) {
       $query["crid"] = $vars['contributionRecurID'];
       $query["cpid"] = $vars['contributionPageID'];
     }
  }

  $url = url(
    $path,
    array(
      'query'=> $query,
      'absolute' => true,
    )
  );
  if( ( !empty($_SERVER['HTTP_HTTPS']) && $_SERVER['HTTP_HTTPS'] == 'on' ) || ( !empty($_SERVER['HTTPS']) && $_SERVER['HTTPS'] == 'on') ){
    return str_replace('http://', 'https://', $url);
  }
  else{
    return $url;
  }
}

