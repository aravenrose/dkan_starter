<?php

/**
 * @file
 * Migration Class for Datasets.
 */

/**
 * Migrate Data.json datasets.
 */
class MigrateDataJsonDatasetBase extends MigrateDKAN {
  /**
   * Here we go.
   */
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->endpoint = isset($arguments['endpoint']) ? $arguments['endpoint'] : 'http://demo.getdkan.com/data.json';
    $this->version = isset($arguments['version']) ? $arguments['version'] : '1.1';
    $this->page = isset($arguments['page']) ? $arguments['page'] : '';
    $this->offset = isset($arguments['offset']) ? $arguments['offset'] : '';
    $fields = $this->getDataJsonDatasetFields();

    $this->highwaterField = array('name' => 'modified');
    $this->source = new MigrateSourceList(new DataJsonListJSON(
      $this->endpoint,
      array('page' => $this->page,
            'offset' => $this->offset),
      $this->version),
      new DataJsonItemJSON($this->endpoint, array(), $this->version), $fields);

    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'uuid' => array(
          'type' => 'varchar',
          'length' => 255,
          'not null' => TRUE,
          'description' => 'id',
        ),
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $this->addDataJsonMapping();

    $this->destination = new MigrateDestinationNode('dataset', array('text_format' => 'html'));
  }

  public function getMapping($version = '1.1') {
    $odfeFields = array();
    $baseFields = array(
      // These are common to both 1.0 and 1.1
      'title' => 'title',
      'body' => 'description',
      'og_group_ref' => 'group_id',
      'field_tags' => 'keyword',
      'changed' => 'modified',
      'created' => 'issued',
      'field_public_access_level' => 'accessLevel',
      'field_resources' => 'resources',
      'field_contact_name' => 'contactPointName',
      'field_contact_email' => 'mbox',
      'uuid' => 'identifier',
      'field_license' => 'license',
      'field_spatial_geographical_cover' => 'spatial',
      'field_temporal_coverage' => 'temporalBegin',
      'field_temporal_coverage:to' => 'temporalEnd',
      'field_frequency' => 'accrualPeriodicity',
      'field_data_dictionary' => 'describedBy',
    );
    if ($version == '1.0') {
      $addFields = array(
        'field_data_dictionary' => 'dataDictionary',
        'language' => 'language',
      );
    }
    else {
      $addFields = array(
        // POD 1.1.
        'field_additional_info' => 'field_additional_info_key',
        'field_additional_info:second' => 'field_additional_info_value',
        'field_related_content' => 'references',
      );
    }
    if (module_exists('open_data_federal_extras')) {
      $odfeFields =  array(
        'field_odfe_bureau_code' => 'bureauCode',
        'field_odfe_program_code' => 'programCode',
        'field_odfe_landing_page' => 'landingPage',
        'field_odfe_rights' => 'rights',
        'field_odfe_conforms_to' => 'conformsTo',
        'field_odfe_data_quality' => 'dataQuality',
        'field_odfe_data_dictionary_type' => 'describedByType',
        'field_odfe_is_part_of' => 'isPartOf',
        'field_odfe_language' => 'language',
        'field_odfe_investment_uii' => 'primaryITInvestmentUII',
        'field_odfe_system_of_records' => 'systemOfRecords',
        'field_odfe_category' => 'theme',
      );
    }
    return $baseFields + $addFields + $odfeFields;
  }

  /**
   * Adds Data.json mappings.
   */
  public function addDataJsonMapping($version = '1.1') {
    $fields = $this->getMapping($version);
    foreach ($fields as $dkan => $pod) {
      $this->addFieldMapping($dkan, $pod, FALSE);
    }
  }

  /**
   * Creates list of fields for Data.json Dataset.
   */
  public function getDataJsonDatasetFields($version = '1.1') {
    if ($version == '1.0') {
      return array(
        // “Common Core” Required Fields.
        "title" => "Title",
        "description" => "Description",
        "keyword" => "Tags",
        "modified" => "Last Update",
        "publisher" => "Publisher",
        "contactPoint" => "Contact Name",
        "mbox" => "Contact Email",
        "identifier" => "Unique Identifier",
        "accessLevel" => "Public",
        // “Common Core” Required-if-Applicable Fields.
        "bureauCode" => " Bureau Code",
        "programCode" => "Program",
        "accessLevelComment" => " Access Level Comment",
        "accessURL" => "Download",
        "webService" => " Endpoint ",
        "format" => " Format",
        "license" => "License",
        "spatial" => "Spatial",
        "temporal" => " Temporal",
        // Beyond Common Core.
        "theme" => "Category",
        "dataDictionary" => "Data Dictionary",
        "dataQuality" => "Data Quality",
        "distribution" => "Distribution",
        "accrualPeriodicity" => "Frequency",
        "landingPage" => "Homepage",
        "language" => "Language",
        "PrimaryITInvestmentUII" => "Primary IT Investment",
        "references" => "Related Documents",
        "issued" => "Release Date",
        "systemOfRecords" => "System of Records",
        // This doesn't actually exist but we are adding it later in prepareRow.
        "uid" => "User ID",
      );
    }
    else {
      return array(
        "title" => "Title",
        "description" => "Description",
        "keyword" => "Tags",
        "modified" => "Last Update ",
        "publisher" => "Publisher",
        "contactPoint" => " Contact Name and Email",
        "identifier" => " Unique Identifier",
        "accessLevel" => "Public",
        "bureauCode" => "Bureau",
        "programCode" => " Program Code ",
        "license" => "License",
        "rights" => " Rights",
        "spatial" => "Spatial",
        "temporal" => " Temporal",
        "distribution" => " Distribution",
        "accrualPeriodicity" => " Frequency",
        "conformsTo" => " Data Standard URI",
        "dataQuality" => " Data Quality",
        "describedBy" => "Data Dictionary",
        "describedByType" => "Data",
        "isPartOf" => " Collection",
        "issued" => " Release Date",
        "language" => " Language",
        "landingPage" => "Homepage",
        "primaryITInvestmentUII" => "Primary",
        "references" => " Related Documents ",
        "systemOfRecords" => " System of Records",
        "theme" => "Theme",
        // This doesn't actually exist but we are adding it later in prepareRow.
        "uid" => "User ID",
      );
    }
  }

  /**
   * Updates custom highwater field "modified" which we've added to the
   * migrate_map table.
   */
  public function prepareCustomHighwater($row) {
    $table = $this->map->getMapTable();
    $modified = db_query(
      'SELECT modified FROM {' . $table . '} WHERE sourceid1 = :identifier',
      array(':identifier' => $row->identifier)
    )->fetchfield();
    // Update the highwater mark if modified is greater than previous.
    if ($row->modified > $modified) {
      db_update($table)
        ->condition('sourceid1', $row->identifier, "=")
        ->fields(array(
          'modified' => strtotime($row->modified)
        ))
        ->execute();
    }
    // If the modified row is greater than the map we want to update.
    if (strtotime($row->modified) > $row->migrate_map_modified) {
      return TRUE;
    }
  }

  /**
   *
   */
  public function mapExisting($row) {
    $existingUnmapped = db_query(
      'select nid from {node} where title = :title',
      array(':title' => $row->title)
    )->fetchfield();
    if (is_numeric($existingUnmapped)) {
      $mapped = db_query(
        'SELECT destid1 FROM {migrate_map_'. $table->machineName. '} WHERE sourceid1 = :uuid',
        array(':uuid' => $row->uuid)
      )->fetchField();
      // Only update existing if it isn't already mapped.
      if (!$mapped) {
        // TODO: add to map.
      }
    }
  }

  /**
   *
   */
  public function loadExisting(&$row) {
    $table = $this->map->getMapTable();
    $existing = db_query(
      'SELECT destid1 FROM {' . $table . '} WHERE sourceid1 = :identifier',
      array(':identifier' => $row->identifier)
    )->fetchfield();
    if (is_numeric($existing)) {
      $row->existing = node_load($existing);
    }
  }

  /**
   * Implements prepareRow.
   */
  public function prepareRow($row) {
    // Only update if the modified row in the source is greater than the
    // previous modified date.
    if (!$this->prepareCustomHighwater($row)) {
      return FALSE;
    }

    //$this->mapExisting($row);

    $this->loadExisting($row);

    if (isset($row->temporal)) {
      $date = dkan_migrate_base_iso_interval_to_timestamp($row->temporal);
      if (isset($date['from'])) {
        $row->temporalBegin = $date['from'];
      }
      if (isset($date['to'])) {
        $row->temporalEnd = $date['to'];
      }
    }
    if (isset($row->keyword)) {
      $tags = taxonomy_vocabulary_machine_name_load('tags');
      foreach ($row->keyword as $tag) {
        $this->createTax($tag, 'tags', $tags->vid);
      }
    }
    if (isset($row->theme) && module_exists('open_data_federal_extras')) {
      $category = taxonomy_vocabulary_machine_name_load('odfe_theme');
      foreach ($row->theme as $cat) {
        $this->createTax($cat, 'odfe_theme', $category->vid);
      }
    }
    if (is_array($row->publisher)) {
      $row->publisher = (object) $row->publisher;
    }
    if (isset($row->publisher->name) && !$row->group_id = $this->getGroupNidByTitle($row->publisher->name)) {
      if ($row->publisher->name) {
        $group = new stdClass;
        $group->title = $row->publisher->name;
        $group = $this->createGroupNode($group);
        $this->map->saveIDMapping($group, array($group->nid), 0, 0, '');
        $row->group_id = $group->nid;
      }
    }
    if (isset($row->theme) && is_array($row->theme)) {
      $category = taxonomy_vocabulary_machine_name_load('odfe_theme');
      foreach ($row->theme as $theme) {
        $this->createTax($theme, 'odfe_theme', $category->vid);
      }
    }
    if (is_array($row->contactPoint)) {
      $row->contactPoint = (object) $row->contactPoint;
    }
    if (is_object($row->contactPoint)
      && $row->contactPoint->hasEmail) {
      $row->mbox = str_replace("mailto:", "", $row->contactPoint->hasEmail) ? str_replace("mailto:", "", $row->contactPoint->hasEmail) : $row->contactPoint->hasEmail;
      $row->contactPointName = isset($row->contactPoint->fn) ? $row->contactPoint->fn : '';
      if (!filter_var($row->mbox, FILTER_VALIDATE_EMAIL)) {
        $row->mbox = '';
      }
    }
    if (isset($row->dataQuality)) {
      if ($row->dataQuality === FALSE) {
        $row->dataQuality = 'false';
      }
      elseif ($row->dataQuality === TRUE) {
        $row->dataQuality = 'true';
      }
    }
    if ($row->existing) {
      $this->deleteExistingResources($row->existing);
    }
    $row->resources = $this->prepareResources($row);
    $this->prepareAdditionalInfo($row);
    $row->uid = 1;
  }

  /**
   *
   */
  public function deleteExistingResources($node) {
    $table = $this->map->getMapTable();
    if ($node->field_resources) {
      foreach ($node->field_resources['und'] as $rid) {
        $resource = node_load($rid['target_id']);
        // Remove from the mapping.
        db_delete($table)
          ->condition('sourceid1', $resource->uuid)
          ->execute();
        // Remove from database;
        node_delete($resource->nid);
      }
    }
  }

  public function prepareAdditionalInfo($row) {
    $ignore = array(
      '@type',
      'distribution',
      'uuid',
      'tag_names',
      'group_id',
      'modified',
      'mbox',
      'contactPointName',
      'temporalBegin',
      'temporalEnd',
      'publisher-name',
      'existing',
      'migrate_map_sourceid1',
      'migrate_map_destid1',
      'migrate_map_needs_update',
      'migrate_map_rollback_action',
      'migrate_map_last_imported',
      'migrate_map_hash',
    );
    $mapped_fields = array_keys($this->getDataJsonDatasetFields());
    $row_keys = (array) $row;
    $row_keys = array_keys($row_keys);
    foreach ($row_keys as $key) {
      if (!in_array($key, $mapped_fields) && !in_array($key, $ignore) && !$this->regexIgnore($key, $ignore)) {
        if (is_object($row->{$key}) || is_array($row->$key)) {
          foreach ((array) $row->{$key} as $k => $v) {
            $k = $key . '-' . $k;
            $row->field_additional_info_key[] = substr($k, 0, 254);
            $row->field_additional_info_value[] = substr($v, 0, 254);
          }
        }
        else {
          $row->field_additional_info_key[] = substr($key, 0, 254);
          $row->field_additional_info_value[] = substr($row->{$key}, 0, 254);
        }
      }
    }
  }

  /**
   * Returns TRUE if it is OK to ignore the $key value.
   */
  public function regexIgnore($key, $ignore) {
    foreach ($ignore as $ign) {
      if (strpos($ign, $key) !== false) {
        return FALSE;
      }
    }
    return TRUE;
  }

  /**
   * Prepare Resources.
   */
  public function prepareResources($row) {
    $resources = array();
    if (isset($row->distribution)) {
      foreach ($row->distribution as $resource) {
        if (!is_array($resource)) {
          $resource = (array) $resource;
        }
        $res = new stdClass();
        $res->format = $resource['format'];
        // Use media type for format if no format provided.
        if (is_null($res->format) && isset($resource['mediaType'])) {
           include_once DRUPAL_ROOT . '/includes/file.mimetypes.inc';
           $mapping = file_mimetype_mapping();
           foreach ($mapping['mimetypes'] as $type_num => $type) {
             if ($type == $resource['mediaType']) {
               foreach ($mapping['extensions'] as $ex_type => $ex_num) {
                 if ($type_num == $ex_num) {
                   $res->format = $ex_type;
                 }
               }
             }
           }
        }
        // If still null try to strip "application".
        if (is_null($res->format) && isset($resource['mediaType']) && strpos($resource['mediaType'],'application') !== false) {
          $res->format = str_replace('application/', '', $resource['mediaType']);
        }
        $res->name = isset($resource['title']) ? $resource['title'] : '';
        // Provide default for distribution items with absent name property.
        if ($res->format && !$res->name) {
          $res->name = $res->format;
        }
        $res->name = $res->name ? $res->name : t('Resource');
        $res->url = $resource['downloadURL'];
        $res->api = $resource['accessURL'];
        $resource_node = $this->createResourceNode($res);
        $this->map->saveIDMapping($resource_node, array($resource_node->nid), 0, 0, '');
        $resources[] = $resource_node->nid;
      }
    }
    return $resources;
  }

  public function processImport(array $options = array()) {
    parent::processImport($options);
    $table = $this->map->getMapTable();
    db_update($table)
      ->condition('modified', "", "=")
      ->fields(array(
        'modified' => time()
      ))
      ->execute();
  }

  /**
   * Implements prepareRow.
   */
  public function prepare($entity, $row) {
    // Remove empy additional fields.
    if (isset($entity->field_additional_info['und'][0]) && $entity->field_additional_info['und'][0]['first'] == NULL) {
      unset($entity->field_additional_info);
    }
    if (isset($entity->field_temporal_coverage['und'][0]['value'])) {
      $entity->field_temporal_coverage['und'][0]['timezone'] = 'UTC';
    }
    $entity->status = 1;
    $entity->vid = isset($entity->vid) ? $entity->vid : '';
  }
}

