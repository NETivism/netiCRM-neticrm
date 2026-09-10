<?php

/**
 * Repair the PCP supporter text template without replacing custom content.
 */
function _neticrm_update_update_x341() {
  civicrm_initialize();

  $dao = CRM_Core_DAO::executeQuery(
    'SELECT mt.id, mt.msg_text FROM civicrm_msg_template mt
     INNER JOIN civicrm_option_value ov ON ov.id = mt.workflow_id
     INNER JOIN civicrm_option_group og ON og.id = ov.option_group_id
     WHERE og.name = %1 AND ov.name = %2 AND mt.msg_text IS NOT NULL',
    [1 => ['msg_tpl_workflow_contribution', 'String'], 2 => ['pcp_supporter_notify', 'String']]
  );
  while ($dao->fetch()) {
    $text = str_replace(
      '{ts}Currently, this page is undergoing the approval process.{/t}',
      '{ts}Currently, this page is undergoing the approval process.{/ts}',
      $dao->msg_text
    );
    // Only the shipped body (after the typo fix) is known to lack the outer
    // {/if}. Do not infer missing tags in an arbitrary customized template.
    $normalized = rtrim(str_replace(["\r\n", "\r"], "\n", $text));
    if (hash('sha256', $normalized) === '5f63316faa80ba153e5ec312d9912346beb4e220ecc6a6d9623d25062d69aeba') {
      $text = rtrim($text) . "\n{/if}\n";
    }
    if ($text !== $dao->msg_text) {
      CRM_Core_DAO::executeQuery('UPDATE civicrm_msg_template SET msg_text = %1 WHERE id = %2', [
        1 => [$text, 'String'], 2 => [$dao->id, 'Integer'],
      ]);
    }
  }
  $dao->free();
  return 'Repaired PCP supporter notification text templates; custom content was preserved.';
}
