<?php
function civicrm_demo_process_contact($vars){
  civicrm_initialize();

  $limit = $vars['limit'];
  while($limit) {
    // 10% to generate eng name
    $language = mt_rand(1,10) == 1 ? 'english' : 'chinese';
    $first_name = _civicrm_demo_rand_firstname($language);
    $last_name = _civicrm_demo_rand_lastname($language);
    if($language == 'chinese'){
      $sort_name = $last_name.$first_name;
    }
    $params = array(
      'version' => 3,
      'first_name'    => _civicrm_demo_rand_firstname($language),
      'last_name'     => _civicrm_demo_rand_lastname($language),
      'sort_name'     => $sort_name,
      'display_name'  => $sort_name,
      'email'         => _civicrm_demo_rand_email(),
      'contact_type'  => 'Individual',
      'individual_prefix' => _civicrm_demo_rand_prefix(),
      'gender_id' => mt_rand(1,2),
      'birth_date' => _civicrm_demo_rand_birth(),
    );

    $api_phone_create_home = array(
      'phone' => '0'.mt_rand(2,39).'-'.mt_rand(1111111, 9999999),
      'phone_type_id' => 1,
      'location_type' => 'Home'
    );
    $api_phone_create_fax = array(
      'phone' => '0'.mt_rand(2,39).'-'.mt_rand(1111111, 9999999),
      'phone_type_id' =>  2,
      'is_primary' => 1,
      'location_type_id' => 2
    );
    $params['api.address.create'] = _civicrm_demo_rand_addr($language);
    $params['api.phone.create'] = array($api_phone_create_home, $api_phone_create_fax);
    $contact = civicrm_api('Contact', 'create', $params );

    $limit--;
  }
}

function civicrm_demo_process_event($vars){
  $now = time();

  civicrm_initialize();
  CRM_Core_DAO::executeQuery("UPDATE civicrm_participant_status_type SET is_active = 1 WHERE is_active = 0");
  $is_test = variable_get('civicrm_demo_sample_data', 0) ? 0 : 1;

  // create 5 events
  $title = array(
      '無名額限制，填表完成送出',
      '有名額限制，不開放候補',
      '有名額限制，開放候補',
      '有名額限制，需事先審核',
      '無名額限制，需事先審核',
    );
  for($i=0;  $i < $vars['limit']; $i++){
    $seed = $i % count($title);

    $params = array('version' => 3,);
    $params['title'] = $title[$seed];
    $params['is_active'] = 1;
    $params['is_monetory'] = 0;
    $params['event_type_id'] = 1;
    $params['is_show_location'] = 0;
    if(variable_get('civicrm_demo_sample_data', 0)){
      $params['start_date'] = date('Ymd', $now+$i*86400*5);
      $params['end_date'] = date('Ymd', $now+$i*86400*5+7200);
      $params['registration_start_date'] = date('Ymd', $now-($i+1)*86400*2);
      $params['registration_end_date'] = date('Ymd', $now+($i+1)*86400*2);
      $params['max_participants'] = 5;
    }
    else {
      $params['is_template'] = 1;
      $params['template_title'] = $title[$seed];
      $params['start_date'] = date('Ymd', $now+$i*86400*5);
      $params['max_participants'] = '';
    }

    if($seed == 0 || $seed == 4){
      $params['max_participants'] = "";
    }
    if($seed == 3 || $seed == 4){
      $params['requires_approval'] = 1;
    }
    if($seed == 2){
      $params['has_waitlist'] = 1;
    }

    $params['is_online_registration'] = 1;

    $result = civicrm_api('Event', 'create', $params);
    if ($params['is_template'] && $result['id']) {
      CRM_Core_DAO::executeQuery("UPDATE civicrm_event SET start_date = NULL WHERE id = %1", array(
        1 => array($result['id'], 'Integer')
      ));
    }

    $event_id = $result['id'];
    // refs #36118, Add new profiles form to registration.
    $profile_id = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_uf_group WHERE name = 'participants_information_proflie'");
    if (!empty($profile_id) && !variable_get('civicrm_demo_sample_data_ci', 0)) {
      CRM_Core_DAO::executeQuery("INSERT INTO civicrm_uf_join (is_active, module, entity_table, entity_id, weight, uf_group_id) VALUES ('1', 'CiviEvent', 'civicrm_event', %1, '1', %2)", array(
        1 => array($event_id, 'String'),
        2 => array($profile_id, 'String'),
      ));
    }

    if (variable_get('civicrm_demo_sample_data', 0)) {
      $trackVars = array(
        'limit' => 2000,
        'pageType' => 'civicrm_event',
        'pageId' => $event_id,
      );
      civicrm_demo_process_track($trackVars);
    }

    $params2 = array(
      'version' => 3,
      'return.contact_id',
      'sort' => 'RAND()',
    );
    $contacts = civicrm_api('Contact', 'get', $params2);

    // add participants
    $j = 0;
    $max = $vars['limit_participants'] ? $vars['limit_participants'] : 4;
    foreach($contacts['values'] as $contact){
      $j++;
      if($j > $max) break;
      $params3 = array('version' => 3,);
      $params3['contact_id'] = $contact['contact_id'];
      $params3['event_id'] = $event_id;
      $params3['status_id'] = 1;
      $params3['role_id'] = 1;
      $params3['register_date'] = date('YmdHis');
      $params3['source'] = '示範用途';
      $params3['is_test'] = $is_test;
      $result3 = civicrm_api('Participant', 'create', $params3);
    }
  }
}

