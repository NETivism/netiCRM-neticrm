<?php
function _neticrm_update_update_7267(){
  CRM_Core_DAO::executeQuery("ALTER TABLE `civicrm_custom_group` MODIFY COLUMN `extends` enum ('Contact','Individual','Household','Organization','Location','Address','Contribution','ContributionPage','Activity','Relationship','Group','Membership','Participant','Event','Grant','Pledge','Case','PriceField')");
}
