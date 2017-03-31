<?php
civicrm_initialize();
require_once 'CRM/Core/Payment/BaseIPN.php';
date_default_timezone_set("Asia/Taipei");

/**
 * Helper function for receive pingback from payment server
 */
function civicrm_spgateway_ipn($instrument, $post = NULL, $get = NULL, $print = TRUE){
  // detect variables
  $post = !empty($post) ? $post : $_POST;
  $get = !empty($get) ? $get : $_GET;

  // detect variables
  if(empty($post)){
    watchdog("civicrm_spgateway", "Could not find POST data from payment server");
    drupal_not_found();
  }
  else{
    $component = $get['module'];
    if(!empty($component)){
      $ipn = new CRM_Core_Payment_SPGATEWAYIPN($post, $get);
      $result = $ipn->main($component, $instrument);
      if(!empty($result) && $print){
        echo $result;
      }
      else{
        return $result;
      }
    }
    else{
      watchdog('civicrm_spgateway', "Could not get module name from request url");
      drupal_not_found();
    }
  }
}

class CRM_Core_Payment_SPGATEWAYIPN extends CRM_Core_Payment_BaseIPN {
  static $_payment_processor = NULL;
  static $_input = NULL;
  public $_post = NULL;
  public $_get = NULL;
  function __construct($post, $get) {
    parent::__construct();
    $this->_post = $post;
    $this->_get = $get;
  }

  function main($component = 'contribute', $instrument){
    require_once 'CRM/Utils/Request.php';
    $objects = $ids = $input = array();
    $this->getIds($ids, $component);
    if(empty($ids['contributionRecur'])){
      $recur = FALSE;
      // get the contribution and contact ids from the GET params
      $input = _civicrm_spgateway_post_decode($this->_post);
      civicrm_spgateway_record($ids['contribution'], $this->_post);
    }
    else{
      $recur = TRUE;
      $ppid = $ids['paymentProcessor'];
      $is_test = CRM_Core_DAO::singleValueQuery("SELECT is_test FROM civicrm_payment_processor WHERE id = $ppid");
      $payment_processors = CRM_Core_BAO_PaymentProcessor::getPayment($ppid, $is_test?'test':'live');
      $this->_post = _civicrm_spgateway_recur_decrypt($this->_post['Period'],$payment_processors);
      $input = _civicrm_spgateway_post_decode($this->_post);
      // we will save record later if this is recurring after second times.
      if(empty($input['AlreadyTimes'])){
        // First time recurring
        civicrm_spgateway_record($ids['contribution'], $this->_post);
      }
    }
    $input['component'] = $component;
    $qfKey = CRM_Utils_Array::value('qfKey', $this->_get);
    $civi_base_url = $component == 'event' ? 'civicrm/event/register' : 'civicrm/contribute/transact';

    // now, retrieve full object by validateData, or false fallback
    if ( ! $this->validateData( $input, $ids, $objects ) ) {
      return false;
    }

    // set global variable for paymentProcessor
    self::$_payment_processor =& $objects['paymentProcessor'];
    self::$_input = $input;

    if($objects['contribution']->contribution_status_id == 1 && empty($input['AlreadyTimes'])){
      // already completed, skip
      return '1|OK';
    }
    else{
      // start validation
      require_once 'CRM/Core/Transaction.php';
      $transaction = new CRM_Core_Transaction();
      $note = '';
      if( $this->validateOthers($input, $ids, $objects, $transaction, $note) ){
        $contribution =& $objects['contribution'];
        if(empty($contribution->receive_date)){
          if(!empty($input['PayTime'])){
            $contribution->receive_date = date('YmdHis', strtotime($input['PayTime']));
          }elseif(!empty($input['AuthTime'])){
            $contribution->receive_date = date('YmdHis', strtotime($input['AuthTime']));
          }elseif(!empty($input['AuthDate'])){
            $contribution->receive_date = date('YmdHis', strtotime($input['AuthDate']));
          }else{
            $contribution->receive_date = date('YmdHis');
          }
        }

        // assign trxn_id before complete transaction
        $input['trxn_id'] = $objects['contribution']->trxn_id;
        $this->completeTransaction( $input, $ids, $objects, $transaction, $recur );
        $note .= ts('Completed')."\n";
        $this->addNote($note, $contribution);
        return '1|OK';
      }
      else{
        $note .= ts('Failed')."\n";
        $note .= ts("Payment Information").": ".ts("Failed").' - '.$input['RtnMsg']."({$input['RtnCode']})";
        $this->addNote($note, $objects['contribution']);
      }
    }
    
    // error stage: doesn't goto and not the background posturl
    // never for front-end user.
  }

