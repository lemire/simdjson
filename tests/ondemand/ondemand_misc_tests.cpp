#include "simdjson.h"
#include "test_ondemand.h"

using namespace simdjson;

namespace misc_tests {
  using namespace std;
  simdjson_warn_unused bool big_integer() {
    TEST_START();
    simdjson::ondemand::parser parser;
    std::string_view docdata =  R"({"value":12321323213213213213213213213211223})";
    simdjson::ondemand::document doc;
    ASSERT_SUCCESS(parser.iterate(docdata).get(doc));
    simdjson::ondemand::object o;
    ASSERT_SUCCESS(doc.get_object().get(o));
    string_view token;
    ASSERT_SUCCESS(o["value"].raw_json_token().get(token));
    ASSERT_EQUAL(token, "12321323213213213213213213213211223");
    return true;
  }
  simdjson_warn_unused bool big_integer_in_string() {
    TEST_START();
    simdjson::ondemand::parser parser;
    std::string_view docdata =  R"({"value":"12321323213213213213213213213211223"})";
    simdjson::ondemand::document doc;
    ASSERT_SUCCESS(parser.iterate(docdata).get(doc));
    simdjson::ondemand::object o;
    ASSERT_SUCCESS(doc.get_object().get(o));
    string_view token;
    ASSERT_SUCCESS(o["value"].raw_json_token().get(token));
    ASSERT_EQUAL(token, "\"12321323213213213213213213213211223\"");
    return true;
  }
  simdjson_warn_unused bool test_raw_json_token(string_view json, string_view expected_token, int expected_start_index = 0) {
    string title = "'";
    title.append(json.data(), json.length());
    title += "'";
    SUBTEST(title, test_ondemand_doc(json, [&](auto doc) {
      string_view token;
      ASSERT_SUCCESS( doc.raw_json_token().get(token) );
      ASSERT_EQUAL( token, expected_token );
      // Validate the text is inside the original buffer
      ASSERT_EQUAL( reinterpret_cast<const void*>(token.data()), reinterpret_cast<const void*>(&json.data()[expected_start_index]));
      return true;
    }));

    // Test values
    auto json_in_hash = string(R"({"a":)");
    json_in_hash.append(json.data(), json.length());
    json_in_hash += "}";
    title = "'";
    title.append(json_in_hash.data(), json_in_hash.length());
    title += "'";
    SUBTEST(title, test_ondemand_doc(json_in_hash, [&](auto doc) {
      string_view token;
      ASSERT_SUCCESS( doc["a"].raw_json_token().get(token) );
      ASSERT_EQUAL( token, expected_token );
      // Validate the text is inside the original buffer
      // Adjust for the {"a":
      ASSERT_EQUAL( reinterpret_cast<const void*>(token.data()), reinterpret_cast<const void*>(&json_in_hash.data()[5+expected_start_index]));
      return true;
    }));

    return true;
  }

  bool raw_json_token() {
    TEST_START();
    return
      test_raw_json_token("{}", "{") &&
      test_raw_json_token("{ }", "{ ") &&
      test_raw_json_token("{ \n }", "{ \n ") &&
      test_raw_json_token(" \n { \n } \n ", "{ \n ", 3) &&
      test_raw_json_token("[]", "[") &&
      test_raw_json_token("1", "1") &&
      test_raw_json_token(" \n 1 \n ", "1 \n ", 3) &&
      test_raw_json_token("-123.456e-789", "-123.456e-789") &&
      test_raw_json_token(" \n -123.456e-789 \n ", "-123.456e-789 \n ", 3) &&
      test_raw_json_token("true", "true") &&
      test_raw_json_token("false", "false") &&
      test_raw_json_token("null", "null") &&
      test_raw_json_token("blah2", "blah2") &&
      test_raw_json_token("true false", "true ") &&
      test_raw_json_token("true \n false", "true \n ") &&
      true;
  }

  bool run() {
    return
           big_integer_in_string() &&
           big_integer() &&
           raw_json_token() &&
           true;
  }

} // namespace twitter_tests

int main(int argc, char *argv[]) {
  return test_main(argc, argv, misc_tests::run);
}
