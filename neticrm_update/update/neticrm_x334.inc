<?php
function _neticrm_update_update_x334() {
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
    return 'CKEditor option label updated to CKEditor4.';
  }
  return 'CKEditor option label already updated or not found, skipping.';
}