function civicrm_demo_process_contribution($vars){
  civicrm_initialize();
  $is_test = variable_get('civicrm_demo_sample_data', 0) ? 0 : 1;

  // ci testing needed
  if (variable_get('civicrm_demo_sample_data_ci', 0)) {
    $contribution_page_id[0] = civicrm_demo_create_contribution_page('Payment_ALLPAY');
    $contribution_page_id[1] = civicrm_demo_create_contribution_page('Payment_Neweb');
    $contribution_page_id[2] = civicrm_demo_create_contribution_page('Payment_SPGATEWAY');
    $contribution_page_id[3] = civicrm_demo_create_contribution_page('Payment_Mobile');
  }
  else {
    // demo site or normal site
    $contribution_page_id[0] = civicrm_demo_create_contribution_page('');
  }

  // query exists contact
  $contrib_per_contact = !empty($vars['contrib_per_contact']) ? $vars['contrib_per_contact'] : 5; // limit every contact only have num of contribution.
  $contrib_limit = $vars['limit']; // total limit this batch
  $sql = "SELECT id FROM civicrm_contact WHERE is_deleted = 0 ORDER BY RAND()";
  $dao = CRM_Core_DAO::executeQuery($sql);

  // prepare variable
  $payment_instrument_id = 1;
  $start_year = $vars['start_year'] ? $vars['start_year'] : ((int)date('Y'))-5;

  // start
  $count_contact = 0;
  $page_id_num = count($contribution_page_id);
  $types = array_keys(CRM_Contribute_PseudoConstant::contributionType());
  $instruments = CRM_Contribute_PseudoConstant::paymentInstrument();
  $contribution_type_id = reset($types);
  $counter = 0;
  while($dao->fetch()){
    $count_contact++;
    $page_id = $contribution_page_id[$count_contact%$page_id_num];
    $contact_id = $dao->id;
    $insert = array();
    for($i = 0; $i < $contrib_per_contact; $i++){
      $counter++;
      if($counter > $contrib_limit) break;
      $amount = mt_rand(300, 12000);
      $a = number_format($amount, 2, '.', '');
      $fee = number_format($amount*0.025, 2, '.', '');

      $year = mt_rand($start_year, date('Y'));
      $month = sprintf('%02s',mt_rand(1,date('n')));
      if($year == date('Y') ){
        $day = sprintf('%02s', mt_rand(1,28));
      }
      else{
        $day = sprintf('%02s', mt_rand(1,date('j')));
      }
      $hour = sprintf('%02s', mt_rand(0,23));
      $min = sprintf('%02s', mt_rand(0,59));
      $trxn_id = 'd-'.mt_rand(100,999).'-'.mt_rand(100, 999).'-'.CRM_Utils_String::createRandom(8);

      $insert[$i] = array(
        'contact_id'             => $contact_id,
        'created_date'           => $year.$month.$day.$hour.$min.'00',
        'receive_date'           => $year.$month.$day.$hour.$min.'00',
        'total_amount'           => $a,
        'payment_instrument_id'  => array_rand($instruments),
        'trxn_id' => $trxn_id,
        'contribution_status_id' => 1,
        'contribution_type_id' => $contribution_type_id,
        'contribution_page_id' => $page_id,
        'is_test' => $is_test,
      );
    }
    if (!empty($insert)) {
      $fields = array_keys($insert[0]);
      $sql = "INSERT INTO civicrm_contribution (".implode(',', $fields).") \nVALUES \n";
      $values = array();
      foreach($insert as $idx => $val) {
        $values[$idx] = "('".implode("','", $val)."')";
      }
      $sql .= implode(",\n", $values);
      CRM_Core_DAO::executeQuery($sql);
      if (php_sapi_name() === 'cli') {
        echo $counter."\n";
      }
    }
  }
}

function civicrm_demo_process_membership($vars){
  civicrm_initialize();
  $is_test = variable_get('civicrm_demo_sample_data', 0) ? 0 : 1;
  $domain_id = CRM_Core_Config::domainID();

  // check if any organization contact exists
	$params = array(
		'contact_type' => 'Organization',
		'options.limit' => 1,
		'version' => 3,
	);
	$results = civicrm_api('Contact', 'get', $params);

  // create organization for membership
  $org_id = 0;
  if (empty($results['id'])) {
    $params = array( 
		  'version' => 3,
		);
    $params = array(
      'contact_type' => 'Organization',
      'organization_name' => '示範組織聯絡人',
      'version' => 3,
    );
    $org = civicrm_api('Contact', 'create', $params);
    if (!empty($org['id'])) {
      $org_id = $org['id'];
    }
  }
  else {
    $org_id = $results['id'];
  }

  if ($org_id) {
    $types = CRM_Contribute_PseudoConstant::contributionType();
    $contribution_type_id = array_search(ts('Member Dues'), $types);
    if (empty($contribution_type_id)) {
      $contribution_type_id = key($types);
    }
    $params = array(
      'version' => 3,
    );
    $membership_type = civicrm_api('membership_type', 'get', $params);
    if (!empty($membership_type['count'])) {
      foreach($membership_type['values'] as $mem_type) {
        if($mem_type['id']) {
          $membership_type_id = $mem_type['id'];
          break;
        }
      }
    }
    else {
      $params = array( 
        'name' => '一般會員',
        'description' => '由系統建立的一般會員類型示範',
        'member_of_contact_id' => $org_id,
        'contribution_type_id' => $contribution_type_id,
        'financial_type_id' => $contribution_type_id,
        'domain_id' => $domain_id,
        'minimum_fee' => '1000',
        'duration_unit' => 'year',
        'duration_interval' => '1',
        'period_type' => 'fixed',
        'visibility' => 'public',
        'is_active' => 1,
        'version' => 3,
      );

      $membership_type = civicrm_api('membership_type', 'create', $params);
      if (!empty($membership_type['id'])) {
        $membership_type_id = $membership_type['id'];
      }
    }
  }

  // only create demo membership when sample data needed
  if ($membership_type_id && variable_get('civicrm_demo_sample_data', 0)) {
		$membership_limit = $vars['limit']; // total limit this batch
    $sql = "SELECT id FROM civicrm_contact WHERE is_deleted = 0 ORDER BY RAND()";
    $dao = CRM_Core_DAO::executeQuery($sql);
    $membership_count = 0;
    while($dao->fetch()){
      $membership_count++;
      $start_year = (int) date('Y');
      $start_year -= mt_rand(0, 5);
      $end_year = $start_year + mt_rand(1, 3);

      $params = array(
        'contact_id' => $dao->id,
        'membership_type_id' => $membership_type_id,
        'join_date' => $start_year.'-01-01',
        'start_date' => $start_year.'-01-01',
        'end_date' => $end_year.'-12-31',
        'source' => '示範用途',
        'is_test' => $is_test,
        'version' => 3,
      );
      $mem = civicrm_api('membership','create', $params);
      if ($mem['id']) {
        $membership_count++;
      }
      if ($membership_count > $membership_limit) {
        break;
      }
    }
  }
}

