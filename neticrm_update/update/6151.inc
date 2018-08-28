<?php
function _neticrm_update_update_6151() {
  $subdivisions = array(
    '01' => 'Municipality of das Ilhas',
    '02' => 'Municipality of Macau',
  );
  foreach ($subdivisions as $abbreviation => $name) {
    _macao_insert_subdivision($name, $abbreviation);
  }
}

function _macao_insert_subdivision($name, $abbreviation){
  civicrm_initialize();
  $sql = "SELECT id FROM civicrm_state_province WHERE country_id = 1127 AND name = %1";
  $params = array(1 => array($name, "String"));
  $id = CRM_Core_DAO::singleValueQuery($sql, $params);
  if(empty($id)){
    $sql = 'INSERT INTO civicrm_state_province (id, country_id, abbreviation, name) VALUES (NULL, 1127, %1, %2)';
    $params = array(
      1 => array($abbreviation, 'Integer'),
      2 => array($name, 'String'),
    );
    CRM_Core_DAO::executeQuery($sql, $params);
  }else{
    CRM_Core_Error::debug_log_message("\n    '".$name."' is already in DB.\n", true);
  }
}