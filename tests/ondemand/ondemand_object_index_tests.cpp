#include "simdjson.h"
#include "test_ondemand.h"

using namespace simdjson;

namespace object_tests {
  using namespace std;
  using simdjson::ondemand::json_type;

  bool object_index() {
    TEST_START();
    auto json = R"({ "a": 1, "b": 2, "c/d": 3})"_padded;
    SUBTEST("ondemand::object", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::object object;
      ASSERT_SUCCESS( doc_result.get(object) );

      ASSERT_EQUAL( object["a"].get_uint64().value_unsafe(), 1 );
      ASSERT_EQUAL( object["b"].get_uint64().value_unsafe(), 2 );
      ASSERT_EQUAL( object["c/d"].get_uint64().value_unsafe(), 3 );

      ASSERT_EQUAL( object["a"].get_uint64().value_unsafe(), 1 );
      ASSERT_ERROR( object["d"], NO_SUCH_FIELD );
      return true;
    }));
    SUBTEST("simdjson_result<ondemand::object>", test_ondemand_doc(json, [&](auto doc_result) {
      simdjson_result<ondemand::object> object;
      object = doc_result.get_object();

      ASSERT_EQUAL( object["a"].get_uint64().value_unsafe(), 1 );
      ASSERT_EQUAL( object["b"].get_uint64().value_unsafe(), 2 );
      ASSERT_EQUAL( object["c/d"].get_uint64().value_unsafe(), 3 );

      ASSERT_EQUAL( object["a"].get_uint64().value_unsafe(), 1 );
      ASSERT_ERROR( object["d"], NO_SUCH_FIELD );
      return true;
    }));
    TEST_SUCCEED();
  }

  bool document_object_index() {
    TEST_START();
    auto json = R"({ "a": 1, "b": 2, "c/d": 3})"_padded;
    SUBTEST("ondemand::document", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::document doc;
      ASSERT_SUCCESS( std::move(doc_result).get(doc) );
      ASSERT_EQUAL( doc["a"].get_uint64().value_unsafe(), 1 );
      ASSERT_EQUAL( doc["b"].get_uint64().value_unsafe(), 2 );
      ASSERT_EQUAL( doc["c/d"].get_uint64().value_unsafe(), 3 );

      ASSERT_EQUAL( doc["a"].get_uint64().value_unsafe(), 1 );
      ASSERT_ERROR( doc["d"], NO_SUCH_FIELD );
      return true;
    }));
    SUBTEST("simdjson_result<ondemand::document>", test_ondemand_doc(json, [&](auto doc_result) {
      ASSERT_EQUAL( doc_result["a"].get_uint64().value_unsafe(), 1 );
      ASSERT_EQUAL( doc_result["b"].get_uint64().value_unsafe(), 2 );
      ASSERT_EQUAL( doc_result["c/d"].get_uint64().value_unsafe(), 3 );

      ASSERT_EQUAL( doc_result["a"].get_uint64().value_unsafe(), 1 );
      ASSERT_ERROR( doc_result["d"], NO_SUCH_FIELD );
      return true;
    }));
    TEST_SUCCEED();
  }

  bool value_object_index() {
    TEST_START();
    auto json = R"({ "outer": { "a": 1, "b": 2, "c/d": 3 } })"_padded;
    SUBTEST("ondemand::value", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::value object;
      ASSERT_SUCCESS( doc_result["outer"].get(object) );
      ASSERT_EQUAL( object["a"].get_uint64().value_unsafe(), 1 );
      ASSERT_EQUAL( object["b"].get_uint64().value_unsafe(), 2 );
      ASSERT_EQUAL( object["c/d"].get_uint64().value_unsafe(), 3 );

      ASSERT_EQUAL( object["a"].get_uint64().value_unsafe(), 1 );
      ASSERT_ERROR( object["d"], NO_SUCH_FIELD );
      return true;
    }));
    SUBTEST("simdjson_result<ondemand::value>", test_ondemand_doc(json, [&](auto doc_result) {
      simdjson_result<ondemand::value> object = doc_result["outer"];
      ASSERT_EQUAL( object["a"].get_uint64().value_unsafe(), 1 );
      ASSERT_EQUAL( object["b"].get_uint64().value_unsafe(), 2 );
      ASSERT_EQUAL( object["c/d"].get_uint64().value_unsafe(), 3 );

      ASSERT_EQUAL( object["a"].get_uint64().value_unsafe(), 1 );
      ASSERT_ERROR( object["d"], NO_SUCH_FIELD );
      return true;
    }));
    TEST_SUCCEED();
  }

  bool document_nested_object_index() {
    TEST_START();
    auto json = R"({ "x": { "y": { "z": 2 } } }})"_padded;
    SUBTEST("simdjson_result<ondemand::document>", test_ondemand_doc(json, [&](auto doc_result) {
      ASSERT_EQUAL( doc_result["x"]["y"]["z"].get_uint64().value_unsafe(), 2 );
      return true;
    }));
    SUBTEST("ondemand::document", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::document doc;
      ASSERT_SUCCESS( std::move(doc_result).get(doc) );
      ASSERT_EQUAL( doc["x"]["y"]["z"].get_uint64().value_unsafe(), 2 );
      return true;
    }));
    TEST_SUCCEED();
  }

  bool nested_object_index() {
    TEST_START();
    auto json = R"({ "x": { "y": { "z": 2 } } }})"_padded;
    SUBTEST("simdjson_result<ondemand::object>", test_ondemand_doc(json, [&](auto doc_result) {
      simdjson_result<ondemand::object> object = doc_result.get_object();
      ASSERT_EQUAL( object["x"]["y"]["z"].get_uint64().value_unsafe(), 2 );
      return true;
    }));
    SUBTEST("ondemand::object", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::object object;
      ASSERT_SUCCESS( doc_result.get(object) );
      ASSERT_EQUAL( object["x"]["y"]["z"].get_uint64().value_unsafe(), 2 );
      return true;
    }));
    TEST_SUCCEED();
  }

  bool value_nested_object_index() {
    TEST_START();
    auto json = R"({ "x": { "y": { "z": 2 } } }})"_padded;
    SUBTEST("simdjson_result<ondemand::value>", test_ondemand_doc(json, [&](auto doc_result) {
      simdjson_result<ondemand::value> x = doc_result["x"];
      ASSERT_EQUAL( x["y"]["z"].get_uint64().value_unsafe(), 2 );
      return true;
    }));
    SUBTEST("ondemand::value", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::value x;
      ASSERT_SUCCESS( doc_result["x"].get(x) );
      ASSERT_EQUAL( x["y"]["z"].get_uint64().value_unsafe(), 2 );
      return true;
    }));
    TEST_SUCCEED();
  }

  bool object_index_partial_children() {
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

      ASSERT_SUCCESS( object["scalar_ignore"] );
      std::cout << "  - After ignoring empty scalar ..." << std::endl;

      ASSERT_SUCCESS( object["empty_array_ignore"] );
      std::cout << "  - After ignoring empty array ..." << std::endl;

      ASSERT_SUCCESS( object["empty_object_ignore"] );
      std::cout << "  - After ignoring empty object ..." << std::endl;

      // Break after using first value in child object
      {
        auto value = object["object_break"];
        for (auto [ child_field, error ] : value.get_object()) {
          ASSERT_SUCCESS(error);
          ASSERT_EQUAL(child_field.key(), "x");
          uint64_t x;
          ASSERT_SUCCESS( child_field.value().get(x) );
          ASSERT_EQUAL(x, 3);
          break; // Break after the first value
        }
        std::cout << "  - After using first value in child object ..." << std::endl;
      }

      // Break without using first value in child object
      {
        auto value = object["object_break_unused"];
        for (auto [ child_field, error ] : value.get_object()) {
          ASSERT_SUCCESS(error);
          ASSERT_EQUAL(child_field.key(), "x");
          break;
        }
        std::cout << "  - After reaching (but not using) first value in child object ..." << std::endl;
      }

      // Only look up one field in child object
      {
        auto value = object["object_index"];

        uint64_t x;
        ASSERT_SUCCESS( value["x"].get(x) );
        ASSERT_EQUAL( x, 5 );
        std::cout << "  - After looking up one field in child object ..." << std::endl;
      }

      // Only look up one field in child object, but don't use it
      {
        auto value = object["object_index_unused"];

        ASSERT_SUCCESS( value["x"] );
        std::cout << "  - After looking up (but not using) one field in child object ..." << std::endl;
      }

      // Break after first value in child array
      {
        auto value = object["array_break"];

        for (auto child_value : value) {
          uint64_t x;
          ASSERT_SUCCESS( child_value.get(x) );
          ASSERT_EQUAL( x, 7 );
          break;
        }
        std::cout << "  - After using first value in child array ..." << std::endl;
      }

      // Break without using first value in child array
      {
        auto value = object["array_break_unused"];

        for (auto child_value : value) {
          ASSERT_SUCCESS(child_value);
          break;
        }
        std::cout << "  - After reaching (but not using) first value in child array ..." << std::endl;
      }

      // Break out of multiple child loops
      {
        auto value = object["quadruple_nested_break"];
        for (auto child1 : value.get_object()) {
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
      }

      // Test the actual value
      {
        auto value = object["actual_value"];
        uint64_t actual_value;
        ASSERT_SUCCESS( value.get(actual_value) );
        ASSERT_EQUAL( actual_value, 10 );
      }

      return true;
    }));

    SUBTEST("simdjson_result<ondemand::document>", test_ondemand_doc(json, [&](auto doc_result) {
      ASSERT_SUCCESS( doc_result["scalar_ignore"] );
      std::cout << "  - After ignoring empty scalar ..." << std::endl;

      ASSERT_SUCCESS( doc_result["empty_array_ignore"] );
      std::cout << "  - After ignoring empty array ..." << std::endl;

      ASSERT_SUCCESS( doc_result["empty_object_ignore"] );
      std::cout << "  - After ignoring empty doc_result ..." << std::endl;

      // Break after using first value in child object
      {
        auto value = doc_result["object_break"];
        for (auto [ child_field, error ] : value.get_object()) {
          ASSERT_SUCCESS(error);
          ASSERT_EQUAL(child_field.key(), "x");
          uint64_t x;
          ASSERT_SUCCESS( child_field.value().get(x) );
          ASSERT_EQUAL(x, 3);
          break; // Break after the first value
        }
        std::cout << "  - After using first value in child object ..." << std::endl;
      }

      // Break without using first value in child object
      {
        auto value = doc_result["object_break_unused"];
        for (auto [ child_field, error ] : value.get_object()) {
          ASSERT_SUCCESS(error);
          ASSERT_EQUAL(child_field.key(), "x");
          break;
        }
        std::cout << "  - After reaching (but not using) first value in child object ..." << std::endl;
      }

      // Only look up one field in child object
      {
        auto value = doc_result["object_index"];

        uint64_t x;
        ASSERT_SUCCESS( value["x"].get(x) );
        ASSERT_EQUAL( x, 5 );
        std::cout << "  - After looking up one field in child object ..." << std::endl;
      }

      // Only look up one field in child object, but don't use it
      {
        auto value = doc_result["object_index_unused"];

        ASSERT_SUCCESS( value["x"] );
        std::cout << "  - After looking up (but not using) one field in child object ..." << std::endl;
      }

      // Break after first value in child array
      {
        auto value = doc_result["array_break"];

        for (auto child_value : value) {
          uint64_t x;
          ASSERT_SUCCESS( child_value.get(x) );
          ASSERT_EQUAL( x, 7 );
          break;
        }
        std::cout << "  - After using first value in child array ..." << std::endl;
      }

      // Break without using first value in child array
      {
        auto value = doc_result["array_break_unused"];

        for (auto child_value : value) {
          ASSERT_SUCCESS(child_value);
          break;
        }
        std::cout << "  - After reaching (but not using) first value in child array ..." << std::endl;
      }

      // Break out of multiple child loops
      {
        auto value = doc_result["quadruple_nested_break"];
        for (auto child1 : value.get_object()) {
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
      }

      // Test the actual value
      {
        auto value = doc_result["actual_value"];
        uint64_t actual_value;
        ASSERT_SUCCESS( value.get(actual_value) );
        ASSERT_EQUAL( actual_value, 10 );
      }

      return true;
    }));

    return true;
  }

