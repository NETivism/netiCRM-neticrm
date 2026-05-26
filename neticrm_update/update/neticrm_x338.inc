<?php

function _neticrm_update_update_x338() {
  civicrm_initialize();

  // Add CKEditor5 (value=4) option to wysiwyg_editor group on existing sites.
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
    return 'CKEditor5 option added to wysiwyg_editor.';
  }
  return 'CKEditor5 option already exists, skipping.';
}
