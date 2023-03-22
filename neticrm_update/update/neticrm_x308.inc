<?php

/**
 * Update extends enum for recur in civicrm_custom_group
 *
 * @return mixed Success: NULL, TRUE, String. Failed: FALSE or string contain "[error]"
 */
function _neticrm_update_update_x308() {
  $q = CRM_Core_DAO::executeQuery("ALTER TABLE civicrm_custom_group MODIFY COLUMN extends enum ('Contact','Individual','Household','Organization','Location','Address','Contribution','ContributionPage','ContributionRecur','Activity','Relationship','Group','Membership','Participant','Event','Grant','Pledge','Case', 'PriceField')");
  return "Successful update for adding contribution page support into civicrm custom field group";
}
