<?php

/**
 * Checkout functions
 *
 * We place the code into drupal inorder to seperate the civicrm.
 * Called by Civicrm doTransferCheckout of Class Payment_ESUNBANK
 * 
 * @param $vars 
 * @param $component 
 * @param $payment_processor 
 */
function civicrm_esunbank_do_transfer_checkout(&$vars, &$component, &$payment_processor){
  // First, we insert every contribution into record. After this, we'll use update for the record.
  $record = array('cid' => $vars['contributionID'], 'created' => time());
  drupal_write_record("civicrm_contribution_esunbank", $record);
  $form_key = $component == 'event' ? 'CRM_Event_Controller_Registration_'.$vars['qfKey'] : 'CRM_Contribute_Controller_Contribution_'.$vars['qfKey'];
  $civi_base_url = $component == 'event' ? 'civicrm/event/register' : 'civicrm/contribute/transact';

  // Generate esunbank payment serial
  // the order is matter, see function esunbank_api
  $p['code'] = $payment_processor['user_name'];
  $p['number'] = $vars['contributionID'];
  $p['length'] = $payment_processor['password'];
  $p['price'] = $vars['amount'];
  $p['timestamp'] = strtotime('+'.CIVICRM_ESUNBANK_EXPIRED_DAY.' days');
  $p['store'] = $payment_processor['subject'];
  $p['type'] = $payment_processor['signature'];

  // contact info
  $p['user'] = CRM_Core_DAO::getFieldValue('CRM_Contact_DAO_Contact', $vars['contactID'], 'display_name');
  $p['item'] = $vars['item_name'];

  // order info
  $p['order_number'] = '';
  $p['order_date'] = '';

  // Generate invoice in web and pdf
  $tpl = array();
  $tpl['invoice_link'] = esunbank_api_link($p);
  $tpl['invoice_html'] = esunbank_api($p);
  $tpl['invoice_html'] = civicrm_esunbank_convert_css($tpl['invoice_html']);
  
  // now process contribution to save some default value
  $contrib_params = array( 'id' => $vars['contributionID'] );
  $contrib_values = $contrib_ids = array();
  CRM_Contribute_BAO_Contribution::getValues($contrib_params, $contrib_values, $contrib_ids);
  $contrib_values['trxn_id'] = $p['order_number'] ? $p['order_number'] : $p['serial']; // we use payment serial to be trxn_id
  $contrib_values['is_pay_later'] = 1;
  $contrib_values['payment_instrument_id'] = CRM_Core_DAO::getFieldValue('CRM_Core_DAO_OptionValue', 'Esunbank', 'value', 'name');
  $contribution =& CRM_Contribute_BAO_Contribution::create($contrib_values, $contrib_ids);

  // Inject some in quickform sessions
  // Special hacking for display trxn_id after thank you page.
  $_SESSION['CiviCRM'][$form_key]['params']['trxn_id'] = $contribution->trxn_id;
  $_SESSION['CiviCRM'][$form_key]['params']['is_pay_later'] = 1;
  if($component == 'event'){
    $values =& $_SESSION['CiviCRM'][$form_key]['values']['event'];
  }
  else{
    $values =& $_SESSION['CiviCRM'][$form_key]['values'];
  }
  /*
  $values['pay_later_text'] = '';
  $values['pay_later_receipt'] = '';
  $values['receipt_text'] = '';
  */
  // TODO: Send payment invoice to contact
  if(empty($_SESSION['CiviCRM'][$form_key]['params']['sent'])){
    $js = '<script type="text/javascript">
$(document).ready(function(){
  $(".colorbox-load").click();
});
</script>';
    $values['thankyou_text'] .= '<div>如果您沒有看到繳費單視窗，點此<strong>開啟'.l('付款單', $tpl['invoice_link'].'?width=900&height=600&iframe=true', array('attributes' => array('class' => 'colorbox-load', 'target' => '_blank'))).'</strong></div>'.$js;
    $email = $vars['email-5'];

    // prepare template parameter

    // default from_email
    $from_emails = array();
    CRM_Core_OptionValue::getValues(array('name' => 'from_email_address'), $from_emails);
    foreach($from_emails as $id => $v){
      if($v['is_default'] == 1){
        $from_default = $v['label'];
        break;
      }
    }
    if($component == 'event'){
      // after saved record, we should modify the status of participant / membership
      $participant_id = CRM_Core_DAO::getFieldValue('CRM_Event_DAO_ParticipantPayment', $vars['contributionID'], 'participant_id', 'contribution_id');
      if($participant_id){
        $pstatus = CRM_Event_PseudoConstant::participantStatus();
        if($new_pstatus = array_search('Pending from pay later', $pstatus)){
          CRM_Core_DAO::setFieldValue('CRM_Event_DAO_Participant', $participant_id, 'status_id', $new_pstatus, 'id');
        }
      }

      if($values['event']['confirm_from_name'] && $values['event']['confirm_from_email']){
        $from = $values['event']['confirm_from_name']. '<'.$values['event']['confirm_from_email'];
      }
    }
    else{ // contribute
      if($values['receipt_from_name'] && $values['receipt_from_email']){
        $from = $values['receipt_from_name']. ' <'.$values['receipt_from_email'].'>';
      }
    }
    $send_params = array(
      'from' => $from ? $from : $from_default,
      'toName' => $p['user'],
      'toEmail' => $email,
      'groupName' => 'msg_tpl_workflow_invoice',
      'valueName' => 'invoice_esunbank',
      'contactId' => $vars['contactID'],
      'tplParams' => $tpl,
      'PDFFilename' => 'invoice.pdf',
    );
    CRM_Core_BAO_MessageTemplates::sendTemplate($send_params);
    // prevent re-send invoice
    $_SESSION['CiviCRM'][$form_key]['params']['sent'] = true;;
  }

  require_once 'CRM/Core/Session.php';
  CRM_Core_Session::storeSessionObjects( );

  // redirect to thank-you page
  drupal_goto($civi_base_url, "_qf_ThankYou_display=true&qfKey={$vars['qfKey']}");
}

function civicrm_esunbank_convert_css($html){
  preg_match('/text\/css.*href="([^"].+\.css)"/i', $html, $matches);
  $css = file_get_contents($matches[1]);
  $html = mb_convert_encoding($html, 'HTML-ENTITIES', 'UTF-8');

  module_load_include('inc', 'civicrm_esunbank', 'css_to_inline_styles');
  $inline = new CSSToInlineStyles($html, $css); //class?
  $inline->setEncoding('UTF-8');
  $html = $inline->convert();

  /*
  $html = preg_replace('/<\!DOC[^>]+>/', '', $html);
  $html = preg_replace('/<\/?html>/', '', $html);
  $html = preg_replace('/<\/?body>/', '', $html);
  */
  return $html;
}
