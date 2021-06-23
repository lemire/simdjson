#include "simdjson.h"
#include "test_ondemand.h"

using namespace simdjson;

namespace parse_api_tests {
  using namespace std;

  const std::string_view BASIC_JSON = "[1,2,3]";


  bool parser_iterate_empty() {
    TEST_START();
    FILE *p;
    // Of course, we could just call iterate on the empty string, but
    // we want to test the whole process.
    const char *const tmpfilename = "emptyondemand.txt";
    if((p = fopen(tmpfilename, "w")) != nullptr) {
      fclose(p);
      auto json = padded_string::load(tmpfilename);
      ondemand::document doc;
      ondemand::parser parser;
      auto error = parser.iterate(json).get(doc);
      remove(tmpfilename);
      if(error != simdjson::EMPTY) {
        std::cerr << "Was expecting empty but got " << error << std::endl;
        return false;
      }
    } else {
      std::cout << "Warning: I could not create temporary file " << tmpfilename << std::endl;
      std::cout << "We omit testing the empty file case." << std::endl;
    }
    TEST_SUCCEED();
  }

  bool parser_iterate() {
    TEST_START();
    ondemand::parser parser;
    cout << "iterating" << endl;
    auto doc = parser.iterate(BASIC_JSON);
    cout << "iterated" << endl;
    ASSERT_SUCCESS( doc.get_array() );
    cout << "got array" << endl;
    return true;
  }

#if SIMDJSON_EXCEPTIONS
  bool parser_iterate_exception() {
    TEST_START();
    ondemand::parser parser;
    auto doc = parser.iterate(BASIC_JSON);
    simdjson_unused ondemand::array array = doc;
    TEST_SUCCEED();
  }

  bool parser_document_reuse() {
    TEST_START();
    ondemand::document doc;
    // A document spans about 40 bytes. Nevertheless, some users
    // would rather reuse them.
    std::cout << sizeof(doc) << std::endl;
    auto json = R"({"key": "value"})"_padded;
    auto jsonbad = R"({"key": "value")"_padded; // deliberaty broken
    auto jsonunclosedstring = "{\"coordinates:[{\"x\":1.1,\"y\":2.2,\"z\":3.3}]}"_padded;
    std::string output;

    ondemand::parser parser;
    std::cout << "correct document (1)" << std::endl;

    ASSERT_SUCCESS( parser.iterate(json).get(doc) );

    ASSERT_SUCCESS(simdjson::to_string(doc).get(output));
    std::cout << output << std::endl;

    std::cout << "correct document (2)" << std::endl;

    ASSERT_SUCCESS( parser.iterate(json).get(doc) );
    for(ondemand::field field : doc.get_object() ) {
      std::cout << "field: " << field.key() << std::endl;
    }
    std::cout << "unclosed string document " << std::endl;
    simdjson::error_code error;
    if((error = parser.iterate(jsonunclosedstring).get(doc)) == SUCCESS) {
      // fallback kernel:
      ASSERT_EQUAL( doc.get_object().find_field("coordinates").error(), TAPE_ERROR );
    } else {
      // regular kernels:
      ASSERT_EQUAL( error, UNCLOSED_STRING );
    }

    std::cout << "truncated document " << std::endl;

    ASSERT_SUCCESS( parser.iterate(jsonbad).get(doc) );

    ASSERT_EQUAL( simdjson::to_string(doc).get(output), TAPE_ERROR );

    std::cout << "correct document with new doc" << std::endl;
    ondemand::document doc2;
    ASSERT_SUCCESS( parser.iterate(json).get(doc2) );
    for(ondemand::field field : doc2.get_object() ) {
      std::cout << "field: " << field.key() << std::endl;
    }
    std::cout << "correct document (3): " << doc.to_debug_string() << std::endl;

    std::cout << "correct document (3)" << std::endl;
    ASSERT_SUCCESS( parser.iterate(json).get(doc) );
    std::cout << doc.to_debug_string() << std::endl;
    for(ondemand::field field : doc.get_object() ) {
      std::cout << "field: " << field.key() << std::endl;
    }

    std::cout << "unclosed string document " << std::endl;
    ASSERT_SUCCESS( parser.iterate(jsonbad).get(doc) );
    ASSERT_EQUAL( simdjson::to_string(doc).get(output), TAPE_ERROR );

    // next two lines are terrible code.
    doc.~document();
    doc = ondemand::document();
    //

    std::cout << "correct document (4)" << std::endl;

    ASSERT_SUCCESS( parser.iterate(json).get(doc) );
    ASSERT_SUCCESS( simdjson::to_string(doc).get(output) );
    std::cout << output << std::endl;

    std::cout << "unclosed string document " << std::endl;

    if((error = parser.iterate(jsonunclosedstring).get(doc)) == SUCCESS) {
      // fallback kernel:
      ASSERT_EQUAL( doc.get_object().find_field("coordinates").error(), TAPE_ERROR );
    } else {
      // regular kernels:
      ASSERT_EQUAL( error, UNCLOSED_STRING );
    }


    // next two lines are terrible code.
    doc.~document();
    doc = ondemand::document();
    //
    std::cout << "correct document (5)" << std::endl;

    ASSERT_SUCCESS( parser.iterate(json).get(doc) );
    ASSERT_SUCCESS( simdjson::to_string(doc).get(output) );
    std::cout << output << std::endl;

    TEST_SUCCEED();
  }
#endif // SIMDJSON_EXCEPTIONS

  bool run() {
    return //parser_iterate_empty() &&
           parser_iterate() &&
#if SIMDJSON_EXCEPTIONS
           parser_document_reuse() &&
           parser_iterate_exception() &&
#endif // SIMDJSON_EXCEPTIONS
           true;
  }
}


int main(int argc, char *argv[]) {
  return test_main(argc, argv, parse_api_tests::run);
}
