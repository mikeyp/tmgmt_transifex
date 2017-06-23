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
    $form['segment'] = array(
      '#type' => 'checkbox',
      '#title' => t('Segment body string into sentences.'),
      '#default_value' => $translator->getSetting('segment'),
      '#required' => TRUE,
      '#element_validate' => array('tmgmt_transifex_plugin_settings_form_validate'),
      '#description' => t('Break up long body texts into sentences, so you get to use your TM'),
    );
    return parent::pluginSettingsForm($form, $form_state, $translator);
  }

}
