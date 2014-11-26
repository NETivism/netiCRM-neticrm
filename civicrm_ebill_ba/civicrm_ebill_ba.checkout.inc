<?php

function civicrm_ebill_ba_do_transfer_checkout(&$vars, $component, &$payment_processor, $is_test){
  // these will prevent double submission in ie 10 and below
  if(empty($vars['contributionID']) || !empty($_SESSION['ebill_ba']['submitted'])){
    unset($_SESSION['ebill_ba']);
    CRM_Utils_System::civiExit();
  }
  $instrument_id = $vars['civicrm_instrument_id'] ? $vars['civicrm_instrument_id'] : variable_get('civicrm_ebill_ba_instrument', 0);

  // use db_merge to prevent duplicate error.
  db_merge("civicrm_contribution_ebillba")->key(array('cid' => $vars['contributionID']))->execute();

  // now process contribution to save some default value
  $contrib_params = array( 'id' => $vars['contributionID'] );
  $contrib_values = $contrib_ids = array();
  CRM_Contribute_BAO_Contribution::getValues($contrib_params, $contrib_values, $contrib_ids);
  $contrib_values['payment_instrument_id'] = $instrument_id;
  $contrib_values['is_pay_later'] = $is_pay_later;
  $contrib_values['trxn_id'] = _civicrm_ebill_ba_trxn_id($is_test, $vars['contributionID']);
  $contribution = CRM_Contribute_BAO_Contribution::create($contrib_values, $contrib_ids);

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

  $_SESSION['ebill_ba']['submitted'] = TRUE;
  print _civicrm_ebill_ba_form_redirect($arguments, $payment_processor);
  CRM_Utils_System::civiExit();
}

function _civicrm_ebill_ba_form_redirect($redirect_vars, $payment_processor){
  header('Pragma: no-cache');
  header('Cache-Control: private, no-store, no-cache, must-revalidate, max-age=0');
  header('Expires: 0');

  $js = 'document.forms.redirect.submit();';
  $o .= '<form action="'.$payment_processor['url_site'].'" name="redirect" method="post" id="redirect-form">';
  foreach($redirect_vars as $k=>$p){
    if($k[0] != '#'){
      $o .= '<input type="hidden" name="'.$k.'" value="'.$p.'" />';
    }
  }
  $o .= '</form>';
  // below ie 10, browser doesn't handling form submission then back history very well
  // we needs meta tag to prevent page cache and form submit twice.
  return '
<!DOCTYPE html> 
<html dir="ltr" lang="zh-TW">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<!--[if lt IE 10]>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<![endif]-->
</head>
<body>
  '.$o.'
  <script type="text/javascript">
  '.$js.'
  </script>
</body>
<head>
<!--[if lt IE 10]>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<![endif]-->
</head>
<html>
';
}