function civicrm_demo_create_contribution_page($payment_processor_type){
  $payment_page = variable_get('civicrm_demo_payment_page', array());
  if(isset($payment_page[$payment_processor_type])){
    return $payment_page[$payment_processor_type];
  }
  // setup payment processor

  // create contribution page
  $types = CRM_Contribute_PseudoConstant::contributionType();
  $contribution_type_id = reset(array_keys($types));

  if(variable_get('civicrm_demo_sample_data', 0)) {
    $from_email = 'demo@demo.neticrm.tw';
  }
  else {
    $from_email = '';
  }
  $params = array(
    'version' => 3,
    'title' => '捐款贊助',
    'contribution_type_id' => reset(array_keys($types)),
    'is_active' => 1,
    'is_monetary' => 1,
    'is_recur' => 1,
    'is_allow_other_amount' => 1,
    'recur_frequency_unit' => 'month',
    'thankyou_title' => '謝謝你的支持',
    'receipt_from_email' => $from_email,
    'installments_option' => 1,
  );
  if (variable_get('civicrm_demo_sample_data_ci', 0)) {
    $processors = _civicrm_demo_setup_paymentProcessor($payment_processor_type);
    $params['payment_processor'] = $processors;
  }
  else {
    $params['is_pay_later'] = 1; 
    $params['pay_later_text'] = '請設定您的金流服務';
    $params['pay_later_receipt'] = '選擇金流服務商完成設定，即可提供正式的線上付款工具。';
  }
  $result = civicrm_api('ContributionPage', 'create', $params);
  if(!empty($result['id'])){
    $contribution_page_id = $result['id'];
    $payment_instrument = array(
      'credit_card' => 1,
      'convenient_store' => 1,
      'convenient_store_code_' => 1,
      'web_atm' => 1,
      'atm' => 1,
    );
    $qvar = array(1 => array(serialize($payment_instrument) ,'String'));
    CRM_Core_DAO::executeQuery("REPLACE INTO civicrm_instrument SET entity_id = {$result['id']}, entity_table='civicrm_contribution_page', payment_instruments=%1", $qvar);
    $payment_page[$payment_processor_type] = $contribution_page_id;
    variable_set('civicrm_demo_payment_page', $payment_page);
  }
  else{
    $contribution_page_id = 1;
  }
  if (variable_get('civicrm_demo_sample_data', 0)) {
    $trackVars = array(
      'limit' => 2000,
      'pageId' => $contribution_page_id,
      'pageType' => 'civicrm_contribution_page',
    );
    civicrm_demo_process_track($trackVars);
  }
  return $contribution_page_id;
}

function civicrm_demo_create_custom_profile(){
  civicrm_initialize();
  $group = _civicrm_demo_create_custom_group('receipt_info', '收據需求欄位');
  $uf_group = _civicrm_demo_create_uf_group('receipt_infomation', '收據資訊');

  // custom fields
  if(!empty($group['id'])){
    $fields = array();
    $options = array(
      'no' => array('label' => '不要（無抵稅需求/不用寄給我收據）', 'value' => '0', 'is_active' => 1, 'weight' => 1),
      'yes' => array('label' => '需要（請寄給我紙本收據）', 'value' => '1', 'is_active' => 1, 'weight' => 2),
      'yes_upload' => array('label' => '需要（不用寄給我/請幫我在隔年五月上傳至國稅局）', 'value' => '2', 'is_active' => 0, 'weight' => 3),
    );
    $fields['receipt_yn'] = _civicrm_demo_create_custom_field('receipt_yn', '是否需要收據', 'String', 'Radio', $group['id'], 1, $options, 1);
    $fields['receipt_title'] = _civicrm_demo_create_custom_field('receipt_title', '收據抬頭', 'String', 'Text', $group['id']);
    $fields['receipt_serial'] = _civicrm_demo_create_custom_field('receipt_serial', '報稅憑證', 'String', 'Text', $group['id']);
    $fields['receipt_donor_credit'] = _civicrm_demo_create_custom_field('receipt_donor_credit', '捐款徵信名稱', 'String', 'Text', $group['id']);
  }

  // uf fields
  if(!empty($uf_group['id'])){
    $uf_fields = array();
    $uf_fields['receipt_yn'] = _civicrm_demo_create_uf_field('是否需要收據', 'Contribution', 'custom_'.$fields['receipt_yn']['id'], $uf_group['id'], TRUE);
    $uf_fields['receipt_title'] = _civicrm_demo_create_uf_field('收據抬頭', 'Contribution', 'custom_'.$fields['receipt_title']['id'], $uf_group['id']);
    $uf_fields['receipt_serial'] = _civicrm_demo_create_uf_field('報稅憑證', 'Contribution', 'custom_'.$fields['receipt_serial']['id'], $uf_group['id']);
    $uf_fields['receipt_donor_credit'] = _civicrm_demo_create_uf_field('捐款徵信名稱', 'Contribution', 'custom_'.$fields['receipt_donor_credit']['id'], $uf_group['id'], TRUE);
    CRM_Core_BAO_UFGroup::updateGroupTypes($uf_group['id'], array('Contribution'));
  }

  // set config
  $params = array(
    'receiptYesNo' => $fields['receipt_yn']['id'],
    'receiptTitle' => $fields['receipt_title']['id'],
    'receiptSerial' => $fields['receipt_serial']['id'],
    'receiptDonorCredit' => $fields['receipt_donor_credit']['id'],
  );
  CRM_Core_BAO_ConfigSetting::add($params);


  // copy uf group
  $params = array(
    'title' => '姓名及地址',
    'version' => 3,
  );
  $uf_group_name_addr = civicrm_api('uf_group', 'get', $params);
  $uf_group_pre_id = NULL;
  if(!empty($uf_group_name_addr['id'])){
    $copy = CRM_Core_BAO_UFGroup::copy($uf_group_name_addr['id']);
    if(!empty($copy->id) && $copy->id != $uf_group_name_addr['id']){
      $copy->title = '捐款基本資料';
      $copy->name = 'donor_basic_profile';
      $billing_location_type_id = CRM_Core_DAO::getFieldValue('CRM_Core_DAO_LocationType', 'Billing', 'id', 'name');
      CRM_Core_DAO::executeQuery("UPDATE civicrm_uf_field SET label = REPLACE(label, '住家', '收據'), location_type_id = $billing_location_type_id WHERE label LIKE '%住家%' AND uf_group_id = $copy->id");
      CRM_Core_DAO::executeQuery("UPDATE civicrm_uf_field SET label = '收據寄送地址' WHERE label LIKE '縣市（收據）' AND uf_group_id = $copy->id");
      CRM_Core_DAO::executeQuery("UPDATE civicrm_uf_field SET label = '街道地址' WHERE label LIKE '街道地址（收據）' AND uf_group_id = $copy->id");
      CRM_Core_DAO::executeQuery("UPDATE civicrm_uf_field SET is_active = 0 WHERE label LIKE '國家%' AND uf_group_id = $copy->id");
      CRM_Core_DAO::executeQuery("UPDATE civicrm_uf_field SET weight = 1 WHERE label LIKE '姓氏%' AND uf_group_id = $copy->id");
      CRM_Core_DAO::executeQuery("UPDATE civicrm_uf_field SET weight = 2 WHERE label LIKE '名字%' AND uf_group_id = $copy->id");
      $uf_fields['legal_identifier'] = _civicrm_demo_create_uf_field('身份證字號', 'Individual', 'legal_identifier', $copy->id);
      $uf_group_pre_id = $copy->id;
      $copy->save();
    }
  }

  // set contribution page default custom pre/post
  $payment_page = variable_get('civicrm_demo_payment_page', array());

  foreach($payment_page as $page_id){
    _civicrm_demo_create_uf_join($page_id, $uf_group_pre_id, $uf_group['id']);
  }
}

