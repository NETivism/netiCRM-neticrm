<?php
function _neticrm_update_update_7268(){
  // wrong traffic source on these date #25894
  $sql = "DELETE from civicrm_track WHERE referrer_type = 'unknown' AND visit_date >= '2019-07-17 12:00:00' AND visit_date <= '2019-07-19 18:00:00' AND entity_id IS NULL";
  CRM_Core_DAO::executeQuery($sql);

  // update all the NULL referrer_type to unknown when there is entity_id
  $sql = "UPDATE civicrm_track SET referrer_type = 'unknown' WHERE entity_id IS NOT NULL AND referrer_type IS NULL";
  CRM_Core_DAO::executeQuery($sql);
}