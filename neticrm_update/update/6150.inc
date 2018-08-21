<?php
function _neticrm_update_update_6150() {
  civicrm_initialize();
  $all_sql = <<< EOT
UPDATE civicrm_country SET region_id=1 WHERE id = 1011;
UPDATE civicrm_country SET region_id=3 WHERE id = 1081;
UPDATE civicrm_country SET region_id=2 WHERE id = 1166;
UPDATE civicrm_country SET region_id=3 WHERE id = 1200;
UPDATE civicrm_country SET region_id=3 WHERE id = 1209;
UPDATE civicrm_country SET name='Congo, Republic of the' WHERE id = 1051;
UPDATE civicrm_country SET name='Côte d’Ivoire' WHERE id = 1054;
UPDATE civicrm_country SET name='Iran, Islamic Republic Of' WHERE id = 1103;
UPDATE civicrm_country SET name="Libya" WHERE id = 1123;
UPDATE civicrm_country SET name="Macedonia, Republic Of" WHERE id = 1128;
UPDATE civicrm_country SET name="Palestine, State of" WHERE id = 1165;
INSERT INTO civicrm_country (id, name,iso_code,region_id,is_province_abbreviated) VALUES("1247", "South Sudan", "SS", "5", "0");
INSERT INTO civicrm_country (id, name,iso_code,region_id,is_province_abbreviated) VALUES("1248", "Curaçao", "CW", "2", "0");
INSERT INTO civicrm_country (id, name,iso_code,region_id,is_province_abbreviated) VALUES("1249", "Sint Maarten (Dutch Part)", "SX", "2", "0");
INSERT INTO civicrm_country (id, name,iso_code,region_id,is_province_abbreviated) VALUES("1250", "Bonaire, Saint Eustatius and Saba", "BQ", "2", "0");
INSERT INTO civicrm_country (id, name,iso_code,region_id,is_province_abbreviated) VALUES("1251", "Kosovo", "XK", "1", "0");
INSERT INTO civicrm_country (id, name,iso_code,region_id,is_province_abbreviated) VALUES("1252", "Saint Barthélemy", "BL", "2", "0");
INSERT INTO civicrm_country (id, name,iso_code,region_id,is_province_abbreviated) VALUES("1253", "Saint Martin (French part)", "MF", "2", "0");
EOT;
  $sqls = explode("\n", $all_sql);
  foreach ($sqls as $sql) {
    // Validate is exist if sql is inserting new country
    if(substr($sql,0,6) == 'INSERT'){
      preg_match('/\("(\d{4})"/', $sql, $result);
      $valid_sql = "SELECT id FROM civicrm_country WHERE id = %1;";
      $params = array(1 => array($result[1], "Integer"));
      $is_exist = CRM_Core_DAO::singleValueQuery($valid_sql, $params);
      if($is_exist){
        CRM_Core_Error::debug_log_message("\nThe country id = {$result[1]} is exist, Skip insert the country.\n", true);
        continue;
      }
    }

    // execute sql;
    CRM_Core_Error::debug_log_message("\nProcess sql: $sql\n", true);
    CRM_Core_DAO::executeQuery($sql);
  }
}