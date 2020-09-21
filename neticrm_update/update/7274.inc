<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7274() {
	$sql = "select c.id, l.id as log_id from civicrm_contact c LEFT JOIN civicrm_log l ON l.entity_id = c.id AND l.entity_table = 'civicrm_contact' WHERE l.id IS NULL;";
	$dao = CRM_Core_DAO::executeQuery($sql);
	while($dao->fetch()) {
		$date = CRM_Core_DAO::singleValueQuery("SELECT MIN(activity_date_time) FROM civicrm_activity a INNER JOIN civicrm_activity_assignment aa ON aa.activity_id = a.id WHERE aa.assignee_contact_id = %1 GROUP BY aa.assignee_contact_id;", array( 1 => array($dao->id, 'Positive')));
		if (empty($date)) {
			$date = CRM_Core_DAO::singleValueQuery("SELECT MIN(activity_date_time) FROM civicrm_activity a INNER JOIN civicrm_activity_target aa ON aa.activity_id = a.id WHERE aa.target_contact_id = %1 GROUP BY aa.target_contact_id;", array( 1 => array($dao->id, 'Positive')));
		}
		if (!empty($date)) {
			CRM_Core_DAO::executeQuery("UPDATE civicrm_contact SET created_date = %1 WHERE id = %2", array(
				1 => array($date, 'String'),
				2 => array($dao->id, 'Positive'),
			));
		}
	}
}
