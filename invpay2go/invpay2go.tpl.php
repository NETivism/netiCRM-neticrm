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
.tax-receipt {
  max-width: 300px;
  font-size: 10pt;
}
.tax-receipt .center {
  text-align: center;
}
.tax-receipt h1,
.tax-receipt h2,
.tax-receipt h3,
.tax-receipt h4 {
  text-align: center;
  margin:0;
}
.tax-receipt h2 {
  font-size: 14pt;
}
.tax-receipt h3 {
  font-size: 12pt;
}
.tax-receipt .tax-receipt-table {
  display: table;
  width: 100%;
}
.tax-receipt .tax-receipt-col {
  display: table-cell;
  vertical-align: top;
  width: 150px;
}
.tax-receipt .tax-receipt-table .barcode {
  width: 95%;
}
.tax-receipt .tax-receipt-table .qrcode {
  width: 140px;
  height: 140px;
}
</style>
<div id="invpay2go-receipt-<?php print $receipt['RandomNum']; ?>" class="tax-receipt invpay2go-wrapper">
  <h3><?php print $receipt['title']; ?></h3>
  <h2>電子發票證明聯</h2>
  <h3><?php print $receipt['ChineseMonth']; ?></h3>
  <h3><?php print $receipt['InvoiceNumber']; ?></h3>
  <div><?php print $receipt['CreateTime']; ?></div>
  <div class="tax-receipt-table">
    <div class="tax-receipt-row">
      <div class="tax-receipt-col">隨機碼：<?php print $receipt['RandomNum']; ?></div>
      <div class="tax-receipt-col">總計：＄<?php print $receipt['TotalAmt']; ?></div>
    </div>
    <div class="tax-receipt-row">
      <div class="tax-receipt-col">賣方：<?php print $receipt['serial']; ?></div>
      <div class="tax-receipt-col">買方：<?php print $receipt['BuyerUBN']; ?></div>
    </div>
    <?php if ($receipt['Barcode']) { ?><div><img class="barcode" src="<?php print $receipt['Barcode']; ?>" /></div><?php } ?>
    <div class="tax-receipt-row">
      <?php if ($receipt['QRcodeLimg']) { ?><div class="tax-receipt-col"><img class="qrcode" src="<?php print $receipt['QRcodeLimg']; ?>" /></div><?php } ?>
      <?php if ($receipt['QRcodeRimg']) { ?><div class="tax-receipt-col"><img class="qrcode" src="<?php print $receipt['QRcodeRimg']; ?>" /></div><?php } ?>
    </div>
    <div class="tax-receipt-row"><h3>交易明細資料</h3></div>
    <div class="tax-receipt-row">
      <div>發票號碼：<?php print $receipt['InvoiceNumber']; ?></div>
      <ul>
      <?php foreach($receipt['ItemDetail'] as $item) { ?>
        <li><?php print $item->ItemName.'x'.$item->ItemNum ?> <?php print 'NTD $'.$item->ItemPrice; ?></li>
      <?php } ?>
      </ul>
    </div>
    <div>銷售額：<?php print $receipt['Amt']; ?></div>
    <?php if ($receipt['TaxAmt']) { ?><div>稅額：<?php print $receipt['TaxAmt']; ?></div><?php } ?>
    <div>總計：<?php print $receipt['TotalAmt']; ?></div>

  </div>
</div>