function civicrm_demo_process_track($trackVars = NULL){
  extract($trackVars);
  if (!$pageType) {
    $pid = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_contribution_page ORDER BY id ASC");
    if ($pid) {
      $vars = array(
        'limit' => $limit ? $limit : '500',
        'pageId' => $pid,
        'pageType' => 'civicrm_contribution_page',
      );
      civicrm_demo_process_track($vars);
    }
    $pid = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_event WHERE is_template != 1 ORDER BY id ASC");
    if ($pid) {
      $vars = array(
        'limit' => $limit ? $limit : '500',
        'pageId' => $pid,
        'pageType' => 'civicrm_event',
      );
      civicrm_demo_process_track($vars);
    } 
    return;
  }
  if (!empty($pageId) && !empty($pageType)) {
    $referrerTypes = CRM_Contribute_PseudoConstant::referrerTypes();
    unset($referrerTypes['internal']);
    unset($referrerTypes['unknown']);
    $limit = $limit ? $limit : 500;
    for($i = 0; $i < $limit; $i++){
      $params = array(
        'page_type' => $pageType,
        'page_id' => $pageId,
        'session_key' => rand(1, 1000),
        'counter' => rand(1, 3),
        'referrer_type' => array_rand($referrerTypes),
        'state' => ($pageType == 'civicrm_event') ? mt_rand(0,4) : mt_rand(1,4),
        'visit_date' => date('Y-m-d H:i:s', time()-86400*mt_rand(1, 365) + 600*mt_rand(1,12)),
      );
      CRM_Core_BAO_Track::add($params);
    }
  }
}

function _civicrm_demo_rand_lastname($type = 'chinese'){
  // chinese
  if($type == 'chinese'){
    $seed = array(
      '李','王','張','劉','陳','楊','黃','趙','周','吳','徐','孫','朱','馬','胡','郭','林','何','高','梁','鄭','羅','宋','謝','唐','韓','曹','許','鄧','蕭',' 馮','曾','程','蔡','彭','潘','袁','于','董','余',' 蘇','葉','呂','魏','蔣','田','杜','丁','沈','姜',' 范','江','傅','鍾','盧','汪','戴','崔','任','陸',' 廖','姚','方','金','邱','夏','譚','韋','賈','鄒',' 石','熊','孟','秦','閻','薛','侯','雷','白','龍','段','郝','孔','邵','史','毛','常','萬','顧','賴','武','康','賀','嚴','尹','錢','施','牛','洪','龔',
    );
  }
  else{
    $seed = array(
      'Smith','Johnson','Williams','Brown','Jones','Miller','Davis','García','Rodríguez','Wilson','Martínez','Anderson','Taylor','Thomas','Hernández','Moore','Martin','Jackson','Thompson','White','López','Lee','González','Harris','Clark','Lewis','Robinson','Walker','Pérez','Hall','Young','Allen','Sánchez','Wright','King','Scott','Green','Baker','Adams','Nelson','Hill','Ramírez','Campbell','Mitchell','Roberts','Carter','Phillips','Evans','Turner','Torres','Parker','Collins','Edwards','Stewart','Flores','Morris','Nguyen','Murphy','Rivera','Cook','Rogers','Morgan','Peterson','Cooper','Reed','Bailey','Bell','Gómez','ward','Ward','Cox','Díaz','Richardson','Wood','Watson','Brooks','Bennett','Gray','James','Reyes','Cruz','Hughes','Price','Myers','Long','Foster','Sanders','Ross','Morales','Powell','Sullivan','Russell','Ortiz','Jenkins','Gutiérrez','Perry','Butler','Barnes','Fisher',
    );
  }
  $key = array_rand($seed);
  return $seed[$key];
}

function _civicrm_demo_rand_firstname($type = 'chinese'){
  // chinese
  if($type == 'chinese'){
    $seed = array(
      '雅婷 ','怡君 ','雅雯 ','宗翰 ','怡婷 ','家豪 ','冠宇 ','詩涵 ','婉婷 ','承翰 ','郁婷 ','柏翰 ','彥廷 ','詩婷 ','鈺婷 ','冠廷 ','佩珊 ','欣怡 ','家瑋 ','思穎 ','佳蓉 ','佳穎','志豪','怡萱','筱涵',
    );
  }
  else{
    $seed = array(
      'Jacob','Ethan','Michael','Jayden','William','Alexander','Noah','Daniel','Aiden','Anthony',
      'Isabella','Mia','Ashley','Emily','Sophia','Brianna','Melanie','Samantha','Genesis','Kayla',
    );
  }
  $key = array_rand($seed);
  return $seed[$key];
}

function _civicrm_demo_rand_birth(){
  $base = time()-86400*365*3;
  $time = mt_rand(0, $base);
  $prev = mt_rand(0, 1);
  if($prev){
    $time = 0-$time;
  }
  return date('YMDHis', $time);
}

function _civicrm_demo_rand_prefix(){
  $array = array(
    '女士',
    '小姐',
    '先生',
  );
  $seed = mt_rand(0,2);
  return $array[$seed];
}

function _civicrm_demo_rand_email(){
  $name = trim(strtolower(_civicrm_demo_rand_firstname('english'))).'.'.trim(strtolower(_civicrm_demo_rand_lastname('english')));
  $company = array(
    'example.com','test.tw','ilovecivicrm.com','netivism-is-best.com.tw',
  );
  $key = array_rand($company);
  return $name.'@'.$company[$key];
}

function _civicrm_demo_rand_addr($type = 'chinese'){
  if($type == 'chinese'){
    $sp = array(
      'Changhua County','Chiayi City','Chiayi County','Hsinchu City','Hsinchu County','Hualien County','Ilan County','Kaohsiung City','Keelung City','Kinmen County','Lienchiang County','Miaoli County','Nantou County','Penghu County','Pingtung County','Taichung City','Tainan City','Taipei City','Taipei County','Taitung County','Taoyuan City','Yunlin Conuty',
    );
    $seed = array_rand($sp);
    $state_province = $sp[$seed];
    list($city, $postal_code) = _civicrm_demo_rand_city_postcode($sp[$seed]);
    $street = '中山路1段1號';
    $country = 1208;
  }
  else{
    $state_province = 'California';
    $city = 'San Francisco';
    $street = rand(1,800).', FC Road';
    $country = 1228;
  }

  return array(
    'location_type_id'       => 1,
    'is_primary'             => 1,
    'city'                   => $city,
    'state_province'         => $state_province,
    'street_address'         => $street,
    'country_id'             => $country,
  );
}

