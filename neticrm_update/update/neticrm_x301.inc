<?php
/**
 * Example function to doing database update
 *
 * @return mixed Success: NULL, TRUE, String. Failed: FALSE or string contain "[error]"
 */
function _neticrm_update_update_x301() {
  CRM_Core_DAO::executeQuery("ALTER TABLE civicrm_log DROP FOREIGN KEY FK_civicrm_log_modified_id");
  CRM_Core_DAO::executeQuery("ALTER TABLE civicrm_log ADD CONSTRAINT FK_civicrm_log_modified_id FOREIGN KEY (modified_id) REFERENCES civicrm_contact(id) ON DELETE SET NULL");

  // success
  #return 'Messages';     # drupal 6,7,9: this will display success message
  #return TRUE            # drupal 6,7,9: will be success
  #return NULL            # drupal 6,7.9: no return value or return null will be success

  // failed, error,  schema_version will not be update
  #return '[error] ooo';  # drupal 6,7,9: this will trigger error, and display meaningful message
  #return FALSE;          # drupal 6,7,9: this will trigger error, 

  // should never use
  #throw UpdateException  # drupal 9 only, will be fatal error on drupal 6,7
  #throw DrupalUpdateException  # drupal 7 only, will be fatal error on drupal 6,9
  #return  array('success' => FALSE, 'query' => 'OOO'); # drupal 6 only, will be fatal error on drupal 7,9
}