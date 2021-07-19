#include "simdjson.h"
#include "test_ondemand.h"

using namespace simdjson;

namespace misc_tests {
  using namespace std;

  bool issue1661a() {
    TEST_START();
    ondemand::parser parser;
    padded_string docdata = R"({"":],"global-groups":[[]}})"_padded;
    ondemand::document doc;
    ASSERT_SUCCESS(parser.iterate(docdata).get(doc));
    ondemand::value global_groups;
    ASSERT_SUCCESS(doc["global-groups"].get(global_groups));
    ondemand::json_type global_type;
    ASSERT_SUCCESS(global_groups.type().get(global_type));
    ASSERT_EQUAL(global_type, ondemand::json_type::array);
    TEST_SUCCEED();
  }

  bool issue1660() {
    TEST_START();
    ondemand::parser parser;
    padded_string docdata =  R"({"globals":{"a":{"shadowable":[}}}})"_padded;
    ondemand::document doc;
    ASSERT_SUCCESS(parser.iterate(docdata).get(doc));
    ondemand::object globals;
    ASSERT_SUCCESS(doc["globals"].get(globals));
    for (auto global_field : globals) {
      ondemand::value global;
      ASSERT_SUCCESS(global_field.value().get(global));
      ondemand::json_type global_type;
      ASSERT_SUCCESS(global.type().get(global_type));
      ASSERT_EQUAL(global_type, ondemand::json_type::object);
      ondemand::object global_object;
      ASSERT_SUCCESS(global.get(global_object));
      ondemand::value shadowable;
      ASSERT_SUCCESS(global_object["shadowable"].get(shadowable));
      ASSERT_ERROR(shadowable.get_object(), INCORRECT_TYPE);
      ondemand::value badvalue;
      auto error = global_object["writable"].get(badvalue);
      if(error == SUCCESS) {
        return false;
      } else {
        break;
      }
    }
    TEST_SUCCEED();
  }


  bool issue1660_with_bool() {
    TEST_START();
    ondemand::parser parser;
    padded_string docdata =  R"({"globals":{"a":{"shadowable":[}}}})"_padded;
    ondemand::document doc;
    ASSERT_SUCCESS(parser.iterate(docdata).get(doc));
    ondemand::object globals;
    ASSERT_SUCCESS(doc["globals"].get(globals));
    for (auto global_field : globals) {
      ondemand::value global;
      ASSERT_SUCCESS(global_field.value().get(global));
      ondemand::json_type global_type;
      ASSERT_SUCCESS(global.type().get(global_type));
      ASSERT_EQUAL(global_type, ondemand::json_type::object);
      ondemand::object global_object;
      ASSERT_SUCCESS(global.get(global_object));
      ondemand::value shadowable;
      ASSERT_SUCCESS(global_object["shadowable"].get(shadowable));
      ASSERT_ERROR(shadowable.get_bool(), INCORRECT_TYPE);
      ondemand::value badvalue;
      auto error = global_object["writable"].get(badvalue);
      if(error == SUCCESS) {
        return false;
      } else {
        break;
      }
    }
    TEST_SUCCEED();
  }


  bool issue1660_with_uint64() {
    TEST_START();
    ondemand::parser parser;
    padded_string docdata =  R"({"globals":{"a":{"shadowable":[}}}})"_padded;
    ondemand::document doc;
    ASSERT_SUCCESS(parser.iterate(docdata).get(doc));
    ondemand::object globals;
    ASSERT_SUCCESS(doc["globals"].get(globals));
    for (auto global_field : globals) {
      ondemand::value global;
      ASSERT_SUCCESS(global_field.value().get(global));
      ondemand::json_type global_type;
      ASSERT_SUCCESS(global.type().get(global_type));
      ASSERT_EQUAL(global_type, ondemand::json_type::object);
      ondemand::object global_object;
      ASSERT_SUCCESS(global.get(global_object));
      ondemand::value shadowable;
      ASSERT_SUCCESS(global_object["shadowable"].get(shadowable));
      ASSERT_ERROR(shadowable.get_uint64(), INCORRECT_TYPE);
      ondemand::value badvalue;
      auto error = global_object["writable"].get(badvalue);
      if(error == SUCCESS) {
        return false;
      } else {
        break;
      }
    }
    TEST_SUCCEED();
  }


