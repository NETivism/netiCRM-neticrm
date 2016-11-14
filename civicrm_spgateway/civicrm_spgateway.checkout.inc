<?php

define('SPGATEWAY_EXPIRE_DAY', 7);

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
    if($vars['participantID']){
      $pstatus = CRM_Event_PseudoConstant::participantStatus();
      if($new_pstatus = array_search('Pending from pay later', $pstatus)){
        CRM_Core_DAO::setFieldValue('CRM_Event_DAO_Participant', $vars['participantID'], 'status_id', $new_pstatus, 'id');
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
  _civicrm_spgateway_checkmacvalue($arguments, $payment_processor);
  watchdog('civicrm_spgateway', var_export($arguments,true));
  print _civicrm_spgateway_form_redirect($arguments, $payment_processor);
  // move things to CiviCRM cache as needed
  CRM_Utils_System::civiExit();
}

function _civicrm_spgateway_order(&$vars, $component, &$payment_processor, $instrument_code, $form_key){
  global $base_url, $base_path, $language;

  // url 
  $notify_url = _civicrm_spgateway_notify_url($vars, 'spgateway/ipn/'.$instrument_code, $component);
  $civi_base_url = CRM_Utils_System::currentPath();
  $thankyou_url = url($civi_base_url, array( 'query' => array( "_qf_ThankYou_display" => "1" , "qfKey" => $vars['qfKey'], ), 'absolute' => true));

  // parameter
  if($component == 'event' && !empty($_SESSION['CiviCRM'][$form_key])){
    $values =& $_SESSION['CiviCRM'][$form_key]['values']['event'];
  }
  else{
    $values =& $_SESSION['CiviCRM'][$form_key]['values'];
  }

  // max 7 days of expire
  if (!empty($values['expiration_time']) && $values['expiration_time'] < 24) {
    $values['expiration_time'] = 24;
  }
  if (!empty($values['expiration_time']) && $values['expiration_time'] > 24*7 ) {
    $values['expiration_time'] = 24*7;
  }

  // building vars
  $amount = $vars['currencyID'] == 'TWD' && strstr($vars['amount'], '.') ? substr($vars['amount'], 0, strpos($vars['amount'],'.')) : $vars['amount'];

  $itemDescription = $vars['description'];
  $itemDescription .= ($vars['description'] == $vars['item_name'])?'':':'.$vars['item_name'];
  $itemDescription .= ':'.$vars['amount'];

  if(!$vars['is_recur']){
    $args = array(
      'MerchantID' => $payment_processor['user_name'],
      'RespondType' => 'JSON',
      #'CheckValue' => '', // add in civicrm_spgateway_checkmacvalue
      'TimeStamp' => time(),
      'Version' => '1.2',
      #'LangType' => '', // add beside this.
      # 'MerchantOrderNo' => $vars['trxn_id'],
      'Amt' => $amount,
      # 'ItemDesc' => $itemDescription, // Required.
      #'TradeLimit' => '', // Limit seconds, dismiss
      #'ExpireDate' => '', // add beside this.
      'NotifyURL' => $notify_url."&qfKey=".$vars['qfKey'],
      #'CustomerURL' => '', // Page after post, Todo.
      'Email' => $vars['email-5'],
      // 'EmailModify' => '0', // Dismiss
      'LoginType' => '0',
      '#url' => $payment_processor['url_site'],
      #'OrderComment' => '', // Dismiss
    );

    switch($instrument_code){
      case 'ATM':
        $args['VACC'] = 1;
        $day = !empty($values['expiration_time']) ? ceil($values['expiration_time']/24) : 7;
        $args['ExpireDate'] = date('Ymd',strtotime("+$day day"));
        $args['CustomerURL'] = $thankyou_url;
        $args['ReturnURL'] = url('spgateway/record/'.$vars['contributionID'], array('absolute' => true));
        $args['MerchantOrderNo'] = $vars['trxn_id'];
        $args['ItemDesc'] = $itemDescription;
        break;
      case 'BARCODE':
        $args['BARCODE'] = 1;
        $day = !empty($values['expiration_time']) ? ceil($values['expiration_time']/24) : 7;
        $args['ExpireDate'] = date('Ymd',strtotime("+$day day"));
        $args['CustomerURL'] = $thankyou_url;
        $args['ReturnURL'] = url('spgateway/record/'.$vars['contributionID'], array('absolute' => true));
        $args['MerchantOrderNo'] = $vars['trxn_id'];
        $args['ItemDesc'] = $itemDescription;
        break;
      case 'CVS':
        $args['CVS'] = 1;
        if($instrument_code == 'CVS' && !empty($values['expiration_time'])) {
          $day = !empty($values['expiration_time']) ? ceil($values['expiration_time']/24) : 7;
          $args['ExpireDate'] = date('Ymd',strtotime("+$day day"));
        }
        $args['ReturnURL'] = url('spgateway/record/'.$vars['contributionID'], array('absolute' => true));
        // $args['Desc_1'] = '';
        // $args['Desc_2'] = '';
        // $args['Desc_3'] = '';
        // $args['Desc_4'] = '';

        #ATM / CVS / BARCODE
        $args['CustomerURL'] = $thankyou_url;
        $args['MerchantOrderNo'] = $vars['trxn_id'];
        $args['ItemDesc'] = $itemDescription;
        // $args['CustomerURL'] = url('spgateway/record/'.$vars['contributionID'], array('absolute' => true));
        break;
      case 'WebATM':
        $args['WEBATM'] = 1;
        $args['CustomerURL'] = $thankyou_url;
        $args['ReturnURL'] = url('spgateway/record/'.$vars['contributionID'], array('absolute' => true));
        $args['MerchantOrderNo'] = $vars['trxn_id'];
        $args['ItemDesc'] = $itemDescription;
        break;
      case 'Credit':
        $args['MerchantOrderNo'] = $vars['trxn_id'];
        $args['ItemDesc'] = $itemDescription;
        $args['ReturnURL'] = $thankyou_url;
        $args['CREDIT'] = 1;
        # Recurring
        break;
    }
    if($language->language == 'en'){
      $args['Language'] = 'ENG';
    }
  }else{
    $data = array(
      'MerchantID' => $payment_processor['user_name'],
      'RespondType' => 'JSON',
      #'CheckValue' => '', // add in civicrm_spgateway_checkmacvalue
      'TimeStamp' => time(),
      'Version' => '1.0',
      #'LangType' => '', // add beside this.
      # 'MerchantOrderNo' => $vars['trxn_id'],
      'Amt' => $amount,
      # 'ItemDesc' => $itemDescription, // Required.
      #'TradeLimit' => '', // Limit seconds, dismiss
      #'ExpireDate' => '', // add beside this.
      'NotifyURL' => $notify_url."&qfKey=".$vars['qfKey'],
      #'CustomerURL' => '', // Page after post, Todo.
      'Email' => $vars['email-5'],
      // 'EmailModify' => '0', // Dismiss
      'LoginType' => '0',
      '#url' => $payment_processor['url_site'],
      #'OrderComment' => '', // Dismiss
    );
    $data['MerOrderNo'] = $vars['trxn_id'];
    $data['ProdDesc'] = $itemDescription;

    // $data['Version'] = '1.0';
    $data['PeriodAmt'] = $amount;
    $period = strtoupper($vars['frequency_unit'][0]);

    if($vars['frequency_unit'] == 'month'){
      $frequency_interval = $vars['frequency_interval'] > 12 ? 12 : $vars['frequency_interval'];
      $data['PeriodType'] = 'M';
      $data['PeriodPoint'] = date('d');
    }
    elseif($vars['frequency_unit'] == 'week'){
      $frequency_interval = (7 * $vars['frequency_interval']) > 365 ? 365 : ($vars['frequency_interval'] * 7);
      $data['PeriodType'] = 'W';
    }
    elseif($vars['frequency_unit'] == 'year'){
      $frequency_interval = 1;
      $data['PeriodType'] = 'Y';
      $data['PeriodPoint'] = date('MD');
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
    $data['PeriodStartType'] = 2;
    $data['NotifyURL'] = $notify_url.'&is_recur=1';
    ksort($data);
    $str = http_build_query($data);
    $strPost = _civicrm_spgateway_recur_encrypt($str, $payment_processor);
    $args['PostData_'] = $strPost;
    $args['MerchantID_'] = $payment_processor['user_name'];
    $args['#url'] = $payment_processor['url_recur'];

  }

  
  return $args ;
}

function _civicrm_spgateway_recur_encrypt($args, $payment_processor){
  $key = $payment_processor['password'];
  $iv = $payment_processor['signature'];
  $str = trim(bin2hex(mcrypt_encrypt(MCRYPT_RIJNDAEL_128, $key,
_civicrm_spgateway_addpadding($str), MCRYPT_MODE_CBC, $iv))); 
  return $str;
}

function _civicrm_spgateway_addpadding($string, $blocksize = 32) {
  $len = strlen($string);
  $pad = $blocksize - ($len % $blocksize);
  $string .= str_repeat(chr($pad), $pad); 
  return $string;
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

