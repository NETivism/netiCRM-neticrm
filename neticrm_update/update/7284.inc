<?php
function _neticrm_update_update_7284() {
  $countries = array(
    1248 => 'Curaçao',
    1054 => 'Côte d’Ivoire',
    1252 => 'Saint Barthélemy',
    1241 => 'Åland Islands',
  );
  foreach ($countries as $country_id => $realName) {
    $sql = "SELECT name FROM civicrm_country WHERE id = %1";
    $params = array( 1 => array($country_id, 'Positive'));
    $nameInDB = CRM_Core_DAO::singleValueQuery($sql, $params);
    if ($nameInDB != $realName) {
      $sql = "UPDATE civicrm_country SET name = %2 WHERE id = %1";
      $params[2] = array($realName, 'String');
      CRM_Core_DAO::executeQuery($sql, $params);
    }
  }
}