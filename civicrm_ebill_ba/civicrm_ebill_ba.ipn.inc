<?php
civicrm_initialize();
require_once 'CRM/Core/Payment/BaseIPN.php';
date_default_timezone_set("Asia/Taipei");

/**
 * Menu callback function of civicrm_ebill_ba_menu
 */
function civicrm_ebill_ba_ipn($post = NULL, $get = NULL){
  $post = !empty($post) ? $post : $_POST;
  $post_raw = file_get_contents('php://input');
  $get = !empty($get) ? $get : $_GET;

  // 8010 / 8020 request response
  if(count($post) <= 1 && !empty($post_raw)){
    watchdog('civicrm_ebill_ba', 'input: '.$post_raw);
    $output = _civicrm_ebill_ba_response( base64_decode($post_raw));
    echo base64_encode($output);
    watchdog('civicrm_ebill_ba', 'output: '.base64_encode($output));
    drupal_exit();
  }
  elseif($get['module']){
    $component = $get['module'];
    $ipn = new CRM_Core_Payment_EbillBaIPN($post, $get);
    $ipn->main($component);
  }
  else{
    watchdog('civicrm_ebill_ba', "Could not get module name from request url");
    drupal_not_found();
  }
}

function _civicrm_ebill_ba_response($raw){
  $debug = $raw;
  if(_civicrm_ebill_ba_byte2length($raw)){
    $request = array(
      'Message Type' => 4,
      'Processing Code' => 4,
      'System trace audit NO' => 10,
      'TXN destination institute ID' => 7,
      'TXN source institute ID' => 7,
      'TXN NO.' => 7,
      'TXN initiate date and time' => 14,
      'Response' => 4,
      'Sync Check Item' => 4,
    );
    $start = 0;
    foreach($request as $k => $length){
      $request[$k] = substr($raw, $start, $length);
      $start += $length;
    }

    // check 8010
    if($request['Message Type'] == '0900' && $request['Processing Code'] == '8010'){
      civicrm_initialize();
      $request2 = array(
        'Amount' => 14,
        'Payment Type' => 5,
        'NoticeNO' => 16,
        'Levying Office' => 3,
        'Biller ID' => 8,
        'FeeType' => 4,
      );
      foreach($request2 as $k => $length){
        $request2[$k] = substr($raw, $start, $length);
        $start += $length;
      }
      $debug .= "\n".var_export($request, TRUE);
      $debug .= "\n".var_export($request2, TRUE);
      $trxn_id = ltrim($request2['NoticeNO'], '0');
      $contrib = _civicrm_ebill_ba_load_contrib($trxn_id, $request);
      if($contrib === FALSE){
        $request['Response'] = '9101';
      }
      else{
        $request['Response'] = '0001';
        $response = array(
          'IDN/BAN' => str_repeat(' ', 11),
          'Amount' => $request2['Amount'],
          'NoticeNO' => $request2['NoticeNO'],
          'Payment Type' => $contrib->payment_processor['signature'],
        );
      }
    }
    // check 8020
    elseif($request['Message Type'] == '0900' && $request['Processing Code'] == '8020'){
      civicrm_initialize();
      $request2 = array(
        'Amount' => 14,
        'Payment Type' => 5,
        'NoticeNO' => 16,
        'Payment Deadline' => 8,
        'IDN/BAN' => 11,
        'Handling Charge' => 4,
        'Handling Charge Unit' => 4,
        'Business Date' => 6,
        'Transferee Account No.' => 16,
        'Rcode' => 4,
      );
      foreach($request2 as $k => $length){
        $request2[$k] = substr($raw, $start, $length);
        $start += $length;
      }
      $trxn_id = ltrim($request2['NoticeNO'], '0');
      $contrib = _civicrm_ebill_ba_load_contrib($trxn_id, $request);
      if($contrib === FALSE){
        $request['Response'] = '9101';
      }
      else{
        $request['Response'] = '0001';
        $response = array(
          'Amount' => $request2['Amount'],
          'NoticeNO' => $request2['NoticeNO'],
        );
      }
    }
    else{
      $request['Response'] = '1001';
    }

    // success output 
    $request['Message Type'] = '0910';
    $debug .= "\n".var_export($request, TRUE);
    if(!empty($response)){
      $debug .= "\n".var_export($response, TRUE);
      $output = implode('', $request).implode('', $response);
    }
    else{
      watchdog('civicrm_ebill_ba', '8010/8020 have fail query');
      $output = implode('', $request);
    }
    _civicrm_ebill_ba_length2byte($output);
    $debug .= "\n".$output;
    watchdog('civicrm_ebill_ba', $debug);
    return $output;
  }
  return FALSE;
}

