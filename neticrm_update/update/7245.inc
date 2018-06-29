<?php

function _neticrm_update_update_7245(){
  $sql = "ALTER TABLE `civicrm_contribution_page` ADD `goal_recurring` INT(10) UNSIGNED NULL DEFAULT NULL COMMENT 'The target recurring goal for this page, allows people to build a goal meter base on subscriptions' AFTER `goal_amount`";
  CRM_Core_DAO::executeQuery($sql);
  return 'Successful add goal_recurring into contirbution page';
}
