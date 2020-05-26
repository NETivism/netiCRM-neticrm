<?php

define("NEWEB_UPLOAD", 1);
define("NEWEB_DOWNLOAD", 0);
define("NEWEB_DEBUG", 1);

/**
 * Sync last 30 days payments
 */
function civicrm_neweb_recur_sync($is_test, $start = 'last month', $pid){
  civicrm_initialize();
  require_once "CRM/Core/DAO.php";
  require_once 'CRM/Core/Payment.php'; 
  require_once 'CRM/Core/BAO/PaymentProcessor.php';
  require_once 'CRM/Core/Payment/NewebIPN.php';
  require_once 'CRM/Contribute/DAO/Contribution.php';
  require_once 'CRM/Utils/Array.php';
  $last_month = strtotime($start);
  $now = time();
  $p = new CRM_Core_BAO_PaymentProcessor();
  if($is_test){
    $ftp = variable_get("civicrm_neweb_ftp_test_".$pid, array());
    $payment = $p->getPayment($pid,'test');
  }
  else{
    $ftp = variable_get("civicrm_neweb_ftp_".$pid, array());
    $payment = $p->getPayment($pid,'live');
  }
  $store_name = $payment['user_name'];
  $newebpath = _civicrm_neweb_path($is_test);
  $list = _civicrm_neweb_sftp('list', '', 'output', $ftp['ftp_host'], $ftp['ftp_user'], $ftp['ftp_password']);

  if(!is_array($list)){
    _civicrm_neweb_error('Sync error. Can\'t get remote file list.', $is_test);
  }
  $xx = 0;
  while($last_month < $now){
  //  if($xx >= 5) break;
    $date = date('Ymd', $last_month);
    $out = "RP_{$store_name}_{$date}".'.out';
    $log = "RP_Trans_{$store_name}_{$date}".'.log';
    if(!is_file($newebpath."/".$out) && array_key_exists($out, $list)){
      // process the out file
      civicrm_neweb_process_response($is_test, $last_month);
    }
    if(!is_file($newebpath."/".$log) && array_key_exists($log, $list)){
      // process transaction 
      civicrm_neweb_process_transaction($is_test, $last_month);
      $xx++;
    }
    $last_month += 86400;
  }
  /*
  foreach($list as $filename => $meta){
    $out = "RP_{$store_name}_".'.out';
    $log = "RP_Trans_{$store_name}_".'.log';
    if(!is_file($newebpath."/".$out)){
      print '<pre>'.$filename.'</pre>';
    }
    if(!is_file($newebpath."/".$log)){
      print '<pre>'.$filename.'</pre>';
    }
  }
  */

}

