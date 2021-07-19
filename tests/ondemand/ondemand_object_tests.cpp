#include "simdjson.h"
#include "test_ondemand.h"

using namespace simdjson;

namespace object_tests {
  using namespace std;
  using simdjson::ondemand::json_type;
  // In this test, no non-trivial object in an array have a missing key
  bool no_missing_keys() {
    TEST_START();
    simdjson::ondemand::parser parser;
    simdjson::padded_string docdata =  R"([{"a":"a"},{}])"_padded;
    simdjson::ondemand::document doc;
    auto error = parser.iterate(docdata).get(doc);
    if(error != simdjson::SUCCESS) { return false; }
    simdjson::ondemand::array a;
    error = doc.get_array().get(a);
    if(error != simdjson::SUCCESS) { return false; }
    size_t counter{0};
    for(auto elem : a) {
      error = elem.find_field_unordered("a").error();
      if(counter == 0) {
        ASSERT_EQUAL( error, simdjson::SUCCESS);
      } else {
        ASSERT_EQUAL( error, simdjson::NO_SUCH_FIELD);
      }
      counter++;
    }
    return true;
  }

  bool missing_key_continue() {
    TEST_START();
    simdjson::ondemand::parser parser;
    simdjson::padded_string docdata =  R"({"a":0, "b":1, "c":2})"_padded;
    simdjson::ondemand::document doc;
    ASSERT_SUCCESS(parser.iterate(docdata).get(doc));
    int64_t num;
    ASSERT_SUCCESS(doc["a"].get(num));
    ASSERT_EQUAL(num, 0);
    ASSERT_SUCCESS(doc["b"].get(num));
    ASSERT_EQUAL(num, 1);
    ASSERT_SUCCESS(doc["c"].get(num));
    ASSERT_EQUAL(num, 2);
    // Start again, but omit a key
    ASSERT_SUCCESS(parser.iterate(docdata).get(doc));
    ASSERT_SUCCESS(doc["a"].get(num));
    ASSERT_EQUAL(num, 0);
    ASSERT_SUCCESS(doc["c"].get(num));
    ASSERT_EQUAL(num, 2);
    // Start again, but request a missing key
    ASSERT_SUCCESS(parser.iterate(docdata).get(doc));
    simdjson::ondemand::object obj;
    ASSERT_SUCCESS(doc.get_object().get(obj));
    ASSERT_SUCCESS(obj["a"].get(num));
    ASSERT_EQUAL(num, 0);
    assert_error(obj["d"].get(num), NO_SUCH_FIELD);
    ASSERT_SUCCESS(obj["c"].get(num));
    ASSERT_EQUAL(num, 2);
    // Start again, but request a missing key first
    ASSERT_SUCCESS(parser.iterate(docdata).get(doc));
    ASSERT_SUCCESS(doc.get_object().get(obj));
    assert_error(obj["d"].get(num), NO_SUCH_FIELD);
    ASSERT_SUCCESS(obj["a"].get(num));
    ASSERT_EQUAL(num, 0);
    // Start again, but request a missing key twice
    ASSERT_SUCCESS(parser.iterate(docdata).get(doc));
    ASSERT_SUCCESS(doc.get_object().get(obj));
    ASSERT_SUCCESS(obj["a"].get(num));
    ASSERT_EQUAL(num, 0);
    assert_error(obj["d"].get(num), NO_SUCH_FIELD);
    assert_error(obj["z"].get(num), NO_SUCH_FIELD);
    // Because we do a full circle, you can query the same
    // key twice!!!
    ASSERT_SUCCESS(obj["a"].get(num));
    ASSERT_EQUAL(num, 0);
    ASSERT_SUCCESS(obj["b"].get(num));
    ASSERT_EQUAL(num, 1);
    ASSERT_SUCCESS(obj["b"].get(num));
    ASSERT_EQUAL(num, 1);
    ASSERT_SUCCESS(obj["a"].get(num));
    ASSERT_EQUAL(num, 0);
    assert_error(obj["d"].get(num), NO_SUCH_FIELD);
    ASSERT_SUCCESS(obj["a"].get(num));
    ASSERT_EQUAL(num, 0);
    TEST_SUCCEED();
  }

  bool missing_keys() {
    TEST_START();
    simdjson::ondemand::parser parser;
    simdjson::padded_string docdata =  R"([{"a":"a"},{}])"_padded;
    simdjson::ondemand::document doc;
    auto error = parser.iterate(docdata).get(doc);
    if(error != simdjson::SUCCESS) { return false; }
    simdjson::ondemand::array a;
    error = doc.get_array().get(a);
    if(error != simdjson::SUCCESS) { return false; }
    for(auto elem : a) {
      error = elem.find_field_unordered("keynotfound").error();
      if(error != simdjson::NO_SUCH_FIELD) {
        std::cout << error << std::endl;
        return false;
      }
    }
    return true;
  }

