#include "simdjson.h"
#include "test_ondemand.h"

using namespace simdjson;

namespace active_tests {

#if SIMDJSON_EXCEPTIONS

  bool parser_child() {
    TEST_START();
    ondemand::parser parser;
    const std::string_view json = R"({ "parent": {"child1": {"name": "John"} , "child2": {"name": "Daniel"}} })";
    auto doc = parser.iterate(json);
    ondemand::object parent = doc["parent"];
    {
      ondemand::object c1 = parent["child1"];
      if(std::string_view(c1["name"]) != "John") { return false; }
    }
    {
      ondemand::object c2 = parent["child2"];
      if(std::string_view(c2["name"]) != "Daniel") { return false; }
    }
    return true;
  }

  bool parser_doc_correct() {
    TEST_START();
    ondemand::parser parser;
    const std::string_view json = R"({ "key1": 1, "key2":2, "key3": 3 })";
    auto doc = parser.iterate(json);
    ondemand::object root_object = doc.get_object();
    int64_t k1 = root_object["key1"];
    int64_t k2 = root_object["key2"];
    int64_t k3 = root_object["key3"];
    return (k1 == 1) && (k2 == 2) && (k3 == 3);
  }

  bool parser_doc_limits() {
    TEST_START();
    ondemand::parser parser;
    const std::string_view json = R"({ "key1": 1, "key2":2, "key3": 3 })";
    auto doc = parser.iterate(json);
    int64_t k1 = doc["key1"];
    try {
      int64_t k2 = doc["key2"];
      (void) k2;
    } catch (simdjson::simdjson_error &) {
      return true; // we expect to fail.
    }
    (void) k1;
    return false;
  }

#endif // SIMDJSON_EXCEPTIONS

  bool run() {
    return
#if SIMDJSON_EXCEPTIONS
      parser_child() &&
      parser_doc_correct() &&
      // parser_doc_limits() && // Failure is dependent on build type here ...
#endif // SIMDJSON_EXCEPTIONS
      true;
  }

} // namespace active_tests

int main(int argc, char *argv[]) {
  return test_main(argc, argv, active_tests::run);
}