function civicrm_neweb_process_response($is_test, $now = NULL, $pid){
  $now = $now ? $now : time();

  civicrm_initialize();
  require_once "CRM/Core/DAO.php";
  require_once 'CRM/Core/Payment.php'; 
  require_once 'CRM/Core/BAO/PaymentProcessor.php';
  require_once 'CRM/Core/Payment/NewebIPN.php';
  require_once 'CRM/Contribute/DAO/Contribution.php';
  require_once 'CRM/Contribute/DAO/ContributionRecur.php';
  require_once 'CRM/Contribute/BAO/ContributionRecur.php';
  require_once 'CRM/Utils/Array.php';
  require_once 'CRM/Contribute/BAO/ContributionPage.php';

  $p = new CRM_Core_BAO_PaymentProcessor();
  if($is_test){
    $payment = $p->getPayment($pid,'test');
  }
  else{
    $payment = $p->getPayment($pid,'live');
  }
  $newebpath = _civicrm_neweb_path($is_test);
  $filename = 'RP_'.$payment['user_name'].'_'.date('Ymd', $now).'.out';
  if(is_file($newebpath."/".$filename)){
    $response_str = file_get_contents($newebpath."/".$filename);
  }
  else{
    $response_str = _civicrm_neweb_connect($pid, $payment['user_name'], $newebpath.'/'.$filename, NEWEB_DOWNLOAD, $is_test);
  }

  if($response_str === false){
    // error when download files
    $error = "error in civicrm_neweb_process_response: Can't download files. ";
    _civicrm_neweb_error($error, $is_test);
    return FALSE;
  }
  elseif($response_str == -1){
    // $error = "error in civicrm_neweb_process_response: No response today.";
    // _civicrm_neweb_error($error, $is_test);
    return -1;
  }
  elseif($response_str){
    $response = _civicrm_neweb_decodeline($response_str, 'response');

    foreach($response as $k => $r){
      // find out recur
      $rid = (is_numeric($r['REFNUMBER']) && !empty($r['REFNUMBER'])) ? $r['REFNUMBER'] : 0;
      $recur = new CRM_Contribute_DAO_ContributionRecur();
      $recur->id = $rid;

      if(!$recur->find(true)){
        // no this recurring contribution
        unset($recur->id);
      }
      else{
        $contribution_id = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_contribution c WHERE c.contribution_recur_id = {$recur->id} ORDER BY c.id ASC");
        $contrib = new CRM_Contribute_DAO_Contribution();
        $contrib->id = $contribution_id;
        $sql = "UPDATE civicrm_contribution_neweb_recur SET response=$now WHERE recur_id = {$recur->id}";
        CRM_Core_DAO::executeQuery($sql);
        $action = strtolower($r['ACTION']);
        $params = $ids = array();
        $params['id'] = $recur->id;
        switch($action){
          case 'new':
            if($r['RESPONSE'] == '0' || $r['RESPONSE'] == '-11'){
            /*
              if ( $contrib->find( true ) ) { 
                // send notification email of recurring contribution
                CRM_Contribute_BAO_ContributionPage::recurringNotify( 'START', $recur->contact_id, $contrib->contribution_page_id, $recur);
              }
            */
              // Process
              $params['contribution_status_id'] = 5;
            }
            else{
              $params['contribution_status_id'] = 4; // failed
              $error = "error : recur_id ". $recur->id . " RESPONSE is ". $r['RESPONSE'];
              _civicrm_neweb_error($error, $is_test);
            }
            $params['modified_date'] = date('YmdHis');
            CRM_Contribute_BAO_ContributionRecur::add( $params, $ids );
            break;
          case 'delete':
            if($r['RESPONSE'] == '0' || $r['RESPONSE'] == '-31'){
              switch($recur->contribution_status_id){
                case 2:
                  // status 2 and has cancel_date -> 3
                  if(empty($recur->cancel_date)){
                    $error = "error : recur_id ". $recur->id . " 's cancel date doesn't set. current value is ". $recur->cancel_date;
                    _civicrm_neweb_error($error, $is_test);
                  }else{
                    $params['contribution_status_id'] = 3;
                  }
                  break;
                case 6:
                  // status 6 (expired) -> 1 (finished) 
                  $params['contribution_status_id'] = 1;
                  break;
                default:
                  $error = "error : recur_id ". $recur->id . " status doesn't set to pending(2) or expired(6), current is ". $recur->contribution_status_id;
                  _civicrm_neweb_error($error, $is_test);
                  break;
              }
            }
            elseif($r['RESPONSE'] == '-32'){
              // RESPONSE == -32 is Failed
              $params['contribution_status_id'] = 4;
              $error = "error : recur_id ". $recur->id . " RESPONSE is ". $r['RESPONSE'];
              _civicrm_neweb_error($error, $is_test);
            }
            $params['modified_date'] = date('YmdHis');
            CRM_Contribute_BAO_ContributionRecur::add( $params, $ids );
            break;
        }
      }

/*
      // fail
      $error = "error in civicrm_neweb_process_response:". $r['REFNUMBER'] . ":" . $r['RESPONSE'];
      _civicrm_neweb_error($error, $is_test);
*/
    }

    return TRUE;
  }
}

