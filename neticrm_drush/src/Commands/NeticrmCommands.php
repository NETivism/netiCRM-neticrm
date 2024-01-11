<?php

namespace Drupal\neticrm_drush\Commands;

use Drush\Commands\DrushCommands;

class NeticrmCommands extends DrushCommands {
  /**
   * Run neticrm schedule job base on frequency
   *
   * @param string $function
   *   Argument that civicrm cron running
   *
   * @command neticrm:job
   * @aliases neticrms
   * @option force Force to execute schedule job
   * @usage drush neticrms run_civimail
   *   Run civimail sending job.
   * @usage drush neticrms run_membership_status_update --force
   *   Run membership status update and reminding job, and ignore frequency detection.
   */
  public function job($function = '', $options = ['force' => FALSE]) {
    $jobs = \Drupal::config('neticrm_drush.settings')->get('drush_neticrm_schedule');
    $error = FALSE;
    $function_file = \Drupal::service('extension.list.module')->getPath('civicrm').'/../bin/cron/'.$function.'.inc';
    if(empty($function)){
      $error = "You need to specify first argument. Possible values:\n  ".implode("\n  ", array_keys($jobs));
    }
    elseif(!file_exists($function_file)){
      $error = "Schedule job function not exists. Possible values:\n  ".implode("\n  ", array_keys($jobs));
    }
  
    if($error){
      throw new \Exception($error);
    }
    else{
      $now = time();
      $force = $options['force'] ? TRUE : FALSE;
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
      $settings = \Drupal::configFactory()->getEditable('neticrm_drush.settings');
      if($force || ($now - $last > $frequency && date('G') >= $min_hour && date('G') <= $max_hour)){
        $this->init();
        require_once($function_file);
        $function();
        $this->logger("neticrm_drush")->success($function." run succefully.");
        $jobs[$function]['last'] = $now;
        $settings->set('drush_neticrm_schedule', $jobs)->save();
      }
      else{
        $this->logger("neticrm_drush")->notice($function." didn't run. (limited on every $frequency seconds)");
      }
    }
  }

  /**
   * Run neticrm menu rebuild
   *
   * @command neticrm:menu-rebuild
   * @aliases neticrm-menu-rebuild,neticrm-rebuild-menu
   * @usage drush neticrm-menu-rebuild
   *   Clear civicrm menu and rebuild.
   */
  public function menu_rebuild() {
    $this->init();
    \CRM_Core_Menu::store();
    \CRM_Core_BAO_Navigation::resetNavigation();
    $this->logger("neticrm_drush")->success("netiCRM menu rebuilt.");
  }


  /**
   * Set civicrm doamin and drupal site-wide email
   *
   * @command neticrm:set-domain
   * @aliases neticrm-set-domain
   * @usage drush neticrm-set-domain
   *   Set civicrm domain info and drupal site-wide email.
   *
   * @return void
   */
  public function set_domain(){
    civicrm_initialize();
    $smtp_mail = \CRM_Mailing_BAO_Mailing::defaultFromMail();
    if (empty($smtp_mail)) {
      $smtp_settings = \Drupal::config('smtp.settings');
      $smtp_mail = $smtp_settings->get('smtp_username');
    }
    $site_info = \Drupal::configFactory()->getEditable('system.site');
    $site_info->set('mail', $smtp_mail);
    $site_info->save();

    \Drupal::moduleHandler()->loadInclude('neticrm_preset', 'utils.inc');
    _neticrm_preset_domain_info();
    $this->logger("neticrm_drush")->success("netiCRM domain setted");
  }

  /**
   * Send welcome email to user account 3
   *
   * @command neticrm:send-welcome-mail
   * @aliases neticrm-send-welcome
   * @usage drush neticrm-send-welcome
   *   Send welcome email to user account 3.
   *
   * @return void
   */
  public function send_welcome_mail(){
    $account = \Drupal\user\Entity\User::load(3);
    $smtp_settings = \Drupal::config('smtp.settings');
    if (!empty($account) && !empty($smtp_settings->get('smtp_username'))) {
      $op = 'register_no_approval_required';
      // Send an email.
      _user_mail_notify($op, $account);
      $this->logger("neticrm_drush")->success("netiCRM welcome mail sent");
    }
  }

  /**
   * Run neticrm cache clear
   *
   * @command neticrm:cache-clear
   * @aliases neticrm-cc,neticrm-cache-clear,neticrm-clear-cache
   * @usage drush neticrm-cc
   *   Clear civicrm database cache and smarty template dir.
   */
  public function cache_clear() {
    $this->init();
    $config = \CRM_Core_Config::singleton();
    $config->clearDBCache();
    // refs #31419, do not clean up template_c because we will loss log files
    // $config->cleanup(1);
    
    // refs #31419, instead cleanup template compiler dir, we clear IDS config
    \CRM_Core_IDS::initConfig(NULL, TRUE);
    $this->logger("neticrm_drush")->success("netiCRM cache cleared");
  }

