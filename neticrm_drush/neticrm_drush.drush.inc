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
      'neticrm-cache-clear',
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
    if($force || $now - $last > $frequency){
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
  CRM_Core_Menu::store();
  CRM_Core_BAO_Navigation::resetNavigation();
}

function drush_neticrm_clear_cache(){
  civicrm_initialize();
  $config = CRM_Core_Config::singleton();
  $config->clearDBCache();
  $config->cleanup(1);
  drush_log("netiCRM cache cleared", "ok");
}

/**
 * Bootstrap function for civicrm
 */
function _neticrm_drush_init(){
  global $user;
  $user = user_load(array('uid' => 1, 'status' => 1));
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
    drush_log("Queue not run", "ok");
  }
}

function drush_neticrm_process_recurring() {
  civicrm_initialize();
  $ppName = drush_get_option('payment-processor', NULL);
  if (!empty($ppName)) {
    if (strtolower($ppName) == 'tappay') {
      $time = CRM_REQUEST_TIME;
      CRM_Core_Payment_TapPay::doExecuteAllRecur($time);
    }
  }
  else {
    $msg = "Please input with payment processor type name\nExample: \n drush neticrm-process-recurring --payment-processor=tappay\n";
    print($msg);
  }
}