function civicrm_neweb_process_transaction($is_test, $now = NULL, $pid, $force_update = FALSE){
  $now = $now ? $now : time();
  civicrm_initialize();
  require_once "CRM/Core/DAO.php";
  require_once 'CRM/Core/Payment.php'; 
  require_once 'CRM/Core/BAO/PaymentProcessor.php';
  require_once 'CRM/Core/Payment/NewebIPN.php';
  require_once 'CRM/Contribute/DAO/Contribution.php';
  require_once 'CRM/Contribute/DAO/ContributionRecur.php';
  require_once 'CRM/Contribute/BAO/ContributionRecur.php';
  require_once 'CRM/Utils/Array.php';

  module_load_include("inc", 'civicrm_neweb', 'civicrm_neweb.ipn');

  $test = $is_test ? '_test' : '';
  $line_counter = variable_get('civicrm_neweb_process_transaction_count_'.$pid.$test.'_'.date('Ymd', $now), -1);

  $p = new CRM_Core_BAO_PaymentProcessor();
  if($is_test){
    $payment = $p->getPayment($pid,'test');
  }
  else{
    $payment = $p->getPayment($pid,'live');
  }
  $newebpath = _civicrm_neweb_path($is_test);

  $filename = 'RP_Trans_'.$payment['user_name'].'_'.date('Ymd', $now).'.log';
  if(is_file($newebpath."/".$filename)){
    $transaction_str  = file_get_contents($newebpath."/".$filename);
  }
  else{
    $transaction_str = _civicrm_neweb_connect($pid, $payment['user_name'], $newebpath.'/'.$filename, NEWEB_DOWNLOAD, $is_test);
  }

  if($transaction_str === false){
    // error when download files
    $error = "In civicrm_neweb_process_transaction: Today have nothing to download. ";
    _civicrm_neweb_error($error, $is_test, WATCHDOG_NOTICE);
  }
  elseif($transaction_str == -1){
    // have file but no response
    // $error = "error in civicrm_neweb_process_response: No response today.";
    // _civicrm_neweb_error($error, $is_test);
    return -1;
  }
  elseif($transaction_str){
    $trans = _civicrm_neweb_decodeline($transaction_str,'transaction');

    $times_counter = 0;
    $times_limit = 100;

    // compatibility php 5.2
    if(function_exists('gc_enable')){
      gc_enable();
    }
    foreach($trans as $k => $t){
      $err = '';
      if(empty($t) || $k <= $line_counter) continue;
      $times_counter++;
      if($times_counter > $times_limit){
        break;
      }
      $line_counter++;
      $recur_params = $ids = array();
      $recur = new CRM_Contribute_DAO_ContributionRecur();
      $recur->id = $t['REFNUMBER'];
      if ( !$recur->find(true) ) {
        $error = "Could not find recur record: $contributionRecurID";
        _civicrm_neweb_error($error, $is_test);
        unset($recur->id);
      }
      $qvar = array(1 => array($t['REFNUMBER'], 'Integer'));
      $contribution_id = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_contribution c WHERE c.contribution_recur_id = %1 ORDER BY c.id ASC", $qvar);
      $contrib = new CRM_Contribute_DAO_Contribution();
      $contrib->id = $contribution_id;
      if ( !$contrib->find( true ) ) { 
        $error = "Can't not find the first contribution id here";
        _civicrm_neweb_error($error, $is_test);
      }

      // exists recurring and not cancelled
      if($recur->id){
        $recur_params['id'] = $recur->id;
        // check trxn_id first
        
        // Find same trxn_id contribution.
        $trxn_id = $t['ORGORDERNUMBER'];
        $recur_id = $t['REFNUMBER'];
        $last_year = date('Y-m-d H:i:s', strtotime('-1 year'));
        $sql = "SELECT * FROM civicrm_contribution WHERE trxn_id LIKE '%$trxn_id%'  AND contribution_recur_id = $recur_id AND created_date > %1";
        $params = array(
          1 => array($last_year, 'String')
        );
        $dao = CRM_Core_DAO::executeQuery($sql, $params); 
        $dao->fetch(); 

        // If the contribution is belong to correct recuring, and same contribution id, Then update receive data. 
        if(!empty($dao->id)){
          if(!$force_update){
            continue; // skip recurring transaction when specifix transaction id exists.
          }
          $contribution = new CRM_Contribute_DAO_Contribution();
          $contribution->id = $dao->id;
          $contribution->find(true);
        }
        else{
          // Doesn't find same contribution.
          
          // clone exists
          unset($contribution);
          $contribution =& $contrib;
          unset($contribution->id);
          unset($contribution->receive_date);
          unset($contribution->cancel_date);
          unset($contribution->cancel_reason);
          unset($contribution->trxn_id);
          unset($contribution->invoice_id);
          unset($contribution->receipt_id);
          unset($contribution->receipt_date);
          $contribution->contribution_status_id = 2;
          $contribution->created_date = date('YmdHis', $now);

          // If the find trxn_id == 'ORGORDERNUMBER' but recur_id isn't correct.
          $sql = "SELECT contribution_recur_id FROM civicrm_contribution WHERE trxn_id LIKE '$trxn_id'";
          $dao = CRM_Core_DAO::executeQuery($sql); 
          $dao->fetch();

          if(!empty($dao->contribution_recur_id)){
            // There are another payment processor, so that we have the same trxn_id but different recur. 
            $contribution->trxn_id = _civicrm_neweb_recur_trxn($t['ORGORDERNUMBER'],$t['MERCHANTNUMBER']);  
          }else{
            // Recuring is same.
            $contribution->trxn_id = $t['ORGORDERNUMBER'];
          }

          $contribution->save();
        }

        CRM_Contribute_BAO_ContributionRecur::syncContribute($recur->id, $contribution->id);
        $responseDetail = civicrm_neweb_response($t['PRC'], $t['SRC'], $t['BANKRESCODE'], 'detail');
        _civicrm_neweb_addnote(implode("\n", $responseDetail), $contribution->id, $contribution->contact_id);

        // now we have correct contribution object. procceed.
        // submit recurring items by ipn functions
        $input = $ids = array();
        $input = array(
          'component' => 'contribute',
          'PRC' => $t['PRC'],
          'SRC' => $t['SRC'],
          'ApprovalCode' => $t['APPROVECODE'],
          'BankResponseCode' => $t['BANKRESCODE'],
          'MerchantNumber' => $t['MERCHANTNUMBER'],
          'OrderNumber' => $contribution->trxn_id,
          'Amount' => (int)$recur->amount,
          'amount' => (int)$recur->amount,
          '_recurring' => TRUE,
        );
        $ids = array(
          'contact' => $recur->contact_id,
          'contribution' => $contribution->id,
          'contributionRecur' => $recur->id,
          'contributionPage' => $contrib->contribution_page_id,
        );
        $objects = array(
          'contribution' => $contribution, 
          'membership' => NULL,
          'participant' => NULL,
          'event' => NULL,
          'paymentProcessor' => $payment,
        );
        require_once 'CRM/Core/Transaction.php';
        $transaction = new CRM_Core_Transaction();
        $post = $get = NULL;
        $IPN = new CRM_Core_Payment_NewebIPN($post, $get);
        // $IPN->validateData($input, $ids, $objects, FALSE); // do not load all object for performance issue

        // Check the response
        // Status: Complete
        if( ($t['PRC'] == 0 && $t['SRC'] == 0) || ($t['PRC'] == '11' && $t['SRC'] == '205') ){
          $objects['contribution']->receive_date = date('YmdHis', $now);
          $IPN->completeTransaction( $input, $ids, $objects, $transaction, true);
          $cycle = $recur->cycle + 1;
          $sql = "UPDATE civicrm_contribution_neweb_recur SET contributions = $now, cycle = $cycle WHERE recur_id = $recur->id";
          CRM_Core_DAO::executeQuery($sql);
        }
        // Status: failed, cancel. We don't have retry mechnism.
        else if($objects['contribution']->contribution_status_id != 4) {
          $err = civicrm_neweb_response($t['PRC'], $t['SRC'], $t['BANKRESCODE'], 'detail');
          $error_code = ts('Error Code:')."PRC-{$input['PRC']},SRC-{$input['SRC']},BRC-{$input['BankResponseCode']}";
          $failed_reason = implode("\n", $err) . "\n" . $error_code;
          $IPN->failed($objects, $transaction, $failed_reason);
          // TODO: record failed reason 
        }
      }

      // compatibility php 5.2
      if(function_exists('gc_collect_cycles')){
        gc_collect_cycles();
      }
    }


    if($times_counter > $times_limit){
      variable_set('civicrm_neweb_process_transaction_count_'.$pid.$test.'_'.date('Ymd', $now), $line_counter);
      return FALSE;
    }
    variable_del('civicrm_neweb_process_transaction_count_'.$pid.$test.'_'.date('Ymd', $now));
    return TRUE;
  }
}

