<?php

/**
 * @file
 * Provides Transifex Translator plugin controller.
 *
 */

/**
 * Transifex translator plugin controller.
 */
class TMGMTTransifexTranslatorPluginController extends TMGMTDefaultTranslatorPluginController {

  /**
   * Translation service URL.
   *
   * @var string
   */
  protected $translatorUrl = 'https://transifex.com/api/2';

  /**
   * Authentication service URL.
   *
   * @var string
   */
  // protected $authUrl = 'https://transifex.com/api/2/user/login';
  protected $authUrl =  'https://requestb.in/q2xje7q2';

  /**
   * Maximum supported characters.
   *
   * @var int
   */
  protected $maxCharacters = 10000;

  /**
   * Overrides TMGMTDefaultTranslatorPluginController::isAvailable().
   */
  public function isAvailable(TMGMTTranslator $translator) {
    if ($translator->getSetting('api')) {
      return TRUE;
    }
    return FALSE;
  }

  /**
   * Overrides TMGMTDefaultTranslatorPluginController::canTranslate().
   */
  public function canTranslate(TMGMTTranslator $translator, TMGMTJob $job) {
    if (!parent::canTranslate($translator, $job)) {
      return FALSE;
    }
    foreach (array_filter(tmgmt_flatten_data($job->getData()), '_tmgmt_filter_data') as $value) {
      // If one of the texts in this job exceeds the max character count the job
      // can't be translated.
      if (drupal_strlen($value['#text']) > $this->maxCharacters) {
        return FALSE;
      }
    }
    return TRUE;
  }

  /**
   * Implements TMGMTTranslatorPluginControllerInterface::requestTranslation().
   */
  public function requestTranslation(TMGMTJob $job) {
    // Pull the source data array through the job and flatten it.
    $data = array_filter(tmgmt_flatten_data($job->getData()), '_tmgmt_filter_data');
    $translation = array();
    $translator = $job->getTranslator();
    foreach ($data as $key => $value) {
      // Query the translator API.
      $result = $this->doRequest($job->getTranslator(), 'Translate', array(
        'from' => $translator->mapToRemoteLanguage($job->source_language),
        'to' => $translator->mapToRemoteLanguage($job->target_language),
        'contentType' => 'text/plain',
        'text' => $value['#text'],
      ), array(
        'headers' => array(
          'Content-Type' => 'text/plain',
        ),
      ));
      // Check the result code for possible errors.
      switch ($result->code) {
        case 200:
          // Lets use DOMDocument for now because this service enables us to
          // send an array of translation sources, and we will probably use
          // this soon.
          $dom = new DOMDocument;
          $dom->loadXML($result->data);
          $items = $dom->getElementsByTagName('string');
          $translation[$key]['#text'] = $items->item(0)->nodeValue;
          break;
        case 400:
          $job->rejected('Rejected by Transifex Translator: !error', array('!error' => $result->data), 'error');
          return;
        default:
          $job->rejected('Unknown error from Transifex Translator: !error', array('!error' => $result->data), 'error');
          return;
      }
    }
    // The translation job has been successfully submitted.
    $job->submitted('The translation job has been submitted.');
    // Save the translated data through the job.
    $job->addTranslatedData(tmgmt_unflatten_data($translation));
  }

  /**
   * Overrides TMGMTDefaultTranslatorPluginController::getSupportedRemoteLanguages().
   */
  public function getSupportedRemoteLanguages(TMGMTTranslator $translator) {
    $request = $this->doRequest($translator, 'languages');
    $languages = array();
    if ($request && $request->code == 200) {
      foreach (json_decode($request->data) as $language) {
        $languages[$language->code] = $language->code;
      }
    }
    return $languages;
  }

  public function getTransifexProject(TMGMTTranslator $translator) {
    $request = $this->doRequest($translator, 'project/' . $translator->getSetting('project'));
    echo '<h1>' . $request->code . '</h1>';
    if ($request && $request->code == 200) {
      return json_decode($request->data);
    }
  }


  /**
   * Overrides TMGMTDefaultTranslatorPluginController::getDefaultRemoteLanguagesMappings().
   */
  public function getDefaultRemoteLanguagesMappings() {
    return array(
      'zh-hans' => 'zh-CHS',
      'zh-hant' => 'zh-CHT',
    );
  }

  /**
   * Overrides TMGMTDefaultTranslatorPluginController::getSupportedTargetLanguages().
   */
  public function getSupportedTargetLanguages(TMGMTTranslator $translator, $source_language) {

    $remote_languages = $this->getSupportedRemoteLanguages($translator);
    $languages = array();

    foreach ($remote_languages as $remote_language) {
      $local_language = $translator->mapToLocalLanguage($remote_language);
      $languages[$local_language] = $local_language;
    }

    // Check if the source language is available.
    if (array_key_exists($source_language, $languages)) {
      unset($languages[$source_language]);
      return $languages;
    }

    return array();
  }

  /**
   * Overrides TMGMTDefaultTranslatorPluginController::hasCheckoutSettings().
   */
  public function hasCheckoutSettings(TMGMTJob $job) {
    return FALSE;
  }

  /**
   * Execute a request against the Transifex API.
   *
   * @param TMGMTTranslator $translator
   *   The translator entity to get the settings from.
   * @param $path
   *   The path that should be appended to the base uri, e.g. Translate or
   *   GetLanguagesForTranslate.
   * @param $query
   *   (Optional) Array of GET query arguments.
   * @param $options
   *   (Optional) Array of additional options passed to drupal_http_request().
   *
   * @return stdclass|null
   *   The response object returned by drupal_http_request() or null if the API
   *   request failed.
   */
  protected function doRequest(TMGMTTranslator $translator, $path, array $query = array(), array $options = array()) {
    $custom_url = $translator->getSetting('tx');
    $url = ($custom_url ? $custom_url : $this->translatorUrl) . '/' . $path;

    $apiKey = $translator->getSetting('api');
    if (empty($apiKey)) {
      watchdog('tmgmt_transifex', "Error: Unable to get API key. Please be sure you have set a value in the module's configuration page", WATCHDOG_ERROR);
      return NULL;
    }

    $options['headers']['Authorization'] = 'Basic ' . base64_encode('api:' . $translator->getSetting('api'));
    $url = url($url, array('query' => $query));
    return drupal_http_request($url, $options);
  }



}
