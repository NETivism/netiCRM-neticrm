<?php
/**
 * Daily simulation of demo site donations.
 *
 * Only runs when civicrm_demo_sample_data is enabled. Every day:
 *  - monthly recurring contributions due this month (by cycle_day) get next installment
 *  - single contributions are triggered at random time, the daily number and
 *    amount follow the normal distribution of last 30 days single contributions
 *  - every day has at least one contribution (single or recurring), days
 *    missing after the last contribution are back-filled as if cron ran once
 *    each day
 */

module_load_include('inc', 'civicrm_demo', 'civicrm_demo.utils');

define('CIVICRM_DEMO_CRON_STATS_DAYS', 30);
define('CIVICRM_DEMO_CRON_NEW_CONTACT_PERCENT', 80);
define('CIVICRM_DEMO_CRON_CONVERT_RECUR_PERCENT', 10);
define('CIVICRM_DEMO_CRON_BACKFILL_DAYS_PER_RUN', 31);
define('CIVICRM_DEMO_CRON_SOURCE', '示範用途自動產生');

/**
 * Entry point of cron.
 *
 * Today's single contributions are planned at first cron run of the day with
 * random timestamps, every cron run only create those already due. So the
 * contributions spread through the day no matter how often cron runs.
 * Before planning today, missing days are back-filled first.
 */
function civicrm_demo_cron_process($now = NULL) {
  $now = $now ? $now : time();
  $today = date('Y-m-d', $now);
  $plan = variable_get('civicrm_demo_cron_plan', []);

  if (empty($plan['date']) || $plan['date'] !== $today) {
    // finish yesterday's plan before start new one
    if (!empty($plan['single'])) {
      foreach ($plan['single'] as $ts) {
        civicrm_demo_cron_single_contribution($ts);
      }
      $plan['single'] = [];
      variable_set('civicrm_demo_cron_plan', $plan);
    }
    // back-fill is too heavy for single run, continue on next cron
    if (!civicrm_demo_cron_backfill($now)) {
      return;
    }
    // recurring first, single plan depends on whether today already has contribution
    civicrm_demo_cron_recur_contribution($now);
    $plan = [
      'date' => $today,
      'single' => civicrm_demo_cron_plan_single($now),
    ];
    variable_set('civicrm_demo_cron_plan', $plan);
  }

  $remain = [];
  foreach ($plan['single'] as $ts) {
    if ($ts <= $now) {
      civicrm_demo_cron_single_contribution($ts);
    }
    else {
      $remain[] = $ts;
    }
  }
  $plan['single'] = $remain;
  variable_set('civicrm_demo_cron_plan', $plan);
}

/**
 * Simulate days between last contribution and today.
 *
 * @return bool TRUE when no more day to fill
 */
