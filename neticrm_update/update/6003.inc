<?php
/**
 * @file
 * Update for 6003
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6003(){
  // see if exists group
  civicrm_initialize();
  $id = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_group WHERE name = 'msg_tpl_workflow_receipt'");
  if(empty($id)){
    return FALSE;
  }

  $group_params = array(
    'id' => $id,
  );
  $option_params = array(
    'label' => '紙本收據 - 年度收據',
    'name' => 'receipt_letter_year',
    'is_active' => TRUE,
  );
  $action = CRM_Core_Action::ADD;
  $option = CRM_Core_OptionValue::addOptionValue($option_params, $group_params, $action, $option_id);
  $html = '
  {if $single_page_letter}
  <div class="single-page-header">
    <div class="info">
      <div class="address">{$address}</div>
      <div><span class="web-name">{$sort_name}收</span></div>
    </div>
  </div>
  {/if}
  {foreach from=$print_type key=type item=type_label}
  {if $type eq "copy"}
  <div class="line"></div>
  {/if}
  <div class="receipt {$type}">
    <div class="receipt-head">
      <div class="logo"><img src="{$logo}" height="30" /></div>
      <div class="title">收據 Receipt</div>
      <div class="date"><label>日期：</label>{$receipt_date}</div>
      <div class="serial">
        <label class="type">{$type_label}</label><br />
        <label>收據編號：</label>{$receipt_id}<br />
      </div>
    </div>
    <div class="receipt-body">
      <table>
        <tr>
          <td class="col-1">姓名/抬頭</td>
          <td class="col-2">{$sort_name}</td>
          <td class="col-3 signature">協會簽章</td>
        </tr>
        <tr>
          <td class="col-1">身分證字號/統一編號</td>
          <td class="col-2">{$serial_id}</td>
          <td class="col-3" rowspan="4">&nbsp;</td>
        </tr>
        <tr>
          <td class="col-1">收入用途/類別</td>
          <td class="col-2">{$contributionTypeName}</td>
        </tr>
        <tr>
          <td class="col-1">繳費方式</td>
          <td class="col-2">{$instrument}</td>
        </tr>
        <tr>
          <td class="col-1">金額（大寫）新台幣</td>
          <td class="col-2">{$amount|crmMoney:$currency:"chinese"}</td>
        </tr>
        <tr>
          <td class="col-1">金額（小寫）</td>
          <td class="col-2">{$amount|crmMoney:$currency} 元整</td>
          <td class="col-3">經辦人：</td>
        </tr>
      </table>
    </div>
    <div class="receipt-footer">
      <table>
        <tr>
          <td class="col-1">組織資訊</td>
          <td class="col-2">{$receiptOrgInfo}</td>
          <td class="col-3">備註說明</td>
          <td class="col-4">{$receiptDescription}</td>
        </tr>
      </table>
    </div>
  </div>
  {/foreach}
  ';

  $params = array(
    'msg_title' => '紙本收據 - 年度整合收據',
    'msg_subject' => '收據',
    'msg_text' => '目前僅提供HTML版本收據',
    'msg_html' => $html,
    'is_active' => TRUE,
    'workflow_id' => $option->id,
    'is_default' => 1,
    'is_reserved' => 0,
  );
  // add default template
  CRM_Core_BAO_MessageTemplates::add($params);
  $params['is_default'] = 0;
  $params['is_reserved'] = 1;

  // add reserved template
  CRM_Core_BAO_MessageTemplates::add($params);
}
