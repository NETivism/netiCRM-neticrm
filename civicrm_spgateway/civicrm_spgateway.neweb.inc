<?php
function civicrm_spgateway_neweb_transfer($post = NULL, $get = NULL, $print = TRUE, $isTest = FALSE) {
  civicrm_initialize();
  // detect variables
  $post = !empty($post) ? $post : $_POST;
  CRM_Core_Error::debug_var('spgateway_neweb_transfer_post', $post);
  $ids = array();
  $pid = variable_get('civicrm_spgateway_neweb_pid', 0);
  if (empty($pid)) {
    $pid = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_payment_processor WHERE payment_processor_type = 'SPGATEWAY'");
    if ($isTest) {
      $pid += 1;
    }
  }

  if (empty($pid)) {
    watchdog('civicrm_spgateway.neweb', "There are no SPGATEWAY payment processor.");
    drupal_not_found();
  }

  if (!empty($post['Period'])) {
    if (is_array($pid)) {
      $pids = $pid;
      foreach ($pids as $pid) {
        $paymentProcessor = CRM_Core_BAO_PaymentProcessor::getPayment($pid, $isTest ? 'test': 'live');
        $decryptParams = _civicrm_spgateway_post_decode(_civicrm_spgateway_recur_decrypt($post['Period'], $paymentProcessor));
        CRM_Core_Error::debug_var('spgateway_neweb_transfer_decrypt_params', $decryptParams);
        if (!empty($decryptParams['MerchantOrderNo'])) {
          $rid = $decryptParams['MerchantOrderNo'];
          break;
        }
      }
      if (empty($pid)) {
        watchdog('civicrm_spgateway.neweb', "You set multiple SPGATEWAY payment processors, but none of them are decryptable.");
        echo "Error. Please check log.";
        drupal_not_found();
      }
    }
    else {
      $paymentProcessor = CRM_Core_BAO_PaymentProcessor::getPayment($pid, $isTest ? 'test': 'live');
      $decryptParams = _civicrm_spgateway_post_decode(_civicrm_spgateway_recur_decrypt($post['Period'], $paymentProcessor));
      CRM_Core_Error::debug_var('spgateway_neweb_transfer_decrypt_params', $decryptParams);
      $rid = $decryptParams['MerchantOrderNo'];
      CRM_Core_Error::debug_var('spgateway_neweb_transfer_rid', $rid);
    }
  }
  else {
    watchdog('civicrm_spgateway.neweb', "\$post['Period'] not exist.");
    drupal_not_found();
  }

  if (!empty($decryptParams) && !empty($rid)) {
    // complex part to simulate spgateway ipn
    $ipn_result = $ipn_get = $ipn_post = array();

    // prepare post, complex logic because recurring have different variable names
    $ipn_result['Result'] = $decryptParams;
    $queryParams = array(1 => array($rid, 'Positive'));
    $ipn_result['Result']['MerchantOrderNo'] = CRM_Core_DAO::singleValueQuery("SELECT trxn_id FROM civicrm_contribution WHERE contribution_recur_id = %1 ORDER BY id ASC LIMIT 1", $queryParams);
    $ipn_result['Status'] = $decryptParams['Status'];
    $ipn_result['Result']['OrderNo'] = 'r_'.$ipn_result['Result']['OrderNo'];
    $periodNo = $ipn_result['Result']['PeriodNo'];
    CRM_Core_Error::debug_var('spgateway_neweb_transfer_ipn_result', $ipn_result);
    $ipn_result = json_encode($ipn_result);
    $ipn_post = array('Period' => _civicrm_spgateway_recur_encrypt($ipn_result, $paymentProcessor));
    CRM_Core_Error::debug_var('spgateway_neweb_transfer_ipn_post', $ipn_post);

    // prepare get
    $firstCid = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_contribution WHERE contribution_recur_id = %1 ORDER BY id ASC LIMIT 1", $queryParams);
    $ids = CRM_Contribute_BAO_Contribution::buildIds($firstCid);
    $query = CRM_Contribute_BAO_Contribution::makeNotifyUrl($ids, NULL, TRUE);
    $query .= '&ppid='.$pid;
    parse_str($query, $ipn_get);
    CRM_Core_Error::debug_var('spgateway_neweb_transfer_ipn_get', $ipn_get);

    // create recurring record
    $result = new stdClass();
    $result->_ipn_result = $ipn_result;
    $result->_post = $ipn_post;
    $result->_get = $ipn_get;
    module_load_include('inc', 'civicrm_spgateway', 'civicrm_spgateway.ipn');
    $result->_response = civicrm_spgateway_ipn('Credit', $ipn_post, $ipn_get, FALSE);
    // If correct, it must return '1|OK'.
    if (!empty($result->_response)) {
      $query = "UPDATE civicrm_contribution SET payment_processor_id = %1 WHERE contribution_recur_id = %2 ORDER BY id DESC LIMIT 1";
      $params = array(
        1 => array($pid, 'Positive'),
        2 => array($ids['contributionRecurID'], 'Positive'),
      );
      CRM_Core_DAO::executeQuery($query, $params);
      // Check processor_id, trxn_id of contribution_recur
      $sql = "SELECT trxn_id, processor_id FROM civicrm_contribution_recur WHERE id = %1";
      $sqlParams = array( 1 => array($ids['contributionRecurID'], 'Positive'));
      $recurDao = CRM_Core_DAO::executeQuery($sql, $sqlParams);
      while ($recurDao->fetch()) {
        if ($recurDao->processor_id != $pid) {
          $recurParams['processor_id'] = $pid;
        }
        if ($recurDao->trxn_id != $periodNo) {
          $recurParams['trxn_id'] = $periodNo;
        }
        if (!empty($recurParams)) {
          $recurParams['id'] = $ids['contributionRecurID'];
          $recurParams['message'] = ts('Modify parameters of original Newebpay by Newebpay data.');
          CRM_Core_Error::debug_var('spgateway_neweb_transfer_update_recur_params', $recurParams);
          CRM_Contribute_BAO_ContributionRecur::addNote($ids['contributionRecurID'], $recurParams['message']);
          CRM_Contribute_BAO_ContributionRecur::add($recurParams, $null);
        }
      }
    }
    CRM_Core_Error::debug_var('spgateway_neweb_transfer_ipn_response', $result->_response);
    echo $result->_response;
  }
  else {
    $msg = 'Error. Decrypt error or Recurring ID is not found.';
    watchdog('civicrm_spgateway.neweb', $msg);
    echo $msg;
  }
  CRM_Utils_System::civiBeforeShutdown();
}


