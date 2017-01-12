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
<div id="invpay2go-receipt" class="tax-receipt invpay2go-wrapper">
  <h3><?php print $receipt['title']; ?></h3>
  <h2>電子發票證明聯</h2>
  <h3><?php print $receipt['ChineseMonth']; ?></h3>
  <h3><?php print $receipt['InvoiceNumber']; ?></h3>
  <div><?php print $receipt['CreateTime']; ?></div>
  <div>
    <div class="row">
      <div class="left">隨機碼：<?php print $receipt['RandomNum']; ?></div>
      <div class="right">總計：＄<?php print $receipt['TotalAmt']; ?></div>
    </div>
    <div class="row">
      <div class="left">賣方：<?php print $receipt['serial']; ?></div>
      <div class="right">買方：<?php print $receipt['BuyerUBN']; ?></div>
    </div>
    <?php if ($receipt['Barcode']) { ?><div><img src="<?php print $receipt['Barcode']; ?>" /></div><?php } ?>
    <div class="row">
      <?php if ($receipt['QRcodeLimg']) { ?><div class="left"><img src="<?php print $receipt['QRcodeLimg']; ?>" /></div><?php } ?>
      <?php if ($receipt['QRcodeRimg']) { ?><div class="right"><img src="<?php print $receipt['QRcodeRimg']; ?>" /></div><?php } ?>
    </div>
    <div class="center">交易明細資料</div>
    <div>
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
