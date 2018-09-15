<?php
function _neticrm_update_update_7248($force = false) {
  civicrm_initialize();
  $exists_expire_date_ids = array();
  $check_query = "SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = 'civicrm_contribution' AND column_name = 'expire_date'";
  $exists = CRM_Core_DAO::singleValueQuery($check_query);
  if(empty($exists)){
    $alter_query = "ALTER TABLE `civicrm_contribution` ADD `expire_date` DATETIME COMMENT 'when is payment expiration' AFTER `receipt_id`";
    CRM_Core_DAO::executeQuery($alter_query);
  }else{
    CRM_Core_Error::debug_log_message("'expire_date' is already exist in civicrm_contribution table. Check expire_date value.", true);
    if($force){
      $sql = "UPDATE civicrm_contribution SET expire_date = NULL";
      $dao = CRM_Core_DAO::executeQuery($sql);
    }
    $sql = "SELECT id FROM civicrm_contribution WHERE expire_date IS NOT NULL";
    $dao = CRM_Core_DAO::executeQuery($sql);
    while($dao->fetch()){
      $exists_expire_date_ids[] = $dao->id;
    }
  }

  $expire_dates = array();
  if(module_exists('civicrm_allpay')){
    $query = db_query("SELECT data, cid FROM {civicrm_contribution_allpay} WHERE data IS NOT NULL");
    while($result = $query->fetchObject()){
      $expire_date = NULL;
      $data = json_decode($result->data, TRUE);
      if(!empty($data['#info']['ExpireDate'])){
        $expire_date = $data['#info']['ExpireDate'];
      }
      if(!empty($data['ExpireDate'])){
        $expire_date = $data['ExpireDate'];
      }
      if(!empty($expire_date)){
        $expire_dates[$result->cid] = $expire_date;
      }
    }
  }

  if(module_exists('civicrm_spgateway')){
    $expire_date = NULL;
    $query = db_query("SELECT data, cid FROM {civicrm_contribution_spgateway} WHERE data IS NOT NULL");
    while($result = $query->fetchObject()){
      $data = _civicrm_spgateway_post_decode($result->data);
      if(!empty($data['ExpireDate'])){
        $expire_date = $data['ExpireDate'].' 23:59:59';
      }
      if(!empty($expire_date)){
        $expire_dates[$result->cid] = $expire_date;
      }
    }
  }

  if(module_exists('civicrm_neweb')){
    $expire_date = NULL;
    $query = db_query("SELECT post_data, cid FROM {civicrm_contribution_neweb} WHERE post_data IS NOT NULL");
    while($result = $query->fetchObject()){
      $data = json_decode($result->post_data, TRUE);
      if(!empty($data['duedate'])){
        $expire_date = $data['duedate'].'235959';
      }
      if(!empty($expire_date)){
        $expire_dates[$result->cid] = $expire_date;
      }
    }
  }

  foreach ($expire_dates as $cid => $expire_date) {
    if(!empty($expire_date) && !in_array($cid, $exists_expire_date_ids)){
      $sql = "UPDATE civicrm_contribution SET expire_date = %1 WHERE id = %2";
      $params = array(
        1 => array( $expire_date, 'String'),
        2 => array( $cid, 'Integer'),
      );
      CRM_Core_DAO::executeQuery($sql, $params);
    }
  }
}