class DataJsonListJSON extends MigrateListJSON {

  /**
   * Adds version to class.
   */
  public function __construct($list_url, $http_options = array(), $version = '1.0') {
    parent::__construct($list_url);
    $this->listUrl = $list_url;
    $this->httpOptions = $http_options;
    $this->page = isset($http_options['page']) ? $http_options['page'] : '';
    $this->offset = isset($http_options['offset']) ? $http_options['offset'] : '';
    $this->version = $version;
  }

  /**
   * Implements computeCount().
   */
  public function computeCount() {
    $count = 0;
    if (!filter_var($this->listUrl, FILTER_VALIDATE_URL)) {
      $json = file_get_contents($this->listUrl);
    }
    else {
      $response = drupal_http_request($this->listUrl, $this->httpOptions);
      $json = $response->data;
    }
    if ($json) {
      $data = drupal_json_decode($json);
      if ($this->version == '1.1') {
        $datasets = 0;
        if (isset($data)) {
          foreach ($data['dataset'] as $item) {
            // Adds resources to count.
            if (isset($item['distribution'])) {
              $count++;
              $count = $count + count($item['distribution']);
            }
            if (isset($item['publisher']['name'])) {
              $count++;
            }
            $datasets++;
            if ($this->page && $datasets >= $this->page) {
              break;
            }
          }
        }
      }
      else {
        foreach ($data as $item) {
          // Adds resources to count.
          if (isset($item['distribution'])) {
            $count++;
            $count = $count + count($item['distribution']);
          }
        }
      }
    }
    if ($this->version == '1.1') {
      $dataset_count = $this->page ? $this->page : count($data['dataset']);
    }
    else {
      $dataset_count = $this->page ? $this->page : count($data);
    }
    $resource_count = $count - $dataset_count;
    drupal_set_message(t("%url contains %datasets datasets and %resources resources.", array('%datasets' => $dataset_count, '%url' => $this->listUrl, '%resources' => $resource_count )));
    return $count;
  }

