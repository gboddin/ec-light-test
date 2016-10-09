<?php

/**
 * @file
 * Provides Google Translator ui controller.
 */

/**
 * DGT connector translator ui controller.
 */
class TMGMTPoetryTranslatorUIController extends TMGMTDefaultTranslatorUIController {
  /**
   * Overrides TMGMTDefaultTranslatorUIController::pluginSettingsForm().
   */
  public function pluginSettingsForm($form, &$form_state, TMGMTTranslator $translator, $busy = FALSE) {

    $form['poetry_settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Poetry Settings'),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
    );

    $poetry_message = $this->checkPoetrySettings();
    $form['poetry_settings']['poetry_message'] = array(
      '#markup' => $poetry_message,
    );

    $settings = $translator->getSetting('settings');
    $form['settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('General settings'),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
      '#description' => t("General settings for the requests"),
    );
    $form['settings']['counter'] = array(
      '#type' => 'textfield',
      '#title' => t('Counter'),
      '#required' => TRUE,
      '#default_value' => $settings['counter'],
    );
    $form['settings']['code'] = array(
      '#type' => 'textfield',
      '#title' => t('Requester code'),
      '#required' => TRUE,
      '#default_value' => $settings['code'],
    );

    $organization = $translator->getSetting('organization');
    $form['organization'] = array(
      '#type' => 'fieldset',
      '#title' => t('Organization'),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
      '#description' => t("Requester organization information"),
    );
    $form['organization']['responsable'] = array(
      '#type' => 'textfield',
      '#title' => t('Responsable'),
      '#required' => TRUE,
      '#default_value' => $organization['responsable'],
      '#description' => t("Eg.: DIGIT"),
    );
    $form['organization']['auteur'] = array(
      '#type' => 'textfield',
      '#title' => t('Author'),
      '#required' => TRUE,
      '#default_value' => $organization['auteur'],
      '#description' => t("Eg.: IE/CE/DIGIT"),
    );
    $form['organization']['demandeur'] = array(
      '#type' => 'textfield',
      '#title' => t('Requester'),
      '#required' => TRUE,
      '#default_value' => $organization['demandeur'],
      '#description' => t("Eg.: IE/CE/DIGIT/A/3"),
    );

    $contacts = $translator->getSetting('contacts');
    $form['contacts'] = array(
      '#type' => 'fieldset',
      '#title' => t('Contact usernames'),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
      '#description' => t("Contact persons for the translation request sent to Poetry. WARNING: Valid EC usernames must be used"),
    );
    $form['contacts']['auteur'] = array(
      '#type' => 'textfield',
      '#title' => t('Author'),
      '#required' => TRUE,
      '#default_value' => $contacts['auteur'],
    );
    $form['contacts']['secretaire'] = array(
      '#type' => 'textfield',
      '#title' => t('Secretaire'),
      '#required' => TRUE,
      '#default_value' => $contacts['secretaire'],
    );
    $form['contacts']['contact'] = array(
      '#type' => 'textfield',
      '#title' => t('Contact'),
      '#required' => TRUE,
      '#default_value' => $contacts['contact'],
    );
    $form['contacts']['responsable'] = array(
      '#type' => 'textfield',
      '#title' => t('Responsible'),
      '#required' => TRUE,
      '#default_value' => $contacts['responsable'],
    );

    $feedback_contacts = $translator->getSetting('feedback_contacts');
    $form['feedback_contacts'] = array(
      '#type' => 'fieldset',
      '#title' => t('DGT Contacts'),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
      '#description' => t("Contact persons for send a feedback"),
    );
    $form['feedback_contacts']['to'] = array(
      '#type' => 'textfield',
      '#title' => t('Email to'),
      '#required' => TRUE,
      '#default_value' => $feedback_contacts['to'],
    );
    $form['feedback_contacts']['cc'] = array(
      '#type' => 'textfield',
      '#title' => t('Email CC'),
      '#required' => TRUE,
      '#default_value' => $feedback_contacts['cc'],
    );

