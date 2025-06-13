<?php

function _neticrm_update_update_x324() {
  CRM_Core_DAO::executeQuery("UPDATE civicrm_state_province SET name = 'New Taipei City' WHERE id = 4860");
  CRM_Core_DAO::executeQuery("UPDATE civicrm_state_province SET name = 'Yunlin County' WHERE id = 4863");

  echo "Successfully update state province.";
}