  function getIds( &$ids , $component){
    $ids['contact'] = CRM_Utils_Array::value('contact_id', $this->_get);
    $ids['contribution'] = CRM_Utils_Array::value('cid', $this->_get);
    if ( $component == 'event' ) {
      $ids['event'] = CRM_Utils_Array::value('eid', $this->_get);
      $ids['participant'] = CRM_Utils_Array::value('pid', $this->_get);
    }
    else {
      $ids['membership'] = CRM_Utils_Array::value('mid', $this->_get);
      $ids['contributionRecur'] = CRM_Utils_Array::value('crid', $this->_get);
      $ids['contributionPage'] = CRM_Utils_Array::value('cpid', $this->_get);
      $ids['related_contact'] = CRM_Utils_Array::value('rid', $this->_get);
      $ids['onbehalf_dupe_alert'] = CRM_Utils_Array::value('onbehalf_dupe_alert', $this->_get);
      $ids['paymentProcessor'] = CRM_Utils_Array::value('ppid', $this->_get);
    }
  }

  function validateOthers( &$input, &$ids, &$objects, &$transaction, &$note){
    $contribution = &$objects['contribution'];
    $pass = TRUE;
    $valid_value = array();
    if(!empty($input['MerchantOrderNo'])){
      $valid_value['MerchantOrderNo'] = $input['MerchantOrderNo'];
    }else{
      $valid_value['MerchantOrderNo'] = $input['MerOrderNo'];
    }
    if($ids['contributionRecur']){
      if(!empty($input['AuthAmt'])){
        // above second period
        $valid_value['Amt'] = $input['AuthAmt'];
      }else{
        // first period
        $valid_value['Amt'] = $input['PeriodAmt'];
      }
    }else{
      $valid_value['Amt'] = $input['Amt'];
      $valid_value['CheckCode'] = $input['CheckCode'];
    }
    
    // check contribution id matches
    if ( $contribution->trxn_id != $valid_value['MerchantOrderNo'] ) {
      watchdog("civicrm_spgateway", "OrderNumber values doesn't match between database and IPN request. {$contribution->trxn_id} : {$valid_value['MerchantOrderNo']} " );
      $note .= ts("Failuare: OrderNumber values doesn't match between database and IPN request. {$contribution->trxn_id} : {$valid_value['MerchantOrderNo']}")."\n";
      $pass = FALSE;
    } 

    // check amount
    if ( round($contribution->total_amount) != $valid_value['Amt'] && $input['Status'] == 'SUCCESS' ) {
      watchdog("civicrm_spgateway", "Amount values dont match between database and IPN request. {$contribution->trxn_id}-{$input['AlreadyTimes']} : {$valid_value['Amt']}" );
      $note .= ts("Failuare: Amount values dont match between database and IPN request. {$contribution->trxn_id}-{$input['AlreadyTimes']} : {$valid_value['Amt']}")."\n";
      $pass = FALSE;
    }

    // spgateway validation
    // only validate this when not test.
    if(!empty($valid_value['CheckCode'])){
      $check_code = _civicrm_spgateway_checkcode($input, self::$_payment_processor);
      if(strtolower($valid_value['CheckCode']) != strtolower($check_code)) {
        $note .= ts("Failuare: CheckCode not match. Contact system admin.")."\n";
        watchdog("civicrm_spgateway", "Failuare: CheckCode not match. Should be '{$check_code}', but '{$input['CheckCode']}' displayed.");
        $pass = FALSE;
      }
    }

    // recurring validation
    // certainly this is recurring contribution
    if($ids['contributionRecur']){
      $recur = &$objects['contributionRecur'];
      $params = $null = array();
      // see if we are first time, if not first time, save new contribution
      // 6 - expired
      // 5 - processing
      // 4 - fail
      // 3 - canceled
      // 2 - pending
      // 1 - completed

      // not the first time (PeriodReturnURL)
      if(!empty($input['AlreadyTimes'])){
        $trxn_id = $input['OrderNo'];
        if($input['Status'] != 'SUCCESS'){
          $contribution->contribution_status_id = 4; // Failed
          $c = $this->copyContribution($contribution, $ids['contributionRecur'], $trxn_id);
        }
        else{
          $contribution->contribution_status_id = 1; // Completed
          $c = $this->copyContribution($contribution, $ids['contributionRecur'], $trxn_id);
        }
        if(!empty($c)){
          unset($objects['contribution']);
          civicrm_spgateway_record($c->id, $this->_post);
          $objects['contribution'] = $c;
          if($input['AlreadyTimes'] == $input['TotalTimes']){
            $recur_param = array(
              'id' => $ids['contributionRecur'],
              'modified_date' => date('YmdHis'),
              'end_date' => date('YmdHis'),
              'contribution_status_id' => 1, // completed
            );
            CRM_Contribute_BAO_ContributionRecur::add($recur_param, $null);
          }
        }
      }
      else{
        // is first time
        if($input['Status'] == 'SUCCESS'){
          $params['id'] = $recur->id;
          $params['start_date'] = date('YmdHis', strtotime($input['AuthTime']));
          $params['contribution_status_id'] = 5; // from pending to processing
          $params['modified_date'] = date('YmdHis');
          $params['trxn_id'] = $input['PeriodNo'];
          CRM_Contribute_BAO_ContributionRecur::add($params, $null);
        }
        else{
          CRM_Contribute_BAO_ContributionRecur::cancelRecurContribution($recur->id, CRM_Core_DAO::$_nullObject, 4);
        }
      }
    }
      
    // process fail response
    if($input['Status'] != 'SUCCESS' && $pass){
      $response_code = $input['Status'];
      $response_msg = $input['RtnMsg'];
      $note .= "Failed $response_code: $response_msg";
      $this->failed( $objects, $transaction );
      $pass = FALSE;
    }

    return $pass;
  }

