<?php

function _neticrm_update_update_7243(){
	user_role_grant_permissions(1, array('view public CiviMail content'));
	user_role_grant_permissions(2, array('view public CiviMail content'));
  return 'Successful add permission view public CiviMail content to anonymous user and authenticated user';
}
