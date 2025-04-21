<?php

namespace Drupal\neticrm_dmenu;

use Drupal\Core\Config\ConfigFactoryInterface;
use Drupal\Core\DependencyInjection\ContainerInjectionInterface;
use Drupal\Core\Menu\MenuLinkTreeInterface;
use Drupal\Core\Menu\MenuTreeParameters;
use Drupal\Core\Security\TrustedCallbackInterface;
use Drupal\Core\Session\AccountProxyInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Toolbar integration handler.
 */
class NeticrmDmenuToolbarHandler implements TrustedCallbackInterface {
  /**
   * {@inheritdoc}
   */
  public static function trustedCallbacks() {
    return ['preRender'];
  }

  /**
   * Lazy builder callback for the devel menu toolbar.
   *
   * @return array
   *   The renderable array rapresentation of the devel menu.
   */
  public static function preRender($build) {
    $menu_tree = \Drupal::service('toolbar.menu_tree');
    $parameters = new MenuTreeParameters();
    $parameters->onlyEnabledLinks();
    $tree = $menu_tree->load('neticrm_dmenu', $parameters);

    $manipulators = [
      ['callable' => 'menu.default_tree_manipulators:checkAccess'],
      ['callable' => 'menu.default_tree_manipulators:generateIndexAndSort'],
      ['callable' => 'toolbar_menu_navigation_links'],
    ];

    $tree = $menu_tree->transform($tree, $manipulators);
    $element['neticrm_dmenu'] = $menu_tree->build($tree);
    $element['#attributes']['class'][0] = 'neticrm-dmenu-toolbar-menu';
    return $element;
  }
}