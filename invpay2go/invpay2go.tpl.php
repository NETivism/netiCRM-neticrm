<?php
/**
echo $receipt['MerchantID'];
echo $receipt['MerchantOrderNo'];
echo $receipt['InvoiceNumber'];
echo $receipt['TotalAmt'];
echo $receipt['InvoiceTransNo'];
echo $receipt['RandomNum'];
echo $receipt['CreateTime'];
echo $receipt['BarCode'];
echo $receipt['QRcodeL'];
echo $receipt['QRcodeR'];
// additional variable
echo $receipt['QRcodeLimg'];
echo $receipt['QRcodeRimg'];
echo $receipt['ChineseMonth'];
echo $receipt['SiteName'];
**/
?>
<style>
/* reset */
.crm-container {
  margin: 0;
  line-height: 1.0;
}
.crm-container table, .crm-container tr, .crm-container td {
  width: auto;
  padding: 0;
  margin: 0;
}
/* main */
.tax-receipt {
  max-width: 180px;
  font-size: 10pt;
  line-height: 1.25;
  border: 1px solid #CCC;
  padding: 5px;
}
.tax-receipt .center {
  text-align: center;
}
.tax-receipt h1,
.tax-receipt h2,
.tax-receipt h3,
.tax-receipt h4 {
  text-align: center !important;
  margin:0;
}
.tax-receipt h2 {
  font-size: 14pt;
}
.tax-receipt h3 {
  font-size: 12pt;
}
.tax-receipt .tax-receipt-table {
  width: 100%;
  border: 0;
}
.tax-receipt .tax-receipt-row {
  width: 100%;
}
.tax-receipt .tax-receipt-col {
  vertical-align: top;
  width: 50%;
  font-size: 9pt;
  border: 0;
}
.tax-receipt .tax-receipt-table .barcode {
  width: 95%;
}
.tax-receipt .tax-receipt-table .qrcode {
  width: 100%;
  height: auto;
}
.tax-receipt table.items {
  text-align: right;
  border-bottom: 1px solid #777;
}
.tax-receipt table.summary {
  text-align: right;
}
</style>
<div id="invpay2go-receipt-<?php print $receipt['RandomNum']; ?>" class="tax-receipt invpay2go-wrapper">
  <h3><?php print $receipt['title']; ?></h3>
  <h2>電子發票證明聯</h2>
  <h3><?php print $receipt['ChineseMonth']; ?></h3>
  <h3><?php print $receipt['InvoiceNumber']; ?></h3>
  <div class="center"><?php print $receipt['CreateTime']; ?></div>
  <table class="tax-receipt-table">
    <tr class="tax-receipt-row">
      <td class="tax-receipt-col">隨機碼：<?php print $receipt['RandomNum']; ?></td>
      <td class="tax-receipt-col">總計：＄<?php print $receipt['TotalAmt']; ?></td>
    </tr>
    <tr class="tax-receipt-row">
      <td class="tax-receipt-col">賣方：<?php print $receipt['serial']; ?></td>
      <td class="tax-receipt-col">買方：<?php print $receipt['BuyerUBN']; ?></td>
    </tr>
    <?php if ($receipt['Barcode']) { ?><tr class="tax-receipt-row"><td class="tax-receipt-col-single center" colspan="2"><img class="barcode" src="<?php print $receipt['Barcode']; ?>" /></td></tr><?php } ?>
    <tr class="tax-receipt-row">
      <?php if ($receipt['QRcodeLimg']) { ?><td class="tax-receipt-col"><img class="qrcode" src="<?php print $receipt['QRcodeLimg']; ?>" /></td><?php } ?>
      <?php if ($receipt['QRcodeRimg']) { ?><td class="tax-receipt-col"><img class="qrcode" src="<?php print $receipt['QRcodeRimg']; ?>" /></td><?php } ?>
    </tr>
  </table>
  <div class="center">發票號碼：<?php print $receipt['InvoiceNumber']; ?></div>
  <h3>交易明細資料</h3>
  <table class="tax-receipt-table items">
    <?php foreach($receipt['ItemDetail'] as $item) { ?>
    <tr class="tax-receipt-row">
      <td class="tax-receipt-col"><?php print $item->ItemName.'x'.$item->ItemNum ?></td>
      <td class="tax-receipt-col"><?php print '$'.$item->ItemPrice; ?></td>
    </tr>
    <?php } ?>
  </table>
  <table class="tax-receipt-table summary">
    <tr class="tax-receipt-row">
      <td class="tax-receipt-col">銷售額：</td>
      <td class="tax-receipt-col"><?php print '$'.$receipt['Amt']; ?></td>
    </tr>
    <?php if ($receipt['TaxAmt']) { ?>
    <tr class="tax-receipt-row">
      <td class="tax-receipt-col">稅額：</td>
      <td class="tax-receipt-col"><?php print '$'.$receipt['TaxAmt']; ?></td>
    </tr>
    <?php } ?>
    <tr class="tax-receipt-row">
      <td class="tax-receipt-col">總計：</td>
      <td class="tax-receipt-col"><?php print '$'.$receipt['TotalAmt']; ?></td>
    </tr>
  </table>
</div>