#if SIMDJSON_EXCEPTIONS

  bool object_index_exception() {
    TEST_START();
    auto json = R"({ "a": 1, "b": 2, "c/d": 3})"_padded;
    SUBTEST("ondemand::object", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::object object = doc_result;

      ASSERT_EQUAL( uint64_t(object["a"]), 1 );
      ASSERT_EQUAL( uint64_t(object["b"]), 2 );
      ASSERT_EQUAL( uint64_t(object["c/d"]), 3 );

      return true;
    }));
    TEST_SUCCEED();
  }
  bool nested_object_index_exception() {
    TEST_START();
    auto json = R"({ "x": { "y": { "z": 2 } } }})"_padded;
    SUBTEST("simdjson_result<ondemand::document>", test_ondemand_doc(json, [&](auto doc_result) {
      ASSERT_EQUAL( uint64_t(doc_result["x"]["y"]["z"]), 2 );
      return true;
    }));
    TEST_SUCCEED();
  }

#endif // SIMDJSON_EXCEPTIONS

  bool run() {
    return
           object_index() &&
           document_object_index() &&
           value_object_index() &&
           nested_object_index() &&
           document_nested_object_index() &&
           value_nested_object_index() &&
           object_index_partial_children() &&
#if SIMDJSON_EXCEPTIONS
           object_index_exception() &&
           nested_object_index_exception() &&
#endif // SIMDJSON_EXCEPTIONS
           true;
  }

} // namespace object_tests

int main(int argc, char *argv[]) {
  return test_main(argc, argv, object_tests::run);
}
