#include "simdjson.h"
#include "test_ondemand.h"

using namespace simdjson;

namespace key_string_tests {
#if SIMDJSON_EXCEPTIONS
  bool parser_key_value() {
    TEST_START();
    ondemand::parser parser;
    const std::string_view json = R"({ "1": "1", "2": "2", "3": "3", "abc": "abc", "\u0075": "\u0075" })";
    auto doc = parser.iterate(json);
    for(auto field : doc.get_object())  {
      std::string_view keyv = field.unescaped_key();
      std::string_view valuev = field.value();
      if(keyv != valuev) { return false; }
    }
    return true;
  }
#endif // SIMDJSON_EXCEPTIONS
  bool run() {
    return
#if SIMDJSON_EXCEPTIONS
      parser_key_value() &&
#endif // SIMDJSON_EXCEPTIONS
      true;
  }

} // namespace key_string_tests

int main(int argc, char *argv[]) {
  return test_main(argc, argv, key_string_tests::run);
}
