<?php
function _neticrm_update_update_7283() {
  $subdivisions = array(
    'C' => 'Central Region',
    'E' => 'East Region',
    'NE' => 'North-East Region',
    'N' => 'North Region',
    'W' => 'West Region',
  );
  foreach ($subdivisions as $abbreviation => $name) {
    _insert_state_province($name, $abbreviation);
  }
}

function _insert_state_province($name, $abbreviation){
  civicrm_initialize();
  $sql = "SELECT id FROM civicrm_state_province WHERE country_id = 1191 AND name IN (%1, %2)";
  $params = array(
    1 => array($name, "String"),
    2 => array(ts($name), "String"),
  );
  $id = CRM_Core_DAO::singleValueQuery($sql, $params);
  if(empty($id)){
    $sql = 'INSERT INTO civicrm_state_province (id, country_id, abbreviation, name) VALUES (NULL, 1191, %1, %2)';
    $params = array(
      1 => array($abbreviation, 'String'),
      2 => array(ts($name), 'String'),
    );
    CRM_Core_DAO::executeQuery($sql, $params);
  }else{
    CRM_Core_Error::debug_log_message("\n    '".ts($name)."' is already in DB.\n", true);
  }
}