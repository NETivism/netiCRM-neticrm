<?php
define('ALLPAY_IMPORT_HEADER', '*上傳時間,授權單號,*刷卡金額,分期期數,刷卡項目,*持卡人姓名,*信用卡卡號,授權碼,*回應碼,回應訊息,IP');

function civicrm_allpay_recur_import_form(&$form_state){
  $step = !empty($form_state['storage']['step']) ? $form_state['storage']['step'] : 1;

  $func = '_civicrm_allpay_recur_import_form_step'.$step;
  $default_values = isset($form_state['storage'][$step]) ? $form_state['storage'][$step] : array();

  $form = call_user_func($func, $form_state, $default_values);
  $form['step'] = array(
    '#type' => 'hidden',
    '#value' => $step,
  );

  $total_step = 4;
  if($step < $total_step) {
    if($step > 1){
      $form['prev'] = array(
        '#type' => 'submit',
        '#value' => '<< 上一步',
        '#id' => 'prev-step',
      );
    }
    // add alert prevent user leave page
    $js = '
window.onbeforeunload = function() { return "離開本頁面，等待匯入的資料都會被清空重來。"; };
$("form").submit(function(){ window.onbeforeunload = null;} );
';
    $form['leave_alert'] = array(
      '#value' => '<script>'.$js.'</script>',
    );
  }
  else{
    $form['done'] = array(
      '#type' => 'submit',
      '#value' => '完成！',
      '#id' => 'done',
    );
  }
  return $form;
}
function _civicrm_allpay_recur_import_form_step1(&$form_state, $default_values){
  civicrm_initialize();
  $form = array();
  $form['markup'] = array(
    '#value' => '<h2>第一步：張貼檔案</h2>',
  );
  $form['separator'] = array(
    '#type' => 'select',
    '#title' => '欄位分隔符號',
    '#required' => TRUE,
    '#options' => array(
      "tab" => '定位鍵(Tab) - Excel複製貼上預設',
//      "comma" => '逗號(,) - CSV格式預設',
//      "semicolon" => '分號(;) - 其他格式',
    ),
    '#description' => '預設為定位鍵',
    '#default_value' => isset($default_values['separator']) ? $default_values['separator'] : "\t",
  );
  $form['contribution_type_id'] = array(
    '#type' => 'select',
    '#title' => ts('Contribution Type'),
    '#required' => TRUE,
    '#options' => CRM_Contribute_PseudoConstant::contributionType(),
    '#description' => '請選擇要匯入系統的類型',
    '#default_value' => isset($default_values['contribution_type_id']) ? $default_values['contribution_type_id'] : 1,
  );
  $form['is_recur'] = array(
    '#type' => 'select',
    '#title' => '匯入的捐款為定期定額？',
    '#required' => TRUE,
    '#options' => array(1 => '是', 0 => '否'),
    '#description' => '若選擇定期定額，將會為聯絡人新增一筆定期定額紀錄，並整併後續匯入該聯絡人的其他定期，並可進行定期定額收入預估。',
    '#default_value' => isset($default_values['is_recur']) ? $default_values['is_recur'] : 1,
  );
  $form['body'] = array(
    '#type' => 'textarea',
    '#title' => '請貼入歐付寶回傳的資料',
    '#required' => TRUE,
    '#description' => '',
    '#default_value' => isset($default_values['body']) ? $default_values['body'] :  '',
  );
  $form['next'] = array(
    '#type' => 'submit',
    '#value' => '選擇對應欄位 >>',
    '#id' => 'next-step',
    '#weight' => 999,
  );
  return $form;
}
function _civicrm_allpay_recur_import_form_step2(&$form_state, $default_values){
  $form = array();
  $form['markup'] = array(
    '#value' => '<h2>第二步：選擇對應欄位</h2><div>* 號為必選欄位</div>',
  );
  $submitted = $form_state['storage'][$form_state['storage']['step'] - 1];
  $rows = _civicrm_allpay_recur_parse_row($submitted['body'], $submitted['separator'], NULL, 3);
  $header = explode(',', ALLPAY_IMPORT_HEADER);
  $header_default = array();
  foreach($header as $k => $h){
    $header_default[$k] = str_replace('*', '', $h);
  }
  $count = 0;
  foreach($rows as $line => $row){
    if(empty($count)){
      $header_import  = $row;
    }
    foreach($row as $k => $v) {
      $form['mapping'][$k]['preview_'.$count.'_'.$k] = array(
        '#value' => $v,
      );
      if(!empty($count)){
        $form['data']['data_'.$k] = array(
          '#type' => 'hidden',
          '#value' => $v,
        );
      }
    }
    $count++;
  }
  foreach($header_import as $k => $v){
    $form['mapping'][$k]['map_'.$k] = array(
      '#type' => 'select',
      '#options' => array('none' => '-- 請選擇 --', 'skip' => '-- 不匯入 --') + $header,
      '#default_value' => !empty($default_values['map_'.$k]) ? $default_values['map_'.$k] : array_search($v, $header_default),
    );
  }
  $form['#theme'] = 'civicrm_allpay_recur_mapping_form';
  $form['next'] = array(
    '#type' => 'submit',
    '#value' => '驗證資料 >>',
    '#id' => 'next-step',
    '#weight' => 999,
  );
  return $form;
}
function _civicrm_allpay_recur_import_form_step3(&$form_state, $default_values){
  $form = array();

  $form['markup'] = array(
    '#value' => '<h2>第三步：驗證資料</h2>',
  );
  $form['next'] = array(
    '#type' => 'submit',
    '#value' => '正式匯入 >>',
    '#id' => 'next-step',
    '#weight' => 999,
    '#disabled' => TRUE,
    '#attributes' => array('style' => 'color: #999;'),
  );

  if(!empty($_SESSION['civicrm_allpay_recur']['import_error'])){
    $table_header = array('行數', '錯誤');
    $form['errors'] = array(
      '#value' => '<h3>請更正以下錯誤後，重新貼入，才能匯入</h3>'.theme('table', $table_header, $_SESSION['civicrm_allpay_recur']['import_error']),
    );
    unset($_SESSION['civicrm_allpay_recur']['import_error']);
    $form['next']['#disabled'] = 'disabled'; 
    $form['next']['#attributes'] = array(
      'style' => 'color: #AAA',
    ); 
  }
  else{
    $form['markup'] = array(
      '#value' => '<div>您上傳的資料格式無誤，可按「正式匯入」進行匯入動作</div>',
    );
    unset($form['next']['#disabled']);
    unset($form['next']['#attributes']['style']);
  }

  return $form;
}
function _civicrm_allpay_recur_import_form_step4(&$form_state, $default_values){
  $form = array();
  $url = array();
  if(isset($_SESSION['civicrm_allpay_recur']['inserted'])){
    foreach($_SESSION['civicrm_allpay_recur']['inserted'] as $link){
      if(is_numeric($link[1])){
        $l = l('編號'.$link[1], 'civicrm/contact/view/contribution', array(
          'query' => "reset=1&id={$link[1]}&cid={$link[0]}&action=view",
          'attributes' => array('target' => '_blank'),
        ));
        if(!empty($link[2])){
          $l .= " ({$link[2]})";
        }
        $url[] = $l;
      }
      else{
        $url[] = '捐款無法匯入，錯誤:'.$link[1].'。<br>原始資料:<pre style="overflow:scroll;color:#777">'.$link[2].'</pre>';
      }
    }
    unset($_SESSION['civicrm_allpay_recur']['inserted']);
  }
  $form['markup'] = array(
    '#value' => theme('item_list', $url),
  );
  return $form;
}

