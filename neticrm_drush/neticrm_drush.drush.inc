<?php

function neticrm_drush_drush_command() {
  $items = [];
  $items['neticrm-schedule'] = [
    'description' => 'Process sending civicrm command line schedule job',
    'examples' => [
      'Usage' => 'drush neticrm-schedule <function>',
      '- sample' => 'drush neticrm-schedule run_civimail',
      '- sample(ignore frequency)' => 'drush neticrms run_participant_processor --force',
    ],
    'arguments' => [
      'function' => 'The schedule function to exec',
    ],
    'options' => [
      'force' => 'Force to execute schedule',
    ],
    'aliases' => ['neticrms'],
    'callback' => 'drush_neticrm_schedule_job',
  ];

  /* Deprecated , keep for compatibility */
  $items['neticrm-civimail'] = [
    'description' => 'Process sending civicrm mailing job.',
    'examples' => [
      'drush neticrm-civimail',
    ],
    'callback' => 'drush_neticrm_civimail',
  ];

  /* Rebuild menu path table */
  $items['neticrm-rebuildmenu'] = [
    'description' => 'Process rebuilding menu table.',
    'examples' => [
      'drush neticrm-rebuildmenu',
    ],
    'aliases' => [
      'neticrm-menu-rebuild',
      'neticrm-rebuild-menu',
    ],
    'callback' => 'drush_neticrm_rebuild_menu',
  ];

  /* Clear civicrm cache */
  $items['neticrm-clear-cache'] = [
    'description' => 'Clear civicrm cache table',
    'examples' => [
      'drush neticrm-clear-cache',
    ],
    'aliases' => ['neticrm-cache-clear'],
    'callback' => 'drush_neticrm_clear_cache',
  ];

  /* Welcome email for user 3 */
  $items['neticrm-set-domain'] = [
    'description' => 'Set domain info.',
    'examples' => [
      'drush neticrm-set-domain'
    ],
    'callback' => 'drush_neticrm_set_domain',
  ];
  $items['neticrm-welcome-mail'] = [
    'description' => 'send welcome email to user 3',
    'examples' => [
      'drush neticrm-welcome-mail'
    ],
    'callback' => 'drush_neticrm_welcome_mail',
  ];

  /* Reminder date update */
  $items['neticrm-member-reminder-date'] = [
    'description' => 'update all member reminder date',
    'arguments' => [
      'membership_type_id' => 'membership_type_id to update',
    ],
    'examples' => [
      'drush neticrm-member-reminder-date'
    ],
    'callback' => 'drush_neticrm_member_reminder_date',
  ];

  /* #26345 run batch queue */
  $items['neticrm-batch-run'] = [
    'description' => 'Run current queuing batch process',
    'examples' => [
      'drush neticrm-batch-run'
    ],
    'callback' => 'drush_neticrm_batch_run',
  ];
  $items['neticrm-process-recurring'] = [
    'description' => "Process recurring of certain payment processor.",
    'examples' => [
      'drush neticrm-process-recurring' => 'Process all recurring',
      'drush neticrm-process-recurring --payment-processor=tappay' => 'Process tappey recurring',
    ],
    'options' => [
      'payment-processor' => 'The payment processor need to process.',
      'contribution-recur-id' => 'The recur payment processor need to process.',
      'time' => 'Timestamp format. The recur simulated execute date.',
    ],
    'callback' => 'drush_neticrm_process_recurring',
  ];
  $items['neticrm-config-set'] = [
    'description' => 'Set CiviCRM CRM_Core_Config object parameter',
    'examples' => [
      'drush neticrm-config-set <name> <value>' => 'Update config to new value. Empty string clear config.',
      'drush neticrm-config-set <name> <value> --new' => 'Add new config, will not change current config when exists.',
    ],
    'aliases' => ['ncset'],
    'arguments' => [
      'name' => 'The config name to find',
      'value' => 'The config value to set',
    ],
    'options' => [
      'new' => 'Add new config which not exists in current object. Will not touch exists config.',
      'force' => 'Force update current config to assign value. Use with --new for add non-exists or update',
    ],
    'callback' => 'drush_neticrm_config_set',
  ];

  return $items;
}