  public function getIdList() {
    migrate_instrument_start("Retrieve $this->listUrl");
    if (!filter_var($this->listUrl, FILTER_VALIDATE_URL)) {
      $json = file_get_contents($this->listUrl);
    }
    else {
      $response = drupal_http_request($this->listUrl, $this->httpOptions);
      $json = $response->data;
    }
    migrate_instrument_stop("Retrieve $this->listUrl");
    if ($json) {
      $data = drupal_json_decode($json);
      if ($data) {
        return $this->getIDsFromJSON($data);
      }
    }
    Migration::displayMessage(t('Loading of !listurl failed:',
        array('!listurl' => $this->listUrl)));
    return NULL;
  }

  /**
   * Retrieves ID from list.
   */
  protected function getIDsFromJSON(array $data) {
    $ids = array();
    if ($this->version == '1.0') {
      foreach ($data as $item) {
        $ids[] = $item['identifier'];
      }
    }
    else {
      $datasets = 0;
      $total = $this->page + $this->offset;
      foreach ($data['dataset'] as $item) {
        if ($datasets < $this->offset) {
          $datasets++;
          continue;
        }
        $uuid = explode("\\", $item['identifier']);
        $ids[] = array_pop($uuid);
        $datasets++;
        if ($total && $datasets >= $total) {
          break;
        }
      }
    }
    return $ids;
  }
}

class DataJsonItemJSON extends MigrateItemJSON {
  protected $data = array();
  /**
   * Adds version to class.
   */
  public function __construct($list_url, $http_options = array(), $version = '1.0') {
    parent::__construct($list_url);
    $this->listUrl = $list_url;
    $this->httpOptions = $http_options;
    $this->version = $version;
  }

  /**
   * Retrieves from itemUrl.
   */
  public function getItem($id) {
    // We cache the parsed JSON at $this->data.
    if (empty($this->data)) {
      $data = $this->loadJSONUrl($this->itemUrl);
      if ($data) {
        if ($this->version == '1.0') {
          foreach ($data as $item) {
            $this->data[$item->identifier] = $item;
          }
        }
        else {
          foreach ($data->dataset as $item) {
            $this->data[$item->identifier] = $item;
          }
        }
      }
      else {
        // TODO: Proper error handling.
      }
    }
    // Return the requested item.
    if (isset($this->data[$id])) {
      return $this->data[$id];
    }
    else {
      return NULL;
    }
  }
}