function civicrm_allpay_recur_import_form_validate($form, &$form_state){
  // step 1
  if(!empty($form_state['values']['separator'])) {
    $allowed_separator = array(
      'tab' => "\t",
      'comma' => ',',
      'semicolon' => ';',
    );
    if(isset($allowed_separator[$form_state['values']['separator']])) {
      $form_state['values']['separator'] = $allowed_separator[$form_state['values']['separator']];
    }
    else{
      $form_state['values']['separator'] = "\t";
    }
  }

  // step 2 
  if($form_state['values']['step'] == '2' && $form_state['clicked_button']['#id'] == 'next-step'){
    $map = array();
    $values = $form_state['values'];
    foreach($values as $k => $v){
      if(strstr($k, 'map_')){
        if($v === 'none'){
          form_set_error($k, '你必須要選擇一個對應欄位，或選擇略過該欄位');
        }
        $k = str_replace('map_', '', $k);
        $map[$k] = $v;
      }
    }
    $form_state['storage']['map'] = $map;

    // required field check
    $header = explode(',', ALLPAY_IMPORT_HEADER);
    foreach($header as $k => $v){
      $mapped_key = array_search($k, $map);
      if($v[0] == '*'){
        if(!is_numeric($mapped_key) || is_null($mapped_key)){
          form_set_error('_map_'.$k, '「'.str_replace('*', '', $v).'」必要選擇的匯入對應欄位');
        }
      }
    }

    $errors = form_get_errors();

    // everything fine, validate all rows
    if(empty($errors)){
      civicrm_initialize();
      $submitted = $form_state['storage'][$form_state['storage']['step'] - 1];
      _civicrm_allpay_recur_parse_row($submitted['body'], $submitted['separator'], '_civicrm_allpay_recur_validate_row', $num_rows = NULL, $map, $skip_first = TRUE);
    }
  }

  // step 3

  if($form_state['values']['step'] == '3' && $form_state['clicked_button']['#id'] == 'next-step'){
    if(empty($form_state['storage']['map'])){
      form_set_error('_map_', '系統出現了一些問題，讓資料無法通過驗證，請把資料重新貼入再匯入一次試試');
    }
    if(empty($form_state['storage'][1]['body'])){
      form_set_error('_map_', '系統出現了一些問題，無法取得資料，請回到第一步驟把資料重新貼入再匯入一次試試');
    }
  }
}