  bool missing_keys_for_empty_top_level_object() {
    TEST_START();
    simdjson::ondemand::parser parser;
    simdjson::padded_string docdata = "{}"_padded;
    simdjson::ondemand::document doc;
    auto error = parser.iterate(docdata).get(doc);
    if(error != simdjson::SUCCESS) { return false; }
    error = doc.find_field_unordered("keynotfound").error();
    if(error != simdjson::NO_SUCH_FIELD) {
      std::cout << error << std::endl;
      return false;
    }
    return true;
  }

#if SIMDJSON_EXCEPTIONS
  // used in issue_1521
  // difficult to use as a lambda because it is recursive.
  void broken_descend(ondemand::object node) {
    if(auto type = node.find_field_unordered("type"); type.error() == SUCCESS && type == "child") {
      auto n = node.find_field_unordered("name");
      if(n.error() == simdjson::SUCCESS) {
          std::cout << std::string_view(n) << std::endl;
      }
    } else {
     for (ondemand::object child_node : node["nodes"]) { broken_descend(child_node); }
    }
  }

  bool broken_issue_1521() {
    TEST_START();
    ondemand::parser parser;
    padded_string json = R"({"type":"root","nodes":[{"type":"child","nodes":[]},{"type":"child","name":"child-name","nodes":[]}]})"_padded;
    ondemand::document file_tree = parser.iterate(json);
    try {
      broken_descend(file_tree);
    } catch(simdjson::simdjson_error& e) {
      std::cout << "The document is valid JSON: " << json << std::endl;
      TEST_FAIL(e.error());
    }
    TEST_SUCCEED();
  }

  bool fixed_broken_issue_1521() {
    TEST_START();
    ondemand::parser parser;
    // We omit the ',"nodes":[]'
    padded_string json = R"({"type":"root","nodes":[{"type":"child"},{"type":"child","name":"child-name","nodes":[]}]})"_padded;
    ondemand::document file_tree = parser.iterate(json);
    try {
      broken_descend(file_tree);
    } catch(simdjson::simdjson_error& e) {
      std::cout << "The document is valid JSON: " << json << std::endl;
      TEST_FAIL(e.error());
    }
    TEST_SUCCEED();
  }

  // used in issue_1521
  // difficult to use as a lambda because it is recursive.
  void descend(ondemand::object node) {
    auto n = node.find_field_unordered("name");
    if(auto type = node.find_field_unordered("type"); type.error() == SUCCESS && type == "child") {
      if(n.error() == simdjson::SUCCESS) {
          std::cout << std::string_view(n) << std::endl;
      }
    } else {
     for (ondemand::object child_node : node["nodes"]) { descend(child_node); }
    }
  }

  bool issue_1521() {
    TEST_START();
    ondemand::parser parser;
    padded_string json = R"({"type":"root","nodes":[{"type":"child","nodes":[]},{"type":"child","name":"child-name","nodes":[]}]})"_padded;
    ondemand::document file_tree = parser.iterate(json);
    try {
      descend(file_tree);
    } catch(simdjson::simdjson_error& e) {
      std::cout << "The document is valid JSON: " << json << std::endl;
      TEST_FAIL(e.error());
    }
    TEST_SUCCEED();
  }
#endif

