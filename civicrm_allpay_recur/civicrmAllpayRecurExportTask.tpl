<div>
請注意！匯出資料將會包含敏感資訊，建議您使用完畢後立刻刪除，勿留檔案在本機電腦中！
</div>
<div class="crm-section">
  <div class="label">{$form.credit_item.label}</div>
  <div class="content">
    {$form.credit_item.html}
    <div class="description">刷卡項目為出現在信用卡帳單的請款項目標示。</div>
  </div>
</div>
<div class="crm-section">
  <div class="label">{$form.allpayexportpasswd.label}</div>
  <div class="content">
    {$form.allpayexportpasswd.html}
    <div class="description">匯出資料所需的密碼</div>
  </div>
</div>
<div class="crm-submit-buttons">{include file="CRM/common/formButtons.tpl" location="bottom"}</div>

