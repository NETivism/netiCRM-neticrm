<?php

function template_preprocess_invpay2go(&$vars) {
  $receipt =& $vars['receipt'];

  // chinese date
  $invoice_time = strtotime($receipt->CreateTime);
  $year = date('Y', $invoice_time);
  $year -= 1911;
  $month = date('n', $invoice_time);
  $monthstr = $month%2 === 0 ? sprintf('%2d-%2d', $month-1, $month) : sprintf('%2d-%2d', $month, $month+1);
  $receipt->ChineseMonth = $year . '年'. $monthstr . '月';

  // site name
  $receipt->SiteName = variable_get('site_name', '');
}