function civicrm_allpay_recur_import_form_submit($form, &$form_state){
  // Store value
  $submitted_step = $form_state['values']['step'];
  if(!empty($form_state['values'])){
    $form_state['storage'][$submitted_step] = $form_state['values'];
  }

  if($form_state['clicked_button']['#id'] == 'next-step') {
    $step = $submitted_step + 1;
    $form_state['storage']['step'] = $step;
  }
  elseif($form_state['clicked_button']['#id'] == 'prev-step') {
    $step = $submitted_step > 1 ? $submitted_step - 1 : 1;
    $form_state['storage']['step'] = $step;
  }

  // real import job here
  if($form_state['values']['step'] == '3' && $form_state['clicked_button']['#id'] == 'next-step'){
    global $civicrm_allpay_recur_storage;
    $civicrm_allpay_recur_storage = $form_state['storage'];
    $body = $form_state['storage'][1]['body']; 
    $separator = $form_state['storage'][1]['separator']; 
    $map = $form_state['storage']['map'];
    civicrm_initialize();
    _civicrm_allpay_recur_parse_row($body, $separator, '_civicrm_allpay_recur_insert_row', $num_rows = NULL, $map, $skip_first = TRUE);
  }

  
  if($form_state['values']['step'] == '4' && $form_state['clicked_button']['#id'] == 'done'){
    drupal_goto('civicrm/allpay_recur_import');
  }

  // necesserry for multi-step
  $form_state['rebuild'] = TRUE;
}

function _civicrm_allpay_recur_parse_row($input, $separator, $callback = NULL, $num_rows = NULL, $mapping = NULL, $skip_first = FALSE){
  $row = array();
  $input = str_replace("\r", '', $input);
  $rows = str_getcsv($input, "\n");
  $data = array();
  $duplicate = 0;
  foreach($rows as $l => $r){
    if(!empty($num_rows) && $l == $num_rows){
      break;
    }
    $row = array();
    $row = str_getcsv($r, $separator);
    $hash = md5(implode('', $row));
    if(empty($callback)){
      if(isset($data[$hash])){
        $duplicate++; 
      }
      else{
        $data[$hash] = $row;
      }
    }
    else{
      if($skip_first && !$l){
        continue; 
      }
      call_user_func($callback, $l, $row, $hash, $mapping);
    }
  }
  if(!empty($data)){
    return $data;
  }
  return NULL;
}

function _civicrm_allpay_recur_validate_row($line, $row, $hash, $mapping){
  $row_flat = implode(',', $row);
  //// required in row
  $header = explode(',', ALLPAY_IMPORT_HEADER);
  foreach($header as $k => $v){
    $mapped = array_search($k, $mapping);
    if($v[0] == '*'){
      $v = str_replace('*', '', $v);
      if(empty($row[$mapped])){
        _civicrm_allpay_recur_import_error($line, $mapped, $row_flat, '缺少必要的欄位：'.$v);
      }
    }
  }

  // date time format error
  $mapped = array_search(0, $mapping);
  $datetime = preg_replace('/\s?上午\s?|\s?下午\s?/u', ' ', $row[$mapped]);
  $timestamp = strtotime($datetime);
  if(empty($timestamp) || $timestamp < strtotime('1990-01-01 00:00:00')){
    _civicrm_allpay_recur_import_error($line, $mapped, $row_flat, '日期格式有誤，正確格式為「2015/12/10 上午 11:10:16」');
  }
  
  // amount
  $mapped = array_search(2, $mapping);
  $amount = preg_replace('/[,]/', '', $row[$mapped]);
  if($amount <= 0 || empty($amount)){
    _civicrm_allpay_recur_import_error($line, $mapped, $row_flat, '金額小於0或沒有填寫。');
  }

  // contact id
  $mapped = array_search(5, $mapping);
  preg_match('/\[(.*)\]/i', $row[$mapped], $contact);
  if(empty($contact[1])){
    _civicrm_allpay_recur_import_error($line, $mapped, $row_flat, '姓名沒有帶入系統ID，或ID沒有用括弧格式，正確格式：「王小明[123]」，或「[123]」');
  }
}

