<?php

function civicrm_neweb_do_transfer_checkout($params, $component, &$payment_processor, $is_test = 0){
  // to see what instrument for neweb
  $neweb_instrument_id = $params['civicrm_instrument_id'];
  $instrument_name  = civicrm_instrument_by_id($neweb_instrument_id, 'name');
  $neweb_instruments = _civicrm_neweb_instrument();
  $instrument_code = $neweb_instruments[$instrument_name]['code'];

  // The first, we insert every contribution into record. After this, we'll use update for the record.
  $record = array('cid' => $params['contributionID'], 'created' => time());
  drupal_write_record("civicrm_contribution_neweb", $record);

  $trxn_id = _civicrm_neweb_trxn_id($params['contributionID'], $is_test, $params['is_recur']);
  $params['trxn_id'] = $trxn_id;
  $is_pay_later = TRUE;
  switch($instrument_code){
    case 'Credit Card':
      $params_form = _civicrm_neweb_mpp($params, $component, $payment_processor, $instrument_code);
      $is_pay_later = FALSE;
      break;
    case 'ATM':
    case 'WEBATM':
    case 'CS':
    case 'MMK':
    case 'ALIPAY':
      $params_form = _civicrm_neweb_ezpay($params, $component, $payment_processor, $instrument_code);
      break;
  }

  // now process contribution to save some default value
  $contrib_params = array( 'id' => $params['contributionID'] );
  $contrib_values = $contrib_ids = array();
  CRM_Contribute_BAO_Contribution::getValues($contrib_params, $contrib_values, $contrib_ids);
  if($params['civicrm_instrument_id']){
    $contrib_values['payment_instrument_id'] = $params['civicrm_instrument_id'];
  }
  if($contrib_values['is_pay_later'] != $is_pay_later && $instrument_code !== 'WEBATM'){
    $contrib_values['is_pay_later'] = $is_pay_later;
  }
  $contrib_values['payment_processor_id'] = $payment_processor['id'];
  $contrib_values['trxn_id'] = $trxn_id;
  $contribution =& CRM_Contribute_BAO_Contribution::create($contrib_values, $contrib_ids);

  // Inject in quickform sessions
  // Special hacking for display trxn_id after thank you page.
  $form_key = $component == 'event' ? 'CRM_Event_Controller_Registration_'.$params['qfKey'] : 'CRM_Contribute_Controller_Contribution_'.$params['qfKey'];
  $_SESSION['CiviCRM'][$form_key]['params']['trxn_id'] = $contribution->trxn_id;
  $_SESSION['CiviCRM'][$form_key]['params']['returnvalue'] = $params_form['returnvalue'];

  // making redirect form
  $alter = array(
    'module' => 'civicrm_neweb',
    'billing_mode' => $payment_processor['billing_mode'],
    'params' => $params_form,
  );
  drupal_alter('civicrm_checkout_params', $alter);

  // avoid doesn't execute update.php , check the new column existed.
  $post_data = json_encode($alter['params']);
  $sql = "SHOW COLUMNS FROM civicrm_contribution_neweb LIKE 'return_data'";
  $has_column = CRM_Core_DAO::singleValueQuery($sql);
  if(!empty($has_column)){
    $sql = "UPDATE civicrm_contribution_neweb SET post_data = '$post_data' WHERE cid = {$contribution->id}";
    CRM_Core_DAO::executeQuery($sql);
  }

  if($instrument_code == 'Credit Card' || $alter['params']['returnvalue'] == 0){
    print _civicrm_neweb_form_redirect($alter['params'], $instrument_code);
    // move things to CiviCRM cache as needed
    CRM_Core_Session::storeSessionObjects();
    exit();
  }else{

    // Set participant status to 'Pending from pay later', Accupied the seat.
    if($params['participantID']){
      $pstatus = CRM_Event_PseudoConstant::participantStatus();
      if($new_pstatus = array_search('Pending from pay later', $pstatus)){
        CRM_Core_DAO::setFieldValue('CRM_Event_DAO_Participant', $params['participantID'], 'status_id', $new_pstatus, 'id');
        $sql = 'SELECT id FROM civicrm_participant WHERE registered_by_id = %1';
        $dao = CRM_Core_DAO::executeQuery($sql, array(1 => array($params['participantID'], 'Integer')));
        while($dao->fetch()){
          CRM_Core_DAO::setFieldValue('CRM_Event_DAO_Participant', $dao->id, 'status_id', $new_pstatus, 'id');
        }
      }
    }

    module_load_include("inc", 'civicrm_neweb', 'civicrm_neweb.cron');
    $result = civicrm_neweb_postdata($alter['params'], $payment_processor, $alter['params']['#action']);

    if(!empty($result)){
      $sql = "SELECT return_data FROM civicrm_contribution_neweb WHERE cid = {$contribution->id}";
      $data = CRM_Core_DAO::singleValueQuery($sql);
      if(!empty($data)){
        $result = json_decode($data);
        $sql = "UPDATE civicrm_contribution_neweb SET return_data = '$result' WHERE cid = {$contribution->id}";
        CRM_Core_DAO::executeQuery($sql);
      }else{
        $result = json_encode($result);
        $sql = "UPDATE civicrm_contribution_neweb SET return_data = '$result' WHERE cid = {$contribution->id}";
        CRM_Core_DAO::executeQuery($sql);
      }

      require_once("civicrm_neweb.notify.inc");
      civicrm_neweb_notify_generate($contribution->id, TRUE);
      $civi_base_url = CRM_Utils_System::currentPath();
      $query = array(
        '_qf_ThankYou_display' => 'true',
        'qfKey' => $params['qfKey'],
        'instrument' => $instrument_code,
      );
      CRM_Core_Session::storeSessionObjects( );
      drupal_goto($civi_base_url, $query);
    }
  }

}