function civicrm_neweb_process_upload($is_test, $pid){
  $now = time();
  $nextday = mktime(6,0,0,date('n'),date('j'),date('Y'))+86400;
  $create_check = $now - 86400; // yesterday
  civicrm_initialize();
  require_once "CRM/Core/DAO.php";
  require_once 'CRM/Core/Payment.php'; 
  require_once 'CRM/Core/BAO/PaymentProcessor.php';
  require_once 'CRM/Core/Payment/NewebIPN.php';
  $p = new CRM_Core_BAO_PaymentProcessor();
  if($is_test){
    $payment = $p->getPayment($pid,'test');
  }
  else{
    $payment = $p->getPayment($pid,'live');
  }

  // check first insert and recurring contributions.
  // need check nextday for support neweb only allow upload new transaction after today
  // only upload new recurring in recurring day.(Duplicate payment bug fix.)
  $sql = "SELECT * FROM civicrm_contribution_recur cr INNER JOIN civicrm_contribution_neweb_recur cnr ON cr.id = cnr.recur_id WHERE FROM_UNIXTIME($create_check) > cr.create_date && (cr.end_date >= FROM_UNIXTIME($now) OR cr.end_date IS NULL) && cr.cancel_date IS NULL && cr.is_test = $is_test && cr.contribution_status_id = 2 ORDER BY cr.next_sched_contribution DESC";
  $recur = CRM_Core_DAO::executeQuery($sql);

  while( $recur->fetch() ){
    // only effect when submit first recurring contribution
    $contrib = CRM_Core_DAO::executeQuery("SELECT * FROM civicrm_contribution WHERE contribution_recur_id = %1 ORDER BY id ASC", array(1 => array($recur->id, 'Integer')));
    $contrib->fetch();

    // skip in-complete transaction if first contribution still have problem
    if($contrib->contribution_status_id != 1 || $contrib->payment_processor_id != $pid) continue;

    // start to collect output file string
    $upload = array(
      'MERCHANTNUMBER' => $payment['user_name'],
      'REFNUMBER' => $recur->id,
      'ORDERNUMBER' => $contrib->trxn_id,
      'BRAND' => '',       // null for our status
      'EXPIREDATE' => '',  // null for our status
      'AMOUNT' => (int)$recur->amount,
      'REGULAR' => $recur->cycle_day,
      'ACTION' => 'New',
      'STATUS' => '01',
      'MODE' => 0, // 0 for month mode, 1 for date mode
    );

    $alter = array(
      'module' => 'civicrm_neweb',
      'params' => $upload,
    );
    drupal_alter('civicrm_recur_params', $alter);
    
    $rows[$recur->id] = $alter['params'];
  }

  // only one rule for cancelation. just look if they have cancel date and still pending, then process cancel job. -> x
  // The rule : status is 6 (experted) and after end_date 2 days. (same as civicrm_neweb_expire_recur() rule)
  //  or
  // status is 2 (pending) and have cancel_date.
  $end = $now - 86400*2;
  $sql = "SELECT * FROM civicrm_contribution_recur cr WHERE ((cr.contribution_status_id = 2 && cr.cancel_date IS NOT NULL && cr.cancel_date < FROM_UNIXTIME($now)) OR (cr.contribution_status_id = 6 && cr.end_date <= FROM_UNIXTIME($end) && cr.end_date IS NOT NULL && cr.cancel_date IS NULL )) && cr.is_test = $is_test ORDER BY cr.id ASC";
  $recur = CRM_Core_DAO::executeQuery($sql);
  while( $recur->fetch() ){
    $contrib = CRM_Core_DAO::executeQuery("SELECT payment_processor_id, trxn_id FROM civicrm_contribution WHERE contribution_recur_id = %1 ORDER BY id ASC", array(1 => array($recur->id, 'Integer')));
    $contrib->fetch();
    if($contrib->payment_processor_id != $pid) continue;
    // start to collect output file string
    $upload = array(
      'MERCHANTNUMBER' => $payment['user_name'],
      'REFNUMBER' => $recur->id,
      'ORDERNUMBER' => $contrib->trxn_id,
      'BRAND' => '',       // null for our status
      'EXPIREDATE' => '',  // null for our status
      'AMOUNT' => (int)$recur->amount,
      'REGULAR' => $recur->cycle_day,
      'ACTION' => 'Delete',
      'STATUS' => '01',
      'MODE' => 0, // 0 for month mode, 1 for date mode
    );
    $rows[$recur->id] = $upload;
    //    CRM_Core_DAO::executeQuery("UPDATE civicrm_contribution_recur SET contribution_status_id = 4 WHERE id = $recur->id");
  }

  if(count($rows)){
    $data = _civicrm_neweb_encodeline($rows);
    $newebpath = _civicrm_neweb_path($is_test);
    $filename = 'RP_'.$payment['user_name'].'_'.date('Ymd').'.dat';
    $saved = file_save_data($data, $newebpath.'/'.$filename, FILE_EXISTS_REPLACE);
    $uploaded =  _civicrm_neweb_connect($pid, $payment['user_name'], $newebpath.'/'.$filename, NEWEB_UPLOAD, $is_test);
    if($uploaded){
      $sql = "UPDATE civicrm_contribution_neweb_recur SET upload=$now WHERE recur_id IN (".implode(',', array_keys($rows)).")";
    }
    CRM_Core_DAO::executeQuery($sql);
    return $uploaded;
  }
  return false;
}

