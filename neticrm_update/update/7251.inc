<?php
function _neticrm_update_update_7251() {
  $subdivisions = [
    '01' => 'Macao',
    '02' => 'Taipa',
    '03' => 'Coloane',
  ];
  foreach ($subdivisions as $abbreviation => $name) {
    _macao_insert_subdivision($name, $abbreviation);
  }
}

function _macao_insert_subdivision($name, $abbreviation){
  civicrm_initialize();
  $sql = "SELECT id FROM civicrm_state_province WHERE country_id = 1127 AND name = %1";
  $params = [1 => [$name, "String"]];
  $id = CRM_Core_DAO::singleValueQuery($sql, $params);
  if(empty($id)){
    $sql = 'INSERT INTO civicrm_state_province (id, country_id, abbreviation, name) VALUES (NULL, 1127, %1, %2)';
    $params = [
      1 => [$abbreviation, 'Integer'],
      2 => [$name, 'String'],
    ];
    CRM_Core_DAO::executeQuery($sql, $params);
  }else{
    CRM_Core_Error::debug_log_message("\n    '".$name."' is already in DB.\n", true);
  }
}