function drush_neticrm_schedule_job($function = NULL) {
  if (!empty(variable_get('maintenance_mode', ''))) {
    drush_log("Site in maintenance mode, no job execute.", "warning");
    return;
  }
  $jobs = variable_get('drush_neticrm_schedule', []);

  $jobs_default = _drush_neticrm_schedule_default();
  if (empty($jobs)) {
    $jobs = $jobs_default;
  }
  else {
    // use default frequency
    foreach ($jobs as $k => $j) {
      if (!empty($jobs_default[$k]['frequency'])) {
        $jobs[$k]['frequency'] = $jobs_default[$k]['frequency'];
      }
    }
  }

  $error = FALSE;
  $function_file = drupal_get_path('module', 'civicrm').'/../bin/cron/'.$function.'.inc';
  if (empty($function)) {
    $error = "You need to specify first argument. Possible values:\n  ".implode("\n  ", array_keys($jobs));
  }
  elseif (!file_exists($function_file)) {
    $error = 'Schedule job function not exists.';
  }

  if ($error) {
    drush_set_error($error);
  }
  else {
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
    if ($force || ($now - $last > $frequency && date('G') >= $min_hour && date('G') <= $max_hour)) {
      _neticrm_drush_init();
      require_once($function_file);
      $function();
      drush_log($function." run succefully.", "ok");
      $jobs[$function]['last'] = $now;
      variable_set('drush_neticrm_schedule', $jobs);
    }
    else {
      drush_log($function." didn't run. (limited on every $frequency seconds)", "warning");
    }
  }
}

/* Deprecated */
function drush_neticrm_civimail() {
  if (!empty(variable_get('maintenance_mode', ''))) {
    drush_log("Site in maintenance mode, no job execute.", "warning");
    return;
  }
  _neticrm_drush_init();

  $file = drupal_get_path('module', 'civicrm').'/../bin/cron/run_civimail.inc';
  require_once($file);
  run_civimail();
}

/* Rebuild crm menu path table from xml files. */
function drush_neticrm_rebuild_menu() {
  civicrm_initialize();
  // refs #31419, clear IDS config
  CRM_Core_IDS::initConfig(NULL, TRUE);
  CRM_Core_Menu::store();
  CRM_Core_BAO_Navigation::resetNavigation();
  drush_log("CRM menu rebuilt", "ok");
}

function drush_neticrm_set_domain() {
  if (variable_get('smtp_username', '')) {
    $smtp_mail = variable_get('smtp_username', '');
    variable_set('site_mail', $smtp_mail);

    civicrm_initialize();
    module_load_include('inc', 'neticrm_preset', 'neticrm_preset.utils');
    _neticrm_preset_domain_info();
    drush_log("netiCRM domain setted", "ok");
  }
}

function drush_neticrm_welcome_mail() {
  $account = user_load(3);
  if ($account->uid && variable_get('smtp_username', '')) {
    $op = 'register_no_approval_required';
    // Send an email.
    _user_mail_notify($op, $account);
    drush_log("netiCRM welcome mail sent", "ok");
  }
}