function civicrm_neweb_expire_recur($is_test){
  $now = time();
  civicrm_initialize();
  require_once "CRM/Core/DAO.php";
  require_once "CRM/Core/Error.php";

  // end recurring calculate
  
  // End date should not be calculated again.
  /*
  $sql = "SELECT * FROM civicrm_contribution_recur cr WHERE cr.installments IS NOT NULL AND cr.end_date IS NULL AND (cr.contribution_status_id = 1 OR cr.contribution_status_id = 2)  && cr.is_test = $is_test";
  $result = CRM_Core_DAO::executeQuery($sql);
  while($result->fetch()){
    $recur = new CRM_Contribute_DAO_ContributionRecur();
    $recur->id = $result->id;

    if ( $recur->find( true ) ) {
      // caculate end_date
      $recur->create_date = date('YmdHis',strtotime($recur->create_date));
      $recur->modified_date =  date('YmdHis',strtotime($now));;
      $recur = _civicrm_neweb_end_date_calculate($recur);
      $the_end = $recur->end_date;
    }
    _civicrm_neweb_error('Exec civicrm_neweb_end_date_calculate() in cron.', $is_test, WATCHDOG_NOTICE);
  }
  */ 
  
  // cancel recurring calculate
  // 2 day after end day
  $end = $now - 86400*2;
  $sql = "SELECT * FROM civicrm_contribution_recur cr INNER JOIN civicrm_contribution_neweb_recur cnr ON cr.id = cnr.recur_id WHERE cr.end_date <= FROM_UNIXTIME($end) && cr.end_date IS NOT NULL && cr.cancel_date IS NULL && cr.is_test = $is_test && cr.contribution_status_id = 5 ORDER BY cr.end_date ASC";
  $recur = CRM_Core_DAO::executeQuery($sql);
  while( $recur->fetch() ){
    // just change status to pending and add cancel date.
    // $cancel_date = date('Y-m-d H:i:s', $now);
    // status 6 is 'expired' , remove cancel_date refs. #15801
    CRM_Core_DAO::executeQuery("UPDATE civicrm_contribution_recur SET contribution_status_id = 6 WHERE id = {$recur->id}");
  }
}

