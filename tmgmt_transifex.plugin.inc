<?php

class TransifexApi {
  var $translator;

  function __construct($translator) {
    $this->translator = $translator;
  }

  function doRequest($method, $url, $data=NULL, $headers = array()) {
    $data_string = json_encode($data);
    $url = join('', array(
      'https://www.transifex.com',
      '/api/2/project/',
      $this->translator->getSetting('project') . '/',
      $url
    ));
    $ch = curl_init($url);
    curl_setopt($ch, CURLOPT_CUSTOMREQUEST, $method);
    curl_setopt($ch, CURLOPT_POSTFIELDS, $data_string);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_HTTPHEADER, array(
     'Content-Type: application/json',
     'Content-Length: ' . strlen($data_string),
     'Authorization: ' . 'Basic ' . base64_encode('api:' . $this->translator->getSetting('api'))
    ));

    $response = curl_exec($ch);
    if (curl_error($ch)) {
     watchdog('tmgmt_transifex', 'Curl error %ce', array('%ce' => curl_error($ch)), WATCHDOG_ERROR, $link = NULL);
     return curl_error($ch);
    }
    curl_close($ch);
    return $response;
 }

 function createResource($slug, $name, $po) {
   return $this->doRequest('POST', 'resources', array(
     'name' => $name,
     'slug' => $slug,
     'content' => $po,
     'i18n_type' => 'PO'
   ));
 }

 function updateResource($slug, $name, $po) {
   return $this->doRequest('PUT', 'resource/' . $slug, array(
     'name' => $name,
     'content' => $po,
     'i18n_type' => 'PO'
   ));
 }


 function upsertResource($name, $slug, $content) {
    $ret = json_decode($this->createResource($name, $slug, $content));
    if (!ret) return $this->updateResource($name, $slug, $content);
    return $ret;
 }

}

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

  private function renderPO($label, $text) {
    return "msgid \"" . $label . "\"\nmsgstr \"" . str_replace("\n","\\n",$text) . "\"\n\n";
  }


  /**
   * Implements TMGMTTranslatorPluginControllerInterface::requestTranslation().
   */
  public function requestTranslation(TMGMTJob $job) {
    // Pull the source data array through the job and flatten it.
    $data = array_filter(tmgmt_flatten_data($job->getData()), '_tmgmt_filter_data');
    $translation = array();
    $translator = $job->getTranslator();
    $tx = new TransifexApi($translator);
    $payloads = array();

    foreach ($data as $tjiid => $value) {
      $tji = entity_load_single('tmgmt_job_item', $tjiid);
      if (!isset($payloads[$tji->item_id])) $payloads[$tji->item_id] = array();
      $payloads[$tji->item_id][$value['#label']] = $value['#text'];
    }

    foreach ($payloads as $node_id => $payload) {
      $slug = $translator->getSetting('project') . '_' . $node_id;

      $title = $payload['Title'];
      $po = $this->renderPO('Title', $payload['Title']);
      unset($payload['Title']);

      $po .= $this->renderPO('Body', $payload['Body']);
      unset($payload['Body']);

      foreach ($payload as $label => $value) {
        $po .= $this->renderPO($label, $value);
      }

      dpm($tx->createResource($slug, $title, $po));
    }
    // $job->submitted('The translation job has been submitted.');
  }

  /**
   * Overrides TMGMTDefaultTranslatorPluginController::getSupportedRemoteLanguages().
   */
  public function getSupportedRemoteLanguages(TMGMTTranslator $translator) {
    $tx = new TransifexApi($translator);
    $request = $tx->doRequest('GET', 'languages');
    $languages = array();
    foreach (json_decode($request) as $language) {
      $languages[$language->code] = $language->code;
    }
    return $languages;
  }

  public function getTransifexProject(TMGMTTranslator $translator) {
    $tx = new TransifexApi($translator);
    $request = $tx->doRequest('GET', '' . $translator->getSetting('project'));
    dpm($request);
    return json_decode($request);
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

}
