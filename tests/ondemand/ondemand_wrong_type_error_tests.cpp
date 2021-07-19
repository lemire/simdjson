#include "simdjson.h"
#include "test_ondemand.h"

using namespace simdjson;

namespace wrong_type_error_tests {
  using namespace std;

#define TEST_CAST_ERROR(JSON, TYPE, ERROR) \
  std::cout << "- Subtest: get_" << (#TYPE) << "() - JSON: " << (JSON) << std::endl; \
  if (!test_ondemand_doc((JSON##_padded), [&](auto doc_result) { \
    ASSERT_ERROR( doc_result.get_##TYPE(), (ERROR) ); \
    return true; \
  })) { \
    return false; \
  } \
  { \
    padded_string a_json(std::string(R"({ "a": )") + JSON + " }"); \
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
          true;
  }

} // namespace wrong_type_error_tests

int main(int argc, char *argv[]) {
  return test_main(argc, argv, wrong_type_error_tests::run);
}