function _civicrm_neweb_mpp(&$params, $component, $payment_processor, $neweb_instrument){
  $civi_base_url = CRM_Utils_System::currentPath();
  $cancel_url = CRM_Utils_System::url($civi_base_url,"_qf_Confirm_display=true&qfKey={$params['qfKey']}",false, null, false );
  $return_url = CRM_Utils_System::url($civi_base_url,"_qf_ThankYou_display=1&qfKey={$params['qfKey']}",true, null, false );

  // notify url for receive payment result
  $notify_url = _civicrm_neweb_notify_url($params, 'neweb/ipn', $component);

  // building params
  $amount = $params['currencyID'] == 'TWD' && strstr($params['amount'], '.') ? substr($params['amount'], 0, strpos($params['amount'],'.')) .'.00' : $params['amount'];
  $name = function_exists('truncate_utf8') ? truncate_utf8($params['item_name'], 10) : $params['item_name'];

  $neweb_params = array(
    "MerchantNumber" => $payment_processor['user_name'],
    "OrderNumber"    => $params['trxn_id'],
    "Amount"         => $amount,
    "OrgOrderNumber" => $params['contributionID'],
    "ApproveFlag"    => 1,
    "DepositFlag"    => 1,
    "Englishmode"    => 0,
    "OrderURL"       => $notify_url,
    "ReturnURL"      => $return_url,
    "op"             => "AcceptPayment",
    "iphonepage"     => 1,
    "#action"        => $payment_processor['url_site'],
    "#paymentProcessor" => $payment_processor,
  );
  $neweb_params["checksum"] = md5($payment_processor['user_name'].$neweb_params['OrderNumber'].$payment_processor['signature'].$amount);
  global $language;
  if ($language->language == 'en') {
    $neweb_params["Englishmode"] = 1;
  }

  return $neweb_params;
}

