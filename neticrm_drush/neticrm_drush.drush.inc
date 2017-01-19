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
    'callback' => 'drush_neticrm_rebuild_menu',
  );

  /* Clear civicrm cache */
  $items['neticrm-clear-cache'] = array(
    'description' => 'Clear civicrm cache table',
    'examples' => array(
      'drush neticrm-clear-cache',
    ),
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
  $config->cleanup(1);
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
  );
}

