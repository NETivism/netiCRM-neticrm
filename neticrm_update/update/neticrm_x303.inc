<?php
/**
 * Example function to doing database update
 *
 * @return mixed Success: NULL, TRUE, String. Failed: FALSE or string contain "[error]"
 */
function _neticrm_update_update_x303() {
  $dao = CRM_Core_DAO::executeQuery("SELECT data_type FROM information_schema.columns WHERE `table_name` = 'civicrm_msg_template' AND column_name = 'msg_text'");
  $dao->fetch();
  if ($dao->data_type !== 'longtext') {
    $sql = "ALTER TABLE `civicrm_msg_template` CHANGE `msg_text` `msg_text` LONGTEXT NULL DEFAULT NULL COMMENT 'Text formatted message'";
    CRM_Core_DAO::executeQuery($sql);
    $sql = "ALTER TABLE `civicrm_msg_template` CHANGE `msg_html` `msg_html` LONGTEXT NULL DEFAULT NULL COMMENT 'HTML formatted message'";
    CRM_Core_DAO::executeQuery($sql);
    return 'civicrm_msg_tempalte.msg_text and html increased size.';
  }
  else {
    return 'Already longtext, skipped';
  }
}