  bool iterate_object() {
    TEST_START();
    auto json = R"({ "a": 1, "b": 2, "c": 3 })"_padded;
    const char* expected_key[] = { "a", "b", "c" };
    const uint64_t expected_value[] = { 1, 2, 3 };
    SUBTEST("ondemand::object", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::object object;
      ASSERT_RESULT( doc_result.type(), json_type::object );
      ASSERT_SUCCESS( doc_result.get(object) );
      size_t i = 0;
      for (auto [ field, error ] : object) {
        ASSERT_SUCCESS(error);
        ASSERT_EQUAL( field.key(), expected_key[i]);
        ASSERT_EQUAL( field.value().get_uint64().value_unsafe(), expected_value[i] );
        i++;
      }
      ASSERT_EQUAL( i*sizeof(uint64_t), sizeof(expected_value) );
      return true;
    }));
    SUBTEST("ondemand::object-document-rewind", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::object object;
      ASSERT_RESULT( doc_result.type(), json_type::object );
      ASSERT_SUCCESS( doc_result.get(object) );
      size_t i = 0;
      for (auto [ field, error ] : object) {
        ASSERT_SUCCESS(error);
        ASSERT_EQUAL( field.key(), expected_key[i]);
        ASSERT_EQUAL( field.value().get_uint64().value_unsafe(), expected_value[i] );
        i++;
      }
      ASSERT_EQUAL( i*sizeof(uint64_t), sizeof(expected_value) );
      doc_result.rewind();
      ASSERT_RESULT( doc_result.type(), json_type::object );
      ASSERT_SUCCESS( doc_result.get(object) );
      i = 0;
      for (auto [ field, error ] : object) {
        ASSERT_SUCCESS(error);
        ASSERT_EQUAL( field.key(), expected_key[i]);
        ASSERT_EQUAL( field.value().get_uint64().value_unsafe(), expected_value[i] );
        i++;
      }
      ASSERT_EQUAL( i*sizeof(uint64_t), sizeof(expected_value) );
      return true;
    }));
    SUBTEST("simdjson_result<ondemand::object>", test_ondemand_doc(json, [&](auto doc_result) {
      simdjson_result<ondemand::object> object_result = doc_result.get_object();
      size_t i = 0;
      for (auto [ field, error ] : object_result) {
        ASSERT_SUCCESS(error);
        ASSERT_EQUAL( field.key(), expected_key[i] );
        ASSERT_EQUAL( field.value().get_uint64().value_unsafe(), expected_value[i] );
        i++;
      }
      ASSERT_EQUAL( i*sizeof(uint64_t), sizeof(expected_value) );
      return true;
    }));
    TEST_SUCCEED();
  }

  bool iterate_object_partial_children() {
    TEST_START();
    auto json = R"(
      {
        "scalar_ignore":       0,
        "empty_array_ignore":  [],
        "empty_object_ignore": {},
        "object_break":        { "x": 3, "y": 33 },
        "object_break_unused": { "x": 4, "y": 44 },
        "object_index":        { "x": 5, "y": 55 },
        "object_index_unused": { "x": 6, "y": 66 },
        "array_break":         [ 7, 77, 777 ],
        "array_break_unused":  [ 8, 88, 888 ],
        "quadruple_nested_break": { "a": [ { "b": [ 9, 99 ], "c": 999 }, 9999 ], "d": 99999 },
        "actual_value": 10
      }
    )"_padded;
    SUBTEST("ondemand::object", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::object object;
      ASSERT_SUCCESS( doc_result.get(object) );
      size_t i = 0;
      for (auto field : object) {
        ondemand::raw_json_string key;
        ASSERT_SUCCESS( field.key().get(key) );

        switch (i) {
          case 0: {
            ASSERT_EQUAL(key, "scalar_ignore");
            std::cout << "  - After ignoring empty scalar ..." << std::endl;
            break;
          }
          case 1: {
            ASSERT_EQUAL(key, "empty_array_ignore");
            std::cout << "  - After ignoring empty array ..." << std::endl;
            break;
          }
          case 2: {
            ASSERT_EQUAL(key, "empty_object_ignore");
            std::cout << "  - After ignoring empty object ..." << std::endl;
            break;
          }
          // Break after using first value in child object
          case 3: {
            ASSERT_EQUAL(key, "object_break");

            for (auto [ child_field, error ] : field.value().get_object()) {
              ASSERT_SUCCESS(error);
              ASSERT_EQUAL(child_field.key(), "x");
              uint64_t x;
              ASSERT_SUCCESS( child_field.value().get(x) );
              ASSERT_EQUAL(x, 3);
              break; // Break after the first value
            }
            std::cout << "  - After using first value in child object ..." << std::endl;
            break;
          }

          // Break without using first value in child object
          case 4: {
            ASSERT_EQUAL(key, "object_break_unused");

            for (auto [ child_field, error ] : field.value().get_object()) {
              ASSERT_SUCCESS(error);
              ASSERT_EQUAL(child_field.key(), "x");
              break;
            }
            std::cout << "  - After reaching (but not using) first value in child object ..." << std::endl;
            break;
          }

          // Only look up one field in child object
          case 5: {
            ASSERT_EQUAL(key, "object_index");

            uint64_t x;
            ASSERT_SUCCESS( field.value()["x"].get(x) );
            ASSERT_EQUAL( x, 5 );
            std::cout << "  - After looking up one field in child object ..." << std::endl;
            break;
          }

          // Only look up one field in child object, but don't use it
          case 6: {
            ASSERT_EQUAL(key, "object_index_unused");

            ASSERT_SUCCESS( field.value()["x"] );
            std::cout << "  - After looking up (but not using) one field in child object ..." << std::endl;
            break;
          }

          // Break after first value in child array
          case 7: {
            ASSERT_EQUAL(key, "array_break");
            for (auto child_value : field.value()) {
              uint64_t x;
              ASSERT_SUCCESS( child_value.get(x) );
              ASSERT_EQUAL( x, 7 );
              break;
            }
            std::cout << "  - After using first value in child array ..." << std::endl;
            break;
          }

          // Break without using first value in child array
          case 8: {
            ASSERT_EQUAL(key, "array_break_unused");
            for (auto child_value : field.value()) {
              ASSERT_SUCCESS(child_value);
              break;
            }
            std::cout << "  - After reaching (but not using) first value in child array ..." << std::endl;
            break;
          }

          // Break out of multiple child loops
          case 9: {
            ASSERT_EQUAL(key, "quadruple_nested_break");
            for (auto child1 : field.value().get_object()) {
              for (auto child2 : child1.value().get_array()) {
                for (auto child3 : child2.get_object()) {
                  for (auto child4 : child3.value().get_array()) {
                    uint64_t x;
                    ASSERT_SUCCESS( child4.get(x) );
                    ASSERT_EQUAL( x, 9 );
                    break;
                  }
                  break;
                }
                break;
              }
              break;
            }
            std::cout << "  - After breaking out of quadruply-nested arrays and objects ..." << std::endl;
            break;
          }

          // Test the actual value
          case 10: {
            ASSERT_EQUAL(key, "actual_value");
            uint64_t actual_value;
            ASSERT_SUCCESS( field.value().get(actual_value) );
            ASSERT_EQUAL( actual_value, 10 );
            break;
          }
        }

        i++;
      }
      ASSERT_EQUAL( i, 11 ); // Make sure we found all the keys we expected
      return true;
    }));
    return true;
  }

  bool iterate_empty_object() {
    TEST_START();
    auto json = R"({})"_padded;

    SUBTEST("ondemand::object", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::object object;
      ASSERT_SUCCESS( doc_result.get(object) );
      for (simdjson_unused auto field : object) {
        TEST_FAIL("Unexpected field");
      }
      return true;
    }));
    SUBTEST("simdjson_result<ondemand::object>", test_ondemand_doc(json, [&](auto doc_result) {
      simdjson_result<ondemand::object> object_result = doc_result.get_object();
      for (simdjson_unused auto field : object_result) {
        TEST_FAIL("Unexpected field");
      }
      return true;
    }));
    TEST_SUCCEED();
  }

  bool value_search_unescaped_key() {
    TEST_START();
    auto json = R"({"k\u0065y": 1})"_padded;
    SUBTEST("ondemand::unescapedkey", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::object object;
      bool got_key = false;
      ASSERT_SUCCESS( doc_result.get(object) );
      for (auto field : object) {
        std::string_view keyv;
        ASSERT_SUCCESS( field.unescaped_key().get(keyv) );
        if(keyv == "key") {
          int64_t value;
          ASSERT_SUCCESS( field.value().get(value) );
          ASSERT_EQUAL( value, 1);
          got_key = true;
        }
      }
      return got_key;
    }));
    SUBTEST("ondemand::rawkey", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::object object;
      ASSERT_SUCCESS( doc_result.get(object) );
      bool got_key = false;
      for (auto field : object) {
        ondemand::raw_json_string keyv;
        ASSERT_SUCCESS( field.key().get(keyv) );
        if(keyv == R"(k\u0065y)") {
          int64_t value;
          ASSERT_SUCCESS( field.value().get(value) );
          ASSERT_EQUAL( value, 1);
          got_key = true;
        }
      }
      return got_key;
    }));
    TEST_SUCCEED();
  }

  bool issue_1480() {
    TEST_START();
    auto json = R"({ "name"  : "something", "version": "0.13.2", "version_major": 0})"_padded;

    SUBTEST("ondemand::issue_1480::object", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::object object;
      ASSERT_SUCCESS( doc_result.get(object) );
      ASSERT_EQUAL( object.find_field_unordered("version_major").get_uint64().value_unsafe(), 0 );
      ASSERT_EQUAL( object.find_field_unordered("version").get_string().value_unsafe(),  "0.13.2");
      return true;
    }));
    SUBTEST("ondemand::issue_1480::big-key", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::object object;
      ASSERT_SUCCESS( doc_result.get(object) );
      std::vector<char> large_buf(512,' ');
      std::string_view lb{large_buf.data(), large_buf.size()};
      ASSERT_ERROR( object.find_field(lb), NO_SUCH_FIELD );
      return true;
    }));

