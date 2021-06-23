#include <cinttypes>
#include <ciso646>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>

#include "simdjson.h"
using namespace simdjson;

#include "test_ondemand.h"
namespace tostring_tests {
const char *test_files[] = {
    TWITTER_JSON, TWITTER_TIMELINE_JSON, REPEAT_JSON, CANADA_JSON,
    MESH_JSON,    APACHE_JSON,           GSOC_JSON};

#if SIMDJSON_EXCEPTIONS
bool issue1607() {
  TEST_START();
  auto cars_json = R"( { "test": "result"  }  )"_padded;
  ondemand::parser parser;
  ondemand::document doc = parser.iterate(cars_json);
  std::string expected = R"("result")";
  std::string result = simdjson::to_string(doc["test"]);
  ASSERT_EQUAL(result, expected);
  TEST_SUCCEED();
}

bool minify_demo() {
  TEST_START();
  ondemand::parser parser;
  auto cars_json = std::string_view(R"( { "test": "result"  }  )");
  ondemand::document doc;
  ASSERT_SUCCESS( parser.iterate(cars_json).get(doc) );
  std::cout << simdjson::to_string(doc["test"]) << std::endl;
  TEST_SUCCEED();
}

bool minify_demo2() {
  TEST_START();
  ondemand::parser parser;
  auto cars_json = std::string_view(R"( { "test": "result"  }  )");
  ondemand::document doc;
  ASSERT_SUCCESS( parser.iterate(cars_json).get(doc) );
  std::cout << std::string_view(doc["test"]) << std::endl;
  TEST_SUCCEED();
}

/**
 * The general idea of these tests if that if you take a JSON file,
 * load it, then convert it into a string, then parse that, and
 * convert it again into a second string, then the two strings should
 * be  identifical. If not, then something was lost or added in the
 * process.
 */

bool load_to_string(const char *filename) {
  simdjson::ondemand::parser parser;
  std::cout << "Loading " << filename << std::endl;
  simdjson::padded_string docdata;
  auto error = simdjson::padded_string::load(filename).get(docdata);
  if (error) {
    std::cerr << "could not load " << filename << " got " << error << std::endl;
    return false;
  }
  std::cout << "file loaded: " << docdata.size() << " bytes." << std::endl;
  simdjson::ondemand::document doc;
  error = parser.iterate(docdata).get(doc);
  if (error) {
    std::cerr << error << std::endl;
    return false;
  }
  std::cout << "serializing once." << std::endl;
  std::string serial1 = simdjson::to_string(doc);
  serial1.reserve(serial1.size() + simdjson::SIMDJSON_PADDING);
  error = parser.iterate(serial1).get(doc);
  if (error) {
    std::cerr << error << std::endl;
    return false;
  }
  std::cout << "serializing twice." << std::endl;
  std::string serial2 = simdjson::to_string(doc);
  bool match = (serial1 == serial2);
  if (match) {
    std::cout << "Parsing to_string and calling to_string again results in the "
                 "same content."
              << "Got " << serial1.size() << " bytes." << std::endl;
  }
  return match;
}

bool minify_test() {
  TEST_START();
  for (size_t i = 0; i < sizeof(test_files) / sizeof(test_files[0]); i++) {
    bool ok = load_to_string(test_files[i]);
    if (!ok) {
      return false;
    }
  }
  TEST_SUCCEED();
}

#endif // SIMDJSON_EXCEPTIONS

bool load_to_string_exceptionless(const char *filename) {
  simdjson::ondemand::parser parser;
  std::cout << "Loading " << filename << std::endl;
  simdjson::padded_string docdata;
  auto error = simdjson::padded_string::load(filename).get(docdata);
  if (error) {
    std::cerr << "could not load " << filename << " got " << error << std::endl;
    return false;
  }
  std::cout << "file loaded: " << docdata.size() << " bytes." << std::endl;
  simdjson::ondemand::document doc;
  error = parser.iterate(docdata).get(doc);
  if (error) {
    std::cerr << error << std::endl;
    return false;
  }
  std::cout << "serializing once." << std::endl;
  std::string serial1;
  error = simdjson::to_string(doc).get(serial1);
  if (error) {
    std::cerr << error << std::endl;
    return false;
  }
  serial1.reserve(serial1.size() + simdjson::SIMDJSON_PADDING);
  error = parser.iterate(serial1).get(doc);
  if (error) {
    std::cerr << error << std::endl;
    return false;
  }
  std::cout << "serializing twice." << std::endl;
  std::string serial2;
  error = simdjson::to_string(doc).get(serial2);
  if (error) {
    std::cerr << error << std::endl;
    return false;
  }
  bool match = (serial1 == serial2);
  if (match) {
    std::cout << "Parsing to_string and calling to_string again results in the "
                 "same content."
              << "Got " << serial1.size() << " bytes." << std::endl;
  }
  return match;
}

bool minify_exceptionless_test() {
  TEST_START();
  for (size_t i = 0; i < sizeof(test_files) / sizeof(test_files[0]); i++) {
    bool ok = load_to_string_exceptionless(test_files[i]);
    if (!ok) {
      return false;
    }
  }
  return true;
}

bool run() {
  return
#if SIMDJSON_EXCEPTIONS
      issue1607() &&
      minify_demo() &&
      minify_demo2() &&
      minify_test() &&
#endif // SIMDJSON_EXCEPTIONS
      minify_exceptionless_test() &&
      true;
}

} // namespace tostring_tests

int main(int argc, char *argv[]) {
  return test_main(argc, argv, tostring_tests::run);
}
