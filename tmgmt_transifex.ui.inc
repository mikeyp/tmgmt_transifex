<?php

/**
 * @file
 * Provides Transifex Translator ui controller.
 */

/**
 * Transifex translator ui controller.
 */
class TMGMTTransifexTranslatorUIController extends TMGMTDefaultTranslatorUIController {

  /**
   * Overrides TMGMTDefaultTranslatorUIController::pluginSettingsForm().
   */
  public function pluginSettingsForm($form, &$form_state, TMGMTTranslator $translator, $busy = FALSE) {
    $instruction_url = 'https://transifex.com/docs';
    $form['api'] = array(
      '#type' => 'textfield',
      '#title' => t('Transifex API Token'),
      '#default_value' => $translator->getSetting('api'),
      '#required' => TRUE,
      '#element_validate' => array('tmgmt_transifex_plugin_settings_form_validate'),
      '#description' => t('You can generate a Transifex API token <a href="!link" target="_blank">here</a>. For more details, visit our <a href="!documentation" target="_blank">documentation</a>.',
        array(
          '!link' => 'https://www.transifex.com/user/settings/api/',
          '!documentation' => 'https://docs.transifex.com/api/introduction#authentication'
        )
      ),
    );
    $form['project'] = array(
      '#type' => 'textfield',
      '#title' => t('Transifex project URL'),
      '#default_value' => $translator->getSetting('project'),
      '#required' => TRUE,
      '#element_validate' => array('tmgmt_transifex_plugin_settings_form_validate'),
      '#description' => t('Enter the URL of the Transifex project you wish to use for translating this Drupal site.'),
    );
    $form['secret'] = array(
      '#type' => 'textfield',
      '#title' => t('Transifex webhook secret'),
      '#default_value' => $translator->getSetting('secret'),
      '#required' => FALSE,
      '#element_validate' => array('tmgmt_transifex_plugin_settings_form_validate'),
      '#description' => t('Leave empty to deactivate webhook listener'),
    );
    $form['onlyreviewed'] = array(
       '#type' => 'checkbox',
       '#title' => t('Pull only when a language is 100% reviewed'),
       '#default_value' => $translator->getSetting('onlyreviewed'),
       '#element_validate' => array('tmgmt_transifex_plugin_settings_form_validate'),
       '#description' => t('By default, translations for a language are imported to Drupal once it is 100% translated'),
    );
    return parent::pluginSettingsForm($form, $form_state, $translator);
  }

  /**
   * Specifies interface to poll translations and resubmit source text.
   *
   * @return an associative array of UI items to display on the management page
   *         for the given translation job, under the heading "Translation
   *         Information".
   */
  public function checkoutInfo(TMGMTJob $job) {
    /* TODO show last known translation statistics overall and per node */

    $info['actions'] = array('#type' => 'actions');

    if ($job->isActive()) {
      $info['actions']['poll'] = array(
        '#type' => 'submit',
        '#value' => t('Check for updates'),
        '#description' => t('Check for updated translations on the Transifex server.'),
        '#submit' => array('tmgmt_transifex_poll_translations'),
      );
    }

    return $info;
  }

}
