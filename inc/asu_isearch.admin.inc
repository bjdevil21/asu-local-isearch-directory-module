<?php

/**
 * Defines administrative options
 */

/**
 * Displays the iSearch Directory settings form
 *
 * url: admin/config/isearch
 */
function asu_isearch_admin_settings_form() {
  drupal_set_title('Configure iSearch');

  $form = array();

  $form['isearch_display_affiliations'] = array(
    '#type' => 'checkbox',
    '#title' => 'Display all department affiliations on profile?',
    '#description' => t("This option will show all the departments a person is affiliated on their profile page. They will appear as tabs."),
    '#default_value' => variable_get('isearch_display_affiliations', FALSE),
  );

  $form['isearch_import_subdepts'] = array(
    '#type' => 'checkbox',
    '#title' => 'Include sub-departments on import?',
    '#access' => FALSE,
    '#default_value' => variable_get('isearch_import_subdepts', TRUE),
  );

  $form['isearch_display_default_photo'] = array(
    '#type' => 'checkbox',
    '#title' => 'Display default photo',
    '#description' => t("Check this option if you want to display a default photo for profiles that do not have one."),
    '#default_value' => variable_get('isearch_display_default_photo', FALSE),
  );

  $form['isearch_default_photo'] = array(
    '#type' => 'managed_file',
    '#title' => t('Default Photo'),
    '#default_value' => variable_get('isearch_default_photo', ''),
    '#description' => t("Upload an image to use as the default photo for profiles that are not displaying a photo."),
    '#required' => FALSE,
    '#upload_location' => 'public://asu_isearch_photos/',
    '#states' => array(
      'visible' => array(
        ':input[name="isearch_display_default_photo"]' => array('checked' => TRUE),
      ),
      'required' => array(
        ':input[name="isearch_display_default_photo"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['isearch_link_buildings'] = array(
    '#type' => 'checkbox',
    '#title' => 'Detect and link building codes?',
    '#default_value' => variable_get('isearch_link_buildings', TRUE),
    '#description' => t('Check this option to link building codes on profiles and directory listings to the ASU Campus map'),
  );

  // retrieve global dept configuration
  $isearch_admin_feed_dept = asu_isearch_get_dept_config();

  $form['isearch_admin_feed_dept'] = array(
    '#type' => 'asu_dept_picker',
    '#default_value' => $isearch_admin_feed_dept,
    '#attributes' => array('id' => 'isearch_admin_feed_dept'),
  );

  $form = system_settings_form($form);
  $form['#submit'] = array('asu_isearch_admin_settings_form_submit');

  // Load configuration eye-candy
  asu_isearch_load_js();
  $form['#attached']['css'][] = drupal_get_path('module', 'asu_isearch') . '/js/asu_isearch_config/bootstrap.css';
  $form['#attached']['css'][] = drupal_get_path('module', 'asu_isearch') . '/js/asu_isearch_config/asu_isearch_config.css';
  $form['#attached']['js'][]  = drupal_get_path('module', 'asu_isearch') . '/js/asu_isearch_config/bootstrap.js';
  $form['#attached']['js'][]  = drupal_get_path('module', 'asu_isearch') . '/js/asu_isearch_config/asu_isearch_config.js';
  $form['#attached']['js'][]  = drupal_get_path('module', 'asu_isearch') . '/js/asu_isearch.admin.js';

  $form['#prefix'] = '<div class="asu-isearch-configuration">';
  $form['#suffix'] = '</div>';

  $form['addons'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add-ons'),
    '#description' => t('Additional add-ons can be enabled as separate modules to extend the functionality of iSearch profiles. They will appear here when they are enabled.'),
  );

  // change display of action buttons
  $form['footer_actions'] = $form['actions'];
  $form['actions']['#weight'] = -100;
  $form['footer_actions']['#weight'] = 100;

  return $form;
}

/**
 * Validation for settings form
 */
function asu_isearch_admin_settings_form_validate($form, &$form_state) {
  // check existence of default photo
  if ($form_state['values']['isearch_display_default_photo'] === 1 && $form_state['values']['isearch_default_photo'] === 0) {
    // user did not upload a photo after specifying they wanted a default photo to display
    drupal_set_message(t('Please upload a default photo or uncheck "Display default photo"'), 'error');
  }
}


/**
 * Submit handler for settings form
 */
function asu_isearch_admin_settings_form_submit($form, &$form_state) {
  system_settings_form_submit($form, $form_state);

  $message = t('Click <a href="@link">Import iSearch Profiles</a> to begin the import.', array(
    '@link' => '/admin/content/isearch/import',
  ));

  drupal_set_message($message, 'status');
}

function asu_isearch_import_isearch_form() {

  drupal_set_title('Import iSearch Profiles');

  $migration = _asu_isearch_get_migration();

  $depts = variable_get('isearch_admin_feed_dept', FALSE);

  if ($depts === FALSE) {
    $message = t('You must first select a department to import. Click on <a href="@link">Configure</a> to make a selection.', array(
      '@link' => '/admin/content/isearch/configure',
    ));

    drupal_set_message($message, 'warning');
  }
  elseif ($migration->getStatus() != MigrationBase::STATUS_IDLE) {
    $message = 'An import is currently running. Please wait a few minutes for it to finish before importing again.';
    drupal_set_message($message, 'warning');
  }
  else {
    drupal_set_message('You can manually trigger an import by clicking the "Begin import" button below. Once pressed, please wait a moment for import to finish.', 'warning', FALSE);
  }

  $form = array();

  $allow_import = ($depts && $migration->getStatus() == MigrationBase::STATUS_IDLE);

  $form['isearch_import_limit'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#attributes' => array('class' => array('container-inline')),
    'value' => array(
      '#type' => 'textfield',
      '#title' => t('Limit to:'),
      '#default_value' => 50,
      '#required' => TRUE,
      '#size' => 10,
    ),
    'unit' => array(
      '#type' => 'select',
      '#options' => array(
        'items' => t('items'),
        'seconds' => t('seconds'),
      ),
      '#description' => t('Set a limit of how many items to process, or how long it should run.'),
    ),
  );

  $form = system_settings_form($form);
  $form['#submit'] = array('asu_isearch_import_isearch_form_submit');

  // change display of action buttons
  $form['actions']['submit']['#value'] = t('Begin import');
  $form['actions']['submit']['#access'] = $allow_import;

  $form['actions']['force_submit'] = array(
    '#name' => 'force_update',
    '#type' => 'submit',
    '#value' => t('Force Update'),
    '#description' => t('This will force all records to update and may need to run several times depending on directory size.'),
    '#access' => $allow_import,
  );

  return $form;
}

/**
 * Submit handler for import isearch form
 */
function asu_isearch_import_isearch_form_submit($form, &$form_state) {
  system_settings_form_submit($form, $form_state);

  $force_update = FALSE;

  if ($form_state['clicked_button']['#name'] == 'force_update') {
    $force_update = TRUE;
  }

  _asu_isearch_begin_batch($force_update);
}


// Temporary function used with devel/php
function asu_isearch_devel() { 
}
