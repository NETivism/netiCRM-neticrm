<?php

function neticrm_drush_drush_command() {
  $items = array();
  $items['neticrm-schedule'] = array(
    'description' => 'Process sending civicrm command line schedule job',
    'examples' => array(
      'Usage' => 'drush neticrm-schedule <function>',
      '- sample' => 'drush neticrm-schedule run_civimail',
      '- sample(ignore frequency)' => 'drush neticrms run_participant_processor --force',
    ),
    'arguments' => array(
      'function' => 'The schedule function to exec',
    ),
    'options' => array(
      'force' => 'Force to execute schedule',
    ),
    'aliases' => array('neticrms'),
    'callback' => 'drush_neticrm_schedule_job',
  );

  /* Deprecated , keep for compatibility */
  $items['neticrm-civimail'] = array(
    'description' => 'Process sending civicrm mailing job.',
    'examples' => array(
      'drush neticrm-civimail',
    ),
    'callback' => 'drush_neticrm_civimail',
  );

  /* Rebuild menu path table */
  $items['neticrm-rebuildmenu'] = array(
    'description' => 'Process rebuilding menu table.',
    'examples' => array(
      'drush neticrm-rebuildmenu',
    ),
    'aliases' => array(
      'neticrm-menu-rebuild',
      'neticrm-rebuild-menu',
    ),
    'callback' => 'drush_neticrm_rebuild_menu',
  );

  /* Clear civicrm cache */
  $items['neticrm-clear-cache'] = array(
    'description' => 'Clear civicrm cache table',
    'examples' => array(
      'drush neticrm-clear-cache',
    ),
    'aliases' => array('neticrm-cache-clear'),
    'callback' => 'drush_neticrm_clear_cache',
  );
  
  /* Welcome email for user 3 */
  $items['neticrm-set-domain'] = array(
    'description' => 'Set domain info.',
    'examples' => array(
      'drush neticrm-set-domain'
    ),
    'callback' => 'drush_neticrm_set_domain',
  );
  $items['neticrm-welcome-mail'] = array(
    'description' => 'send welcome email to user 3',
    'examples' => array(
      'drush neticrm-welcome-mail'
    ),
    'callback' => 'drush_neticrm_welcome_mail',
  );

  /* Reminder date update */
  $items['neticrm-member-reminder-date'] = array(
    'description' => 'update all member reminder date',
    'arguments' => array(
      'membership_type_id' => 'membership_type_id to update',
    ),
    'examples' => array(
      'drush neticrm-member-reminder-date'
    ),
    'callback' => 'drush_neticrm_member_reminder_date',
  );

  /* #26345 run batch queue */
  $items['neticrm-batch-run'] = array(
    'description' => 'Run current queuing batch process',
    'examples' => array(
      'drush neticrm-batch-run'
    ),
    'callback' => 'drush_neticrm_batch_run',
  );
  $items['neticrm-process-recurring'] = array(
    'description' => "Process recurring of certain payment processor.",
    'examples' => array(
      'drush neticrm-process-recurring' =>
      'Process all recurring',
      'drush neticrm-process-recurring --payment-processor=tappay' => 'Process tappey recurring',
    ),
    'options' => array(
      'payment-processor' => 'The payment processor need to process.',
      'contribution-recur-id' => 'The recur payment processor need to process.',
      'time' => 'Timestamp format. The recur simulated execute date.',
    ),
    'callback' => 'drush_neticrm_process_recurring',
  );

  return $items;
}

function drush_neticrm_schedule_job($function = NULL){
  $jobs = variable_get('drush_neticrm_schedule', array());

  $jobs_default = _drush_neticrm_schedule_default();
  if(empty($jobs)){
    $jobs = $jobs_default;
  }
  else{
    // use default frequency
    foreach($jobs as $k => $j){
      if(!empty($jobs_default[$k]['frequency'])){
        $jobs[$k]['frequency'] = $jobs_default[$k]['frequency'];
      }
    }
  }

  $error = FALSE;
  $function_file = drupal_get_path('module', 'civicrm').'/../bin/cron/'.$function.'.inc';
  if(empty($function)){
    $error = "You need to specify first argument. Possible values:\n  ".implode("\n  ", array_keys($jobs));
  }
  elseif(!file_exists($function_file)){
    $error = 'Schedule job function not exists.';
  }

  if($error){
    drush_set_error($error);
  }
  else{
    $now = time();
    $force = drush_get_option('force', FALSE);
    $last = $jobs[$function]['last'];
    $frequency = $jobs[$function]['frequency'];

    $hour = 0;
    $min_hour = 0;
    $max_hour = 24;
    if (!empty($jobs[$function]['hour'])) {
      $hour = $jobs[$function]['hour'];
    }
    if ($hour) {
      preg_match('/(\d)+-?(\d+)?/', $hour, $matches);
      if (count($matches) > 2 && $matches[2] > $matches[1]) {
        $min_hour = $matches[1];
        $max_hour = $matches[2];
      }
      else {
        $min_hour = $matches[1];
      }
    }
    if($force || ($now - $last > $frequency && date('G') >= $min_hour && date('G') <= $max_hour)){
      _neticrm_drush_init();
      require_once($function_file);
      $function();
      drush_log($function." run succefully.", "ok");
      $jobs[$function]['last'] = $now;
      variable_set('drush_neticrm_schedule', $jobs);
    }
    else{
      drush_log($function." didn't run. (limited on every $frequency seconds)", "warning");
    }
  }
}

/* Deprecated */
function drush_neticrm_civimail() {
  _neticrm_drush_init();

  $file = drupal_get_path('module', 'civicrm').'/../bin/cron/run_civimail.inc';
  require_once($file);
  run_civimail();
}