function _civicrm_neweb_sslftp($id, $localfile, $upload, $is_test){
  $dir = $upload ? 'import' : 'output';
  if($is_test){
    $ftp = variable_get("civicrm_neweb_ftp_test_".$id, array());
    if($ftp['ftp_host'] && $ftp['ftp_user']){
      $cid = ftp_connect($ftp['ftp_host']);
      $login = ftp_login($cid, $ftp['ftp_user'], $ftp['ftp_password']);
    }
  }
  else{
    $ftp = variable_get("civicrm_neweb_ftp_".$id, array());
    if($ftp['ftp_host'] && $ftp['ftp_user']){
      $cid = ssh2_connect($ftp['ftp_host'], 22);
      $login = ssh2_auth_password($cid, $ftp['ftp_user'], $ftp['ftp_password']);
    }
    // $sftp = ssh2_sftp($cid);
  }

  if($login){
    $path = $is_test ? $id.'/'.$dir : $dir;
    $remotefile = $path .'/'. basename($localfile);

    if($upload){
      if($is_test){
        $success = ftp_put($cid, $remotefile, $localfile, FTP_ASCII);
      }
      else{
        $buffer = file_get_contents($localfile);
        $success = file_put_contents("ssh2.sftp://".$ftp['ftp_user'].":".$ftp['ftp_password']."@".$ftp['ftp_host'].":22/$remotefile", $buffer);
      }
    }
    else{
      // download
      if($is_test){
        $pass = $ftp['ftp_password'] ? ':'.$ftp['ftp_password'] : '';
        $contents = file_get_contents("ftp://".$ftp['ftp_user'].$pass."@".$ftp['ftp_host']."/$remotefile");
        if($contents){
          $success = file_put_contents($localfile, $contents);
        }
      }
      else{
        $contents = file_get_contents("ssh2.sftp://".$ftp['ftp_user'].":".$ftp['ftp_password']."@".$ftp['ftp_host'].":22/$remotefile");
        if($contents){
          $success = file_put_contents($localfile, $contents);
        }
      }
    }
    if($is_test){
      ftp_close($cid);
    }

    // see if success login and get result
    if($success){
      if($upload){
        return true;
      }
      else{
        return file_get_contents($localfile);
      }
    }
    elseif(!$upload){
      // only return -1 when download.
      // upload will retry untill done.
      _civicrm_neweb_error("File not found: $localfile", $is_test, WATCHDOG_NOTICE);
      return -1;
    }
  }
  else{
    _civicrm_neweb_error("FTP loging or setup error.", $is_test);
  }
  return false;
}

function _civicrm_neweb_encodeline($in){
  foreach($in as $line){
    $o .= implode(',', $line)."\n";
  }
  return rtrim($o, "\n");
}

/**
 * type: upload / response / transaction
 */
function _civicrm_neweb_decodeline($in, $type){
  $o = array();
  switch($type){
    case 'response':
      $key= array(
        'MERCHANTNUMBER',
        'REFNUMBER',
        'CARDNUMBER',
        'BRAND',
        'EXPIREDATE',
        'AMOUNT',
        'REGULAR',
        'ACTION',
        'STATUS',
        'MODE',
        'RESPONSE',
      );
      break;
    case 'transaction':
      $key= array(
        'MERCHANTNUMBER',
        'REFNUMBER',
        'ORGORDERNUMBER',
        'HTTPCODE',
        'PRC',
        'SRC',
        'BANKRESCODE',
        'APPROVECODE',
        'BATCHNUMBER',
        'MERCHANTORDERNUMBER',
        'MODE',
      );
      break;
  }
  $in = str_replace (array("\r\n", "\n", "\r"), "\n", $in);
  $lines = explode("\n",$in);
  foreach($lines as $l){
    if($l[0] == '#') continue;
    if($l){ // check if empty line
      $value = explode(",", $l);
      $o[] = array_combine($key, $value);
    }
  }
  return $o;
}
function _civicrm_neweb_path($is_test){
  if($is_test){
    $newebfolder = 'neweb_test';
  }
  else{
    $newebfolder = 'neweb';
  }
  $newebfolderurl = 'public://' . $newebfolder;
  file_prepare_directory($newebfolderurl, FILE_CREATE_DIRECTORY);
  $newebpath = variable_get('file_public_path', conf_path() . '/files') . '/' . $newebfolder;
  file_prepare_directory($newebpath, FILE_MODIFY_PERMISSIONS);
  return $newebfolderurl;
}


