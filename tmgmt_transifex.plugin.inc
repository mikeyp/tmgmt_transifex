<?php

include 'utils/segment.php';


class TransifexApi {
  var $translator;

  function __construct($translator) {
    $this->translator = $translator;
  }

  function doRequest($method, $url, $data=NULL, $headers = array()) {
    $data_string = json_encode($data);
    $url = join('', array(
      'https://www.transifex.com/api/2/',
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

  function doRequestToProject($method, $url, $data=NULL, $headers = array()) {
    $data_string = json_encode($data);
    $url = join('', array(
      'project/',
      $this->translator->getSetting('project') . '/',
      $url
    ));
    return $this->doRequest($method, $url, $data, $headers);
  }

  function getResource($slug) {
    return json_decode($this->doRequestToProject('GET', 'resource/' . $slug));
  }

  function getTranslations($slug, $lang) {
    return json_decode($this->doRequestToProject('GET', 'resource/' . $slug . "/" . "translation/" . $lang  . "/strings"));
  }

  function createResource($slug, $name, $tjid, $po) {
    return $this->doRequestToProject('POST', 'resources', array(
     'name' => $name,
     'slug' => $slug,
     'content' => $po,
     'i18n_type' => 'PO',
     'category' => 'tjid:' . $tjid
   ));
  }

  function updateResource($slug, $name, $tjid, $po) {
    return $this->doRequestToProject('PUT', 'resource/' . $slug, array(
      'name' => $name,
      'content' => $po,
      'categories' => $tjid
    ));
  }

  function upsertResource($slug, $name, $tjid, $content) {
    $ret = $this->createResource($slug, $name, $tjid, $content);
    if (!isset($ret)) {
      $ret = $this->getResource($slug);
      $ret->categories[] = 'tjid:' . $tjid;
      return $this->updateResource($slug, $name, $ret->categories, $content);
    }
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


  public function extractStringsFromNode(TMGMTJobItem $job_item) {
    $translation_payload = array();
    $translator = $job_item->getTranslator();
    foreach ($job_item->data as $fieldName => $value ) {
      if (isset($value['#text'])) { // Single property field
        $translation_payload[$fieldName] = $value['#text'];
      } else { // Multi property field
        $propCount = 0;
        while (isset($value[$propCount])) {
          $translation_payload[$fieldName . '_' . $propCount] = $value[$propCount]['value']['#text'];
          $propCount++;
        }
      }
    }
    return $translation_payload;
  }

  public function renderPOFromStrings($strings, $segment) {
    $po = '';
    // Get title
    if (isset($strings['node_title'])) {
      $po .= $this->renderPO('node_title', $strings['node_title']);
      unset($strings['node_title']);
    }
    // Get title in case of title module active
    if (isset($strings['title_field_0'])) {
      $po .= $this->renderPO('title_field_0', $strings['title_field_0']);
      unset($strings['title_field_0']);
    }
    // Get body and segment
    $bodyCount = 0;
    while (isset($strings['body_' . $bodyCount])) {
      if ($segment) {
        $Sentence	= new Sentence;
        $sentence_count = 0;
        foreach ($Sentence->split($strings['body_' . $bodyCount]) as $sentence) {
          $po .= $this->renderPO('body_' . $bodyCount . '_sentence_' . $sentence_count, $sentence);
          $sentence_count++;
        }
        unset($strings['body_' . $bodyCount]);
      } else {
        $po .= $this->renderPO('body_' . $bodyCount, $strings['body_' . $bodyCount]);
      }
      $bodyCount++;
    }
    // Get the rest
    foreach ($strings as $fieldName => $value) {
      $po .= $this->renderPO($fieldName, $value);
    }
    return $po;
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
    foreach ($job->getItems() as $job_item) {
      $strings = $this->extractStringsFromNode($job_item);
      $po = $this->renderPOFromStrings(
        $strings,
        $translator->getSetting('segment')
      );
      $slug = $translator->getSetting('project') . '_' . $job_item->item_id;
      if (isset($strings['node_title'])) { // Handle using the Title module for translatable titles
        $title = $strings['node_title'];
      } else {
        $title = $strings['title_field_0'];
      }
      $res = $tx->upsertResource($slug, $title, $job->tjid, $po);
      dpm($res);
      if (json_decode($res)) dpm('Added ' . json_decode($res)[0] . ' strings to resource named: ' . $title);
    }
    $job->submitted('The translation job has been submitted to Transifex.');
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
    $request = $tx->doRequestToProject('GET', '');
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