    return parent::pluginSettingsForm($form, $form_state, $translator);
  }

  /**
   * {@inheritdoc}
   */
  public function checkoutInfo(TMGMTJob $job) {
    $form_state = array();
    $form['settings'] = $this->checkoutSettingsForm(array(), $form_state, $job);

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function checkoutSettingsForm($form, &$form_state, TMGMTJob $job) {
    // Build the sortable table header.
    $header = array(
      'language' => t('Language'),
    );

    if ($job->state > TMGMT_JOB_STATE_UNPROCESSED) {
      $header['state'] = t('State');
      $header['delai'] = t('Delivery date');
      $header['operations'] = t('Operations');
    }

    $target_language = isset($form_state['values']['target_language'])
      ? $form_state['values']['target_language']
      : $job->target_language;

    $default_value = array();

    foreach (language_list() as $language) {
      if ($language->language == $job->source_language) {
        continue;
      }

      $language_job_ids = _tmgmt_poetry_obtain_related_translation_jobs(array($language->language), '%' . str_replace('MAIN_', '', $job->reference))->fetchAll();
      if (count($language_job_ids) > 0) {
        $language_job_id = array_shift($language_job_ids);
        $language_job_item = tmgmt_job_item_load($language_job_id->tjiid);
        $result = db_select('poetry_status', 'ps')
          ->fields('ps')
          ->condition('tjiid', $language_job_item->tjiid, '=')
          ->execute()
          ->fetchAssoc();
        if (!empty($result)) {
          $options[$language->language]['state'] = $result['status'];
          $options[$language->language]['delai'] = $result['delai'];
        }
        if (in_array($language_job_item->state, array(TMGMT_JOB_ITEM_STATE_REVIEW, TMGMT_JOB_ITEM_STATE_ACCEPTED))) {
          $options[$language->language]['operations']  = l(t('View'), 'admin/tmgmt/items/' . $language_job_item->tjiid, array('query' => array('destination' => 'admin/tmgmt/jobs/' . $job->tjid)));
        }
      }

      if (!isset($options[$language->language]['state'])) {
        $options[$language->language]['state'] = t('Not sent');
      }

      if (!isset($options[$language->language]['delai'])) {
        $options[$language->language]['delai'] = '';
      }

      if (!isset($options[$language->language]['operations'])) {
        $options[$language->language]['operations'] = '';
      }
      $options[$language->language]['language'] = $language->name;
      if ($language->language == $target_language) {
        $default_value[$target_language] = $target_language;
      }
    }

    if (isset($job->settings['languages'])) {
      foreach ($job->settings['languages'] as $selected_language) {
        if (!empty($selected_language)) {
          $default_value[$selected_language] = $selected_language;
        }
      }
    }

    // Build the tableselect.
    $form['languages'] = array(
      '#type' => 'tableselect',
      '#header' => $header,
      '#options' => $options,
      '#default_value' => $default_value,
      '#empty' => t('No content available.'),
    );
    $form['languages'][$target_language]['#disabled'] = TRUE;

    $job_items = $job->getItems();
    if (count($job_items) > 0) {
      $job_item = array_shift($job_items);
    }

    // Load translator settings : Contacts.
    $translator = $job->getTranslator();
    $contacts = $translator->getSetting('contacts');
    $form['contacts'] = array(
      '#type' => 'fieldset',
      '#title' => t('Contact usernames'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#description' => t("Contact persons for the translation request sent to Poetry. WARNING: Valid EC usernames must be used"),
    );
    $form['contacts']['auteur'] = array(
      '#type' => 'textfield',
      '#title' => t('Author'),
      '#required' => TRUE,
      '#default_value' => (!empty($job->settings['contacts']['auteur']) ? $job->settings['contacts']['auteur'] : $contacts['auteur']),
    );
    $form['contacts']['secretaire'] = array(
      '#type' => 'textfield',
      '#title' => t('Secretaire'),
      '#required' => TRUE,
      '#default_value' => (!empty($job->settings['contacts']['secretaire']) ? $job->settings['contacts']['secretaire'] : $contacts['secretaire']),
    );
    $form['contacts']['contact'] = array(
      '#type' => 'textfield',
      '#title' => t('Contact'),
      '#required' => TRUE,
      '#default_value' => (!empty($job->settings['contacts']['contact']) ? $job->settings['contacts']['contact'] : $contacts['contact']),
    );
    $form['contacts']['responsable'] = array(
      '#type' => 'textfield',
      '#title' => t('Responsible'),
      '#required' => TRUE,
      '#default_value' => (!empty($job->settings['contacts']['responsable']) ? $job->settings['contacts']['responsable'] : $contacts['responsable']),
    );

    // Load translator settings : Organization.
    $organization = $translator->getSetting('organization');
    $form['organization'] = array(
      '#type' => 'fieldset',
      '#title' => t('Organization'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#description' => t("Requester organization information"),
    );
    $form['organization']['responsable'] = array(
      '#type' => 'textfield',
      '#title' => t('Responsable'),
      '#required' => TRUE,
      '#default_value' => $organization['responsable'],
      '#description' => t("Eg.: DIGIT"),
    );
    $form['organization']['auteur'] = array(
      '#type' => 'textfield',
      '#title' => t('Author'),
      '#required' => TRUE,
      '#default_value' => $organization['auteur'],
      '#description' => t("Eg.: IE/CE/DIGIT"),
    );
    $form['organization']['demandeur'] = array(
      '#type' => 'textfield',
      '#title' => t('Requester'),
      '#required' => TRUE,
      '#default_value' => $organization['demandeur'],
      '#description' => t("Eg.: IE/CE/DIGIT/A/3"),
    );

    $form['delai'] = array(
      '#type' => 'date_popup',
      '#title' => t('Expected delivery time'),
      '#default_value' => (!empty($job->settings['delai']) ? $job->settings['delai'] : date('Y-m-d', strtotime('+1 month'))),
      '#date_format' => 'd/m/Y',
      '#date_year_range' => '-3:+3',

    );

    $form['remark'] = array(
      '#type' => 'textfield',
      '#title' => t('Remark'),
      '#size' => 128,
      '#maxlength' => 255,
      '#default_value' => (!empty($job->settings['remark']) ? $job->settings['remark'] : ''),
      '#description' => t("Comment to be sent along with the request"),
    );

    if ($job->state != TMGMT_JOB_STATE_UNPROCESSED) {
      $form['languages']['#disabled'] = TRUE;
      $form['contacts']['#disabled'] = TRUE;
      $form['organization']['#disabled'] = TRUE;
      $form['delai']['#disabled'] = TRUE;
      $form['remark']['#disabled'] = TRUE;
    }

    return $form;
  }

  /**
   * Helper module to check MT@EC settings.
   */
  protected function checkPoetrySettings() {
    $messages = array();
    $poetry_settings = variable_get('poetry_service');
    if (!isset($poetry_settings) || empty($poetry_settings)) {
      $messages[] = t('Main "poetry_service" variable was not set or is empty. See the README.md file or read the wiki page.');
      $type = 'error';
    }
    if (empty($messages)) {
      $messages[] = t('Main "poetry_service" variable is properly set.');
      $type = 'status';
    }
    return $this->themeMessages($messages, $type);
  }

  /**
   * Helper module to theme MT@EC messages.
   *
   * @param array $messages
   *   Array of messages to be displayed.
   * @param string $type
   *   Type of the messages.
   *
   * @return string
   *   Themed messages.
   */
  protected function themeMessages(array $messages, $type) {
    $output = '';
    if (!empty($messages)) {
      $output .= "<div class=\"messages $type\">\n";
      if (count($messages) > 1) {
        $output .= " <ul>\n";
        foreach ($messages as $message) {
          $output .= '  <li>' . $message . "</li>\n";
        }
        $output .= " </ul>\n";
      }
      else {
        $output .= $messages[0];
      }
      $output .= "</div>\n";
    }
    return $output;
  }

  /**
   * {@inheritdoc}
   */
  public function reviewFormSubmit($form, &$form_state, TMGMTJobItem $item) {
    // Check if the user clicked on 'Accept', 'Submit' or 'Reject'.
    if (!empty($form['actions']['accept']) && $form_state['triggering_element']['#value'] == $form['actions']['accept']['#value']) {
      db_update('poetry_status')
        ->fields(array('status' => 'Translation accepted'))
        ->condition('tjiid', $item->tjiid, '=')
        ->execute();
    }
  }

}