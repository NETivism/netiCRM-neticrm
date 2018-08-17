<?php
function _neticrm_update_update_7249() {
  civicrm_initialize();
  $uf_group_id = 1;
  // varify uf_group_id = 1 have first_name and last_name fields;
  $sql = "SELECT id FROM civicrm_uf_field WHERE uf_group_id = 1 AND is_active = 1 AND field_name = 'first_name';";
  $first_name_fid = CRM_Core_DAO::singleValueQuery($sql);

  $sql = "SELECT id FROM civicrm_uf_field WHERE uf_group_id = 1 AND is_active = 1 AND field_name = 'last_name';";
  $last_name_fid = CRM_Core_DAO::singleValueQuery($sql);
  if(!$first_name_fid || !$last_name_fid){
    echo "The fields 'last_name', 'first_name' is not found in uf group where id = 1. Create another one.";
    // Add new uf_group to edit first_name and last_name
    $uf_group_id = _update_7249_create_uf_group('first_and_last_name', '姓氏與名字');
    $last_name_fid = _update_7249_create_uf_field('姓氏', 'Individual', 'last_name', $uf_group_id, 1);
    $first_name_fid = _update_7249_create_uf_field('名字', 'Individual', 'first_name', $uf_group_id, 1);
    if(!$uf_group_id || !$last_name_fid || !$first_name_fid){
      return false;
    }else{
      echo "Created new uf group contains 'last_name', 'first_name' fields. Where uf_group_id = $uf_group_id";
    }
  }else{
    echo "Found '姓名與地址' uf group. Id = 1.";
    _update_7249_update_uf_field_is_required($first_name_fid);
    _update_7249_update_uf_field_is_required($last_name_fid);
  }

  $_1 = _update_7249_check_add_user_custom_profile($uf_group_id, 'Profile');
  $_2 = _update_7249_check_add_user_custom_profile($uf_group_id, 'User Registration');
  $_3 = _update_7249_check_add_user_custom_profile($uf_group_id, 'User Account');
  if(!$_1 || !$_2 || !$_3){
    echo "Some error occurs when add uf_join to uf_group.Please check CiviCRM logs";
    return false
  }
}

function _update_7249_update_uf_field_is_required($uf_field_id){
  $sql = "UPDATE civicrm_uf_field SET is_required = 1 WHERE id = %1";
  $params = array(1 => array($uf_field_id, 'Integer'));
  CRM_Core_DAO::executeQuery($sql, $params);
}

function _update_7249_create_uf_group($name, $title){
  $params = array(
    'name' => $name,
    'title' => $title,
    'add_captcha' => 0,
    'created_date' => date('Y-m-d H:i:s'),
    'group_type' => 'Individual',
    'help_post' => '',
    'help_pre' => '',
    'is_active' => 1,
    'is_cms_user' => 0,
    'is_edit_link' => 0,
    'is_map' => 0,
    'is_reserved' => 0,
    'is_uf_link' => 0,
    'is_update_dupe' => 1, // update duplicate contact info
    'notify' => '',
    'cancel_URL' => '',
    'post_URL' => '',
    'group' => 'null',
    'add_contact_to_group' => 'null',
    'version' => 3,
    'debug' => 1,
  );
  $uf_group = civicrm_api('uf_group', 'create', $params);
  if($uf_group['is_error']){
    CRM_Core_Error::debug_log_message("Create uf group failed.", true);
    CRM_Core_Error::debug_var('api_return', $uf_group, false);
    return false;
  }
  return $uf_group['id'];
}

function _update_7249_create_uf_field($title, $field_type, $field_name, $uf_group_id, $is_required = 1){
  $last_weight = CRM_Core_DAO::singleValueQuery("SELECT weight FROM civicrm_uf_field WHERE uf_group_id = $uf_group_id ORDER BY weight DESC");
  $params = array(
    'label' => $title,
    'field_type' => $field_type,
    'field_name' => $field_name,
    'visibility' => 'User and User Admin Only',
    'is_required' => $is_required ? $is_required : 0,
    'is_searchable' => 0,
    'is_active' => 1,
    'version' => 3,
    'weight' => !empty($last_weight) ? $last_weight + 1 : 1,
    'uf_group_id' => $uf_group_id,
  );

  $uf_field = civicrm_api('uf_field', 'create', $params);
  if($uf_field['is_error']){
    CRM_Core_Error::debug_log_message("Create uf field failed.", true);
    CRM_Core_Error::debug_var('api_return', $uf_field, false);
    return false;
  }
  return $uf_field['id'];
}

function _update_7249_check_add_user_custom_profile($uf_group_id, $module){
  $sql = "SELECT id FROM civicrm_uf_join WHERE uf_group_id = %1 AND module = %2;";
  $params = array(
    1 => array($uf_group_id, 'Integer'),
    2 => array($module, 'String'),
  );
  if(!CRM_Core_DAO::singleValueQuery($sql, $params)){
    $params = array(
      'module' => $module,
      'uf_group_id' => $uf_group_id,
      'weight' => 1,
      'is_active' => 1,
      'version' => 3,
      'sequential' => 1,
    );
    $uf_join = civicrm_api('uf_join', 'create', $params);
    if($uf_join['is_error']){
      CRM_Core_Error::debug_log_message("Create uf join failed.", true);
      CRM_Core_Error::debug_var('api_return', $uf_join, false);
      return false;
    }
    return true;
  }
}