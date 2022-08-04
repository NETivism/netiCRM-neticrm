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
    $function_file = drupal_get_path('module', 'civicrm').'/../bin/cron/'.$function.'.inc';
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
  public function process_recurring($options = ['payment-processor' => '']) {
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
   * Bootstrap function for civicrm
   */
  private function init(){
    $user = \Drupal\user\Entity\User::load(1);
    civicrm_initialize();
  }
}