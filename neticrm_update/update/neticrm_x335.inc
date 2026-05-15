<?php
function _neticrm_update_update_x335() {
  $domainId = CRM_Core_Config::domainID();
  $backend = CRM_Core_DAO::singleValueQuery(
    "SELECT config_backend FROM civicrm_domain WHERE id = %1",
    [1 => [$domainId, 'Integer']]
  );
  $values = $backend ? unserialize($backend) : [];

  // Skip if already explicitly written (avoid overwriting manual choice)
  if (is_array($values) && array_key_exists('useReceiptTitleForDonorCredit', $values)) {
    return 'useReceiptTitleForDonorCredit already set, skipping.';
  }

  // Detect existing site: has contribution records => upgrade (preserve legacy behavior)
  $contribCount = CRM_Core_DAO::singleValueQuery("SELECT COUNT(*) FROM civicrm_contribution");
  if ($contribCount > 0) {
    $config = CRM_Core_Config::singleton();
    $configParams = get_object_vars($config);
    $configParams['useReceiptTitleForDonorCredit'] = 0;
    CRM_Core_BAO_ConfigSetting::add($configParams);
    return 'Existing site detected: useReceiptTitleForDonorCredit set to 0 to preserve legacy behavior.';
  }
  return 'Fresh install detected: keeping default behavior.';
}