function civicrm_demo_cron_backfill($now) {
  $today_start = strtotime(date('Y-m-d 00:00:00', $now));
  $last = CRM_Core_DAO::singleValueQuery("
SELECT MAX(receive_date) FROM civicrm_contribution
WHERE is_test = 0 AND contribution_status_id = 1 AND receive_date < %1", [1 => [date('Y-m-d H:i:s', $today_start), 'String']]);
  if (empty($last)) {
    return TRUE;
  }
  $day = strtotime(date('Y-m-d 00:00:00', strtotime($last)) . ' +1 day');
  $processed = 0;
  while ($day < $today_start) {
    if ($processed >= CIVICRM_DEMO_CRON_BACKFILL_DAYS_PER_RUN) {
      return FALSE;
    }
    civicrm_demo_cron_recur_contribution($day + 86399);
    foreach (civicrm_demo_cron_plan_single($day) as $ts) {
      civicrm_demo_cron_single_contribution($ts);
    }
    $day = strtotime('+1 day', $day);
    $processed++;
  }
  return TRUE;
}

/**
 * Create installment for every in-progress monthly recurring which is due
 * this month (cycle_day <= today) and doesn't have contribution this month yet.
 * Amount and payment settings continue from the last installment.
 */
function civicrm_demo_cron_recur_contribution($now) {
  $days_in_month = (int) date('t', $now);
  $today = (int) date('j', $now);
  $month_start = date('Y-m-01 00:00:00', $now);
  $month_end = date('Y-m-01 00:00:00', strtotime('+1 month', strtotime($month_start)));
  $sql = "
SELECT r.id, r.contact_id, r.amount, r.cycle_day, r.start_date
FROM civicrm_contribution_recur r
WHERE r.is_test = 0
  AND r.contribution_status_id = 5
  AND r.frequency_unit = 'month'
  AND r.frequency_interval = 1
  AND r.start_date <= %5
  AND (r.end_date IS NULL OR r.end_date >= %1)
  AND LEAST(GREATEST(r.cycle_day, 1), %2) <= %3
  AND NOT EXISTS (
    SELECT 1 FROM civicrm_contribution c
    WHERE c.contribution_recur_id = r.id AND c.receive_date >= %1 AND c.receive_date < %4
  )";
  $dao = CRM_Core_DAO::executeQuery($sql, [
    1 => [$month_start, 'String'],
    2 => [$days_in_month, 'Integer'],
    3 => [$today, 'Integer'],
    4 => [$month_end, 'String'],
    5 => [date('Y-m-d H:i:s', $now), 'String'],
  ]);
  while ($dao->fetch()) {
    $due_day = min(max((int) $dao->cycle_day, 1), $days_in_month);
    $due_start = strtotime(date('Y-m-', $now) . sprintf('%02d', $due_day) . ' 00:00:00');
    if ($due_start < strtotime(date('Y-m-d 00:00:00', strtotime($dao->start_date)))) {
      continue;
    }
    $due_end = min($due_start + 86399, $now);
    $receive_ts = mt_rand($due_start, $due_end);

    $last = CRM_Core_DAO::executeQuery("
SELECT total_amount, payment_instrument_id, contribution_type_id, contribution_page_id
FROM civicrm_contribution
WHERE contribution_recur_id = %1 AND contribution_status_id = 1 AND receive_date < %2
ORDER BY receive_date DESC, id DESC LIMIT 1", [1 => [$dao->id, 'Integer'], 2 => [date('Y-m-d H:i:s', $receive_ts), 'String']]);
    if ($last->fetch()) {
      $params = [
        'total_amount' => $last->total_amount,
        'payment_instrument_id' => $last->payment_instrument_id,
        'contribution_type_id' => $last->contribution_type_id,
        'contribution_page_id' => $last->contribution_page_id,
      ];
    }
    else {
      $params = [
        'total_amount' => $dao->amount,
        'payment_instrument_id' => 1, // credit card
        'contribution_type_id' => _civicrm_demo_cron_default_contribution_type(),
        'contribution_page_id' => _civicrm_demo_cron_contribution_page(),
      ];
    }
    $params['contact_id'] = $dao->contact_id;
    $params['contribution_recur_id'] = $dao->id;
    $params['trxn_id'] = 'r-' . $dao->id . '-' . date('Ym', $receive_ts) . '-' . CRM_Utils_String::createRandom(6);
    _civicrm_demo_cron_add_contribution($params, $receive_ts);
  }
}

/**
 * Decide today's number of single contributions by normal distribution of
 * daily count in last 30 days.
 *
 * @return array sorted timestamps of today
 */
function civicrm_demo_cron_plan_single($now) {
  $day_start = strtotime(date('Y-m-d 00:00:00', $now));
  $from = date('Y-m-d 00:00:00', strtotime('-' . CIVICRM_DEMO_CRON_STATS_DAYS . ' days', $day_start));
  $to = date('Y-m-d 00:00:00', $day_start);
  $total = CRM_Core_DAO::singleValueQuery("
SELECT COUNT(*) FROM civicrm_contribution
WHERE is_test = 0 AND contribution_status_id = 1 AND contribution_recur_id IS NULL
  AND receive_date >= %1 AND receive_date < %2", [1 => [$from, 'String'], 2 => [$to, 'String']]);
  if (empty($total)) {
    // no history to follow, keep site alive with small numbers
    $counts = [1, 2, 3, 3, 4, 5];
  }
  else {
    $counts = array_fill(0, CIVICRM_DEMO_CRON_STATS_DAYS, 0);
    $dao = CRM_Core_DAO::executeQuery("
SELECT DATEDIFF(%2, DATE(receive_date)) AS diff, COUNT(*) AS cnt FROM civicrm_contribution
WHERE is_test = 0 AND contribution_status_id = 1 AND contribution_recur_id IS NULL
  AND receive_date >= %1 AND receive_date < %2
GROUP BY diff", [1 => [$from, 'String'], 2 => [$to, 'String']]);
    while ($dao->fetch()) {
      $counts[$dao->diff - 1] = (int) $dao->cnt;
    }
  }
  list($mean, $std) = _civicrm_demo_cron_mean_std($counts);
  // every day has at least one contribution, recurring included
  $has_contribution = CRM_Core_DAO::singleValueQuery("
SELECT 1 FROM civicrm_contribution
WHERE is_test = 0 AND contribution_status_id = 1 AND receive_date >= %1 AND receive_date < %2 LIMIT 1", [
    1 => [date('Y-m-d H:i:s', $day_start), 'String'],
    2 => [date('Y-m-d H:i:s', $day_start + 86400), 'String'],
  ]);
  $num = max($has_contribution ? 0 : 1, (int) round(_civicrm_demo_rand_normal($mean, $std)));

  $timestamps = [];
  for ($i = 0; $i < $num; $i++) {
    $timestamps[] = mt_rand($day_start, $day_start + 86399);
  }
  sort($timestamps);
  return $timestamps;
}

/**
 * Create one single contribution.
 *
 * 80% from new contact, 20% from exists contact without recurring.
 * Exists contact who had single contribution before has 10% chance to
 * convert into monthly recurring donor.
 */
function civicrm_demo_cron_single_contribution($receive_ts) {
  $contact_id = NULL;
  $is_new = mt_rand(1, 100) <= CIVICRM_DEMO_CRON_NEW_CONTACT_PERCENT;
  if (!$is_new) {
    $contact_id = CRM_Core_DAO::singleValueQuery("
SELECT c.id FROM civicrm_contact c
WHERE c.is_deleted = 0 AND c.contact_type = 'Individual'
  AND NOT EXISTS (
    SELECT 1 FROM civicrm_contribution_recur r
    WHERE r.contact_id = c.id AND r.is_test = 0 AND r.contribution_status_id IN (2, 5)
  )
ORDER BY RAND() LIMIT 1");
  }
  if (empty($contact_id)) {
    $is_new = TRUE;
    $contact_id = _civicrm_demo_create_contact(['contact_source' => CIVICRM_DEMO_CRON_SOURCE]);
    if (empty($contact_id)) {
      return;
    }
  }

  if (!$is_new && mt_rand(1, 100) <= CIVICRM_DEMO_CRON_CONVERT_RECUR_PERCENT) {
    $had_single = CRM_Core_DAO::singleValueQuery("
SELECT COUNT(*) FROM civicrm_contribution
WHERE contact_id = %1 AND is_test = 0 AND contribution_status_id = 1 AND contribution_recur_id IS NULL", [1 => [$contact_id, 'Integer']]);
    if ($had_single) {
      civicrm_demo_cron_convert_recur($contact_id, $receive_ts);
      return;
    }
  }

  $types = array_keys(CRM_Contribute_PseudoConstant::contributionType());
  $instruments = CRM_Contribute_PseudoConstant::paymentInstrument();
  $params = [
    'contact_id' => $contact_id,
    'total_amount' => _civicrm_demo_cron_single_amount($receive_ts),
    'payment_instrument_id' => array_rand($instruments),
    'contribution_type_id' => $types[array_rand($types)],
    'contribution_page_id' => _civicrm_demo_cron_contribution_page(),
    'trxn_id' => 'd-' . mt_rand(100, 999) . '-' . mt_rand(100, 999) . '-' . CRM_Utils_String::createRandom(8),
  ];
  _civicrm_demo_cron_add_contribution($params, $receive_ts);
}

/**
 * Start a monthly recurring from this donation, first installment included.
 */
function civicrm_demo_cron_convert_recur($contact_id, $receive_ts) {
  $amount = _civicrm_demo_rand_recur_amount();
  $start_date = date('Y-m-d H:i:s', $receive_ts);
  $recur = civicrm_api('ContributionRecur', 'create', [
    'version' => 3,
    'contact_id' => $contact_id,
    'amount' => $amount,
    'currency' => 'TWD',
    'frequency_unit' => 'month',
    'frequency_interval' => 1,
    'installments' => mt_rand(6, 42),
    'start_date' => $start_date,
    'create_date' => $start_date,
    'contribution_status_id' => 5,
    'is_test' => 0,
    'cycle_day' => (int) date('j', $receive_ts),
  ]);
  if (empty($recur['id'])) {
    return;
  }
  $params = [
    'contact_id' => $contact_id,
    'total_amount' => $amount,
    'payment_instrument_id' => 1, // credit card
    'contribution_type_id' => _civicrm_demo_cron_default_contribution_type(),
    'contribution_page_id' => _civicrm_demo_cron_contribution_page(),
    'contribution_recur_id' => $recur['id'],
    'trxn_id' => 'r-' . $recur['id'] . '-' . date('Ym', $receive_ts) . '-' . CRM_Utils_String::createRandom(6),
  ];
  _civicrm_demo_cron_add_contribution($params, $receive_ts);
}

/**
 * Amount of single contribution, follow log-normal distribution of last 30 days
 * single contributions (donation amounts are right skewed, normal curve fits
 * better on log scale), round to hundred.
 */
function _civicrm_demo_cron_single_amount($receive_ts) {
  static $cache = [];
  $day_start = strtotime(date('Y-m-d 00:00:00', $receive_ts));
  if (!isset($cache[$day_start])) {
    $stats = [];
    $dao = CRM_Core_DAO::executeQuery("
SELECT total_amount FROM civicrm_contribution
WHERE is_test = 0 AND contribution_status_id = 1 AND contribution_recur_id IS NULL
  AND total_amount > 0 AND receive_date >= %1 AND receive_date < %2", [
      1 => [date('Y-m-d H:i:s', strtotime('-' . CIVICRM_DEMO_CRON_STATS_DAYS . ' days', $day_start)), 'String'],
      2 => [date('Y-m-d H:i:s', $day_start), 'String'],
    ]);
    $logs = [];
    while ($dao->fetch()) {
      $logs[] = log($dao->total_amount);
    }
    if (count($logs) >= 2) {
      list($mean, $std) = _civicrm_demo_cron_mean_std($logs);
      $stats = ['mean' => $mean, 'std' => $std, 'max' => ceil(exp(max($logs)) / 100) * 100];
    }
    $cache[$day_start] = $stats;
  }
  $stats = $cache[$day_start];
  if (empty($stats)) {
    return _civicrm_demo_rand_amount();
  }
  $amount = exp(_civicrm_demo_rand_normal($stats['mean'], $stats['std']));
  $amount = round($amount / 100) * 100;
  return min(max($amount, 100), $stats['max']);
}

function _civicrm_demo_cron_mean_std($values) {
  $n = count($values);
  $mean = array_sum($values) / $n;
  $sum = 0;
  foreach ($values as $v) {
    $sum += ($v - $mean) * ($v - $mean);
  }
  return [$mean, sqrt($sum / $n)];
}

function _civicrm_demo_cron_add_contribution($params, $receive_ts) {
  $params['receive_date'] = $params['created_date'] = date('YmdHis', $receive_ts);
  $params['total_amount'] = number_format($params['total_amount'], 2, '.', '');
  $params['contribution_status_id'] = 1;
  $params['is_test'] = 0;
  $params['source'] = CIVICRM_DEMO_CRON_SOURCE;
  $contribution = new CRM_Contribute_DAO_Contribution();
  $contribution->copyValues($params);
  $contribution->save();
  return $contribution->id;
}

function _civicrm_demo_cron_contribution_page() {
  $payment_page = variable_get('civicrm_demo_payment_page', []);
  if (!empty($payment_page)) {
    return reset($payment_page);
  }
  $page_id = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_contribution_page ORDER BY id ASC LIMIT 1");
  return $page_id ? $page_id : 1;
}

function _civicrm_demo_cron_default_contribution_type() {
  $types = array_keys(CRM_Contribute_PseudoConstant::contributionType());
  return reset($types);
}