  bool issue1660_with_int64() {
    TEST_START();
    ondemand::parser parser;
    padded_string docdata =  R"({"globals":{"a":{"shadowable":[}}}})"_padded;
    ondemand::document doc;
    ASSERT_SUCCESS(parser.iterate(docdata).get(doc));
    ondemand::object globals;
    ASSERT_SUCCESS(doc["globals"].get(globals));
    for (auto global_field : globals) {
      ondemand::value global;
      ASSERT_SUCCESS(global_field.value().get(global));
      ondemand::json_type global_type;
      ASSERT_SUCCESS(global.type().get(global_type));
      ASSERT_EQUAL(global_type, ondemand::json_type::object);
      ondemand::object global_object;
      ASSERT_SUCCESS(global.get(global_object));
      ondemand::value shadowable;
      ASSERT_SUCCESS(global_object["shadowable"].get(shadowable));
      ASSERT_ERROR(shadowable.get_int64(), INCORRECT_TYPE);
      ondemand::value badvalue;
      auto error = global_object["writable"].get(badvalue);
      if(error == SUCCESS) {
        return false;
      } else {
        break;
      }
    }
    TEST_SUCCEED();
  }

  bool issue1660_with_double() {
    TEST_START();
    ondemand::parser parser;
    padded_string docdata =  R"({"globals":{"a":{"shadowable":[}}}})"_padded;
    ondemand::document doc;
    ASSERT_SUCCESS(parser.iterate(docdata).get(doc));
    ondemand::object globals;
    ASSERT_SUCCESS(doc["globals"].get(globals));
    for (auto global_field : globals) {
      ondemand::value global;
      ASSERT_SUCCESS(global_field.value().get(global));
      ondemand::json_type global_type;
      ASSERT_SUCCESS(global.type().get(global_type));
      ASSERT_EQUAL(global_type, ondemand::json_type::object);
      ondemand::object global_object;
      ASSERT_SUCCESS(global.get(global_object));
      ondemand::value shadowable;
      ASSERT_SUCCESS(global_object["shadowable"].get(shadowable));
      ASSERT_ERROR(shadowable.get_double(), INCORRECT_TYPE);
      ondemand::value badvalue;
      auto error = global_object["writable"].get(badvalue);
      if(error == SUCCESS) {
        return false;
      } else {
        break;
      }
    }
    TEST_SUCCEED();
  }


  bool issue1660_with_null() {
    TEST_START();
    ondemand::parser parser;
    padded_string docdata =  R"({"globals":{"a":{"shadowable":[}}}})"_padded;
    ondemand::document doc;
    ASSERT_SUCCESS(parser.iterate(docdata).get(doc));
    ondemand::object globals;
    ASSERT_SUCCESS(doc["globals"].get(globals));
    for (auto global_field : globals) {
      ondemand::value global;
      ASSERT_SUCCESS(global_field.value().get(global));
      ondemand::json_type global_type;
      ASSERT_SUCCESS(global.type().get(global_type));
      ASSERT_EQUAL(global_type, ondemand::json_type::object);
      ondemand::object global_object;
      ASSERT_SUCCESS(global.get(global_object));
      ondemand::value shadowable;
      ASSERT_SUCCESS(global_object["shadowable"].get(shadowable));
      ASSERT_TRUE(!shadowable.is_null());
      ondemand::value badvalue;
      auto error = global_object["writable"].get(badvalue);
      if(error == SUCCESS) {
        return false;
      } else {
        break;
      }
    }
    TEST_SUCCEED();
  }


