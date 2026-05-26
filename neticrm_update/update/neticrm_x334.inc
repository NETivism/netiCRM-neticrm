<?php
function _neticrm_update_update_x334() {
  civicrm_initialize();

  // Step 1: Rename CKEditor -> CKEditor4 in wysiwyg_editor option group
  $currentLabel = CRM_Core_DAO::singleValueQuery("
    SELECT ov.label
    FROM civicrm_option_value ov
    INNER JOIN civicrm_option_group og ON og.id = ov.option_group_id
    WHERE og.name = 'wysiwyg_editor' AND ov.value = '2'
  ");
  if ($currentLabel === 'CKEditor') {
    CRM_Core_DAO::executeQuery("
      UPDATE civicrm_option_value ov
      INNER JOIN civicrm_option_group og ON og.id = ov.option_group_id
      SET ov.label = 'CKEditor4'
      WHERE og.name = 'wysiwyg_editor' AND ov.value = '2'
    ");
    CRM_Core_OptionGroup::values('wysiwyg_editor', FALSE, FALSE, FALSE, NULL, 'label', TRUE, TRUE);
  }

  // Step 2: Add CKEditor5 (value=4) option to wysiwyg_editor group on existing sites.
  // New sites get this from the civicrm_data.tpl seed; old sites need it here.
  $exists = CRM_Core_DAO::singleValueQuery("
    SELECT COUNT(*)
    FROM civicrm_option_value ov
    INNER JOIN civicrm_option_group og ON og.id = ov.option_group_id
    WHERE og.name = 'wysiwyg_editor' AND ov.value = '4'
  ");
  if (empty($exists)) {
    // Values must match the civicrm_data.tpl seed row exactly so new and old
    // sites share an identical data model.
    CRM_Core_DAO::executeQuery("
      INSERT INTO civicrm_option_value
        (option_group_id, label, value, name, grouping, filter, is_default, weight, description, is_optgroup, is_reserved, is_active, component_id, visibility_id)
      SELECT og.id, 'CKEditor5', 4, NULL, NULL, 0, NULL, 4, NULL, 0, 1, 1, NULL, NULL
      FROM civicrm_option_group og
      WHERE og.name = 'wysiwyg_editor'
    ");
    CRM_Core_OptionGroup::values('wysiwyg_editor', FALSE, FALSE, FALSE, NULL, 'label', TRUE, TRUE);
  }

  // Step 3: Add editor_allow_all_content column on existing sites. New sites get it via
  // the schema XML (CREATE TABLE); old sites need this ALTER to stay in sync.
  $result = CRM_Core_DAO::executeQuery("SHOW COLUMNS FROM `civicrm_preferences` LIKE 'editor_allow_all_content'");
  if ($result->N === 0) {
    CRM_Core_DAO::executeQuery("
      ALTER TABLE `civicrm_preferences`
      ADD COLUMN `editor_allow_all_content` tinyint DEFAULT 0 COMMENT 'If set, WYSIWYG editors allow all HTML content including script tags (XSS risk).'
    ");
  }

  // Step 4: Disable TinyMCE (value=1). It has no registered QuickForm element,
  // so it is a dead option; existing editor_id=1 sites fall back to textarea.
  $tinyMceActive = CRM_Core_DAO::singleValueQuery("
    SELECT ov.is_active
    FROM civicrm_option_value ov
    INNER JOIN civicrm_option_group og ON og.id = ov.option_group_id
    WHERE og.name = 'wysiwyg_editor' AND ov.value = '1'
  ");
  if ($tinyMceActive == 1) {
    CRM_Core_DAO::executeQuery("
      UPDATE civicrm_option_value ov
      INNER JOIN civicrm_option_group og ON og.id = ov.option_group_id
      SET ov.is_active = 0
      WHERE og.name = 'wysiwyg_editor' AND ov.value = '1'
    ");
    CRM_Core_OptionGroup::values('wysiwyg_editor', FALSE, FALSE, FALSE, NULL, 'label', TRUE, TRUE);
  }

  return 'CKEditor4 rename + CKEditor5 option + editor_allow_all_content + TinyMCE disabled done.';
}
