<?php

function _neticrm_update_update_7290() {
  $search = array(
    'html_1' => '{if $is_deductible AND $price}
        <tr>
         <td colspan="2" {$valueStyle}>
          <p>{ts 1=$price|crmMoney:$currency}The value of this premium is %1. This may affect the amount of the tax deduction you can claim. Consult your tax advisor for more information.{/ts}</p>
         </td>
        </tr>
      {/if}',
    'html_2' => '{if $is_deductible AND $price}
        <tr>
         <td colspan="2" {$valueStyle}>
          <p>{ts 1=$price|crmMoney}The value of this premium is %1. This may affect the amount of the tax deduction you can claim. Consult your tax advisor for more information.{/ts}</p>
         </td>
        </tr>
      {/if}',
    'text_1' => '{ts 1=$price|crmMoney:$currency}The value of this premium is %1. This may affect the amount of the tax deduction you can claim. Consult your tax advisor for more information.{/ts}',
    'text_2' => '{ts 1=$price|crmMoney}The value of this premium is %1. This may affect the amount of the tax deduction you can claim. Consult your tax advisor for more information.{/ts}',
  );
  foreach($search as $k => $pattern) {
    $k = preg_replace('/_\d$/', '', $k);
    $sql = "UPDATE civicrm_msg_template SET msg_{$k} = REPLACE(msg_{$k}, '$pattern', '') WHERE msg_{$k} LIKE '%{$pattern}%' AND workflow_id IS NOT NULL";
    CRM_Core_DAO::executeQuery($sql);
  }
}