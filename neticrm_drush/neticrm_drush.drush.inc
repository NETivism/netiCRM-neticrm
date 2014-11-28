<?php

function neticrm_drush_drush_command() {
  $items = array();
  $items['neticrm-civimail'] = array(
    'description' => 'Process sending civicrm mailing job.',
    'examples' => array(
      'drush neticrm-civimail'
    ),
    'callback' => 'drush_neticrm_civimail',
  );
  return $items;
}

/**
 * A command callback
 */
function drush_neticrm_civimail() {
  _neticrm_drush_init();
  $file = drupal_get_path('module', 'civicrm').'/../bin/cron/run_civimail.inc';
  require_once($file);
  run_civimail();
}

/**
 * Bootstrap function for civicrm
 */
function _neticrm_drush_init(){
  global $user;
  $user = user_load(array('uid' => 1, 'status' => 1));
  civicrm_initialize();
}
