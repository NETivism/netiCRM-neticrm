<div class="pcplist-item pcplist-item-<?php print $pcp_id ?> clear-block">
  <div class="pcplist-title"><a href="<?php print $view_url ?>"><?php print $title ?></a></div>
  <div class="pcplist-achievement">
    <div class="pcplist-goal"><?php print t('Goal') ?>: <?php print $goal_amount ?></div>
    <div class="pcplist-fill-wrapper">
      <div class="pcplist-fill-height" style="width:<?php print $achieved_percentage ?>">
        <div class="pcplist-fill-pointer"><?php print $achieved_percentage ?></div>
      </div>
    </div> 
    <div class="pcplist-achieved"><?php print t('Achieved') ?>: <?php print $achieved_amount ?></div>
    <div class="pcplist-contrib">&raquo; <?php print $contribution_link ?><span></span></div>
  </div>
  <div class="pcplist-image"><img src="<?php print $image_src ?>" /></div>
  <div class="pcplist-count"><label><?php print t("Contribution count") ?>:</label> <?php print $contribution_count ?></div>
  <div class="pcplist-intro"><?php print $intro_text ?>  <span class="more">&raquo; <a href="<?php print $view_url ?>"><?php print t('More') ?></a></span></div>
</div>
