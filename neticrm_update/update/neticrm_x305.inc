<?php
/**
 * Insert '550.+blacklisted' for type Relay in mailing_bounce_pattern table.
 *
 * @return mixed Success: NULL, TRUE, String. Failed: FALSE or string contain "[error]"
 */
function _neticrm_update_update_x305() {
  $ruleId = CRM_Core_DAO::singleValueQuery("SELECT bt.`id` FROM `civicrm_mailing_bounce_pattern` bp LEFT JOIN `civicrm_mailing_bounce_type` bt ON bp.`bounce_type_id` = bt.`id` WHERE bt.name = 'Relay' AND bp.`pattern` = '550.+blacklisted'");
  if (empty($ruleId)) {
    $typeId = CRM_Core_DAO::singleValueQuery("SELECT id FROM `civicrm_mailing_bounce_type` WHERE name = 'Relay'");
    $sql = "INSERT INTO `civicrm_mailing_bounce_pattern` (`bounce_type_id`, `pattern`) VALUES (%1, '550.+blacklisted');";
    CRM_Core_DAO::executeQuery($sql, array(
      1 => array($typeId, 'Positive'),
    ));
    return 'Added pattern successed.';
  }
  else {
    return 'The pattern already exists, skipped.';
  }
}