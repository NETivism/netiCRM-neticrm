<?php

/**
 * The only reason this class exists is to change html output of specific element
 * Do not touch anything else
 */
class CiviCRM_Floating_Label_textarea extends HTML_QuickForm_textarea {
  function __construct($elementName=null, $elementLabel=null, $attributes=null) {
    if (is_string($attributes)) {
      if (!strstr($attributes, 'placeholder=')) {
        $attributes .= ' placeholder="' . ts('Enter text here') . ' ..."';
      }
    }
    elseif (!isset($attributes['placeholder'])) {
      $attributes['placeholder'] = ts('Enter text here') . ' ...';
    }

    parent::__construct($elementName, $elementLabel, $attributes);
    $this->_persistantFreeze = true;
    $this->_type = 'textarea';
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
