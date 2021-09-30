<?php

/**
 * The only reason this class exists is to change html output of specific element
 * Do not touch anything else
 */
class CiviCRM_Floating_Label_text extends HTML_QuickForm_text {
  function __construct($elementName=null, $elementLabel=null, $attributes=null)
  {
    parent::__construct($elementName, $elementLabel, $attributes);
    $this->_persistantFreeze = true;
    $this->setType('text');

    // set placeholder to make sure floating label work
    $label = $this->_label != "" ? $this->_label : ts('Enter text here');
    if (!isset($this->_attributes['placeholder'])) {
      $this->setAttribute('placeholder', $label);
    }
  } //end constructor

  function toHtml() {
    $label = $this->_attributes['data-label'];
    if ($this->_flagFrozen) {
      $this->setLabel($label);
      return $this->getFrozenHtml();
    }
    else {
      $html = '<div class="crm-form-elem crm-form-textfield crm-floating-label">' . $this->_getTabs() . '<input' . $this->_getAttrString($this->_attributes) . ' /><label for="'.$this->_attributes['name'].'">'.$label.'</label></div>';
      return $html;
    }
  } 
}