  /**
   * Update specific membership type reminder date
   *
   * @command neticrm:member-reminder-date
   * @aliases neticrm-member-rdate
   * @usage drush neticrm-member-rdate <membership_type_id:optional>
   *   Update membership reminder date on given type.
   */
  public function member_reminder_date($membership_type_id = NULL) {
    civicrm_initialize();
    if ($membership_type_id && is_numeric($membership_type_id)) {
      $sql = "SELECT * FROM civicrm_membership WHERE membership_type_id = %1";
      $dao = \CRM_Core_DAO::executeQuery($sql, array(
        1 => array($membership_type_id, 'Integer'),
      ));
    }
    else {
      $sql = "SELECT * FROM civicrm_membership WHERE 1";
      $dao = \CRM_Core_DAO::executeQuery($sql);
    }
    while($dao->fetch()) {
      $calcDates = \CRM_Member_BAO_MembershipType::getDatesForMembershipType($dao->membership_type_id, $dao->join_date, $dao->start_date, $dao->end_date);
      $params = array();
      if (!empty($calcDates['reminder_date'])) {
        $params['reminder_date'] = $calcDates['reminder_date'];
        \CRM_Core_DAO::executeQuery("UPDATE civicrm_membership SET reminder_date = %2 WHERE id = %1", array(
          1 => array($dao->id, 'Integer'),
          2 => array($calcDates['reminder_date'], 'Date'),
        ));
      }
      elseif ($dao->reminder_date) {
        \CRM_Core_DAO::executeQuery("UPDATE civicrm_membership SET reminder_date = NULL WHERE id = %1", array(
          1 => array($dao->id, 'Integer'),
        ));
      }
    }
    if ($membership_type_id) {
      $this->logger("neticrm_drush")->success("Updated reminder date of $dao->N members for membership type id $membership_type_id");
    }
    else {
      $this->logger("neticrm_drush")->success("Updated reminder date of $dao->N members for all membership");
    }
  }

  /**
   * Run neticrm bash
   *
   * @command neticrm:batch
   * @aliases neticrm-batch-run,neticrm-batch
   * @usage drush neticrm-batch
   *   Run current queuing batch process.
   */
  public function batch() {
    $this->init();
    $msg = \CRM_Batch_BAO_Batch::runQueue();
    if ($msg) {
      $this->logger("neticrm_drush")->success($msg);
    }
    else {
      $this->logger("neticrm_drush")->notice("Batch not run.");
    }
  }

  /**
   * Process recurring of certain payment processor.
   *
   * @command neticrm:process-recurring
   * @aliases neticrm-process-recurring,neticrm-pr
   * @option payment-processor The payment processor need to process.
   * @option time The time to execute tappay.
   * @option contribution-recur-id The contribution recur id to process payment.
   * @usage drush neticrm-pr --payment-processor=tappay
   *   Run current queuing batch process.
   */
  public function process_recurring($options = ['payment-processor' => '', 'time' => '', 'contribution-recur-id' => '']) {
    civicrm_initialize();
    $paymentProcessor = !empty($options['payment-processor']) ? $options['payment-processor'] : NULL;
    if (empty($paymentProcessor)) {
      $this->logger("neticrm_drush")->notice("You need specify payment processor to process recurring contribution.\neg. drush neticrm-pr --payment-processor=tappay");
      return;
    }

    $paymentProcessor = strtolower($paymentProcessor);
    $rid = !empty($options['contribution-recur-id']) && is_numeric($options['contribution-recur-id']) ? $options['contribution-recur-id'] : 'ridIsEmpty';
    $time = is_numeric($options['time']) ? $options['time'] : CRM_REQUEST_TIME;

    if($paymentProcessor == 'tappay' && $rid === 'ridIsEmpty') {
      $error = \CRM_Core_Payment_TapPay::doExecuteAllRecur($time);
      if (!empty($error)) {
        throw new \Exception($error);
      }
      else {
        $this->logger("neticrm_drush")->success("$paymentProcessor recurring process success.");
      }
    }
    elseif ($paymentProcessor == 'tappay' && empty($rid)) {
      $str = "Recurring id is empty,please check\n";
      print($str);
      \CRM_Core_Error::debug_log_message($str);
    }
    elseif ($paymentProcessor == 'tappay' && !empty($rid)) {
      \CRM_Core_Payment_TapPay::doCheckRecur($rid, $time);
    }
  }

  /**
   * Set CiviCRM config
   *
   * @command neticrm:config-set
   *
   * @param string $name
   *   Name of civicrm config to search
   *
   * @param string $value
   *   Value of civicrm config to set
   *
   * @aliases neticrm-config-set,ncset
   * @option new Add new config which not exists in current object. Will not touch exists config.
   * @option force Force update current config to assign value. Use with --new for add non-exists or update.
   * @option contribution-recur-id The contribution recur id to process payment.
   * @usage drush ncset <name> <value>
   *   Update exist config to given value.
   * @usage drush ncset <name> <value> --new
   *   Add non-exist config to given value. Will not update exists config.
   * @usage drush ncset <name> <value> --force
   *   Add new config or update exists config to given value.
   */
  function config_set($name, $value, $options = ['new' => FALSE, 'force' => FALSE]) {
    civicrm_initialize();
    $params = array();
    $params[$name] = $value;
    $new = $options['new'];
    $force = $options['force'];
    $config = \CRM_Core_Config::singleton();
    $success = TRUE;

    if ($force) {
      \CRM_Core_BAO_ConfigSetting::add($params);
    }
    elseif ($new && !isset($config->$name)) {
      \CRM_Core_BAO_ConfigSetting::add($params);
    }
    elseif (empty($new) && isset($config->$name)) {
      \CRM_Core_BAO_ConfigSetting::add($params);
    }
    else {
      $success = FALSE;
    }

    if ($success) {
      $config = \CRM_Core_Config::singleton(TRUE, TRUE);
      $this->logger('neticrm_drush')->success(sprintf("Successful set config %s to value %s", $name, $config->$name));
    }
    else {
      $this->logger('neticrm_drush')->failed(sprintf("Skip set %s to value %s because using --new with exists config, or update on non-exists config.", $name, $value));
    }
  }

  /**
   * Bootstrap function for civicrm
   */
  private function init(){
    $user = \Drupal\user\Entity\User::load(1);
    civicrm_initialize();
  }
}