function _civicrm_ebill_ba_byte2length(&$raw){
  $byte2 = substr($raw, 0, 2);
  $array = unpack('H*', $byte2);
  $length = hexdec($array[1]);
  if($length == strlen($raw) - 2){
    $raw = substr($raw, 2);
    return TRUE;
  }
  else{
    return FALSE;
  }
}
function _civicrm_ebill_ba_length2byte(&$raw){
  $length = strlen($raw);
  $length = sprintf('%04s', dechex($length));
  $byte2 = pack('H*', $length);
  $raw = $byte2.$raw;
  watchdog('civicrm_ebill_ba', base64_encode($raw));
  return $byte2.$raw;
}

function _civicrm_ebill_ba_load_contrib($trxn_id, $request){
  $contrib = new CRM_Contribute_DAO_Contribution();
  $contrib->trxn_id = (string) $trxn_id;

  if($contrib->find(TRUE)){
    if(!empty($contrib->payment_processor_id)){
      $mode = $contrib->is_test ? 'test' : 'live';
      $payment_processor = CRM_Core_BAO_PaymentProcessor::getPayment($contrib->payment_processor_id, $mode);
      if(!empty($payment_processor)){
        $contrib->payment_processor = $payment_processor;

        // basic validate
        if(!empty($request['Biller ID'])){
          if($contrib->payment_processor['user_name'] != $request['Biller ID']){
            watchdog('civicrm_ebill_ba', 'Biller ID error (payment processor user_name)');
            return FALSE;
          }
        }
        if(!empty($request['Amount'])){
          $remote_amount = ltrim($request['Amount'], '+0');
          $local_amount = (int) $contrib->total_amount;
          if($remote_amount != $local_amount){
            watchdog('civicrm_ebill_ba', 'Amount error.');
            return FALSE;
          }
        }
        return $contrib;
      }
    }
  }
  return FALSE;
}

class CRM_Core_Payment_EbillBaIPN extends CRM_Core_Payment_BaseIPN {
  static $_payment_processor = NULL;
  static $_input = NULL;
  public $_post = NULL;
  public $_get = NULL;

  function __construct($post, $get) {
    parent::__construct();
    $this->_post = $post;
    $this->_get = $get;
  }