function civicrm_neweb_process_ezpay($is_test, $now, $pid){
  $time = date('YmdHis');
  civicrm_initialize();
  require_once 'CRM/Contribute/DAO/Contribution.php';
  require_once 'CRM/Core/BAO/PaymentProcessor.php';
  require_once 'CRM/Core/Payment/BaseIPN.php';
  require_once 'CRM/Core/Transaction.php';
  require_once "CRM/Core/DAO.php";
  $p = new CRM_Core_BAO_PaymentProcessor();
  if($is_test){
    $payment_processor = $p->getPayment($pid,'test');
    $is_test = 1;
  }
  else{
    $payment_processor = $p->getPayment($pid,'live');
    $is_test = 0;
  }

  if(empty($payment_processor)){
    return;
  }

  $instrument = _civicrm_neweb_instrument();
  unset($instrument['Credit Card']);
  $gid = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_group WHERE name LIKE 'payment_instrument'");

  foreach($instrument as $name => $obj){
    $vid = CRM_Core_DAO::singleValueQuery("SELECT value FROM civicrm_option_value WHERE name = %1 AND option_group_id = %2 AND is_active = 1", array(1 => array($name, 'String'), 2 => array($gid, 'Integer') ));
    if($vid){
      $instrument_id[$vid] = $vid;
    }
  }
  $instrument_str = implode(',', $instrument_id);
  $dao = CRM_Core_DAO::executeQuery("SELECT * FROM civicrm_contribution WHERE contribution_status_id = 2 AND payment_instrument_id IN ($instrument_str) AND is_test = $is_test AND payment_processor_id = $pid ORDER BY id DESC");
  while($dao->fetch()){
    $post = $input = $objects = $ids = $result = array();

    if(!$dao->contribution_page_id){
      // this is event payment
      $pp = CRM_Core_DAO::executeQuery("SELECT cpp.participant_id, e.id FROM civicrm_participant_payment cpp INNER JOIN civicrm_participant cp ON cpp.participant_id = cp.id INNER JOIN civicrm_event e ON e.id = cp.event_id WHERE cpp.contribution_id = {$dao->id}");
      $pp->fetch();
      $input['component'] = 'event';
      $ids['event'] = $pp->id;
      $ids['participant'] = $pp->participant_id;
    }
    else{
      $input['component'] = 'contribute';
    }

    $post['merchantnumber'] = $payment_processor['password'];
    $post['ordernumber'] = $dao->id;
    $post['operation'] = 'queryorders';
    $post['time'] = $time;
    $post['hash'] = md5($post['operation'].$payment_processor['subject'].$time);

    // initialize objects and ids
    $note = '';
    $c = new CRM_Contribute_DAO_Contribution();
    $c->id = $dao->id;
    $c->find(true);
    $ids['contact'] = $c->contact_id;
    $ids['contribution'] = $c->id;

    // fetch result and object
    $result = civicrm_neweb_postdata($post, $payment_processor);
    $created = isset($result['timecreated']) ? strtotime($result['timecreated']) : strtotime($c->created_date);
    if ($created){
      // 4 weeks
      $overdue = $now - $created > 86400 * variable_get('civicrm_neweb_expired_day', 28) ? TRUE : FALSE;
    }
    else{
      $overdue = $created = FALSE;
    }

    $transaction = new CRM_Core_Transaction();
    $ipn = new CRM_Core_Payment_BaseIPN();

    if($ipn->validateData($input, $ids, $objects, TRUE, $pid) && $result){
      // check result
      if($result['rc'] == 0 && $result['status'] == 1){
        // after validate, start to complete some transaction
        $input['trxn_id'] = $c->trxn_id;
        $input['payment_instrument_id'] = $c->payment_instrument_id;
        $input['check_number'] = $result['writeoffnumber'];
        $input['amount'] = $result['amount'];
        if($result['timepaid']){
          $objects['contribution']->receive_date = $result['timepaid'];
        }
        else{
          $objects['contribution']->receive_date = date('YmdHis');
        }
        $ipn->completeTransaction($input, $ids, $objects, $transaction);

        // note here;
        $note .= ts("Serial number").": ".$result['serialnumber']."\n";
        $note .= ts("Payment Instrument").": ". $result['paymenttype']."\n";
        $note .= ts("External order number").": ".$result['writeoffnumber']."\n";
        $note .= ts("Create date").": ".$result['timecreated']."\n";
        $note .= ts("Paid date").": ".$result['timepaid']."\n";
        $note .= ts("Pay count").": ".$result['paycount']."\n";
        $note .= ts("Completed");
        _civicrm_neweb_addnote($note, $c->id, $c->contact_id);
      }
      elseif(!$result['status'] && $result['rc'] == 0 && $overdue) {
        // cancel contribution over 4 weeks
        $input['reasonCode'] = ts('Overdue');
        $input['trxn_id'] = $c->trxn_id;
        $input['payment_instrument_id'] = $c->payment_instrument_id;
        $ipn->cancelled($objects, $transaction);
        $note .= ts("Canceled").": ".ts('Overdue')."\n";
        $note .= "Over ".variable_get('civicrm_neweb_expired_day', 28)." days doesn't have payment from {$result['timecreated']}\n";
        _civicrm_neweb_addnote($note, $c->id, $c->contact_id);
      }
      elseif($result['rc'] == 0 && count($result) == 2 && !empty($c->trxn_id) && $overdue){
        // not found on neweb and over 4 weeks
        $input['reasonCode'] = ts('Overdue');
        $input['trxn_id'] = $c->trxn_id;
        $input['payment_instrument_id'] = $c->payment_instrument_id;
        $ipn->cancelled($objects, $transaction);
        $note .= ts("Canceled").": ".ts('Overdue')."\n";
        $note .= "Order number not found in neweb.\n";
        _civicrm_neweb_addnote($note, $c->id, $c->contact_id);
      }
      elseif($result['rc']){
        // FIXME to see if go failed contribution
        $note .= ts("Error").": ".$result['rc']."/".$result['rc2']."\n";
        _civicrm_neweb_addnote($note, $c->id, $c->contact_id);
      }
    }
  }
}

