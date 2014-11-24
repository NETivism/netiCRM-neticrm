<?php

function civicrm_ebill_ba_do_transfer_checkout(&$vars, $component, &$payment_processor, $is_test){
  $instrument_id = $vars['civicrm_instrument_id'] ? $vars['civicrm_instrument_id'] : variable_get('civicrm_ebill_ba_instrument', 0);

  // now process contribution to save some default value
  $contrib_params = array( 'id' => $vars['contributionID'] );
  $contrib_values = $contrib_ids = array();
  CRM_Contribute_BAO_Contribution::getValues($contrib_params, $contrib_values, $contrib_ids);
  if($vars['civicrm_instrument_id']){
    $contrib_values['payment_instrument_id'] = $vars['civicrm_instrument_id'];
  }
  $contrib_values['is_pay_later'] = $is_pay_later;
  $contrib_values['trxn_id'] = _civicrm_ebill_ba_trxn_id($is_test, $vars['contributionID']);
  $contribution =& CRM_Contribute_BAO_Contribution::create($contrib_values, $contrib_ids);
  if($contribution->id){
    $record = array('cid' => $contribution->id);
    drupal_write_record("civicrm_contribution_ebillba", $record);
  }


  // Inject in quickform sessions
  // Special hacking for display trxn_id after thank you page.
  $_SESSION['CiviCRM'][$form_key]['params']['trxn_id'] = $contribution->trxn_id;
  $vars['trxn_id'] = $contribution->trxn_id;

  $amount = $vars['currencyID'] == 'TWD' && strstr($vars['amount'], '.') ? substr($vars['amount'], 0, strpos($vars['amount'],'.')) : $vars['amount'];
  $arguments = array(
    'billerid' => $payment_processor['user_name'],
    'feetype' => $payment_processor['password'],
    'payment_type' => $payment_processor['signature'],
    'notice_no' => $vars['trxn_id'],
    'payment_deadline' => date('Ymd'),
    'levying_office' => '',
    'amount' => $amount,
  );

  print _civicrm_ebill_ba_form_redirect($arguments, $payment_processor);
  // move things to CiviCRM cache as needed
  CRM_Utils_System::civiExit();
}

function _civicrm_ebill_ba_form_redirect($redirect_vars, $payment_processor){
  header('Pragma: no-cache');
  header('Cache-Control: no-store, no-cache, must-revalidate');
  header('Expires: 0');

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
