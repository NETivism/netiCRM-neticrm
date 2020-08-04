<?php
function _neticrm_update_update_7285() {
  $sql = "SELECT * FROM civicrm_address WHERE city LIKE '%左區區%'";
  $dao = CRM_Core_DAO::executeQuery($sql);
  while ($dao->fetch()) {
    CRM_Core_DAO::executeQuery("UPDATE civicrm_address SET city = '左鎮區' WHERE id = %1", array(1 => array($dao->id, 'Positive')));
  }

  $sql = "SELECT * FROM civicrm_address WHERE street_address LIKE '左鎮區%'";
  $dao = CRM_Core_DAO::executeQuery($sql);
  while ($dao->fetch()) {
    if (empty($dao->city) || $dao->city == '左鎮區') {
      $address = mb_substr($dao->street_address, 3);
      CRM_Core_DAO::executeQuery("UPDATE civicrm_address SET city = '左鎮區', street_address = %2 WHERE id = %1", array(
        1 => array($dao->id, 'Positive'),
        2 => array($address, 'String'),
      ));
    }
  }
  echo "SUCCESS update 左鎮區、左區區 Error.";
}