function civicrm_neweb_postdata($post, $payment_processor, $url = NULL){
  $postdata = _civicrm_neweb_vars2str($post);
  if(is_null($url)){
    $url = rtrim($payment_processor['url_api'],'/')."/Query";
  }

  $ch = curl_init($url);
  curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 1);
  curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
  curl_setopt($ch, CURLOPT_POST, 1);
  curl_setopt($ch, CURLOPT_POSTFIELDS, $postdata);
  curl_setopt($ch, CURLOPT_HEADER, 0);  // DO NOT RETURN HTTP HEADERS
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);  // RETURN THE CONTENTS OF THE CALL
  $receive = curl_exec($ch);
  if(curl_errno($ch)){
    _civicrm_neweb_error("CURL Error: ".curl_error($ch));
    $ch2 = curl_init($url);
    curl_setopt($ch2, CURLOPT_SSL_VERIFYHOST, 1);
    curl_setopt($ch2, CURLOPT_SSL_VERIFYPEER, false);
    curl_setopt($ch2, CURLOPT_POST, 1);
    curl_setopt($ch2, CURLOPT_POSTFIELDS, $postdata);
    curl_setopt($ch2, CURLOPT_HEADER, 0);  // DO NOT RETURN HTTP HEADERS
    curl_setopt($ch2, CURLOPT_RETURNTRANSFER, 1);  // RETURN THE CONTENTS OF THE CALL
    $receive = curl_exec($ch2);
  }
  curl_close($ch);

  if($receive){
    $vars = _civicrm_neweb_vars2array($receive);
    if($vars['rc'] == 70){
      $regetorder = curl_init($query_url);
      $post['operation'] = "regetorder";
      $postdata = _civicrm_neweb_vars2str($post);
      curl_setopt($regetorder, CURLOPT_POST, 1);
      curl_setopt($regetorder, CURLOPT_POSTFIELDS, $postdata);
      curl_setopt($regetorder, CURLOPT_HEADER, 0);
      curl_setopt($regetorder, CURLOPT_RETURNTRANSFER, 1);
      $receive2 = curl_exec($regetorder);
      curl_close($regetorder);
      $vars2 = _civicrm_neweb_vars2array($receive2);
      return $vars2;
    }
    return $vars;
  }
  else{
    return FALSE;
  }
}

function _civicrm_neweb_vars2str($post){
  $array = array();
  foreach($post as $name => $value){
    if($value){
      $array[] = $name."=".urlencode($value);
    }
  }
  return implode('&', $array);
}

function _civicrm_neweb_vars2array($str){
  $vars = explode('&', $str);
  foreach($vars as $var){
    list($name, $value) = explode('=', $var, 2);
    if($name == 'errormessage'){
      $value = iconv("Big5","UTF-8",$value);
    }
    $params[$name] = $value;
  }
  return $params;
}

function _civicrm_neweb_addnote($note, $contribution_id, $contact_id){
  require_once 'CRM/Core/BAO/Note.php';
  $note = date("Y/m/d H:i:s")." ". ts("Transaction record").": \n".$note."\n===============================\n";
  $note_exists = CRM_Core_BAO_Note::getNote( $contribution_id, 'civicrm_contribution' );
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
    'entity_id'     => $contribution_id,
    'contact_id'    => $contact_id,
  );
  CRM_Core_BAO_Note::add( $noteParams, $note_id );
}

function _civicrm_neweb_error($msg, $is_test = NULL, $type = NULL){
  $type = $type ? $type : WATCHDOG_ERROR;
  if($is_test && NEWEB_DEBUG){
    watchdog('civicrm_neweb', 'TEST Env: '.$msg, array(), $type);
  }
  else{
    watchdog('civicrm_neweb', $msg, array(), $type);
  }
}
