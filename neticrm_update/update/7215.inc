<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7215(){
  CRM_Core_BAO_Cache::deleteGroup('contact fields');
  return "Clear matching field cache.";
}
