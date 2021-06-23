#include "simdjson.h"
#include "test_ondemand.h"

using namespace simdjson;

namespace array_tests {
  using namespace std;
  using simdjson::ondemand::json_type;
  bool issue1588() {
    TEST_START();
    const auto json = std::string_view(R"({
    "nodes" : [
        {
            "rotation" : [
                0.16907575726509094,
                0.7558803558349609,
                -0.27217137813568115,
                0.570947527885437
            ],
            "translation" : [
                4.076245307922363,
                5.903861999511719,
                -1.0054539442062378
            ]
        },
        {
            "camera" : 0,
            "rotation" : [
                -0.7071067690849304,
                0,
                0,
                0.7071067690849304
            ]
        },
        {
            "children" : [
                1
            ],
            "translation" : [
                7.358891487121582,
                4.958309173583984,
                6.925790786743164
            ]
        },
        {
            "mesh" : 1,
            "scale" : [
                4.7498908042907715,
                4.7498908042907715,
                4.7498908042907715
            ]
        }
    ]
})");
    // we query 'rotation', 'scale', 'translation' in sequence
    const bool expected_value[][3] = { {true, false, true},
      {true, false, false}, {false, false, true}, {false, true, false} };

    SUBTEST("ondemand::issue1588::sequence", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::array array;
      ASSERT_SUCCESS( doc_result["nodes"].get(array) );

      size_t i=0;
      for (auto value : array) {
        ondemand::object current_object;
        ASSERT_SUCCESS( value.get_object().get(current_object) );
        std::cout << "[ondemand::issue1588::sequence] acquired a new object ==========" << std::endl;

        simdjson::ondemand::array rotation;
        if(expected_value[i][0]) {
          ASSERT_SUCCESS( current_object["rotation"].get(rotation) );
          std::cout << "[ondemand::issue1588::sequence] found 'rotation' " << std::endl;
        } else {
          ASSERT_ERROR( current_object["rotation"].get(rotation), NO_SUCH_FIELD );
          std::cout << "[ondemand::issue1588::sequence] rotation not found" << std::endl;
        }
        simdjson::ondemand::array scale;
        if(expected_value[i][1]) {
          ASSERT_SUCCESS( current_object["scale"].get(scale) );
          std::cout << "[ondemand::issue1588::sequence] found 'scale' " << std::endl;
        } else {
          ASSERT_ERROR( current_object["scale"].get(scale), NO_SUCH_FIELD );
          std::cout << "[ondemand::issue1588::sequence] scale not found" << std::endl;
        }
        simdjson::ondemand::array translation;
        if(expected_value[i][2]) {
          ASSERT_SUCCESS( current_object["translation"].get(translation) );
          std::cout << "[ondemand::issue1588::sequence] found 'translation' " << std::endl;
        } else {
          ASSERT_ERROR( current_object["translation"].get(translation), NO_SUCH_FIELD );
          std::cout << "[ondemand::issue1588::sequence] translation not found" << std::endl;
        }
        i++;
      }
      ASSERT_EQUAL(i, 4);
      return true;
    }));
    SUBTEST("ondemand::issue1588::originalcode", test_ondemand_doc(json, [&](auto doc_result) {
      int count_nodes = 0;
      simdjson::ondemand::array doc_nodes;
      auto error = doc_result["nodes"].get(doc_nodes);
      ASSERT_SUCCESS( error );
      for (auto node_iterator : doc_nodes) {
        ondemand::object node_obj;
        ASSERT_SUCCESS( node_iterator.get_object().get(node_obj) );
        simdjson::ondemand::array rotation;
        std::cout << "checking rotation" << std::endl;
        if(expected_value[count_nodes][0]) {
          ASSERT_SUCCESS( node_obj["rotation"].get(rotation) );
        } else {
          ASSERT_ERROR( node_obj["rotation"].get(rotation), NO_SUCH_FIELD );
        }

        simdjson::ondemand::array scale;
        std::cout << "checking scale" << std::endl;
        if(expected_value[count_nodes][1]) {
          ASSERT_SUCCESS( node_obj["scale"].get(scale) );
        } else {
          ASSERT_ERROR( node_obj["scale"].get(scale), NO_SUCH_FIELD );
        }

        simdjson::ondemand::array translation;
        std::cout << "checking translation" << std::endl;
        if (!error) { std::cout << "translation!\n"; }
         if(expected_value[count_nodes][2]) {
          ASSERT_SUCCESS( node_obj["translation"].get(translation) );
        } else {
          ASSERT_ERROR( node_obj["translation"].get(translation), NO_SUCH_FIELD );
        }

        ++count_nodes;
      }
      ASSERT_EQUAL(count_nodes, 4);
      return true;
    }));
    TEST_SUCCEED();
  }

  bool iterate_complex_array_count() {
    TEST_START();
    ondemand::parser parser;
    auto cars_json = R"( { "test":[ { "val1":1, "val2":2 }, { "val1":1, "val2":2 } ] }   )"_padded;
    ondemand::document doc;
    ASSERT_SUCCESS(parser.iterate(cars_json).get(doc));
    ondemand::array myarray;
    ASSERT_SUCCESS(doc.find_field("test").get_array().get(myarray));
    size_t count;
    ASSERT_SUCCESS(myarray.count_elements().get(count));
    size_t new_count = 0;
    for(simdjson_unused auto elem: myarray) { new_count++; }
    ASSERT_EQUAL(count, new_count);
    TEST_SUCCEED();
  }

  bool iterate_sub_array_count() {
    TEST_START();
    ondemand::parser parser;
    auto key_value_json = R"( { "test":[ 1,2,3], "joe": [1,2] }   )"_padded;
    ondemand::document doc;
    ASSERT_SUCCESS(parser.iterate(key_value_json).get(doc));
    ondemand::object obj;
    ASSERT_SUCCESS(doc.get_object().get(obj));
    ondemand::value v;
    ASSERT_SUCCESS(doc.find_field("test").get(v));
    size_t count;
    ASSERT_SUCCESS(v.count_elements().get(count));
    ASSERT_EQUAL(count, 3);
    ASSERT_SUCCESS(doc.find_field("joe").get(v));
    ASSERT_SUCCESS(v.count_elements().get(count));
    ASSERT_EQUAL(count, 2);
    TEST_SUCCEED();
  }

  bool iterate_array_count() {
    TEST_START();
    const auto json = R"([ 1, 10, 100 ])"_padded;
    const auto badjson = R"([ 1, 10 100 ])"_padded;
    const vector<uint64_t> expected_value = { 1, 10, 100 };

    SUBTEST("ondemand::count_elements", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::array array;
      ASSERT_RESULT( doc_result.type(), json_type::array );
      ASSERT_SUCCESS( doc_result.get(array) );
      size_t count;
      ASSERT_SUCCESS( array.count_elements().get(count) );
      ASSERT_EQUAL(count, expected_value.size());
      return true;
    }));
    SUBTEST("ondemand::count_elements_and_decode", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::array array;
      ASSERT_RESULT( doc_result.type(), json_type::array );
      ASSERT_SUCCESS( doc_result.get(array) );
      size_t count;
      ASSERT_SUCCESS( array.count_elements().get(count) );
      ASSERT_EQUAL(count, expected_value.size());
      size_t i = 0;
      std::vector<uint64_t> receiver(count);
      for (auto value : array) {
        uint64_t actual;
        ASSERT_SUCCESS( value.get(actual) );
        ASSERT_EQUAL(actual, expected_value[i]);
        receiver[i] = actual;
        i++;
      }
      return true;
    }));
    TEST_SUCCEED();
  }

  bool iterate_bad_array_count() {
    TEST_START();
    const auto badjson = R"([ 1, 10 100 ])"_padded;


    SUBTEST("ondemand::count_elements", test_ondemand_doc(badjson, [&](auto doc_result) {
      ondemand::array array;
      ASSERT_RESULT( doc_result.type(), json_type::array );
      ASSERT_SUCCESS( doc_result.get(array) );
      size_t count;
      auto e = array.count_elements().get(count);
      if( e != TAPE_ERROR) {
        std::cout << e << "\n";
        std::cout << "expected: " << TAPE_ERROR << "\n";
        std::cout << "count = " << count << "\n";
        return false;
      }
      return true;
    }));
    TEST_SUCCEED();
  }
  bool iterate_document_array() {
    TEST_START();
    const auto json = std::string_view(R"([ 1, 10, 100 ])");
    const uint64_t expected_value[] = { 1, 10, 100 };

    SUBTEST("ondemand::array", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::array array;
      ASSERT_RESULT( doc_result.type(), json_type::array );
      ASSERT_SUCCESS( doc_result.get(array) );

      size_t i = 0;
      for (auto value : array) {
        int64_t actual;
        ASSERT_SUCCESS( value.get(actual) );
        ASSERT_EQUAL(actual, expected_value[i]);
        i++;
      }
      ASSERT_EQUAL(i*sizeof(int64_t), sizeof(expected_value));
      return true;
    }));

    SUBTEST("ondemand::array-document-rewind", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::array array;
      ASSERT_RESULT( doc_result.type(), json_type::array );
      ASSERT_SUCCESS( doc_result.get(array) );

      size_t i = 0;
      for (auto value : array) { (void)value; i++; }
      ASSERT_EQUAL(i*sizeof(uint64_t), sizeof(expected_value));
      std::vector<int64_t> container(i); // container of size 'i'.

      doc_result.rewind();
      ASSERT_RESULT( doc_result.type(), json_type::array );
      ASSERT_SUCCESS( doc_result.get(array) );
      i = 0;
      for (auto value : array) {
        int64_t actual;
        ASSERT_SUCCESS( value.get(actual) );
        container[i] = actual;
        i++;
      }
      ASSERT_EQUAL(i * sizeof(int64_t), sizeof(expected_value));
      for(size_t j = 0; j < sizeof(expected_value)/sizeof(int64_t); j++) {
        ASSERT_EQUAL(container[j], expected_value[j]);
      }
      return true;
    }));

    SUBTEST("simdjson_result<ondemand::array>", test_ondemand_doc(json, [&](auto doc_result) {
      simdjson_result<ondemand::array> array = doc_result.get_array();
      ASSERT_RESULT( doc_result.type(), json_type::array );
      size_t i=0;
      for (simdjson_unused auto value : array) { int64_t actual; ASSERT_SUCCESS( value.get(actual) ); ASSERT_EQUAL(actual, expected_value[i]); i++; }
      ASSERT_EQUAL(i*sizeof(uint64_t), sizeof(expected_value));
      return true;
    }));

    SUBTEST("ondemand::document", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::document doc;
      ASSERT_SUCCESS( std::move(doc_result).get(doc) );
      ASSERT_RESULT( doc.type(), json_type::array );
      size_t i=0;
      for (simdjson_unused auto value : doc) { int64_t actual; ASSERT_SUCCESS( value.get(actual) ); ASSERT_EQUAL(actual, expected_value[i]); i++; }
      ASSERT_EQUAL(i*sizeof(uint64_t), sizeof(expected_value));
      return true;
    }));
    SUBTEST("simdjson_result<ondemand::document>", test_ondemand_doc(json, [&](auto doc_result) {
      size_t i=0;
      ASSERT_RESULT( doc_result.type(), json_type::array );
      for (simdjson_unused auto value : doc_result) { int64_t actual; ASSERT_SUCCESS( value.get(actual) ); ASSERT_EQUAL(actual, expected_value[i]); i++; }
      ASSERT_EQUAL(i*sizeof(uint64_t), sizeof(expected_value));
      return true;
    }));
    TEST_SUCCEED();
  }

  bool iterate_array() {
    TEST_START();
    const auto json = std::string_view(R"( [ [ 1, 10, 100 ] ] )");
    const uint64_t expected_value[] = { 1, 10, 100 };

    SUBTEST("ondemand::array", test_ondemand_doc(json, [&](auto doc_result) {
      bool found = false;
      for (simdjson_result<ondemand::value> array_result : doc_result) {
        ASSERT_TRUE(!found); found = true;

        ondemand::array array;
        ASSERT_RESULT( array_result.type(), json_type::array );
        ASSERT_SUCCESS( array_result.get(array) );

        size_t i=0;
        for (auto value : array) {
          int64_t actual;
          ASSERT_SUCCESS( value.get(actual) );
          ASSERT_EQUAL(actual, expected_value[i]);
          i++;
        }
        ASSERT_EQUAL(i*sizeof(uint64_t), sizeof(expected_value));
      }
        ASSERT_TRUE(found);
      return true;
    }));

    SUBTEST("simdjson_result<ondemand::array>", test_ondemand_doc(json, [&](auto doc_result) {
      bool found = false;
      for (simdjson_result<ondemand::value> array_result : doc_result) {
        ASSERT_TRUE(!found); found = true;

        ondemand::array array;
        ASSERT_RESULT( array_result.type(), json_type::array );
        ASSERT_SUCCESS( array_result.get(array) );

        size_t i=0;
        for (simdjson_unused auto value : array) { int64_t actual; ASSERT_SUCCESS( value.get(actual) ); ASSERT_EQUAL(actual, expected_value[i]); i++; }
        ASSERT_EQUAL(i*sizeof(uint64_t), sizeof(expected_value));
      }
      ASSERT_TRUE(found);
      return true;
    }));

    SUBTEST("ondemand::value", test_ondemand_doc(json, [&](auto doc_result) {
      bool found = false;
      for (simdjson_result<ondemand::value> array_result : doc_result) {
        ASSERT_TRUE(!found); found = true;

        ondemand::value array;
        ASSERT_SUCCESS( std::move(array_result).get(array) );
        ASSERT_RESULT( array.type(), json_type::array );
        size_t i=0;
        for (simdjson_unused auto value : array) { int64_t actual; ASSERT_SUCCESS( value.get(actual) ); ASSERT_EQUAL(actual, expected_value[i]); i++; }
        ASSERT_EQUAL(i*sizeof(uint64_t), sizeof(expected_value));
      }
      ASSERT_TRUE(found);
      return true;
    }));
    SUBTEST("simdjson_result<ondemand::value>", test_ondemand_doc(json, [&](auto doc_result) {
      bool found = false;
      for (simdjson_result<ondemand::value> array_result : doc_result) {
        ASSERT_TRUE(!found); found = true;

        size_t i=0;
        ASSERT_RESULT( array_result.type(), json_type::array );
        for (simdjson_unused auto value : array_result) { int64_t actual; ASSERT_SUCCESS( value.get(actual) ); ASSERT_EQUAL(actual, expected_value[i]); i++; }
        ASSERT_EQUAL(i*sizeof(uint64_t), sizeof(expected_value));
      }
      ASSERT_TRUE(found);
      return true;
    }));
    TEST_SUCCEED();
  }

  bool iterate_array_partial_children() {
    TEST_START();
    auto json = std::string_view(R"(
      [
        0,
        [],
        {},
        { "x": 3, "y": 33 },
        { "x": 4, "y": 44 },
        { "x": 5, "y": 55 },
        { "x": 6, "y": 66 },
        [ 7, 77, 777 ],
        [ 8, 88, 888 ],
        { "a": [ { "b": [ 9, 99 ], "c": 999 }, 9999 ], "d": 99999 },
        10
      ]
    )");
    SUBTEST("simdjson_result<ondemand::document>", test_ondemand_doc(json, [&](auto doc_result) {
      size_t i = 0;
      for (auto value : doc_result) {
        ASSERT_SUCCESS(value);

        switch (i) {
          case 0: {
            std::cout << "  - After ignoring empty scalar ..." << std::endl;
            break;
          }
          case 1: {
            std::cout << "  - After ignoring empty array ..." << std::endl;
            break;
          }
          case 2: {
            std::cout << "  - After ignoring empty object ..." << std::endl;
            break;
          }
          // Break after using first value in child object
          case 3: {
            for (auto [ child_field, error ] : value.get_object()) {
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
            for (auto [ child_field, error ] : value.get_object()) {
              ASSERT_SUCCESS(error);
              ASSERT_EQUAL(child_field.key(), "x");
              break;
            }
            std::cout << "  - After reaching (but not using) first value in child object ..." << std::endl;
            break;
          }

          // Only look up one field in child object
          case 5: {
            uint64_t x;
            ASSERT_SUCCESS( value["x"].get(x) );
            ASSERT_EQUAL( x, 5 );
            std::cout << "  - After looking up one field in child object ..." << std::endl;
            break;
          }

          // Only look up one field in child object, but don't use it
          case 6: {
            ASSERT_SUCCESS( value["x"] );
            std::cout << "  - After looking up (but not using) one field in child object ..." << std::endl;
            break;
          }

          // Break after first value in child array
          case 7: {
            for (auto [ child_value, error ] : value) {
              ASSERT_SUCCESS(error);
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
            for (auto child_value : value) {
              ASSERT_SUCCESS(child_value);
              break;
            }
            std::cout << "  - After reaching (but not using) first value in child array ..." << std::endl;
            break;
          }

          // Break out of multiple child loops
          case 9: {
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
            break;
          }

          // Test the actual value
          case 10: {
            uint64_t actual_value;
            ASSERT_SUCCESS( value.get(actual_value) );
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

  bool iterate_empty_array() {
    TEST_START();
    auto json = std::string_view("[]");
    SUBTEST("ondemand::array", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::array array;
      ASSERT_SUCCESS( doc_result.get(array) );
      for (simdjson_unused auto value : array) { TEST_FAIL("Unexpected value"); }
      return true;
    }));
    SUBTEST("simdjson_result<ondemand::array>", test_ondemand_doc(json, [&](auto doc_result) {
      simdjson_result<ondemand::array> array_result = doc_result.get_array();
      for (simdjson_unused auto value : array_result) { TEST_FAIL("Unexpected value"); }
      return true;
    }));
    SUBTEST("ondemand::document", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::document doc;
      ASSERT_SUCCESS( std::move(doc_result).get(doc) );
      for (simdjson_unused auto value : doc) { TEST_FAIL("Unexpected value"); }
      return true;
    }));
    SUBTEST("simdjson_result<ondemand::document>", test_ondemand_doc(json, [&](auto doc_result) {
      for (simdjson_unused auto value : doc_result) { TEST_FAIL("Unexpected value"); }
      return true;
    }));
    SUBTEST("ondemand::array-document-rewind", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::array array;
      ASSERT_RESULT( doc_result.type(), json_type::array );
      ASSERT_SUCCESS( doc_result.get(array) );

      size_t i = 0;
      for (auto value : array) { (void) value; i++; }
      ASSERT_EQUAL(i, 0);

      doc_result.rewind();
      ASSERT_RESULT( doc_result.type(), json_type::array );
      ASSERT_SUCCESS( doc_result.get(array) );
      i = 0;
      for (auto value : array) { (void) value; i++; }
      ASSERT_EQUAL(i, 0);
      return true;
    }));
    TEST_SUCCEED();
  }
#if SIMDJSON_EXCEPTIONS

  bool iterate_array_exception() {
    TEST_START();
    auto json = std::string_view(R"([ 1, 10, 100 ])");
    const uint64_t expected_value[] = { 1, 10, 100 };

    ASSERT_TRUE(test_ondemand_doc(json, [&](auto doc_result) {
      size_t i=0;
      for (int64_t actual : doc_result) { ASSERT_EQUAL(actual, expected_value[i]); i++; }
      ASSERT_EQUAL(i*sizeof(uint64_t), sizeof(expected_value));
      return true;
    }));
    TEST_SUCCEED();
  }

  bool iterate_empty_object_exception() {
    TEST_START();
    auto json = std::string_view(R"({})");

    ASSERT_TRUE(test_ondemand_doc(json, [&](auto doc_result) {
      for (simdjson_unused ondemand::field field : doc_result.get_object()) {
        TEST_FAIL("Unexpected field");
      }
      return true;
    }));

    TEST_SUCCEED();
  }

  bool iterate_empty_array_exception() {
    TEST_START();
    auto json = std::string_view("[]");

    ASSERT_TRUE(test_ondemand_doc(json, [&](auto doc_result) {
      for (simdjson_unused ondemand::value value : doc_result) { TEST_FAIL("Unexpected value"); }
      return true;
    }));

    TEST_SUCCEED();
  }

#endif // SIMDJSON_EXCEPTIONS

  bool run() {
    return
           iterate_sub_array_count() &&
           iterate_complex_array_count() &&
           iterate_bad_array_count() &&
           iterate_array_count() &&
           issue1588() &&
           iterate_array() &&
           iterate_document_array() &&
           iterate_empty_array() &&
           iterate_array_partial_children() &&
#if SIMDJSON_EXCEPTIONS
           iterate_array_exception() &&
#endif // SIMDJSON_EXCEPTIONS
           true;
  }

} // namespace dom_api_tests

int main(int argc, char *argv[]) {
  return test_main(argc, argv, array_tests::run);
}