#if SIMDJSON_EXCEPTIONS
    SUBTEST("ondemand::issue_1480::object-iteration", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::object object;
      ASSERT_SUCCESS( doc_result.get(object) );
      size_t counter_version{0};
      size_t counter_version_major{0};
      for(auto field : object)  {
        std::string_view keyv = field.unescaped_key();
        if(keyv == "version") { counter_version++; }
        if(keyv == "version_major") { counter_version_major++; }
      }
      ASSERT_EQUAL( counter_version, 1 );
      ASSERT_EQUAL( counter_version_major, 1 );
      return true;
    }));
    SUBTEST("ondemand::issue_1480::object-iteration-string-view", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::object object;
      ASSERT_SUCCESS( doc_result.get(object) );
      size_t counter_version{0};
      size_t counter_version_major{0};
      const char * bufv = "version";
      const char * bufvm = "version_major";

      std::string_view v{bufv};
      std::string_view vm{bufvm};

      for(auto field : object)  {
        std::string_view keyv = field.unescaped_key();
        if(keyv == v) { counter_version++; }
        if(keyv == vm) { counter_version_major++; }
      }
      ASSERT_EQUAL( counter_version, 1 );
      ASSERT_EQUAL( counter_version_major, 1 );
      return true;
    }));
    SUBTEST("ondemand::issue_1480::original", test_ondemand_doc(json, [&](auto doc_result) {
      size_t counter{0};
      for (auto field : doc_result.get_object()) {
        auto key = field.key().value();
        if (key == "version") { counter++; }
      }
      ASSERT_EQUAL( counter, 1 );
      return true;
    }));
    SUBTEST("ondemand::issue_1480::object-operator", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::object object;
      ASSERT_SUCCESS( doc_result.get(object) );
      ASSERT_EQUAL( std::string_view(object["version"]), "0.13.2" );
      ASSERT_EQUAL( uint64_t(object["version_major"]), 0 );
      return true;
    }));