function _civicrm_demo_rand_city_postcode($city){
  $array = array(
    'Keelung City' => array(
      '仁愛區' => '200', '信義區' => '201', '中正區' => '202', '中山區' => '203', '安樂區' => '204', '暖暖區' => '205', '七堵區' => '206'
    ),
    'Taipei City' => array(
      '中正區' => '100', '大同區' => '103', '中山區' => '104', '松山區' => '105', '大安區' => '106', '萬華區' => '108', '信義區' => '110', '士林區' => '111', '北投區' => '112', '內湖區' => '114', '南港區' => '115', '文山區' => '116'
    ),
    'Taipei County' => array(
      '萬里區' => '207', '金山區' => '208', '板橋區' => '220', '汐止區' => '221', '深坑區' => '222', '石碇區' => '223',
      '瑞芳區' => '224', '平溪區' => '226', '雙溪區' => '227', '貢寮區' => '228', '新店區' => '231', '坪林區' => '232',
      '烏來區' => '233', '永和區' => '234', '中和區' => '235', '土城區' => '236', '三峽區' => '237', '樹林區' => '238',
      '鶯歌區' => '239', '三重區' => '241', '新莊區' => '242', '泰山區' => '243', '林口區' => '244', '蘆洲區' => '247',
      '五股區' => '248', '八里區' => '249', '淡水區' => '251', '三芝區' => '252', '石門區' => '253'
    ),
    'Ilan County' => array(
      '宜蘭市' => '260', '頭城鎮' => '261', '礁溪鄉' => '262', '壯圍鄉' => '263', '員山鄉' => '264', '羅東鎮' => '265',
      '三星鄉' => '266', '大同鄉' => '267', '五結鄉' => '268', '冬山鄉' => '269', '蘇澳鎮' => '270', '南澳鄉' => '272',
      '釣魚台列嶼' => '290'
    ),
    'Hsinchu City' => array(
      '新竹市' => '300'
    ),
    'Hsinchu County' => array(
      '竹北市' => '302', '湖口鄉' => '303', '新豐鄉' => '304', '新埔鎮' => '305', '關西鎮' => '306', '芎林鄉' => '307',
      '寶山鄉' => '308', '竹東鎮' => '310', '五峰鄉' => '311', '橫山鄉' => '312', '尖石鄉' => '313', '北埔鄉' => '314',
      '峨嵋鄉' => '315'
    ),
    'Taoyuan City' => array(
      '中壢市' => '320', '平鎮市' => '324', '龍潭鄉' => '325', '楊梅鎮' => '326', '新屋鄉' => '327', '觀音鄉' => '328',
      '桃園市' => '330', '龜山鄉' => '333', '八德市' => '334', '大溪鎮' => '335', '復興鄉' => '336', '大園鄉' => '337',
      '蘆竹鄉' => '338'
    ),
    'Miaoli County' => array(
      '竹南鎮' => '350', '頭份市' => '351', '三灣鄉' => '352', '南庄鄉' => '353', '獅潭鄉' => '354', '後龍鎮' => '356',
      '通霄鎮' => '357', '苑裡鎮' => '358', '苗栗市' => '360', '造橋鄉' => '361', '頭屋鄉' => '362', '公館鄉' => '363',
      '大湖鄉' => '364', '泰安鄉' => '365', '銅鑼鄉' => '366', '三義鄉' => '367', '西湖鄉' => '368', '卓蘭鎮' => '369'
    ),
    'Taichung City' => array('中區' => '400', '東區' => '401', '南區' => '402', '西區' => '403', '北區' => '404', '北屯區' => '406',
      '西屯區' => '407', '南屯區' => '408',
      '太平區' => '411', '大里區' => '412', '霧峰區' => '413', '烏日區' => '414', '豐原區' => '420', '后里區' => '421',
      '石岡區' => '422', '東勢區' => '423', '和平區' => '424', '新社區' => '426', '潭子區' => '427', '大雅區' => '428',
      '神岡區' => '429', '大肚區' => '432', '沙鹿區' => '433', '龍井區' => '434', '梧棲區' => '435', '清水區' => '436',
      '大甲區' => '437', '外埔區' => '438', '大安區' => '439'
    ),
    'Changhua County' => array(
      '彰化市' => '500', '芬園鄉' => '502', '花壇鄉' => '503', '秀水鄉' => '504', '鹿港鎮' => '505', '福興鄉' => '506',
      '線西鄉' => '507', '和美鎮' => '508', '伸港鄉' => '509', '員林市' => '510', '社頭鄉' => '511', '永靖鄉' => '512',
      '埔心鄉' => '513', '溪湖鎮' => '514', '大村鄉' => '515', '埔鹽鄉' => '516', '田中鎮' => '520', '北斗鎮' => '521',
      '田尾鄉' => '522', '埤頭鄉' => '523', '溪州鄉' => '524', '竹塘鄉' => '525', '二林鎮' => '526', '大城鄉' => '527',
      '芳苑鄉' => '528', '二水鄉' => '530'
    ),
    'Nantou County' => array(
      '南投市' => '540', '中寮鄉' => '541', '草屯鎮' => '542', '國姓鄉' => '544', '埔里鎮' => '545', '仁愛鄉' => '546',
      '名間鄉' => '551', '集集鎮' => '552', '水里鄉' => '553', '魚池鄉' => '555', '信義鄉' => '556', '竹山鎮' => '557',
      '鹿谷鄉' => '558'
    ),
    'Chiayi County' => array(
      '嘉義市' => '600'
    ),
    'Chiayi City' => array(
      '番路鄉' => '602', '梅山鄉' => '603', '竹崎鄉' => '604', '阿里山' => '605', '中埔鄉' => '606', '大埔鄉' => '607',
      '水上鄉' => '608', '鹿草鄉' => '611', '太保市' => '612', '朴子市' => '613', '東石鄉' => '614', '六腳鄉' => '615',
      '新港鄉' => '616', '民雄鄉' => '621', '大林鎮' => '622', '溪口鄉' => '623', '義竹鄉' => '624', '布袋鎮' => '625'
    ),
    'Yunlin Conuty' => array(
      '斗南鎮' => '630', '大埤鄉' => '631', '虎尾鎮' => '632', '土庫鎮' => '633', '褒忠鄉' => '634', '東勢鄉' => '635',
      '臺西鄉' => '636', '崙背鄉' => '637', '麥寮鄉' => '638', '斗六市' => '640', '林內鄉' => '643', '古坑鄉' => '646',
      '莿桐鄉' => '647', '西螺鎮' => '648', '二崙鄉' => '649', '北港鎮' => '651', '水林鄉' => '652', '口湖鄉' => '653',
      '四湖鄉' => '654', '元長鄉' => '655'
    ),
    'Tainan City' => array('中西區' => '700', '東區' => '701', '南區' => '702', '北區' => '704', '安平區' => '708', '安南區' => '709',
      '永康區' => '710', '歸仁區' => '711', '新化區' => '712', '左鎮區' => '713', '玉井區' => '714', '楠西區' => '715',
      '南化區' => '716', '仁德區' => '717', '關廟區' => '718', '龍崎區' => '719', '官田區' => '720', '麻豆區' => '721',
      '佳里區' => '722', '西港區' => '723', '七股區' => '724', '將軍區' => '725', '學甲區' => '726', '北門區' => '727',
      '新營區' => '730', '後壁區' => '731', '白河區' => '732', '東山區' => '733', '六甲區' => '734', '下營區' => '735',
      '柳營區' => '736', '鹽水區' => '737', '善化區' => '741', '大內區' => '742', '山上區' => '743', '新區區' => '744',
      '安定區' => '745'
    ),
    'Kaohsiung City' => array(
      '新興區' => '800', '前金區' => '801', '苓雅區' => '802', '鹽埕區' => '803', '鼓山區' => '804', '旗津區' => '805',
      '前鎮區' => '806', '三民區' => '807', '楠梓區' => '811', '小港區' => '812', '左營區' => '813',
      '仁武區' => '814', '大社區' => '815', '岡山區' => '820', '路竹區' => '821', '阿蓮區' => '822', '田寮區' => '823',
      '燕巢區' => '824', '橋頭區' => '825', '梓官區' => '826', '彌陀區' => '827', '永安區' => '828', '湖內區' => '829',
      '鳳山區' => '830', '大寮區' => '831', '林園區' => '832', '鳥松區' => '833', '大樹區' => '840', '旗山區' => '842',
      '美濃區' => '843', '六龜區' => '844', '內門區' => '845', '杉林區' => '846', '甲仙區' => '847', '桃源區' => '848',
      '那瑪夏區' => '849', '茂林區' => '851', '茄萣區' => '852'
    ),
    'Pingtung County' => array(
      '屏東市' => '900', '三地門' => '901', '霧臺鄉' => '902', '瑪家鄉' => '903', '九如鄉' => '904', '里港鄉' => '905',
      '高樹鄉' => '906', '鹽埔鄉' => '907', '長治鄉' => '908', '麟洛鄉' => '909', '竹田鄉' => '911', '內埔鄉' => '912',
      '萬丹鄉' => '913', '潮州鎮' => '920', '泰武鄉' => '921', '來義鄉' => '922', '萬巒鄉' => '923', '崁頂鄉' => '924',
      '新埤鄉' => '925', '南州鄉' => '926', '林邊鄉' => '927', '東港鎮' => '928', '琉球鄉' => '929', '佳冬鄉' => '931',
      '新園鄉' => '932', '枋寮鄉' => '940', '枋山鄉' => '941', '春日鄉' => '942', '獅子鄉' => '943', '車城鄉' => '944',
      '牡丹鄉' => '945', '恆春鎮' => '946', '滿州鄉' => '947'
    ),
    'Taitung County' => array(
      '臺東市' => '950', '綠島鄉' => '951', '蘭嶼鄉' => '952', '延平鄉' => '953', '卑南鄉' => '954', '鹿野鄉' => '955',
      '關山鎮' => '956', '海端鄉' => '957', '池上鄉' => '958', '東河鄉' => '959', '成功鎮' => '961', '長濱鄉' => '962',
      '太麻里鄉' => '963', '金峰鄉' => '964', '大武鄉' => '965', '達仁鄉' => '966'
    ),
    'Hualien County' => array(
      '花蓮市' => '970', '新城鄉' => '971', '秀林鄉' => '972', '吉安鄉' => '973', '壽豐鄉' => '974', '鳳林鎮' => '975',
      '光復鄉' => '976', '豐濱鄉' => '977', '瑞穗鄉' => '978', '萬榮鄉' => '979', '玉里鎮' => '981', '卓溪鄉' => '982',
      '富里鄉' => '983'
    ),
    'Linmen County' => array(
      '金沙鎮' => '890', '金湖鎮' => '891', '金寧鄉' => '892', '金城鎮' => '893', '烈嶼鄉' => '894', '烏坵鄉' => '896'
     ),
    'Lienchiang County' => array(
      '南竿鄉' => '209', '北竿鄉' => '210', '莒光鄉' => '211', '東引鄉' => '212'
    ),
    'Penghu County' => array(
      '馬公市' => '880', '西嶼鄉' => '881', '望安鄉' => '882', '七美鄉' => '883', '白沙鄉' => '884', '湖西鄉' => '885'
    ),
  );
  if(!empty($array[$city])){
    $key = array_rand($array[$city]);
    return array($key , $array[$city][$key]);
  }
  else{
    return;
  }
}

