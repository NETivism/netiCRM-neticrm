<?php
function _neticrm_update_update_7255() {
  $sql = "SELECT l.entity_id, f.label as field_title, fv.label, l.qty, l.line_total, l.participant_count, f.html_type  FROM `civicrm_line_item` l INNER JOIN civicrm_price_field f ON f.id = l.price_field_id INNER JOIN civicrm_price_field_value fv ON fv.id = l.price_field_value_id WHERE `entity_table` LIKE 'civicrm_participant' ORDER BY l.entity_id, f.weight, fv.weight ASC";
  $dao = CRM_Core_DAO::executeQuery($sql);
  while($dao->fetch()) {
    $values = (array) $dao;
		$amount_level[$dao->entity_id][] = $values;
  }
  foreach($amount_level as $entity_id => $amounts) {
    $fee_level = CRM_Price_BAO_Set::amountLevelText($amounts);
    if (!empty($fee_level)) {
      CRM_Core_DAO::executeQuery("UPDATE civicrm_participant SET fee_level = %2 WHERE id = %1", array(
        1 => array($entity_id, 'Integer'),
        2 => array($fee_level, 'String'),
      ));
    }
  }
  return 'Sucessfully update participant price fee level';
}