#endif
    TEST_SUCCEED();
  }

#if SIMDJSON_EXCEPTIONS

  bool iterate_object_exception() {
    TEST_START();
    auto json = R"({ "a": 1, "b": 2, "c": 3 })"_padded;
    const char* expected_key[] = { "a", "b", "c" };
    const uint64_t expected_value[] = { 1, 2, 3 };
    ASSERT_TRUE(test_ondemand_doc(json, [&](auto doc_result) {
      size_t i = 0;
      for (ondemand::field field : doc_result.get_object()) {
        ASSERT_EQUAL( field.key(), expected_key[i] );
        ASSERT_EQUAL( uint64_t(field.value()), expected_value[i] );
        i++;
      }
      ASSERT_EQUAL( i*sizeof(uint64_t), sizeof(expected_value) );
      return true;
    }));
    TEST_SUCCEED();
  }

  bool iterate_empty_object_exception() {
    TEST_START();
    auto json = R"({})"_padded;

    ASSERT_TRUE(test_ondemand_doc(json, [&](auto doc_result) {
      for (simdjson_unused ondemand::field field : doc_result.get_object()) {
        TEST_FAIL("Unexpected field");
      }
      return true;
    }));
    SUBTEST("ondemand::object-document-rewind", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::object object;
      ASSERT_RESULT( doc_result.type(), json_type::object );
      ASSERT_SUCCESS( doc_result.get(object) );
      size_t i = 0;
      for (auto [ field, error ] : object) {
        (void)field;
        (void)error;
        i++;
      }
      ASSERT_EQUAL( i, 0 );
      doc_result.rewind();
      i = 0;
      for (auto [ field, error ] : object) {
        (void)field;
        (void)error;
        i++;
      }
      ASSERT_EQUAL( i, 0 );
      return true;
    }));

    TEST_SUCCEED();
  }

#endif // SIMDJSON_EXCEPTIONS

  bool run() {
    return
           value_search_unescaped_key() &&
           missing_key_continue() &&
           no_missing_keys() &&
           missing_keys() &&
           missing_keys_for_empty_top_level_object() &&
#if SIMDJSON_EXCEPTIONS
           fixed_broken_issue_1521() &&
           issue_1521() &&
           broken_issue_1521() &&
#endif
           iterate_object() &&
           iterate_empty_object() &&
           iterate_object_partial_children() &&
           issue_1480() &&
#if SIMDJSON_EXCEPTIONS
           iterate_object_exception() &&
#endif // SIMDJSON_EXCEPTIONS
           true;
  }

} // namespace object_tests

int main(int argc, char *argv[]) {
  return test_main(argc, argv, object_tests::run);
}