function _civicrm_demo_setup_paymentProcessor($class_name = 'Payment_ALLPAY'){
  civicrm_initialize();
  $payment_processors = array();
  $params = array(
    'version' => 3,
    'class_name' => $class_name,
  );
  $default = _civicrm_demo_processor_default($class_name);

  $result = civicrm_api('PaymentProcessorType', 'get', $params);
  if(!empty($result['count'])){
    $domain_id = CRM_Core_Config::domainID();
    foreach($result['values'] as $type_id => $p){
      $p = array_merge($p, $default);
      $payment_name = $p['name'].'金流';
      $payment_processor = array(
        'version' => 3,
        'name' => $payment_name,
      );
      $result = civicrm_api('PaymentProcessor', 'get', $payment_processor);
      if(!empty($result['count'])){
        $first = reset($result['values']);
        $payment_processors[] = $first['id'];
      }
      else{
        $payment_processor = array(
          'version' => 3,
          'domain_id' => $domain_id,
          'name' => $payment_name,
          'payment_processor_type_id' => $type_id,
          'payment_processor_type' => $p['name'],
          'is_active' => 1,
          'is_default' => 0,
          'is_test' => 0,
          'user_name' => !empty($p['user_name_label']) ? $p['user_name'] : NULL,
          'password' => !empty($p['password_label']) ? $p['password'] : NULL,
          'signature' => !empty($p['signature_label']) ? $p['signature'] : NULL,
          'subject' => !empty($p['subject_label']) ? $p['subject'] : NULL,
          'url_site' => !empty($p['url_site_default']) ? $p['url_site_default'] : NULL,
          'url_api' => !empty($p['url_api_default']) ? $p['url_api_default'] : NULL,
          'url_recur' => !empty($p['url_recur_default']) ? $p['url_recur_default'] : NULL,
          'class_name' => $p['class_name'],
          'billing_mode' => $p['billing_mode'],
          'is_recur' => $p['is_recur'],
          'payment_type' => $p['payment_type'],
        );
        $result = civicrm_api('PaymentProcessor', 'create', $payment_processor);
        if(is_numeric($result['id'])){
          $payment_processors[] = $result['id'];
          if($class_name == 'Payment_Neweb'){
            $ftp = array();
            $ftp['ftp_host'] = $p['ftp_host'];
            $ftp['ftp_user'] = $p['ftp_user'];
            $ftp['ftp_password'] = $p['ftp_password'];
            variable_set("civicrm_neweb_ftp_".$result['id'], $ftp);
            $payment_processors[] = $result['id'];
          }
        }

        $payment_processor['is_test'] = 1;
        $payment_processor['url_site'] = !empty($p['url_site_test_default']) ? $p['url_site_test_default'] : NULL;
        $payment_processor['url_api'] = !empty($p['url_api_test_default']) ? $p['url_api_test_default'] : NULL;
        $payment_processor['url_recur'] = !empty($p['url_recur_test_default']) ? $p['url_recur_test_default'] : NULL;
        $result = civicrm_api('PaymentProcessor', 'create', $payment_processor);
        if(is_numeric($result['id'])){
          if($class_name == 'Payment_Neweb'){
            variable_set("civicrm_neweb_ftp_test_".$result['id'], $ftp);
            $payment_processors[] = $result['id'];
          }
        }
      }
    }
  }
  return $payment_processors;
}

