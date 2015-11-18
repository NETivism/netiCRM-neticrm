<?php
civicrm_initialize();
require_once 'CRM/Core/Payment/BaseIPN.php';
date_default_timezone_set("Asia/Taipei");

/**
 * Helper function for receive pingback from payment server
 */
function civicrm_allpay_ipn($instrument, $post = NULL, $get = NULL, $print = TRUE){
      
  // detect variables
  $post = !empty($post) ? $post : $_POST;
  $get = !empty($get) ? $get : $_GET;

  // detect variables
  if(empty($post)){
    watchdog("civicrm_allpay", "Could not find POST data from payment server");
    drupal_not_found();
  }
  else{
    $component = $get['module'];
    if(!empty($component)){
      $ipn = new CRM_Core_Payment_ALLPAYIPN($post, $get);
      $result = $ipn->main($component, $instrument);
      if(!empty($result) && $print){
        echo $result;
      }
      else{
        return $result;
      }
    }
    else{
      watchdog('civicrm_allpay', "Could not get module name from request url");
      drupal_not_found();
    }
  }
}

class CRM_Core_Payment_ALLPAYIPN extends CRM_Core_Payment_BaseIPN {
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

    // get the contribution and contact ids from the GET params
    require_once 'CRM/Utils/Request.php';
    $objects = $ids = $input = array();
    $input = $this->_post;
    $this->getIds($ids, $component);
    $input['component'] = $component;
    $qfKey = CRM_Utils_Array::value('qfKey', $this->_get);
    $civi_base_url = $component == 'event' ? 'civicrm/event/register' : 'civicrm/contribute/transact';

    if(empty($this->_get['is_recur'])){
      civicrm_allpay_record($ids['contribution'], $this->_post);
    }
    if(empty($ids['contributionRecur'])){
      // we will save record later if this is recurring
      $recur = FALSE;
    }
    else{
      $recur = TRUE;
    }

    // now, retrieve full object by validateData, or false fallback
    if ( ! $this->validateData( $input, $ids, $objects ) ) {
      return false;
    }


    // set global variable for paymentProcessor
    self::$_payment_processor =& $objects['paymentProcessor'];
    self::$_input = $input;

    if($objects['contribution']->contribution_status_id == 1 && empty($this->_get['is_recur'])){
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
          $contribution->receive_date = $input['PaymentDate'] ? date('YmdHis', strtotime($input['PaymentDate'])) : date('YmdHis');
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
    }
  }

  function validateOthers( &$input, &$ids, &$objects, &$transaction, &$note){
    $contribution = &$objects['contribution'];
    $pass = TRUE;
    
    // check contribution id matches
    if ( $contribution->trxn_id != $input['MerchantTradeNo'] ) {
      watchdog("civicrm_allpay", "OrderNumber values doesn't match between database and IPN request. {$contribution->trxn_id} : {$input['MerchantTradeNo']} " );
      $note .= ts("Failuare: OrderNumber values doesn't match between database and IPN request. {$contribution->trxn_id} : {$input['MerchantTradeNo']}")."\n";
      $pass = FALSE;
    } 

    // check amount
    $amount = $input['TradeAmt'] ? $input['TradeAmt'] : $input['Amount'];
    if ( round($contribution->total_amount) != $amount && $input['RtnCode'] == 1 ) {
      watchdog("civicrm_allpay", "Amount values dont match between database and IPN request. {$contribution->trxn_id}-{$input['Gwsr']} : {$input['amount']}" );
      $note .= ts("Failuare: Amount values dont match between database and IPN request. {$contribution->trxn_id}-{$input['Gwsr']} : {$input['amount']}")."\n";
      $pass = FALSE;
    }

    // allpay validation
    // only validate this when not test.
    if(!empty($input['CheckMacValue'])){
      $mac = _civicrm_allpay_checkmacvalue($this->_post, self::$_payment_processor);
      if(strtolower($input['CheckMacValue']) != strtolower($mac)) {
        $note .= ts("Failuare: CheckMacValue not match. Contact system admin.")."\n";
        watchdog("civicrm_allpay", "Failuare: CheckMacValue not match. Should be '{$mac}', but '{$input['CheckMacValue']}' displayed.");
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
      if($this->_get['is_recur']){
        $trxn_id = _civicrm_allpay_recur_trxn($input['MerchantTradeNo'], $input['Gwsr']);
        $local_succ_times = CRM_Core_DAO::singleValueQuery("SELECT count(*) FROM civicrm_contribution WHERE contribution_recur_id = %1 AND contribution_status_id = 1", array(1 => array($recur->id, 'Integer')));
        if($input['RtnCode'] != 1){
          $contribution->contribution_status_id = 4; // Failed
          $c = $this->copyContribution($contribution, $ids['contributionRecur'], $trxn_id);
        }
        elseif($input['RtnCode'] == 1 && $local_succ_times < $input['TotalSuccessTimes']){
          $contribution->contribution_status_id = 1; // Completed
          $c = $this->copyContribution($contribution, $ids['contributionRecur'], $trxn_id);
        }
        else{
          // possible duplicate notify
          watchdog('civicrm_allpay', 'Possible duplicate notify, detect from TotalSuccessTimes: '.$input['TotalSuccessTimes']);
          $note .= "Possible duplicate notify. Already success $local_succ_times, but notify us only {$input['TotalSuccessTimes']} success execution.";
          $pass = FALSE;
        }
        if(!empty($c)){
          unset($objects['contribution']);
          civicrm_allpay_record($c->id, $this->_post);
          $objects['contribution'] = $c;
          
          // update recurring object
          // never end if TotalSuccessTimes not excceed the ExecTimes
          if($input['TotalSuccessTimes'] == $recur->installments){
            $params['id'] = $recur->id;
            $params['modified_date'] = date('YmdHis');
            $params['end_date'] = date('YmdHis');
            $params['contribution_status_id'] = 1; // completed
            CRM_Contribute_BAO_ContributionRecur::add($params, $null);
          }
        }
      }
      else{
        // is first time
        if($input['RtnCode'] == 1){
          $params['id'] = $recur->id;
          $params['start_date'] = date('YmdHis', strtotime($input['PaymentDate']));
          $params['contribution_status_id'] = 5; // from pending to processing
          $params['modified_date'] = date('YmdHis');
          CRM_Contribute_BAO_ContributionRecur::add($params, $null);
        }
        else{
          CRM_Contribute_BAO_ContributionRecur::cancelRecurContribution($recur->id, CRM_Core_DAO::$_nullObject, 4);
        }
      }
    }
      
    // process fail response
    if($input['RtnCode'] != 1 && $pass){
      $response_code = $input['RtnCode'];
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
      $c->trxn_id = $trxn_id;
      $time = strtotime(self::$_input['ProcessDate']);
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