  function main($component = 'contribute'){
    if(is_numeric($this->_get['cid']) && !empty($this->_get['billerid']) && !empty($this->_get['rcode'])){
      db_merge("civicrm_contribution_ebillba")
        ->key(array('cid' => $this->_get['cid']))
        ->fields(array('data' => json_encode($this->_get)))
        ->execute();
      $objects = $ids = $input = array();
      $input = $this->_get;
      $qfkey = CRM_Utils_Array::value('qfkey', $this->_get);
      $this->getIds($ids, $component);
      $input['component'] = $component;
      $civi_base_url = $component == 'event' ? 'civicrm/event/register' : 'civicrm/contribute/transact';

      // now, retrieve full object by validateData, or false fallback
      if($this->validateData($input, $ids, $objects)) {
        $contribution = $objects['contribution'];
        if($contribution->contribution_status_id == 1){
          // already complete, go to thank you page
          if($qfkey){
            drupal_goto($civi_base_url, array(
              'query' => array(
                '_qf_ThankYou_display' => 'true',
                'qfKey' => $qfkey,
              )
            ));
          }
        }
        elseif($input['rcode'] == '4001'){
          $input['trxn_id'] = $contribution->trxn_id;
          $contribution->receive_date = date('YmdHis');
          $transaction = new CRM_Core_Transaction();
          $recur = FALSE;
          $this->completeTransaction($input, $ids, $objects, $transaction, $recur);
          $note .= ts('Completed')."\n";
          $this->addNote($note, $contribution);
          if($qfkey){
            drupal_goto($civi_base_url, array(
              'query' => array(
                '_qf_ThankYou_display' => 'true',
                'qfKey' => $qfkey,
              )
            ));
          }
        }
        else{
          // error
          $msg = $this->errorMsg($input['rcode']);
          $note .= ts('Failed')."\n";
          $note .= 'Error number: '. $input['rcode']."\n";
          $note .= 'Error msg: '. $msg;
          $this->addNote($note, $objects['contribution']);
          $transaction = new CRM_Core_Transaction();
          $this->failed($objects, $transaction);
          if($qfkey){
            drupal_set_message(ts("Payment Information").": ".ts("Failed").' - '.$input['rcode'].":".$msg, 'error');
            drupal_goto($civi_base_url, array(
              'query' => array(
                '_qf_ThankYou_display' => 'true',
                'qfKey' => $qfkey,
              )
            ));
          }
        }
      }
      else{
        watchdog('civicrm_ebill_ba', 'Validation error of civicrm base ipn object.');
      }
    }
  }
  function getIds( &$ids , $component){
    $ids['contact'] = CRM_Utils_Array::value('contact_id', $this->_get);
    $ids['contribution'] = CRM_Utils_Array::value('cid', $this->_get);
    if($component == 'event') {
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

  function errorMsg($code){
    $msg = array(
      '4001' => '交易成功',
      '0101' => '訊息格式或內容編輯錯誤',
      '0102' => '訊息回應代碼錯誤',
      '0104' => '訊息發生一般檢核類錯誤',
      '0108' => '清算日期錯誤',
      '0201' => '發信單位未參加本項跨行業務或系統暫時無法運作',
      '0202' => '發信單位本項跨行業務停止或暫停營業',
      '0203' => '發信單位未在跨行作業運作狀態',
      '0204' => '收信單位未參加本項跨行業務或系統暫時無法運作',
      '0205' => '收信單位本項跨行業務停止或暫停營業',
      '0206' => '收信單位未在跨行作業運作狀態',
      '2999' => '其他類檢核錯誤',
      '0302' => '訊息押碼錯誤',
      '0401' => '訊息之 MAPPING 欄位資料與原交易相關訊息之欄位 資料不相符。',
      '0601' => '逾時交易取消',
      '0701' => '財金公司本項業務暫停或停止營業',
      '0702' => '財金主機未在跨行作業運作狀態',
      '0703' => '財金跨行業務系統錯誤',
      '0801' => '檔案故障',
      '1001' => '無效之訊息類別代碼或交易類別代碼',
      '1002' => '跨行交易序號重覆或繳費單位交易序號重覆',
      '1004' => '訊息內之跨行交易序號或繳費單位交易序號錯誤',
      '4202' => '轉帳累計金額超過限額',
      '4401' => '帳號失效',
      '4405' => '帳號尚未生效',
      '4406' => '帳號已掛失',
      '4414' => '卡號冇誤(IC交易用)',
      '4501' => '申請帳戶為問題帳戶',
      '4505' => '餘額不足',
      '4507' => '轉入帳戶或特店錯誤',
      '4508' => '非約定轉入帳號',
      '4601' => '轉帳累計次數超過限制',
      '4801' => '繳款類別有誤；未經註冊之繳費交易',
      '4802' => '銷帳編號檢查碼有誤',
      '4803' => '繳款金額有誤',
      '4804' => '繳款期限有誤；預期、未到',
      '4805' => '事業機構代號有誤',
      '4806' => '身份證營利事業統一編號與系統紀錄不符',
      '4807' => '入帳單位有誤',
      '4808' => '無此帳戶',
      '5004' => '中文欄位或變動長度欄位長度錯誤(IC交易用)',
      '6101' => '跨行可用餘額小於零,不得交易',
      '8101' => '交易IC金融卡交易驗證碼檢核錯誤(IC交易用)',
      '8104' => 'IC金融卡交易序號重覆(IC交易用)',
      '8108' => 'IC卡片序號錯誤,回發卡單位處理(IC交易用)',
      '8115' => 'IC卡備註欄位檢核有誤(IC交易用)',
      '9001' => '該銀行未加入轉帳交易',
      '9002' => '該銀行未加入餘額查詢交易',
      '9003' => '稅籍資料檢查錯誤',
      '9004' => '關貿連結之申報或自繳資料不存在',
      '9005' => '營業稅稅籍編號有誤,請關閉瀏覽器後重新輸入',
      '9006' => '繳費單位本項跨行業務停止或暫停營業',
      '9007' => '繳費單位本項跨行業務未停止或未暫停營業',
      '9008' => '訊息傳輸長度小於實際傳輸資料長度',
      '9009' => '縣市機關鄉鎮代號與營業稅稅籍編號檢核不符',
      '9101' => '繳費資料錯誤',
      '9102' => '無帳單資料',
      '9997' => '系統服務到達上限。請稍後再試。',
      '9998' => '無法正確判斷電子錢包版本，請確認是否已經安裝新版電子錢包',
      '9999' => '系統發生不明錯誤，若已啟動轉帳或繳稅交易，請洽財金公司詢問該筆交易狀況，以避免重覆扣帳。',
    ); 
    if(isset($msg[$code])){
      return $msg[$code];
    }
    else{
      return '未知錯誤，請聯繫系統管理員';
    }
  }
}
