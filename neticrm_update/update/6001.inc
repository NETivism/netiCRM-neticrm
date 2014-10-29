<?php
/**
 * @file
 * Update for 6000
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6001(){
  $q = CRM_Core_DAO::executeQuery("ALTER TABLE `civicrm_custom_group` MODIFY COLUMN `extends` enum ('Contact','Individual','Household','Organization','Location','Address','Contribution','ContributionPage','Activity','Relationship','Group','Membership','Participant','Event','Grant','Pledge','Case')");
  return "Successful update for adding contribution page support into civicrm custom field group";
}