function _civicrm_neweb_ezpay(&$params, $component, $payment_processor, $neweb_instrument){
  require_once 'CRM/Contact/DAO/Contact.php';
  $contact = new CRM_Contact_DAO_Contact( );
  $contact->id = $params['contact'];
  $contact->find(true);

  if(strpos($params['amount'],'.') ){
    $amount = substr($params['amount'], 0, strpos($params['amount'],'.'));
  }
  else{
    $amount = $params['amount'];
  }
  
  $post = array();
  $post['merchantnumber'] = $payment_processor['password'];
  $post['ordernumber'] = $params['contributionID'];
  $post['amount'] = $amount;
  $post['paymenttype'] = $neweb_instrument;
  $post['paytitle'] = truncate_utf8($params['item_name'], 12);
  $post['bankid'] = '007';
  if($component == 'event' ){
    require_once 'CRM/Core/DAO.php';
    $eventID = $params['eventID'];
    $sql = "SELECT expiration_time FROM civicrm_event WHERE id = $eventID";
    $expiration_time = CRM_Core_DAO::singleValueQuery($sql);
  }
  if (!empty($params['payment_expired_timestamp'])) { 
    $post['duedate'] = date('Ymd', $params['payment_expired_timestamp']);
  }
  else {
    $post['duedate'] = date('Ymd', CRM_Core_Payment::calcExpirationDate(0));
  }
  if($neweb_instrument == 'CS'){
    $post['payname'] = $params['last_name']." ".$params['first_name'];
    if($params['phone']){
      $post['payphone'] = preg_replace("/[^\d]+/i", $params['phone']);
    }
  }
  $sql = "SHOW COLUMNS FROM civicrm_contribution_neweb LIKE 'return_data'";
  $is_returnable = CRM_Core_DAO::singleValueQuery($sql);
  $post['returnvalue'] = (($neweb_instrument == 'WEBATM') || empty($is_returnable)) ? 0 : 1;
  $post['hash'] = md5($post['merchantnumber'].$payment_processor['subject'].$amount.$post['ordernumber']);
  $civi_base_url = CRM_Utils_System::currentPath();
  if($neweb_instrument == 'WEBATM'){
    $post['nexturl'] = CRM_Utils_System::url($civi_base_url,"_qf_ThankYou_display=1&qfKey={$params['qfKey']}",true, null, false );
  }
  else{
    $post['nexturl'] = CRM_Utils_System::url($civi_base_url,"_qf_ThankYou_display=1&qfKey={$params['qfKey']}&instrument={$neweb_instrument}",true, null, false );
  }
  $post["#action"] = rtrim($payment_processor['url_api'],'/')."/Payment";

  return $post;
}

function _civicrm_neweb_notify_url(&$vars, $path, $component){
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
     }
  }

  global $language;
  if ($language->language == 'en') {
    $query['language'] = 'en';
  }

  return url(
    $path,
    array(
      'query'=> implode('&', $query),
      'absolute' => true,
    )
  );
}

function _civicrm_neweb_form_redirect($redirect_params, $instrument){
  header('Pragma: no-cache');
  header('Cache-Control: no-store, no-cache, must-revalidate');
  header('Expires: 0');

  switch($instrument){
    case 'Credit Card':
    case 'WEBATM':
    case 'ALIPAY':
      $js = 'document.forms.redirect.submit();
      var btn = document.getElementById("submit-button");
      btn.style.display = "none";
      setTimeout(function(){
        btn.style.display = "block";
      }, 1000);
      ';
      $o .= '<form action="'.$redirect_params['#action'].'" name="redirect" method="post" id="redirect-form">';
      foreach($redirect_params as $k=>$p){
        if($k[0] != '#'){
          $o .= '<input type="hidden" name="'.$k.'" value="'.$p.'" />';
        }
      }
      $o .= '<div align="center" id="submit-button"><p>若未自動轉向繳款頁面，請您按「下一步」來繼續進行動作</p><div><input type="submit" value="下一步" /></div></div>';
      $o .= '</form>';
      break;
    case 'ATM':
    case 'CS':
    case 'MMK':
      $js = '
function print_redirect(){
  // creating the "newebresult" window with custom features prior to submitting the form
  // window.open("", "newebresult", "scrollbars=yes,menubar=no,height=600,width=800,resizable=yes,toolbar=no,status=no,left=150,top=150");
  document.forms.print.submit();
  // window.location = "'.$redirect_params['nexturl'].'";
}
      ';

      $o .= '<form action="'.$redirect_params['#action'].'" name="print" method="post" id="redirect-form">';
      foreach($redirect_params as $k=>$p){
        if($k[0] != '#'){
          $o .= '<input type="hidden" name="'.$k.'" value="'.$p.'" />';
        }
      }
      $o .= '</form>';
      $o .= '<div align="center"><p>若網頁沒有自動跳出付款資訊，您可自行按下「取得付款資訊」按鈕以獲得繳款訊息</p><div><input type="button" value="取得付款資訊" onclick="print_redirect();" /></div></div>';
      break;
  }
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

function _civicrm_neweb_get_mobile_params(){
  $params = array(
    'session_url' => 'https://testmaple2.neweb.com.tw/NewebPayment2/applepay/sessions',
    'transact_url' => 'https://testmaple2.neweb.com.tw/NewebPayment2/applepay/ccaccept',
  );
  return $params;
}