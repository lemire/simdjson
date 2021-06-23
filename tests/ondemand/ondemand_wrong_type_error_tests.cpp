#include "simdjson.h"
#include "test_ondemand.h"

using namespace simdjson;

namespace wrong_type_error_tests {
  using namespace std;

#define TEST_CAST_ERROR(JSON, TYPE, ERROR) \
  std::cout << "- Subtest: get_" << (#TYPE) << "() - JSON: " << (JSON) << std::endl; \
  { \
    /* Put padding into the string to check the buffer overrun code as well */ \
    auto doc_json = std::string(JSON) + "1111111111111111111111111111111111111111111111111111111111111111"; \
    if (!test_ondemand_doc(doc_json, [&](auto doc_result) { \
      ASSERT_ERROR( doc_result.get_##TYPE(), (ERROR) ); \
      return true; \
    })) { \
      return false; \
    } \
  } \
  { \
    auto a_json = std::string(R"({ "a": )") + JSON + " })"; \
    std::cout << R"(- Subtest: get_)" << (#TYPE) << "() - JSON: " << a_json << std::endl; \
    if (!test_ondemand_doc(a_json, [&](auto doc_result) { \
      ASSERT_ERROR( doc_result["a"].get_##TYPE(), (ERROR) ); \
      return true; \
    })) { \
      return false; \
    }; \
  }

  bool wrong_type_array() {
    TEST_START();
    TEST_CAST_ERROR("[]", object, INCORRECT_TYPE);
    TEST_CAST_ERROR("[]", bool, INCORRECT_TYPE);
    TEST_CAST_ERROR("[]", int64, INCORRECT_TYPE);
    TEST_CAST_ERROR("[]", uint64, INCORRECT_TYPE);
    TEST_CAST_ERROR("[]", double, INCORRECT_TYPE);
    TEST_CAST_ERROR("[]", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("[]", raw_json_string, INCORRECT_TYPE);
    TEST_SUCCEED();
  }

  bool wrong_type_object() {
    TEST_START();
    TEST_CAST_ERROR("{}", array, INCORRECT_TYPE);
    TEST_CAST_ERROR("{}", bool, INCORRECT_TYPE);
    TEST_CAST_ERROR("{}", int64, INCORRECT_TYPE);
    TEST_CAST_ERROR("{}", uint64, INCORRECT_TYPE);
    TEST_CAST_ERROR("{}", double, INCORRECT_TYPE);
    TEST_CAST_ERROR("{}", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("{}", raw_json_string, INCORRECT_TYPE);
    TEST_SUCCEED();
  }

  bool wrong_type_true() {
    TEST_START();
    TEST_CAST_ERROR("true", array, INCORRECT_TYPE);
    TEST_CAST_ERROR("true", object, INCORRECT_TYPE);
    TEST_CAST_ERROR("true", int64, INCORRECT_TYPE);
    TEST_CAST_ERROR("true", uint64, INCORRECT_TYPE);
    TEST_CAST_ERROR("true", double, INCORRECT_TYPE);
    TEST_CAST_ERROR("true", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("true", raw_json_string, INCORRECT_TYPE);
    TEST_SUCCEED();
  }

  bool wrong_type_false() {
    TEST_START();
    TEST_CAST_ERROR("false", array, INCORRECT_TYPE);
    TEST_CAST_ERROR("false", object, INCORRECT_TYPE);
    TEST_CAST_ERROR("false", int64, INCORRECT_TYPE);
    TEST_CAST_ERROR("false", uint64, INCORRECT_TYPE);
    TEST_CAST_ERROR("false", double, INCORRECT_TYPE);
    TEST_CAST_ERROR("false", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("false", raw_json_string, INCORRECT_TYPE);
    TEST_SUCCEED();
  }

  bool wrong_type_null() {
    TEST_START();
    TEST_CAST_ERROR("null", array, INCORRECT_TYPE);
    TEST_CAST_ERROR("null", object, INCORRECT_TYPE);
    TEST_CAST_ERROR("null", bool, INCORRECT_TYPE);
    TEST_CAST_ERROR("null", int64, INCORRECT_TYPE);
    TEST_CAST_ERROR("null", uint64, INCORRECT_TYPE);
    TEST_CAST_ERROR("null", double, INCORRECT_TYPE);
    TEST_CAST_ERROR("null", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("null", raw_json_string, INCORRECT_TYPE);
    TEST_SUCCEED();
  }

  bool wrong_type_1() {
    TEST_START();
    TEST_CAST_ERROR("1", array, INCORRECT_TYPE);
    TEST_CAST_ERROR("1", object, INCORRECT_TYPE);
    TEST_CAST_ERROR("1", bool, INCORRECT_TYPE);
    TEST_CAST_ERROR("1", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("1", raw_json_string, INCORRECT_TYPE);
    TEST_SUCCEED();
  }

  bool wrong_type_negative_1() {
    TEST_START();
    TEST_CAST_ERROR("-1", array, INCORRECT_TYPE);
    TEST_CAST_ERROR("-1", object, INCORRECT_TYPE);
    TEST_CAST_ERROR("-1", bool, INCORRECT_TYPE);
    TEST_CAST_ERROR("-1", uint64, INCORRECT_TYPE);
    TEST_CAST_ERROR("-1", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("-1", raw_json_string, INCORRECT_TYPE);
    TEST_SUCCEED();
  }

  bool wrong_type_float() {
    TEST_START();
    TEST_CAST_ERROR("1.1", array, INCORRECT_TYPE);
    TEST_CAST_ERROR("1.1", object, INCORRECT_TYPE);
    TEST_CAST_ERROR("1.1", bool, INCORRECT_TYPE);
    TEST_CAST_ERROR("1.1", int64, INCORRECT_TYPE);
    TEST_CAST_ERROR("1.1", uint64, INCORRECT_TYPE);
    TEST_CAST_ERROR("1.1", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("1.1", raw_json_string, INCORRECT_TYPE);
    TEST_SUCCEED();
  }

  bool wrong_type_negative_int64_overflow() {
    TEST_START();
    TEST_CAST_ERROR("-9223372036854775809", array, INCORRECT_TYPE);
    TEST_CAST_ERROR("-9223372036854775809", object, INCORRECT_TYPE);
    TEST_CAST_ERROR("-9223372036854775809", bool, INCORRECT_TYPE);
    TEST_CAST_ERROR("-9223372036854775809", int64, INCORRECT_TYPE);
    TEST_CAST_ERROR("-9223372036854775809", uint64, INCORRECT_TYPE);
    TEST_CAST_ERROR("-9223372036854775809", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("-9223372036854775809", raw_json_string, INCORRECT_TYPE);
    TEST_SUCCEED();
  }

  bool wrong_type_int64_overflow() {
    TEST_START();
    TEST_CAST_ERROR("9223372036854775808", array, INCORRECT_TYPE);
    TEST_CAST_ERROR("9223372036854775808", object, INCORRECT_TYPE);
    TEST_CAST_ERROR("9223372036854775808", bool, INCORRECT_TYPE);
    TEST_CAST_ERROR("9223372036854775808", int64, INCORRECT_TYPE);
    TEST_CAST_ERROR("9223372036854775808", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("9223372036854775808", raw_json_string, INCORRECT_TYPE);
    TEST_SUCCEED();
  }

  bool wrong_type_uint64_overflow() {
    TEST_START();
    TEST_CAST_ERROR("18446744073709551616", array, INCORRECT_TYPE);
    TEST_CAST_ERROR("18446744073709551616", object, INCORRECT_TYPE);
    TEST_CAST_ERROR("18446744073709551616", bool, INCORRECT_TYPE);
    TEST_CAST_ERROR("18446744073709551616", int64, INCORRECT_TYPE);
    TEST_CAST_ERROR("18446744073709551616", uint64, INCORRECT_TYPE);
    TEST_CAST_ERROR("18446744073709551616", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("18446744073709551616", raw_json_string, INCORRECT_TYPE);
    TEST_SUCCEED();
  }

  bool wrong_type_nulll() {
    TEST_START();
    TEST_CAST_ERROR("nulll", array, INCORRECT_TYPE);
    TEST_CAST_ERROR("nulll", object, INCORRECT_TYPE);
    TEST_CAST_ERROR("nulll", bool, INCORRECT_TYPE);
    TEST_CAST_ERROR("nulll", int64, INCORRECT_TYPE);
    TEST_CAST_ERROR("nulll", uint64, INCORRECT_TYPE);
    TEST_CAST_ERROR("nulll", double, INCORRECT_TYPE);
    TEST_CAST_ERROR("nulll", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("nulll", raw_json_string, INCORRECT_TYPE);
    ONDEMAND_SUBTEST("nulll.is_null", "nulll", !doc.is_null());
    ONDEMAND_SUBTEST(R"({"a":nulll}.is_null)", R"({"a":nulll})", !doc["a"].is_null());
    TEST_SUCCEED();
  }

  bool wrong_type_n() {
    TEST_START();
    TEST_CAST_ERROR("n", array, INCORRECT_TYPE);
    TEST_CAST_ERROR("n", object, INCORRECT_TYPE);
    TEST_CAST_ERROR("n", bool, INCORRECT_TYPE);
    TEST_CAST_ERROR("n", int64, INCORRECT_TYPE);
    TEST_CAST_ERROR("n", uint64, INCORRECT_TYPE);
    TEST_CAST_ERROR("n", double, INCORRECT_TYPE);
    TEST_CAST_ERROR("n", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("n", raw_json_string, INCORRECT_TYPE);
    ONDEMAND_SUBTEST("n.is_null", "n", !doc.is_null());
    ONDEMAND_SUBTEST(R"({"a":n}.is_null)", R"({"a":n})", !doc["a"].is_null());
    TEST_SUCCEED();
  }

  bool wrong_type_null_padding() {
    TEST_START();
    ondemand::parser parser;
    std::string_view json = "null                                                                     ";
    // Even though it *would* match if the document is bigger, it should not match using the padding
    ASSERT_FALSE( parser.iterate(json.substr(0, 1)).is_null() );
    ASSERT_FALSE( parser.iterate(json.substr(0, 1)).is_null() );
    ASSERT_FALSE( parser.iterate(json.substr(0, 3)).is_null() );
    // Validate that the JSON parses correctly in when it's big enough
    ASSERT_TRUE(  parser.iterate(json.substr(0, 4)).is_null() );
    ASSERT_TRUE(  parser.iterate(json.substr(0, 5)).is_null() );

    // Validate that identifier characters in the padding don't trip up the parser
    json = "nulll                                                                ";
    ASSERT_TRUE(  parser.iterate(json.substr(0, 4)).is_null() );
    ASSERT_FALSE( parser.iterate(json.substr(0, 5)).is_null() );
    TEST_SUCCEED();
  }

  bool wrong_type_truee() {
    TEST_START();
    TEST_CAST_ERROR("truee", array, INCORRECT_TYPE);
    TEST_CAST_ERROR("truee", object, INCORRECT_TYPE);
    TEST_CAST_ERROR("truee", bool, INCORRECT_TYPE);
    TEST_CAST_ERROR("truee", int64, INCORRECT_TYPE);
    TEST_CAST_ERROR("truee", uint64, INCORRECT_TYPE);
    TEST_CAST_ERROR("truee", double, INCORRECT_TYPE);
    TEST_CAST_ERROR("truee", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("truee", raw_json_string, INCORRECT_TYPE);
    ONDEMAND_SUBTEST("truee.is_null", "truee", !doc.is_null());
    ONDEMAND_SUBTEST(R"({"a":truee}.is_null)", R"({"a":truee})", !doc["a"].is_null());
    TEST_SUCCEED();
  }

  bool wrong_type_t() {
    TEST_START();
    TEST_CAST_ERROR("t", array, INCORRECT_TYPE);
    TEST_CAST_ERROR("t", object, INCORRECT_TYPE);
    TEST_CAST_ERROR("t", bool, INCORRECT_TYPE);
    TEST_CAST_ERROR("t", int64, INCORRECT_TYPE);
    TEST_CAST_ERROR("t", uint64, INCORRECT_TYPE);
    TEST_CAST_ERROR("t", double, INCORRECT_TYPE);
    TEST_CAST_ERROR("t", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("t", raw_json_string, INCORRECT_TYPE);
    ONDEMAND_SUBTEST("t.is_null", "t", !doc.is_null());
    ONDEMAND_SUBTEST(R"({"a":t}.is_null)", R"({"a":t})", !doc["a"].is_null());
    TEST_SUCCEED();
  }

  bool wrong_type_true_padding() {
    TEST_START();
    ondemand::parser parser;
    auto json = "true                                                                     ";
    // Even though it *would* match if the document is bigger, it should not match using the padding
    ASSERT_ERROR(  parser.iterate(std::string_view(json, 1)).get_bool(), INCORRECT_TYPE );
    ASSERT_ERROR(  parser.iterate(std::string_view(json, 2)).get_bool(), INCORRECT_TYPE );
    ASSERT_ERROR(  parser.iterate(std::string_view(json, 3)).get_bool(), INCORRECT_TYPE );
    // Validate that the JSON parses correctly in when it's big enough
    ASSERT_RESULT( parser.iterate(std::string_view(json, 4)).get_bool(), true );
    ASSERT_RESULT( parser.iterate(std::string_view(json, 5)).get_bool(), true );

    // Validate that identifier characters in the padding don't trip up the parser
    json = "truel                                                                ";
    ASSERT_RESULT( parser.iterate(std::string_view(json, 4)).get_bool(), true);
    ASSERT_ERROR(  parser.iterate(std::string_view(json, 5)).get_bool(), INCORRECT_TYPE );
    TEST_SUCCEED();
  }

  bool wrong_type_falsee() {
    TEST_START();
    TEST_CAST_ERROR("falsee", array, INCORRECT_TYPE);
    TEST_CAST_ERROR("falsee", object, INCORRECT_TYPE);
    TEST_CAST_ERROR("falsee", bool, INCORRECT_TYPE);
    TEST_CAST_ERROR("falsee", int64, INCORRECT_TYPE);
    TEST_CAST_ERROR("falsee", uint64, INCORRECT_TYPE);
    TEST_CAST_ERROR("falsee", double, INCORRECT_TYPE);
    TEST_CAST_ERROR("falsee", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("falsee", raw_json_string, INCORRECT_TYPE);
    ONDEMAND_SUBTEST("nulll.is_null", "nulll", !doc.is_null());
    ONDEMAND_SUBTEST(R"({"a":nulll}.is_null)", R"({"a":nulll})", !doc["a"].is_null());
    TEST_SUCCEED();
  }

  bool wrong_type_fals() {
    TEST_START();
    TEST_CAST_ERROR("fals", array, INCORRECT_TYPE);
    TEST_CAST_ERROR("fals", object, INCORRECT_TYPE);
    TEST_CAST_ERROR("fals", bool, INCORRECT_TYPE);
    TEST_CAST_ERROR("fals", int64, INCORRECT_TYPE);
    TEST_CAST_ERROR("fals", uint64, INCORRECT_TYPE);
    TEST_CAST_ERROR("fals", double, INCORRECT_TYPE);
    TEST_CAST_ERROR("fals", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("fals", raw_json_string, INCORRECT_TYPE);
    ONDEMAND_SUBTEST("fals.is_null", "fals", !doc.is_null());
    ONDEMAND_SUBTEST(R"({"a":fals}.is_null)", R"({"a":fals})", !doc["a"].is_null());
    TEST_SUCCEED();
  }

  bool wrong_type_f() {
    TEST_START();
    TEST_CAST_ERROR("f", array, INCORRECT_TYPE);
    TEST_CAST_ERROR("f", object, INCORRECT_TYPE);
    TEST_CAST_ERROR("f", bool, INCORRECT_TYPE);
    TEST_CAST_ERROR("f", int64, INCORRECT_TYPE);
    TEST_CAST_ERROR("f", uint64, INCORRECT_TYPE);
    TEST_CAST_ERROR("f", double, INCORRECT_TYPE);
    TEST_CAST_ERROR("f", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("f", raw_json_string, INCORRECT_TYPE);
    ONDEMAND_SUBTEST("f.is_null", "f", !doc.is_null());
    ONDEMAND_SUBTEST(R"({"a":f}.is_null)", R"({"a":f})", !doc["a"].is_null());
    TEST_SUCCEED();
  }

  bool wrong_type_false_padding() {
    TEST_START();
    ondemand::parser parser;
    auto json = "false                                                                     ";
    // Even though it *would* match if the document is bigger, it should not match using the padding
    ASSERT_ERROR(  parser.iterate(std::string_view(json, 1)).get_bool(), INCORRECT_TYPE );
    ASSERT_ERROR(  parser.iterate(std::string_view(json, 2)).get_bool(), INCORRECT_TYPE );
    ASSERT_ERROR(  parser.iterate(std::string_view(json, 3)).get_bool(), INCORRECT_TYPE );
    ASSERT_ERROR(  parser.iterate(std::string_view(json, 4)).get_bool(), INCORRECT_TYPE );
    // Validate that the JSON parses correctly in when it's big enough
    ASSERT_RESULT( parser.iterate(std::string_view(json, 5)).get_bool(), false );
    ASSERT_RESULT( parser.iterate(std::string_view(json, 6)).get_bool(), false );

    // Validate that identifier characters in the padding don't trip up the parser
    json = "falsel                                                                ";
    ASSERT_RESULT( parser.iterate(std::string_view(json, 5)).get_bool(), false);
    ASSERT_ERROR(  parser.iterate(std::string_view(json, 6)).get_bool(), INCORRECT_TYPE );
    TEST_SUCCEED();
  }

  bool wrong_type_x() {
    TEST_START();
    TEST_CAST_ERROR("x", array, INCORRECT_TYPE);
    TEST_CAST_ERROR("x", object, INCORRECT_TYPE);
    TEST_CAST_ERROR("x", bool, INCORRECT_TYPE);
    TEST_CAST_ERROR("x", int64, INCORRECT_TYPE);
    TEST_CAST_ERROR("x", uint64, INCORRECT_TYPE);
    TEST_CAST_ERROR("x", double, INCORRECT_TYPE);
    TEST_CAST_ERROR("x", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("x", raw_json_string, INCORRECT_TYPE);
    ONDEMAND_SUBTEST("x.is_null", "x", !doc.is_null());
    ONDEMAND_SUBTEST(R"({"a":x}.is_null)", R"({"a":x})", !doc["a"].is_null());
    TEST_SUCCEED();
  }

  bool wrong_type_xxxx() {
    TEST_START();
    TEST_CAST_ERROR("xxxx", array, INCORRECT_TYPE);
    TEST_CAST_ERROR("xxxx", object, INCORRECT_TYPE);
    TEST_CAST_ERROR("xxxx", bool, INCORRECT_TYPE);
    TEST_CAST_ERROR("xxxx", int64, INCORRECT_TYPE);
    TEST_CAST_ERROR("xxxx", uint64, INCORRECT_TYPE);
    TEST_CAST_ERROR("xxxx", double, INCORRECT_TYPE);
    TEST_CAST_ERROR("xxxx", string, INCORRECT_TYPE);
    TEST_CAST_ERROR("xxxx", raw_json_string, INCORRECT_TYPE);
    ONDEMAND_SUBTEST("xxxx.is_null", "xxxx", !doc.is_null());
    ONDEMAND_SUBTEST(R"({"a":xxxx}.is_null)", R"({"a":xxxx})", !doc["a"].is_null());
    TEST_SUCCEED();
  }
  bool run() {
    return
          wrong_type_1() &&
          wrong_type_array() &&
          wrong_type_false() &&
          wrong_type_float() &&
          wrong_type_int64_overflow() &&
          wrong_type_negative_1() &&
          wrong_type_negative_int64_overflow() &&
          wrong_type_null() &&
          wrong_type_object() &&
          wrong_type_true() &&
          wrong_type_uint64_overflow() &&

          wrong_type_nulll() &&
          wrong_type_n() &&
          wrong_type_null_padding() &&
          wrong_type_truee() &&
          wrong_type_t() &&
          wrong_type_true_padding() &&
          wrong_type_falsee() &&
          wrong_type_fals() &&
          wrong_type_false_padding() &&
          wrong_type_f() &&
          wrong_type_x() &&
          wrong_type_xxxx() &&
          true;
  }

} // namespace wrong_type_error_tests

int main(int argc, char *argv[]) {
  return test_main(argc, argv, wrong_type_error_tests::run);
}
