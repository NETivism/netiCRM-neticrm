<?php

function _neticrm_update_update_x324() {
  CRM_Core_DAO::executeQuery("UPDATE civicrm_state_province SET name = 'New Taipei City' WHERE name = 'Taipei County'");
  CRM_Core_DAO::executeQuery("UPDATE civicrm_state_province SET name = 'Yunlin County' WHERE name = 'Yunlin Conuty'");

  echo "Successfully update state province.";
}
