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
    $form['tx'] = array(
      '#type' => 'textfield',
      '#title' => t('Transifex instance'),
      '#default_value' => $translator->getSetting('tx'),
      '#required' => FALSE,
      '#element_validate' => array('tmgmt_transifex_plugin_settings_form_validate'),
      '#description' => '',
    );
    $form['api'] = array(
      '#type' => 'textfield',
      '#title' => t('Transifex API Key'),
      '#default_value' => $translator->getSetting('api'),
      '#required' => TRUE,
      '#element_validate' => array('tmgmt_transifex_plugin_settings_form_validate'),
      '#description' => t('Please enter your Transifex API key. Instructions on how to set up your Azure account and get the key can be found <a href="!link" target="_blank">here</a>',array('!link' => $instruction_url)),
    );
    $form['project'] = array(
      '#type' => 'textfield',
      '#title' => t('Transifex target project'),
      '#default_value' => $translator->getSetting('project'),
      '#required' => TRUE,
      '#element_validate' => array('tmgmt_transifex_plugin_settings_form_validate'),
      '#description' => t('Please enter the target project, should exist in Transifex'),
    );
    $form['secret'] = array(
      '#type' => 'textfield',
      '#title' => t('Transifex webhook secret'),
      '#default_value' => $translator->getSetting('secret'),
      '#required' => FALSE,
      '#element_validate' => array('tmgmt_transifex_plugin_settings_form_validate'),
      '#description' => t('Leave empty to deactivate webhook listener'),
    );
    $form['autoreview'] = array(
      '#type' => 'checkbox',
      '#title' => t('Autoreview translations'),
      '#default_value' => $translator->getSetting('autoreview'),
      '#element_validate' => array('tmgmt_transifex_plugin_settings_form_validate'),
      '#description' => t('Consider translation strings coming from Transifex already reviewed.'),
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
        '#description' => t('Check for updated translations on the Zanata server.'),
        '#submit' => array('tmgmt_transifex_poll_translations'),
      );
    }

    return $info;
  }

}
