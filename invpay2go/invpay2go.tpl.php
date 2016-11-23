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
  <h3><?php print $receipt['SiteName']; ?></h3>
  <h2>電子發票證明聯</h2>
  <h3><?php print $receipt['ChineseMonth']; ?></h3>
  <h3><?php print $receipt['InvoiceNumber']; ?></h3>
  <div><?php print $receipt['CreateTime']; ?></div>
  <div>
    <div>隨機碼：<?php print $receipt['RandomNum']; ?></div>
    <div>總計：＄<?php print $receipt['TotalAmt']; ?></div>

  </div>
</div>
