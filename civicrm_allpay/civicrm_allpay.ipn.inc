<?php
civicrm_initialize();
require_once 'CRM/Core/Payment/BaseIPN.php';
date_default_timezone_set("Asia/Taipei");

/**
 * Helper function for receive pingback from payment server
 */
function civicrm_allpay_ipn($instrument, $post = NULL, $get = NULL, $print = TRUE){
  // detect variables
  $post = !empty($post) ? $post : $_POST;
  $get = !empty($get) ? $get : $_GET;

  // detect variables
  if(empty($post)){
    watchdog("civicrm_allpay", "Could not find POST data from payment server");
    drupal_not_found();
  }
  else{
    $component = $get['module'];
    if(!empty($component)){
      $ipn = new CRM_Core_Payment_ALLPAYIPN($post, $get);
      $result = $ipn->main($component, $instrument);
      if(!empty($result) && $print){
        echo $result;
      }
      else{
        return $result;
      }
    }
    else{
      watchdog('civicrm_allpay', "Could not get module name from request url");
      drupal_not_found();
    }
  }
}