  function copyContribution(&$contrib, $rid, $trxn_id){
    if(is_object($contrib)){
      $c = clone $contrib;
      unset($c->id);
      unset($c->receive_date);
      unset($c->invoice_id);
      unset($c->receipt_id);
      $c->contribution_status_id = 2;
      $c->trxn_id = $trxn_id;
      $time = strtotime(self::$_input['AuthDate']);
      $c->receive_date = date('YmdHis', $time);
      $c->created_date = date('YmdHis');
      $c->save();
      CRM_Contribute_BAO_ContributionRecur::syncContribute($rid, $c->id);
      return $c;
    }
    return FALSE;
  }

  function addNote($note, &$contribution){
    require_once 'CRM/Core/BAO/Note.php';
    $note = date("Y/m/d H:i:s"). ts("Transaction record").": \n".$note."\n===============================\n";
    $note_exists = CRM_Core_BAO_Note::getNote( $contribution->id, 'civicrm_contribution' );
    if(count($note_exists)){
      $note_id = array( 'id' => reset(array_keys($note_exists)) );
      $note = $note . reset($note_exists);
    }
    else{
      $note_id = NULL;
    }
    
    $noteParams = array(
      'entity_table'  => 'civicrm_contribution',
      'note'          => $note,
      'entity_id'     => $contribution->id,
      'contact_id'    => $contribution->contact_id,
      'modified_date' => date('Ymd')
    );
    CRM_Core_BAO_Note::add( $noteParams, $note_id );
  }

}