function civicrm_spgateway_neweb_resync($ppid_new, $day = '', $recurNo = array()) {

  CRM_Core_Error::debug_log_message("Start doResyncOldNewebRecur, time:".date('Y-m-d H:i:s').", ppid_new = {$ppid_new}, day = {$day}");
  $payment_processor = CRM_Core_BAO_PaymentProcessor::getPayment($ppid_new, 'live');
  if (!empty($payment_processor)) {

    if (empty($recurNo)) {
      $offset = variable_get("spgateway_neweb_resync_offset_{$ppid_new}", 0);
      $cycle_day = empty($day) ? date('j') : $day;
      $sql = "SELECT DISTINCT r.id AS rid from civicrm_contribution_recur r INNER JOIN civicrm_contribution c ON r.id = c.contribution_recur_id WHERE c.trxn_id LIKE 'r_%' AND r.cycle_day = %1 AND c.payment_processor_id = %2 LIMIT 300  OFFSET %3";
      $params = array(
        1 => array($cycle_day, 'Integer'),
        2 => array($ppid_new, 'Integer'),
        3 => array($offset, 'Integer'),
      );
      $bao = CRM_Core_DAO::executeQuery($sql, $params);
      if ($bao->N == 0) {
        // offset variable roll back to zero.
        variable_set("spgateway_neweb_resync_offset_{$ppid_new}", 0);
        
        // There are no recur need execute. return finished.
        $message = "已無需要處理之捐款，結束PPID為{$ppid_new}的所有舊藍新同步。";
        CRM_Core_Error::debug_log_message($message);
        return TRUE;
      }
      while($bao->fetch()) {
        $recurNo[] = $bao->rid;
      }
    }
    $total = count($recurNo);
    $skip = $count = $alreadyExist = $transactFailed = $success = $queryFailed = 0;

    foreach ($recurNo as $recurId) {
      $sql = "SELECT id, last_receive_date , last_failed_date,  ltid.last_trxn_id FROM civicrm_contribution_recur AS r 
LEFT JOIN (SELECT contribution_recur_id AS rid, MAX(receive_date) AS last_receive_date FROM civicrm_contribution WHERE contribution_status_id = 1 AND contribution_recur_id = %1 GROUP BY contribution_recur_id) lrd ON lrd.rid = r.id 
LEFT JOIN (SELECT contribution_recur_id AS rid, MAX(cancel_date) AS last_failed_date FROM civicrm_contribution WHERE contribution_status_id = 4 AND contribution_recur_id = %1 GROUP BY contribution_recur_id) lfd ON lfd.rid = r.id 
LEFT JOIN (SELECT contribution_recur_id AS rid, CONCAT('r_', contribution_recur_id, '_', MAX(CAST(REGEXP_SUBSTR(trxn_id, '[0-9]+$') as INT))) AS last_trxn_id FROM civicrm_contribution WHERE contribution_recur_id = %1 AND trxn_id LIKE CONCAT('r_', contribution_recur_id, '_%') GROUP BY contribution_recur_id) ltid ON ltid.rid = r.id 
WHERE r.id = %1";
      $dao = CRM_Core_DAO::executeQuery($sql, array(1 => array($recurId, 'Integer')));
      $dao->fetch();

      // Only execute the contributions last month have successed.
      $lastReceiveMonth = date('Ym', strtotime($dao->last_receive_date));
      $lastCancelMonth = date('Ym', strtotime($dao->last_failed_date));
      $lastExecuteMonth = max($lastReceiveMonth, $lastCancelMonth);
      $thisMonth = date('Ym');
      $subtractMonth = $thisMonth - $lastExecuteMonth;
      if (!in_array($subtractMonth, array(1, 89))) {
        $skip++;
        continue;
      }
      $output = 'Recur ID: '.$recurId;
      CRM_Core_Error::debug_log_message("Find not complete contribution in this month, recur_id = {$recurId}, trxn_id = {$dao->last_trxn_id}, receive_date = {$dao->last_receive_date}, cancel_date = {$dao->last_failed_date}, last_execute_month = {$lastExecuteMonth}");
      $lastTrxnId = $dao->last_trxn_id;
      $explodeTrxnId = explode('_', $lastTrxnId);
      $no = $explodeTrxnId[2]+1;
      $trxn_id = $recurId.'_'.$no;
      $count++;
      
      $sql = "SELECT amount FROM civicrm_contribution_recur WHERE id = %1";
      $params = array( 1 => array($recurId, 'Positive'));
      $amount = CRM_Core_DAO::singleValueQuery($sql, $params);
      $amount = (int)$amount;
      $request_data = array(
        'Amt' => $amount,
        'MerchantID' => $payment_processor['user_name'],
        'MerchantOrderNo' => $trxn_id,
        'RespondType' => SPGATEWAY_RESPONSE_TYPE,
        'TimeStamp' => CRM_REQUEST_TIME,
        'Version' => SPGATEWAY_QUERY_VERSION,
      );
      CRM_Core_Error::debug_var("doResyncOldNewebRecur:request_data", $request_data);
      $used_args = array('IV','Amt','MerchantID','MerchantOrderNo', 'Key');
      _civicrm_spgateway_encode($request_data, $payment_processor, $used_args);
      $urlApi = SPGATEWAY_REAL_DOMAIN.SPGATEWAY_URL_API;
      sleep(0.5);
      CRM_Core_Error::debug_var("doResyncOldNewebRecur:encoded_request_data", $request_data);
      $result = _civicrm_spgateway_postdata($urlApi, $request_data);
      CRM_Core_Error::debug_var("doResyncOldNewebRecur:result", $result);
    
      if (!empty($result) && $result->Status == 'SUCCESS') {
        $output .= ":查詢成功";
        echo ":查詢成功";
        // prepare contribution
        $contribution = new CRM_Contribute_DAO_Contribution();
        $contribution->contribution_recur_id = $recurId;
        $contribution->orderBy("id ASC");
        $contribution->find(TRUE);
        if ($contribution->id && $contribution->contribution_recur_id == $recurId) {
          $exists = CRM_Core_DAO::singleValueQuery("SELECT COUNT(*) FROM civicrm_contribution WHERE trxn_id LIKE 'r_{$trxn_id}'");
          if ($exists) {
            $alreadyExist++;
            $output .= ":資料已存在:";
            $output .= $result->Result->TradeStatus."\n";
            echo ":資料已存在:";
            echo $result->Result->TradeStatus."\n";
            continue;
          }
          // complex part to simulate spgateway ipn
          $ipn_get = $ipn_post = array();
    
          // prepare post, complex logic because recurring have different variable names
          $ipn_result = clone $result;
          if ($result->Result->TradeStatus != 1) {
            $output .= ":交易失敗:".$result->Result->RespondMsg;
            echo ":交易失敗:".$result->Result->RespondMsg;
            $ipn_result->Status = 'Error';
            $transactFailed++;
          }
          else {
            $output .= ":交易成功";
            echo ":交易成功";
            $success++;
          }
          $ipn_result->Message = $result->Result->RespondMsg;
          $ipn_result->Result->AuthAmt = $result->Result->Amt;
          unset($ipn_result->Result->Amt);
          unset($ipn_result->Result->CheckCode);
          $ipn_result->Result->OrderNo = $result->Result->MerchantOrderNo;
          $ipn_result->Result->MerchantOrderNo = $recurId;
          $ipn_result->Result->do_not_email = TRUE;
          $ipn_result->Result->AlreadyTimes = 1;
          CRM_Core_Error::debug_var('doResyncOldNewebRecur:ipn_result', $ipn_result);
          $ipn_result = json_encode($ipn_result);
          $ipn_post = array('Period' => _civicrm_spgateway_recur_encrypt($ipn_result, $payment_processor));
          CRM_Core_Error::debug_var('doResyncOldNewebRecur:ipn_post', $ipn_post);
          module_load_include('inc', 'civicrm_spgateway', 'civicrm_spgateway.neweb');
          civicrm_spgateway_neweb_transfer($ipn_post);
        }
      }
      else {
        var_dump($result);
        $output .= ":查詢失敗";
        echo ":查詢失敗";
        $queryFailed++;
      }
      echo "\n";
      CRM_Core_Error::debug_log_message($output);
      
    }
    $message = "本次 cron 成果：";
    $start = $offset+1;
    $message .= "從第{$start}筆開始進行，";
    if ($skip > 0) {
      $message .= "跳過{$skip}筆本月已更新完成；";
    }
    $message .= "總共執行{$count}筆更新, {$alreadyExist}筆已存在, {$transactFailed}筆交易失敗, {$success}筆交易成功, {$queryFailed}筆查詢失敗";
    if ($total > ($count+$skip)) {
      $unexecuted = $total - $count - $skip;
      $message .= "，剩{$unexecuted}筆未執行，移到下次進行。";
    }
    CRM_Core_Error::debug_log_message($message);
    variable_set("spgateway_neweb_resync_offset_{$ppid_new}", $offset+300);
  }

}  
