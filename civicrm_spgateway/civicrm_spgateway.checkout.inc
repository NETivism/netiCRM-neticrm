<?php

/**
 * Checkout functions
 *
 * We place the code into drupal inorder to seperate the civicrm role.
 * Called by Civicrm doTransferCheckout of Class Payment_SPGATEWAY
 * 
 * @param $vars 
 * @param $component 
 * @param $payment_processor 
 */
function civicrm_spgateway_do_transfer_checkout(&$vars, &$component, &$payment_processor, $is_test){
  // once they enter here, we will check SESSION
  // to see what instrument for newweb
  $instrument_id = $vars['civicrm_instrument_id'];
  $instrument_name = civicrm_instrument_by_id($instrument_id, 'name');
  $spgateway_instruments = _civicrm_spgateway_instrument('code');
  $instrument_code = $spgateway_instruments[$instrument_name];
  $form_key = $component == 'event' ? 'CRM_Event_Controller_Registration_'.$vars['qfKey'] : 'CRM_Contribute_Controller_Contribution_'.$vars['qfKey'];

  // The first, we insert every contribution into record. After this, we'll use update for the record.
  $record = array('cid' => $vars['contributionID']);
  drupal_write_record("civicrm_contribution_spgateway", $record);

  $_SESSION['spgateway']['submitted'] = TRUE;
  $_SESSION['spgateway']['instrument'] = $instrument_code;

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
        $sql = 'SELECT id FROM civicrm_participant WHERE registered_by_id = %1';
        $params = array(
          1 => array($vars['participantID'], 'Integer'),
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
  $contrib_values['trxn_id'] = _civicrm_spgateway_trxn_id($is_test, $vars['contributionID']);
  $contribution =& CRM_Contribute_BAO_Contribution::create($contrib_values, $contrib_ids);

  // Inject in quickform sessions
  // Special hacking for display trxn_id after thank you page.
  $_SESSION['CiviCRM'][$form_key]['params']['trxn_id'] = $contribution->trxn_id;
  $_SESSION['CiviCRM'][$form_key]['params']['is_pay_later'] = $is_pay_later;
  $vars['trxn_id'] = $contribution->trxn_id;

  $arguments = _civicrm_spgateway_order($vars, $component, $payment_processor, $instrument_code, $form_key);
  if(!$contrib_values['is_recur']){
    _civicrm_spgateway_checkmacvalue($arguments, $payment_processor);
  }
  CRM_Core_Error::debug_var('spgateway_post_data', $arguments);
  // making redirect form
  $alter = array(
    'module' => 'civicrm_spgateway',
    'billing_mode' => $payment_processor['billing_mode'],
    'params' => $arguments,
  );
  drupal_alter('civicrm_checkout_params', $alter);
  print _civicrm_spgateway_form_redirect($alter['params'], $payment_processor);
  // move things to CiviCRM cache as needed
  CRM_Utils_System::civiExit();
}

function _civicrm_spgateway_order(&$vars, $component, &$payment_processor, $instrument_code, $form_key){
  global $base_url, $base_path, $language;

  // url 
  $notify_url = _civicrm_spgateway_notify_url($vars, 'spgateway/ipn/'.$instrument_code, $component);
  $civi_base_url = CRM_Utils_System::currentPath();
  $thankyou_url = url($civi_base_url, array( 'query' => "_qf_ThankYou_display=1&qfKey={$vars['qfKey']}", 'absolute' => true));

  // parameter
  if($component == 'event' && !empty($_SESSION['CiviCRM'][$form_key])){
    $values =& $_SESSION['CiviCRM'][$form_key]['values']['event'];
  }
  else{
    $values =& $_SESSION['CiviCRM'][$form_key]['values'];
  }

  // max 180 days of expire
  $baseTime = time() + 86400; // because not include today
  if (!empty($vars['payment_expired_timestamp'])) {
    $hours = ($vars['payment_expired_timestamp'] - $baseTime) / 3600;
  }
  else {
    $hours = (CRM_Core_Payment::calcExpirationDate(0) - $baseTime) / 3600;
  }
  if ($hours < 24) {
    $values['expiration_day'] = 1;
  }
  elseif ($hours > 24 * SPGATEWAY_MAX_EXPIRE_DAY ) {
    $values['expiration_day'] = SPGATEWAY_MAX_EXPIRE_DAY;
  }
  elseif(!empty($hours)){
    $values['expiration_day'] = ceil($hours/24);
  }

  // building vars
  $amount = $vars['currencyID'] == 'TWD' && strstr($vars['amount'], '.') ? substr($vars['amount'], 0, strpos($vars['amount'],'.')) : $vars['amount'];

  $itemDescription = $vars['description'];
  $itemDescription .= ($vars['description'] == $vars['item_name'])?'':':'.$vars['item_name'];
  $itemDescription .= ':'.$vars['amount'];

  if(!$vars['is_recur']){
    $args = array(
      'MerchantID' => $payment_processor['user_name'],
      'RespondType' => SPGATEWAY_RESPONSE_TYPE,
      'TimeStamp' => time(),
      'Version' => SPGATEWAY_MPG_VERSION,
      'Amt' => $amount,
      'NotifyURL' => $notify_url."&qfKey=".$vars['qfKey'],
      'Email' => $vars['email-5'],
      'LoginType' => '0',
      'ItemDesc' => $itemDescription,
      'MerchantOrderNo' => $vars['trxn_id'],
      '#url' => $payment_processor['url_site'],
    );

    switch($instrument_code){
      case 'ATM':
        $args['VACC'] = 1;
        $day = !empty($values['expiration_day']) ? $values['expiration_day'] : SPGATEWAY_EXPIRE_DAY;
        $args['ExpireDate'] = date('Ymd',strtotime("+$day day"));
        $args['CustomerURL'] = $thankyou_url;
        // $args['ReturnURL'] = url('spgateway/record/'.$vars['contributionID'], array('absolute' => true));
        break;
      case 'BARCODE':
        $args['BARCODE'] = 1;
        $day = !empty($values['expiration_day']) ? $values['expiration_day'] : SPGATEWAY_EXPIRE_DAY;
        $args['ExpireDate'] = date('Ymd',strtotime("+$day day"));
        $args['CustomerURL'] = $thankyou_url;
        // $args['ReturnURL'] = url('spgateway/record/'.$vars['contributionID'], array('absolute' => true));
        break;
      case 'CVS':
        $args['CVS'] = 1;
        if($instrument_code == 'CVS' && !empty($values['expiration_day'])) {
          $day = !empty($values['expiration_day']) ? $values['expiration_day'] : SPGATEWAY_EXPIRE_DAY;
          $args['ExpireDate'] = date('Ymd',strtotime("+$day day"));
        }
        // $args['ReturnURL'] = url('spgateway/record/'.$vars['contributionID'], array('absolute' => true));
        // $args['Desc_1'] = '';
        // $args['Desc_2'] = '';
        // $args['Desc_3'] = '';
        // $args['Desc_4'] = '';

        #ATM / CVS / BARCODE
        $args['CustomerURL'] = $thankyou_url;
        break;
      case 'WebATM':
        $args['WEBATM'] = 1;
        $args['ReturnURL'] = $thankyou_url;
        break;
      case 'Credit':
        $args['CREDIT'] = 1;
        $args['ReturnURL'] = $thankyou_url;
        break;
    }

    if($language->language == 'en'){
      $args['LangType'] = 'en';
    }
  }else{
    $data = array(
      'MerchantID' => $payment_processor['user_name'],
      'RespondType' => SPGATEWAY_RESPONSE_TYPE,
      'TimeStamp' => time(),
      'Version' => SPGATEWAY_RECUR_VERSION,
      'Amt' => $amount,
      'NotifyURL' => $notify_url."&qfKey=".$vars['qfKey'],
      'PayerEmail' => $vars['email-5'],
      'LoginType' => '0',
      'MerOrderNo' => $vars['trxn_id'],
      'ProdDesc' => $itemDescription,
      'PeriodAmt' => $amount,
      'PeriodStartType' => 2,
      'ReturnURL' => $thankyou_url,
      'PaymentInfo' => 'N',
      'OrderInfo' => 'N',
    );
    $period = strtoupper($vars['frequency_unit'][0]);

    if($vars['frequency_unit'] == 'month'){
      $frequency_interval = $vars['frequency_interval'] > 12 ? 12 : $vars['frequency_interval'];
      $data['PeriodType'] = 'M';
      $data['PeriodPoint'] = (date('d') >= 29) ? 28 : date('d');
    }
    elseif($vars['frequency_unit'] == 'week'){
      $frequency_interval = (7 * $vars['frequency_interval']) > 365 ? 365 : ($vars['frequency_interval'] * 7);
      $data['PeriodType'] = 'W';
    }
    elseif($vars['frequency_unit'] == 'year'){
      $frequency_interval = 1;
      $data['PeriodType'] = 'Y';
      $data['PeriodPoint'] = (date('md') == '0229') ? '0228' : date('md');
    }
    if(empty($frequency_interval)){
      $frequency_interval = 1;
    }
    // $data['PeriodTimes'] = $frequency_interval;
    if($vars['frequency_unit'] == 'year'){
      $data['PeriodTimes'] = empty($vars['installments']) ? 9 : $vars['installments'];
    }else{
      $data['PeriodTimes'] = empty($vars['installments']) ? 99 : $vars['installments']; // support endless
    }
    $str = http_build_query($data, '', '&');
    $strPost = _civicrm_spgateway_recur_encrypt($str, $payment_processor);
    $args['PostData_'] = $strPost;
    $args['MerchantID_'] = $payment_processor['user_name'];
    $args['#url'] = $payment_processor['url_recur'];
  }


  return $args ;
}

function _civicrm_spgateway_form_redirect($redirect_vars, $payment_processor){
  header('Pragma: no-cache');
  header('Cache-Control: no-store, no-cache, must-revalidate');
  header('Expires: 0');

  $o = "";

  $js = 'document.forms.redirect.submit();';
  $o .= '<form action="'.$redirect_vars['#url'].'" name="redirect" method="post" id="redirect-form">';
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

function _civicrm_spgateway_notify_url(&$vars, $path, $component){
  $query = array();
  $query[] = "contact_id={$vars['contactID']}";
  $query[] = "cid={$vars['contributionID']}";
  $query[] = "module={$component}";

  if ( $component == 'event' ) {
    $query[] = "eid={$vars['eventID']}";
    $query[] = "pid={$vars['participantID']}";
  }
  else {
    if ( $vars['membershipID'] ) {
      $query[] = "mid=".$vars['membershipID'];
    }
    if ($vars['related_contact']) {
      $query[] = "rid=".$vars['related_contact'];
      if ($vars['onbehalf_dupe_alert']) {
        $query[] = "onbehalf_dupe_alert=".$vars['onbehalf_dupe_alert'];
      }
    }
  }

  // if recurring donations, add a few more items
  if ( !empty( $vars['is_recur']) ) {
     if ($vars['contributionRecurID']) {
       $query[] = "crid={$vars['contributionRecurID']}";
       $query[] = "cpid={$vars['contributionPageID']}";
       $query[] = "ppid={$vars['payment_processor']}";
     }
  }

  $url = url(
    $path,
    array(
      'query'=> implode('&', $query),
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


/**
 * $objects should be an array of the dao of contribution, email, merchant_payment_processor
 */
function _civicrm_spgateway_mobile_checkout($type, $post, $objects) {
  $contribution = $objects['contribution'];
  $merchantPaymentProcessor = $objects['payment_processor'];

  if($type = 'applepay') {
    $email = new CRM_Core_DAO_Email();
    $email->contact_id = $contribution->contact_id;
    $email->is_primary = true;
    $email->find(TRUE);

    $token = urlencode(json_encode($post['token']));
    $is_test = $contribution->is_test;

    $params = array(
      'TimeStamp' => time(),
      'Version' => '1.0',
      'MerchantOrderNo' => _civicrm_spgateway_trxn_id($is_test, $contribution->id),
      'Amt' => $contribution->total_amount,
      'ProdDesc' => $post['description'], 
      'PayerEmail' => $email->email,
      'CardNo' => '',
      'Exp' => '',
      'CVC' => '',
      'APPLEPAY' => $token,
      'APPLEPAYTYPE' => '02',
    );

    $data = _civicrm_spgateway_recur_encrypt(http_build_query($params, '', '&'), get_object_vars($merchantPaymentProcessor));

    $data = array(
      'MerchantID_' => $merchantPaymentProcessor->user_name,
      'PostData_' => $data,
      'Pos_' => 'JSON',
    );
    if($contribution->is_test){
      $url = 'https://ccore.spgateway.com/API/CreditCard';
    }else{
      $url = 'https://core.spgateway.com/API/CreditCard';
    }

    $ch = curl_init($url);
    $opt = array();
    $opt[CURLOPT_RETURNTRANSFER] = TRUE;
    $opt[CURLOPT_POST] = TRUE;
    $opt[CURLOPT_POSTFIELDS] = $data;
    curl_setopt_array($ch, $opt);

    $result = curl_exec($ch);
    $status = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    if ($result === FALSE) {
      $errno = curl_errno($ch);
      $err = curl_error($ch);
      $curlError = array($errno => $err);
    }
    else{
      $curlError = array();
    }
    curl_close($ch);

    $result = json_decode($result);
    civicrm_spgateway_record($contribution->id, get_object_vars($result));
    $return = array();
    if($result->Status == 'SUCCESS'){
      $return['is_success'] = true;
    }
    $return['message'] = $result->Message;
  }
  return $return;

}