function _civicrm_allpay_recur_insert_row($line, $row, $hash, $mapping){
  global $civicrm_allpay_recur_storage;
  $header = explode(',', ALLPAY_IMPORT_HEADER);
  $data = array();
  foreach($header as $k => $v){
    $mapped = array_search($k, $mapping);
    $data[$k] = $row[$mapped];
  }

  $amount = preg_replace('/[,]/', '', $data[2]);
  preg_match('/\[(.*)\]/i', $row[5], $contact);
  $contact_id = $contact[1];

  module_load_include('inc', 'civicrm_allpay_recur', 'civicrm_allpay_recur.utils');
  $recur_params = array(
    'amount' => $amount,
  );
  $is_recur = $civicrm_allpay_recur_storage[1]['is_recur'];


  $code = $data[8];
  $msg = $data[9];
  $datetime = preg_replace('/\s?上午\s?|\s?下午\s?/u', ' ', $data[0]);
  $timestamp = strtotime($datetime);
  if(strstr($data[0], '下午')) {
    $timestamp += 3600*12;
  }
  $datetime = date('YmdHis', $timestamp);
  $is_error = _civicrm_allpay_error_msg($code);
  $contribution_type_id = $civicrm_allpay_recur_storage[1]['contribution_type_id'];
  $contrib_params = array(
    'total_amount' => $amount,
    'contribution_status_id' => $is_error ? 4 : 1,
    'contribution_type_id' => $contribution_type_id,
    'source' => '由歐付寶檔案匯入',
    'receive_date' => $datetime,
    'currency' => 'TWD',
    'invoice_id' => $hash,
  );

  $is_recur = $civicrm_allpay_recur_storage[1]['is_recur'];
  $instruments = CRM_Contribute_PseudoConstant::paymentInstrument('name');
  if($is_recur){
    $rid = _civicrm_allpay_recur_create_recur($contact_id, $recur_params);
    $contrib_params['contribution_recur_id'] = $rid;
    $instrument_id = array_search('Credit Card Recurring (Offline)', $instruments);
  }
  else{
    $instrument_id = array_search('Credit Card (Offline)', $instruments);
  }
  $contrib_params['payment_instrument_id'] = $instrument_id ? $instrument_id : 1;

  $result = _civicrm_allpay_recur_create_contribution($contact_id, $contrib_params);
  if(is_numeric($result) && !empty($result)){
    $cid = $result;
    $_SESSION['civicrm_allpay_recur']['inserted'][] = array($contact_id, $cid);
    $note = '交易結果: '.$msg." (回應碼 : $code)\n";
    $note .= "授權單號: {$data[1]}\n";
    $note .= "授權碼: {$data[7]}\n";
    $note .= "刷卡項目: {$data[4]}\n";
    $note .= "IP: {$data[10]}\n";
    _civicrm_allpay_recur_create_note($contact_id, $cid, $note);
  }
  else{
    // result is array
    $row_flat = implode(',', $row);
    if($result['duplicate'] && !empty($result['id'])){
      $_SESSION['civicrm_allpay_recur']['inserted'][] = array($contact_id, $result['id'], '重複的紀錄，略過不匯入');
    }
    elseif(civicrm_error($result)){
      $_SESSION['civicrm_allpay_recur']['inserted'][] = array($contact_id, $result['error_message'], $row_flat);
    }
  }
}

function _civicrm_allpay_recur_import_error($line, $key, $row, $message){
  if(isset($_SESSION['civicrm_allpay_recur']['import_error'][$line])){
    $exists = $_SESSION['civicrm_allpay_recur']['import_error'][$line]['message'];
    $_SESSION['civicrm_allpay_recur']['import_error'][$line]['message'] = $message.'<br >'.$exists;
  }
  else{
    $_SESSION['civicrm_allpay_recur']['import_error'][$line] = array(
      'line' => $line,
      'message' => $message.'<br>原始資料:<pre style="overflow:scroll;color:#777">'.$row.'</pre>',
    );
  }
}

function theme_civicrm_allpay_recur_mapping_form($form){
  $output = drupal_render($form['markup']);
  if(!empty($form['mapping'])) {
    $rows = array();
    foreach(element_children($form['mapping']) as $k) {
      $row = array();
      foreach($form['mapping'][$k] as $column => $f){
        if($column[0] !== '#'){
          $row[] = drupal_render($form['mapping'][$k][$column]);
        }
      }
      $rows[] = $row;
    }
    if(count($rows)){
      $header = array('上傳資料欄位', '第一列資料', '第二列資料', '對應匯入欄位');
      $output .= theme('table', $header, $rows);
    }
  }

  return $output . drupal_render($form);
}