  bool issue1660_with_string() {
    TEST_START();
    ondemand::parser parser;
    padded_string docdata =  R"({"globals":{"a":{"shadowable":[}}}})"_padded;
    ondemand::document doc;
    ASSERT_SUCCESS(parser.iterate(docdata).get(doc));
    ondemand::object globals;
    ASSERT_SUCCESS(doc["globals"].get(globals));
    for (auto global_field : globals) {
      ondemand::value global;
      ASSERT_SUCCESS(global_field.value().get(global));
      ondemand::json_type global_type;
      ASSERT_SUCCESS(global.type().get(global_type));
      ASSERT_EQUAL(global_type, ondemand::json_type::object);
      ondemand::object global_object;
      ASSERT_SUCCESS(global.get(global_object));
      ondemand::value shadowable;
      ASSERT_SUCCESS(global_object["shadowable"].get(shadowable));
      ASSERT_ERROR(shadowable.get_string(), INCORRECT_TYPE);
      ondemand::value badvalue;
      auto error = global_object["writable"].get(badvalue);
      if(error == SUCCESS) {
        return false;
      } else {
        break;
      }
    }
    TEST_SUCCEED();
  }

  bool issue1661() {
    TEST_START();
    ondemand::parser parser;
    padded_string docdata = R"({"":],"global-groups":[[]}})"_padded;
    ondemand::document doc;
    ASSERT_SUCCESS(parser.iterate(docdata).get(doc));
    ondemand::object global_groups;
    ASSERT_ERROR(doc["global-groups"].get(global_groups), INCORRECT_TYPE);
    ondemand::object globals;
    auto error = doc["globals"].get(globals);
    if(error == SUCCESS) { return false; }
    TEST_SUCCEED();
  }

  simdjson_warn_unused bool big_integer() {
    TEST_START();
    simdjson::ondemand::parser parser;
    simdjson::padded_string docdata =  R"({"value":12321323213213213213213213213211223})"_padded;
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
    simdjson::padded_string docdata =  R"({"value":"12321323213213213213213213213211223"})"_padded;
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
    padded_string json_padded = json;
    SUBTEST(title, test_ondemand_doc(json_padded, [&](auto doc) {
      string_view token;
      ASSERT_SUCCESS( doc.raw_json_token().get(token) );
      ASSERT_EQUAL( token, expected_token );
      // Validate the text is inside the original buffer
      ASSERT_EQUAL( reinterpret_cast<const void*>(token.data()), reinterpret_cast<const void*>(&json_padded.data()[expected_start_index]));
      return true;
    }));

    // Test values
    auto json_in_hash = string(R"({"a":)");
    json_in_hash.append(json.data(), json.length());
    json_in_hash += "}";
    json_padded = json_in_hash;
    title = "'";
    title.append(json_in_hash.data(), json_in_hash.length());
    title += "'";
    SUBTEST(title, test_ondemand_doc(json_padded, [&](auto doc) {
      string_view token;
      ASSERT_SUCCESS( doc["a"].raw_json_token().get(token) );
      ASSERT_EQUAL( token, expected_token );
      // Validate the text is inside the original buffer
      // Adjust for the {"a":
      ASSERT_EQUAL( reinterpret_cast<const void*>(token.data()), reinterpret_cast<const void*>(&json_padded.data()[5+expected_start_index]));
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
           issue1660_with_uint64() &&
           issue1660_with_int64() &&
           issue1660_with_double() &&
           issue1660_with_null() &&
           issue1660_with_string() &&
           issue1660_with_bool() &&
           issue1661a() &&
           issue1660() &&
           issue1661() &&
           big_integer_in_string() &&
           big_integer() &&
           raw_json_token() &&
           true;
  }

} // namespace twitter_tests

int main(int argc, char *argv[]) {
  return test_main(argc, argv, misc_tests::run);
}