function _civicrm_demo_processor_default($class_name){
  $default = array();
  if($class_name == 'Payment_ALLPAY'){
    module_load_include('install', 'civicrm_allpay', 'civicrm_allpay');
    $default['user_name'] = '2000132';
    $default['password'] = '5294y06JbISpM5x9';
    $default['signature'] = 'v77hoKGq4kWxNNIS';
    $default['subject'] = '';
    $default['url_site'] = ALLPAY_TEST_DOMAIN . ALLPAY_URL_SITE;
    $default['url_api'] = ALLPAY_TEST_DOMAIN . ALLPAY_URL_API;
    $default['url_recur'] = ALLPAY_TEST_DOMAIN . ALLPAY_URL_RECUR;
    $default['url_site_test'] = ALLPAY_TEST_DOMAIN . ALLPAY_URL_SITE;
    $default['url_api_test'] = ALLPAY_TEST_DOMAIN . ALLPAY_URL_API;
    $default['url_recur_test'] = ALLPAY_TEST_DOMAIN . ALLPAY_URL_RECUR;
  }

  if($class_name == 'Payment_Neweb'){
    $default['user_name'] = '758200';
    $default['password'] = '456608';
    $default['signature'] = 'abcd1234';
    $default['subject'] = 'abcd1234';
    $default['url_site_default'] = 'https://testmaple2.neweb.com.tw/NewebmPP/cdcard.jsp';
    $default['url_api_default'] = 'http://testmaple2.neweb.com.tw/CashSystemFrontEnd/';
    $default['url_recur_default'] = 'https://testmaple2.neweb.com.tw/NewebmPP/cdcard.jsp';
    $default['url_site_test_default'] = 'https://testmaple2.neweb.com.tw/NewebmPP/cdcard.jsp';
    $default['url_api_test_default'] = 'http://testmaple2.neweb.com.tw/CashSystemFrontEnd/';
    $default['url_recur_test_default'] = 'https://testmaple2.neweb.com.tw/NewebmPP/cdcard.jsp';
    $default['ftp_host'] = '127.0.0.1';
    $default['ftp_user'] = 'user';
    $default['ftp_password'] = 'abcd1234';
  }

  if($class_name == 'Payment_SPGATEWAY'){
    module_load_include('install', 'civicrm_spgateway', 'civicrm_spgateway');
    $default['user_name'] = 'MS3240578';
    $default['password'] = 'DLJ2NeCY3FY54ahvmZGHVW7POQIcz48c';
    $default['signature'] = 'QReE6RCaxBgCLx78';
    $default['subject'] = '';
  }
  return $default;
}

function _civicrm_demo_create_custom_group($name, $title){
  $params = array(
    'title' => $title,
    'version' => 3,
  );
  $custom_group = civicrm_api('custom_group', 'get', $params);
  if(empty($custom_group['id'])){
    $params = array(
      'title' => $title,
      'name' => $name,
      'extends' => 'Contribution',
      'weight' => 4,
      'collapse_display' => 0,
      'style' => 'Inline',
      'help_pre' => '',
      'help_post' => '',
      'is_active' => 1,
      'version' => 3,
    );
    $custom_group = civicrm_api( 'custom_group','create',$params );
  }
  return $custom_group;
}

function _civicrm_demo_create_custom_field($name, $title, $data_type, $html_type, $group_id, $default = NULL, $options = NULL, $options_per_line = NULL){
  $params = array(
    'label' => $title,
    'version' => 3,
  );
  $last_weight = CRM_Core_DAO::singleValueQuery("SELECT weight FROM civicrm_custom_field WHERE custom_group_id = $group_id ORDER BY weight DESC");
  $field = civicrm_api('custom_field', 'get', $params);
  if(empty($field['id'])){
    $params = array(
      'custom_group_id' => $group_id,
      'name' => $name,
      'label' => $title,
      'data_type' => $data_type,
      'html_type' => $html_type,
      'default_value' => '',
      'weight' => !empty($last_weight) ? $last_weight + 1 : 1,
      'is_required' => 0,
      'is_searchable' => 1,
      'is_active' => 1,
      'version' => 3,
    );
    if (!empty($options)) {
      $params['option_values'] = $options; 
      $params['options_per_line'] = $options_per_line;
    }
    if(!is_null($default)){
      $params['default_value'] = $default;
    }
    $field = civicrm_api('custom_field', 'create', $params);
  }
  return $field;
}

function _civicrm_demo_create_uf_group($name, $title){
  $params = array(
    'title' => $title,
    'version' => 3,
  );
  $uf_group = civicrm_api('uf_group', 'get', $params);
  if(empty($uf_group['id'])){
    $params = array(
      'name' => $name,
      'title' => $title,
      'add_captcha' => 0,
      'created_date' => date('Y-m-d H:i:s'),
      'group_type' => 'Contribution',
      'help_post' => '',
      'help_pre' => '',
      'is_active' => 1,
      'is_cms_user' => 0,
      'is_edit_link' => 0,
      'is_map' => 0,
      'is_reserved' => 1,
      'is_uf_link' => 0,
      'is_update_dupe' => 1, // update duplicate contact info
      'notify' => '',
      'cancel_URL' => '',
      'post_URL' => '',
      # 'created_id' => 69, // current user
      'group' => 'null',
      'add_contact_to_group' => 'null',
      'version' => 3,
      'debug' => 1,
    );
    $uf_group = civicrm_api('uf_group', 'create', $params);

    // remove all unecessery uf_join
    if($uf_group['id']){
      $query_params = array(
        1 => array($uf_group['id'], 'Integer'),
      );
      CRM_Core_DAO::executeQuery("DELETE FROM civicrm_uf_join WHERE uf_group_id = %1 AND module != 'CiviContribute'", $query_params);
    
    }
  }
  return $uf_group;
}

