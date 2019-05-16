<?php
function _neticrm_update_update_6163(){
  CRM_Core_DAO::executeQuery("ALTER TABLE civicrm_note MODIFY modified_date datetime;");
  return "Success changed modified_date type to datetime in civicrm_note.";
}