function drush_neticrm_clear_cache() {
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
function _neticrm_drush_init() {
  global $user;
  $user = user_load(1);
  civicrm_initialize();
}

/**
 * Scheduled Job
 */
function _drush_neticrm_schedule_default() {
  return [
    // mass malling
    'run_civimail' => [
      'frequency' => 180,
      'last' => 0,
    ],

    // process aborting mail
    'run_civimail_process' => [
      'frequency' => 7200, // seconds
      'last' => 0, // unix timestamp
    ],

    // process greeting update
    'run_contact_greeting_update' => [
      'frequency' => 21600, // seconds
      'last' => 0, // unix timestamp
    ],

    // process membership status
    'run_membership_status_update' => [
      'frequency' => 86399,
      'last' => 0, // unix timestamp
    ],

    // process participant statuses
    'run_participant_processor' => [
      'frequency' => 3600,
      'last' => 0, // unix timestamp
    ],

    // process report
    'run_civireport_mail' => [
      'frequency' => 86399,
      'last' => 0, // unix timestamp
      'hour' => '6-8',
    ],
  ];
}

function drush_neticrm_member_reminder_date($membership_type_id = NULL) {
  civicrm_initialize();
  if ($membership_type_id && is_numeric($membership_type_id)) {
    $sql = "SELECT * FROM civicrm_membership WHERE membership_type_id = %1";
    $dao = CRM_Core_DAO::executeQuery($sql, [
      1 => [$membership_type_id, 'Integer'],
    ]);
  }
  else {
    $sql = "SELECT * FROM civicrm_membership WHERE 1";
    $dao = CRM_Core_DAO::executeQuery($sql);
  }
  while ($dao->fetch()) {
    $calcDates = CRM_Member_BAO_MembershipType::getDatesForMembershipType($dao->membership_type_id, $dao->join_date, $dao->start_date, $dao->end_date);
    $params = [];
    if (!empty($calcDates['reminder_date'])) {
      $params['reminder_date'] = $calcDates['reminder_date'];
      CRM_Core_DAO::executeQuery("UPDATE civicrm_membership SET reminder_date = %2 WHERE id = %1", [
        1 => [$dao->id, 'Integer'],
        2 => [$calcDates['reminder_date'], 'Date'],
      ]);
    }
    elseif ($dao->reminder_date) {
      CRM_Core_DAO::executeQuery("UPDATE civicrm_membership SET reminder_date = NULL WHERE id = %1", [
        1 => [$dao->id, 'Integer'],
      ]);
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
  if (!empty(variable_get('maintenance_mode', ''))) {
    drush_log("Site in maintenance mode, no job execute.", "warning");
    return;
  }
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
  if (!empty(variable_get('maintenance_mode', ''))) {
    drush_log("Site in maintenance mode, no job execute.", "warning");
    return;
  }
  civicrm_initialize();
  watchdog('drush_neticrm', 'Executing drush neticrm-process-recurring');
  $ppName = drush_get_option('payment-processor', NULL);
  $rId = drush_get_option('contribution-recur-id', 'ridIsEmpty');
  if (!empty($ppName)) {
    $time = drush_get_option('time', CRM_REQUEST_TIME);
    if (!empty($rId) && $rId === 'ridIsEmpty') {
      switch ($ppName) {
        case 'tappay':
          CRM_Core_Payment_TapPay::doExecuteAllRecur($time);
          break;
        case 'spgateway':
          CRM_Core_Payment_SPGATEWAY::doExecuteAllRecur($time);
          break;
      }
    }
    elseif (!empty($rId) && is_numeric($rId)) {
      switch ($ppName) {
        case 'tappay':
          CRM_Core_Payment_TapPay::doCheckRecur($rId, $time);
          break;
        case 'spgateway':
          CRM_Core_Payment_SPGATEWAY::doCheckRecur($rId, $time);
          break;
      }
    }
    else {
      $msg = "Recurring id is empty. Nothing execute.";
    }
  }
  else {
    $msg = "Please input with payment processor type name\nExample: \n drush neticrm-process-recurring --payment-processor=tappay\n";
    print($msg);
  }
}

function drush_neticrm_config_set($name, $value) {
  civicrm_initialize();
  $params = [];
  $params[$name] = $value;
  $new = drush_get_option('new');
  $force = drush_get_option('force');
  $config = CRM_Core_Config::singleton();
  $success = TRUE;

  if ($force) {
    CRM_Core_BAO_ConfigSetting::add($params);
  }
  elseif ($new && !isset($config->$name)) {
    CRM_Core_BAO_ConfigSetting::add($params);
  }
  elseif (empty($new) && isset($config->$name)) {
    CRM_Core_BAO_ConfigSetting::add($params);
  }
  else {
    $success = FALSE;
  }

  if ($success) {
    $config = CRM_Core_Config::singleton(TRUE, TRUE);
    drush_log(sprintf("Successful set config %s to value %s", $name, $config->$name), "ok");
  }
  else {
    drush_log(sprintf("Skip set %s to value %s because using --new with exists config, or update on non-exists config.", $name, $value), "error");
  }
}
