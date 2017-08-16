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
      extractSlugFromUrl($this->translator->getSetting('project')),
      '/',
      $url
    ));
    return $this->doRequest($method, $url, $data, $headers);
  }

  function getResource($slug) {
    return json_decode($this->doRequestToProject('GET', 'resource/' . $slug));
  }

  function getTranslations($slug, $lang) {
    return json_decode($this->doRequestToProject('GET', 'resource/' . $slug . "/" . "translation/" . $lang));
  }

  function createResource($slug, $name, $tjid, $po) {
    return $this->doRequestToProject('POST', 'resources', array(
     'name' => $name,
     'slug' => $slug,
     'content' => $po,
     'i18n_type' => 'HTML',
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

  function upsertResource($slug, $name, $tjid, $po) {
    $ret = $this->createResource($slug, $name, $tjid, $po);
    $ret = json_decode($ret);
    if (!isset($ret)) {
      $ret = $this->getResource($slug);
      $ret->categories[] = 'tjid:' . $tjid;
      return $this->updateResource($slug, $name, $ret->categories, $po);
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

  public function extractStringsFromNode(TMGMTJobItem $job_item) {
    $translation_payload = array();
    $translator = $job_item->getTranslator();
    foreach ($job_item->data as $fieldName => $value ) {
      $propCount = 0;
      while (isset($value[$propCount])) {
        if (in_array($value[$propCount]['format']['#text'], array('full_html', 'filtered_html'))) {
          $translation_payload[] = array(
            'key' => $fieldName . '_' . $propCount,
            'value' => $value[$propCount]['value']['#text']
          );
        } else {
          $Sentence	= new Sentence;
          $sentence_count = 0;
          $translation_payload[] = array(
            'key' => $fieldName . '_' . $propCount,
            'value' => $Sentence->split($value[$propCount]['value']['#text'])
          );
        }
        $propCount++;
      }
    }
    return $translation_payload;
  }

  private function renderHTML($label, $text) {
    if (!is_array($text)) {
      return "<div class=\"tx_string\" id=\"" . $label . "\">" . $text . "</div>";
    } else {
      $ret = "<div class=\"tx_string\" id=\"" . $label . "\">";
      foreach ($text as $sentence) {
        $ret .= "<div class=\"tx_string_sentence\">" . $sentence . '</div>';
      }
      return $ret . "</div>";
    }
  }

  public function renderHTMLFromStrings($strings) {
    $html = '';
    // Get title
    $title_index = (array_search('title_field_0', array_column($strings, 'key')));
    if ($title_index) {
      $html .= $this->renderHTML('title_field_0', $strings[$title_index]['value']);
      unset($strings[$title_index]);
    }
    // Get the rest
    foreach ($strings as $string) {
      $html .= $this->renderHTML($string['key'], $string['value']);
    }
    return $html;
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
      if (isset($job_item->data['node_title'] )) {
        $title = $job_item->data['node_title']['#text'];
      } else {
        $title = $job_item->data['title_field'][0]['value']['#text'];
      }
      $strings = $this->extractStringsFromNode($job_item);
      $payload = $this->renderHTMLFromStrings($strings);
      $slug = extractSlugFromUrl($translator->getSetting('project')) . '_' . $job_item->item_id;
      $res = $tx->upsertResource($slug, $title, $job->tjid, $payload);
      if (is_array($res)) {
        drupal_set_message('Added ' . $res[0] . ' strings to resource named: ' . $title, 'status');
        $job->submitted('The translation job has been submitted to Transifex.');
      } else {
        drupal_set_message($res, 'error');
      }
    }
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