/* Rebuild crm menu path table from xml files. */
function drush_neticrm_rebuild_menu(){
  civicrm_initialize();
  // refs #31419, clear IDS config
  CRM_Core_IDS::initConfig(NULL, TRUE);
  CRM_Core_Menu::store();
  CRM_Core_BAO_Navigation::resetNavigation();
  drush_log("CRM menu rebuilt", "ok");
}

function drush_neticrm_set_domain(){
  if (variable_get('smtp_username', '')) {
    $smtp_mail = variable_get('smtp_username', '');
    variable_set('site_mail', $smtp_mail);

    civicrm_initialize();
    module_load_include('inc', 'neticrm_preset', 'neticrm_preset.utils');
    _neticrm_preset_domain_info();
    drush_log("netiCRM domain setted", "ok");
  }
}

function drush_neticrm_welcome_mail(){
  $account = user_load(3);
  if ($account->uid && variable_get('smtp_username', '')) {
    $op = 'register_no_approval_required';
    // Send an email.
    _user_mail_notify($op, $account);
    drush_log("netiCRM welcome mail sent", "ok");
  }
}

function drush_neticrm_clear_cache(){
  civicrm_initialize();
  $config = CRM_Core_Config::singleton();
  $config->clearDBCache();
  // refs #31419, do not clean up template_c because we will loss log files
  // $config->cleanup(1);
  
  // refs #31419, instead cleanup template compiler dir, we clear IDS config
  CRM_Core_IDS::initConfig(NULL, TRUE);
  drush_log("netiCRM cache cleared", "ok");
}


/**
 * Bootstrap function for civicrm
 */
function _neticrm_drush_init(){
  global $user;
  $user = user_load(1);
  civicrm_initialize();
}

/**
 * Scheduled Job
 */
function _drush_neticrm_schedule_default(){
  return array(
    // mass malling
    'run_civimail' => array(
      'frequency' => 180,
      'last' => 0,
    ),

    // process aborting mail
    'run_civimail_process' => array(
      'frequency' => 7200, // seconds
      'last' => 0, // unix timestamp
    ),

    // process greeting update
    'run_contact_greeting_update' => array(
      'frequency' => 21600, // seconds
      'last' => 0, // unix timestamp
    ),

    // process membership status
    'run_membership_status_update' => array(
      'frequency' => 86399,
      'last' => 0, // unix timestamp
    ),
    
    // process participant statuses
    'run_participant_processor' => array(
      'frequency' => 3600,
      'last' => 0, // unix timestamp
    ),

    // process report 
    'run_civireport_mail' => array(
      'frequency' => 86399,
      'last' => 0, // unix timestamp
      'hour' => '6-8',
    ),
  );
}

function drush_neticrm_member_reminder_date($membership_type_id = NULL) {
  civicrm_initialize();
  if ($membership_type_id && is_numeric($membership_type_id)) {
    $sql = "SELECT * FROM civicrm_membership WHERE membership_type_id = %1";
    $dao = CRM_Core_DAO::executeQuery($sql, array(
      1 => array($membership_type_id, 'Integer'),
    ));
  }
  else {
    $sql = "SELECT * FROM civicrm_membership WHERE 1";
    $dao = CRM_Core_DAO::executeQuery($sql);
  }
  while($dao->fetch()) {
    $calcDates = CRM_Member_BAO_MembershipType::getDatesForMembershipType($dao->membership_type_id, $dao->join_date, $dao->start_date, $dao->end_date);
    $params = array();
    if (!empty($calcDates['reminder_date'])) {
      $params['reminder_date'] = $calcDates['reminder_date'];
      CRM_Core_DAO::executeQuery("UPDATE civicrm_membership SET reminder_date = %2 WHERE id = %1", array(
        1 => array($dao->id, 'Integer'),
        2 => array($calcDates['reminder_date'], 'Date'),
      ));
    }
    elseif ($dao->reminder_date) {
      CRM_Core_DAO::executeQuery("UPDATE civicrm_membership SET reminder_date = NULL WHERE id = %1", array(
        1 => array($dao->id, 'Integer'),
      ));
    }
  }
  if ($membership_type_id) {
    drush_log("Updated reminder date of $dao->N members for membership type id $membership_type_id", "ok");
  }
  else {
    drush_log("Updated reminder date of $dao->N members for all membership", "ok");
  }
}

function drush_neticrm_batch_run() {
  civicrm_initialize();
  $msg = CRM_Batch_BAO_Batch::runQueue();
  if ($msg) {
    drush_log($msg, "ok");
  }
  else {
    drush_log("Queue not run", "warning");
  }
}

function drush_neticrm_process_recurring() {
  civicrm_initialize();
  watchdog('drush_neticrm', 'Executing drush neticrm-process-recurring');
  $ppName = drush_get_option('payment-processor', NULL);
  $rId = drush_get_option('contribution-recur-id', 'ridIsEmpty');
  if (!empty($ppName)) {
    $time = drush_get_option('time', CRM_REQUEST_TIME);
    if (strtolower($ppName) == 'tappay' && $rId == 'ridIsEmpty') {
      CRM_Core_Payment_TapPay::doExecuteAllRecur($time);
    }
    else if (strtolower($ppName) == 'tappay' && empty($rId)) {
      $str = "Recurring id is empty,please check\n";
      print($str);
      CRM_Core_Error::debug_log_message($str);
    }
    else if (strtolower($ppName) == 'tappay' && !empty($rId)) {
      CRM_Core_Payment_TapPay::doCheckRecur($rId, $time);
    }
  }
  else {
    $msg = "Please input with payment processor type name\nExample: \n drush neticrm-process-recurring --payment-processor=tappay\n";
    print($msg);
  }
}