function _civicrm_demo_create_uf_field($title, $field_type, $field_name, $uf_group_id, $is_required = 0){
  $params = array(
    'field_type' => $field_type,
    'field_name' => $field_name,
    'version' => 3,
  );
  $uf_field = civicrm_api('uf_field', 'get', $params);
  if(empty($uf_field['id'])){
    $last_weight = CRM_Core_DAO::singleValueQuery("SELECT weight FROM civicrm_uf_field WHERE uf_group_id = $uf_group_id ORDER BY weight DESC");
    $params = array(
      'label' => $title,
      'field_type' => $field_type,
      'field_name' => $field_name,
      'visibility' => 'User and User Admin Only',
      'is_required' => $is_required ? $is_required : 0,
      'is_searchable' => 0,
      'is_active' => 1,
      'version' => 3,
      'weight' => !empty($last_weight) ? $last_weight + 1 : 1,
      'uf_group_id' => $uf_group_id,
    );

    $uf_field = civicrm_api('uf_field', 'create', $params);
  }
  return $uf_field;
}

function _civicrm_demo_create_uf_join($page_id, $custom_pre, $custom_post){
  $params = array(
    'module' => 'CiviContribute',
    'entity_table' => 'civicrm_contribution_page',
    'entity_id' => $page_id,
    'weight' => 1,
    'uf_group_id' => $custom_pre,
    'is_active' => 1,
    'version' => 3,
    'sequential' => 1,
  );
  civicrm_api('uf_join', 'create', $params);

  $params = array(
    'module' => 'CiviContribute',
    'entity_table' => 'civicrm_contribution_page',
    'entity_id' => $page_id,
    'weight' => 2,
    'uf_group_id' => $custom_post,
    'is_active' => 1,
    'version' => 3,
    'sequential' => 1,
  );
  civicrm_api('uf_join', 'create', $params);
}

/**
 * Update message template via workflow
 *
 * @param string $group_name workflow group name
 * @param string $value_name
 *   Workflow value name. Will using this name to lookup template in civicrm/xml/templates/$value_name.tpl
 * @param string $tpl_language
 *   CRM template language, will fetch language specific file from civicrm/xml/templates/$value_name.$tpl_language.tpl for update.
 *   Fallback to non-language specific template if not found.
 * @param array $additional_params
 *   associative array for update tpl, will fallback to tpl file if not specify params
 *     option_label => option value lable at civicrm_option_value
 *     msg_title => message/workflow title at civicrm_msg_template
 *     msg_subject => message subject (when sending mail) at civicrm_msg_template
 *     msg_text => message text at civicrm_msg_template
 *     msg_html => message html at civicrm_msg_template
 * @return bool
 */
function _civicrm_demo_create_update_msg_tpl($group_name, $value_name, $tpl_language = NULL, $additional_params = array()){
  civicrm_initialize();
  if (empty($group_name)) {
    return FALSE;
  }
  if (empty($value_name)) {
    return FALSE;
  }

  $tpl = array();
  $tpl_path = drupal_get_path('module', 'civicrm').'/../xml/templates/message_templates/';
  foreach(array('subject', 'text', 'html') as $tpl_type) {
    if ($tpl_language) {
      $tmp_path = $tpl_path.$value_name.'_'.$tpl_type.'.'.$tpl_language.'.tpl';
      if (file_exists($tmp_path)) {
        $tpl[$tpl_type] = file_get_contents($tmp_path);
        continue;
      }
    }

    // fallback to no-language specific tpl
    $tmp_path = $tpl_path.$value_name.'_'.$tpl_type.'.tpl';
    if (file_exists($tmp_path)) {
      $tpl[$tpl_type] = file_get_contents($tmp_path);
    }
  }

  $option_group_id = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_group WHERE name = %1", array(
    1 => array($group_name, 'String'),
  ));
  if(empty($option_group_id)){
    return FALSE;
  }

  $group_values = CRM_Core_OptionGroup::values($group_name, FALSE, FALSE, FALSE, NULL, 'name', FALSE, FALSE, 'id');
  $group_params = array(
    'id' => $option_group_id,
  );
  $option_params = array(
    'label' => !empty($additional_params['option_label']) ? $additional_params['option_label'] : $tpl['subject'],
    'name' => $value_name,
    'is_active' => TRUE,
  );

  $option_id = array_search($value_name, $group_values);
  if(!empty($option_id)){
    $action = CRM_Core_Action::UPDATE;
  }
  else{
    $option_id = NULL;
    $action = CRM_Core_Action::ADD;
  }
  $option = CRM_Core_OptionValue::addOptionValue($option_params, $group_params, $action, $option_id);

  // check if default being customized
  // query borrow FROM CRM_Admin_Page_MessageTemplates
  $diverted_query = '
SELECT diverted.id
FROM civicrm_msg_template diverted JOIN civicrm_msg_template orig ON (
  diverted.workflow_id = orig.workflow_id AND
  orig.is_reserved = 1                    AND (
    diverted.msg_subject != orig.msg_subject OR
    diverted.msg_text    != orig.msg_text    OR
    diverted.msg_html    != orig.msg_html
  )
)
WHERE diverted.workflow_id IS NOT NULL AND orig.workflow_id IS NOT NULL AND orig.workflow_id = %1
  ';
  $diverted = CRM_Core_DAO::singleValueQuery($diverted_query, array(
    1 => array($option->id, 'Integer'),
  ));

  $params = array(
    'msg_title' => !empty($additional_params['msg_title']) ? $additional_params['msg_title'] : $tpl['subject'],
    'msg_subject' => !empty($additional_params['msg_subject']) ? $additional_params['msg_subject'] : $tpl['subject'],
    'msg_text' => !empty($additional_params['msg_text']) ? $additional_params['msg_text'] : $tpl['text'],
    'msg_html' => !empty($additional_params['msg_html']) ? $additional_params['msg_html'] : $tpl['html'],
    'is_active' => TRUE,
    'workflow_id' => $option->id,
    'is_default' => 0,
    'is_reserved' => 1,
  );

  $reserved_id = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_msg_template WHERE workflow_id = %1 AND is_default = 0 AND is_reserved = 1", array(
    1 => array($option->id, 'Integer'),
  ));
  if($reserved_id){
    $params['id'] = $reserved_id;
  }

  // add or update reserved template
  $reserved = CRM_Core_BAO_MessageTemplates::add($params);

  if(!$diverted){
    $params['is_default'] = 1;
    $params['is_reserved'] = 0;
    $default_id = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_msg_template WHERE workflow_id = %1 AND is_default = 1 AND is_reserved = 0", array(
      1 => array($option->id, 'Integer'),
    ));
    if($default_id){
      $params['id'] = $default_id;
    }
    $default = CRM_Core_BAO_MessageTemplates::add($params);
    if ($reserved->id && $default->id) {
      return TRUE;
    }
    return FALSE;
  }
  else {
    if ($reserved->id) {
      return TRUE;
    }
  }
  return FALSE;
}
