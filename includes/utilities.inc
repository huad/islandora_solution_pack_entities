<?php
/**
 * @file
 * Holds helper functions for islandora_entities.
 */


/**
 * This function will add the entity content model to an object.
 *
 * Due to natural inheritance in the data model all entity type
 * objects will have the content model of entity as well as one
 * for their types.
 *
 * @param AbstractObject $fedora_object
 *   The object to add the relationship to.
 */
function islandora_entities_add_entity_content_model(AbstractObject $fedora_object) {
  $fedora_object->relationships->add(
    FEDORA_MODEL_URI,
    'hasModel',
    'islandora:entityCModel'
  );
}

/**
 * This function will run a query using Solr for the specified entity type.
 *
 * @param string $content_model
 *   The PID of the content model object representing they type of entity.
 * @param string $entities_query
 *   The value of the query to run for the Solr search.
 *
 * @return array
 *   The results of the query. Limited to PID, object label and EAC-CPF name.
 *   IslandoraSolrQueryProcessor->islandoraSolrResult
 */
function islandora_entities_solr_search_entities($content_model, $entities_query) {
  module_load_include('inc', 'islandora_solr_search', 'includes/utilities');
  $results = array();

  $content_model_field = variable_get(
    'islandora_solr_content_model_field',
    'RELS_EXT_hasModel_uri_ms'
  );
  $entities_query_field = variable_get(
    'islandora_entities_query_field',
    'eaccpf_name_complete_et'
  );
  $object_label_field = variable_get(
    'islandora_solr_object_label_field',
    'fgs_label_s'
  );

  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->solrQuery = format_string(
    '!entities_query_field:(!entities_query)',
    array(
      '!entities_query_field' => $entities_query_field,
      '!entities_query' => islandora_solr_lesser_escape($entities_query),
    )
  );
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = 10;
  $query_processor->solrParams = array(
    'fl' => "$entities_query_field, $object_label_field, PID",
    'fq' => format_string(
      '!content_model_field:"info:fedora/!content_model"',
      array(
        '!content_model_field' => $content_model_field,
        '!content_model' => $content_model,
      )
    ),
  );

  $query_processor->executeQuery(FALSE);
  $solr_results = $query_processor->islandoraSolrResult;
  return $solr_results;
}

/**
 * This function will add an entity DS composite datastream to an object.
 *
 * This is a workaround of a known issue with temp files being lost in Tuque in
 * batches because of object destruction during the serialization process.
 *
 * @param AbstractObject $islandora_object
 *   The object to add the DS composite datastream to.
 */
function islandora_entities_ingest__entity_ds_composite_datastream(AbstractObject $islandora_object) {
  $module_path = drupal_get_path('module', 'islandora_entities');

  $ds_composite_datastream = $islandora_object->constructDatastream('DS-COMPOSITE-MODEL', 'X');
  $ds_composite_datastream->label = 'DS-COMPOSITE-MODEL';
  $ds_composite_datastream->mimetype = 'text/xml';
  $ds_composite_datastream->setContentFromFile("$module_path/xml/islandora_entity_ds_composite_model.xml", FALSE);
  $islandora_object->ingestDatastream($ds_composite_datastream);
}
