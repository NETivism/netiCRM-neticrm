<?php
function _neticrm_update_update_7284() {
  $countries = [
    1248 => 'Curaçao',
    1054 => 'Côte d’Ivoire',
    1252 => 'Saint Barthélemy',
    1241 => 'Åland Islands',
  ];
  foreach ($countries as $country_id => $realName) {
    $sql = "SELECT name FROM civicrm_country WHERE id = %1";
    $params = [ 1 => [$country_id, 'Positive']];
    $nameInDB = CRM_Core_DAO::singleValueQuery($sql, $params);
    if ($nameInDB != $realName) {
      $sql = "UPDATE civicrm_country SET name = %2 WHERE id = %1";
      $params[2] = [$realName, 'String'];
      CRM_Core_DAO::executeQuery($sql, $params);
    }
  }
}
