<?php

/**
 * The only reason this class exists is to change html output of specific element
 * Do not touch anything else
 */
class CiviCRM_Floating_Label_textarea extends HTML_QuickForm_textarea {
  function __construct($elementName=null, $elementLabel=null, $attributes=null) {
    parent::__construct($elementName, $elementLabel, $attributes);
    $this->_persistantFreeze = true;
    $this->_type = 'textarea';

    // set placeholder to make sure floating label work
    $label = $this->_label != "" ? $this->_label : ts('Enter text here');
    if (!isset($this->_attributes['placeholder'])) {
      $this->setAttribute('placeholder', $label);
    }
  }

  function toHtml() {
    $label = $this->_attributes['data-label'];
    if ($this->_flagFrozen) {
      $this->setLabel($label);
      return $this->getFrozenHtml();
    }
    else {
      $html = '<div class="crm-form-elem crm-form-textarea crm-floating-label">' . $this->_getTabs() . '<textarea' . $this->_getAttrString($this->_attributes) . '>' .
      // because we wrap the form later we don't want the text indented
      preg_replace("/(\r\n|\n|\r)/", '&#010;', htmlspecialchars($this->_value)) .
      '</textarea><label for="'.$this->_attributes['name'].'">' . $label . '</label></div>';

      return $html;
    }
  }
}
