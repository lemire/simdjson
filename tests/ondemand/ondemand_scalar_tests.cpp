#include "simdjson.h"
#include "test_ondemand.h"

using namespace simdjson;

namespace scalar_tests {
  using namespace std;
  using simdjson::ondemand::json_type;

  template<typename T> json_type expected_json_type();
  template<> json_type expected_json_type<std::string_view>() { return json_type::string; }
  template<> json_type expected_json_type<double>() { return json_type::number; }
  template<> json_type expected_json_type<uint64_t>() { return json_type::number; }
  template<> json_type expected_json_type<int64_t>() { return json_type::number; }
  template<> json_type expected_json_type<bool>() { return json_type::boolean; }

  template<typename T>
  bool test_scalar_value(std::string_view json, const T &expected, bool test_twice=true) {
    std::cout << "- JSON: " << json << endl;
    SUBTEST( "simdjson_result<document>", test_ondemand_doc(json, [&](auto doc_result) {
      T actual;
      ASSERT_RESULT( doc_result.type(), expected_json_type<T>() );
      ASSERT_SUCCESS( doc_result.get(actual) );
      ASSERT_EQUAL( actual, expected );
      // Test it twice (scalars can be retrieved more than once)
      if (test_twice) {
        ASSERT_SUCCESS( doc_result.get(actual) );
        ASSERT_EQUAL( actual, expected );
        ASSERT_RESULT( doc_result.type(), expected_json_type<T>() );
      }
      return true;
    }));
    SUBTEST( "document", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::document doc;
      ASSERT_SUCCESS( std::move(doc_result).get(doc) );
      T actual;
      ASSERT_RESULT( doc.type(), expected_json_type<T>() );
      ASSERT_SUCCESS( doc.get(actual) );
      ASSERT_EQUAL( actual, expected );
      // Test it twice (scalars can be retrieved more than once)
      if (test_twice) {
        ASSERT_SUCCESS( doc.get(actual) );
        ASSERT_EQUAL( actual, expected );
        ASSERT_RESULT( doc.type(), expected_json_type<T>() );
      }
      return true;
    }));

    {
      auto whitespace_json = std::string(json) + " ";
      std::cout << "- JSON: " << whitespace_json << endl;
      SUBTEST( "simdjson_result<document>", test_ondemand_doc(whitespace_json, [&](auto doc_result) {
        T actual;
        ASSERT_RESULT( doc_result.type(), expected_json_type<T>() );
        ASSERT_SUCCESS( doc_result.get(actual) );
        ASSERT_EQUAL( actual, expected );
        // Test it twice (scalars can be retrieved more than once)
        if (test_twice) {
          ASSERT_SUCCESS( doc_result.get(actual) );
          ASSERT_EQUAL( actual, expected );
          ASSERT_RESULT( doc_result.type(), expected_json_type<T>() );
        }
        return true;
      }));
      SUBTEST( "document", test_ondemand_doc(whitespace_json, [&](auto doc_result) {
        ondemand::document doc;
        ASSERT_SUCCESS( std::move(doc_result).get(doc) );
        T actual;
        ASSERT_RESULT( doc.type(), expected_json_type<T>() );
        ASSERT_SUCCESS( doc.get(actual) );
        ASSERT_EQUAL( actual, expected );
        // Test it twice (scalars can be retrieved more than once)
        if (test_twice) {
          ASSERT_SUCCESS( doc.get(actual) );
          ASSERT_EQUAL( actual, expected );
          ASSERT_RESULT( doc.type(), expected_json_type<T>() );
        }
        return true;
      }));
    }

    {
      auto array_json = std::string("[") + std::string(json) + "]";
      std::cout << "- JSON: " << array_json << endl;
      SUBTEST( "simdjson_result<value>", test_ondemand_doc(array_json, [&](auto doc_result) {
        int count = 0;
        for (simdjson_result<ondemand::value> val_result : doc_result) {
          T actual;
          ASSERT_RESULT( val_result.type(), expected_json_type<T>() );
          ASSERT_SUCCESS( val_result.get(actual) );
          ASSERT_EQUAL(actual, expected);
          // Test it twice (scalars can be retrieved more than once)
          if (test_twice) {
            ASSERT_SUCCESS( val_result.get(actual) );
            ASSERT_EQUAL(actual, expected);
            ASSERT_RESULT( val_result.type(), expected_json_type<T>() );
          }
          count++;
        }
        ASSERT_EQUAL(count, 1);
        return true;
      }));
      SUBTEST( "value", test_ondemand_doc(array_json, [&](auto doc_result) {
        int count = 0;
        for (simdjson_result<ondemand::value> val_result : doc_result) {
          ondemand::value val;
          ASSERT_SUCCESS( val_result.get(val) );
          T actual;
          ASSERT_RESULT( val.type(), expected_json_type<T>() );
          ASSERT_SUCCESS( val.get(actual) );
          ASSERT_EQUAL(actual, expected);
          // Test it twice (scalars can be retrieved more than once)
          if (test_twice) {
            ASSERT_SUCCESS( val.get(actual) );
            ASSERT_EQUAL(actual, expected);
            ASSERT_RESULT( val.type(), expected_json_type<T>() );
          }
          count++;
        }
        ASSERT_EQUAL(count, 1);
        return true;
      }));
    }

    {
      auto whitespace_array_json = std::string("[") + std::string(json) + " ]";
      std::cout << "- JSON: " << whitespace_array_json << endl;

      SUBTEST( "simdjson_result<value>", test_ondemand_doc(whitespace_array_json, [&](auto doc_result) {
        int count = 0;
        for (simdjson_result<ondemand::value> val_result : doc_result) {
          T actual;
          ASSERT_RESULT( val_result.type(), expected_json_type<T>() );
          ASSERT_SUCCESS( val_result.get(actual) );
          ASSERT_EQUAL(actual, expected);
          // Test it twice (scalars can be retrieved more than once)
          if (test_twice) {
            ASSERT_SUCCESS( val_result.get(actual) );
            ASSERT_EQUAL(actual, expected);
            ASSERT_RESULT( val_result.type(), expected_json_type<T>() );
          }
          count++;
        }
        ASSERT_EQUAL(count, 1);
        return true;
      }));

      SUBTEST( "value", test_ondemand_doc(whitespace_array_json, [&](auto doc_result) {
        int count = 0;
        for (simdjson_result<ondemand::value> val_result : doc_result) {
          ondemand::value val;
          ASSERT_SUCCESS( val_result.get(val) );
          T actual;
          ASSERT_RESULT( val.type(), expected_json_type<T>() );
          ASSERT_SUCCESS( val.get(actual) );
          ASSERT_EQUAL(actual, expected);
          // Test it twice (scalars can be retrieved more than once)
          if (test_twice) {
            ASSERT_SUCCESS( val.get(actual) );
            ASSERT_EQUAL(actual, expected);
            ASSERT_RESULT( val_result.type(), expected_json_type<T>() );
          }
          count++;
        }
        ASSERT_EQUAL(count, 1);
        return true;
      }));
    }

    TEST_SUCCEED();
  }

  bool string_value() {
    TEST_START();
    // We can't retrieve a small string twice because it will blow out the string buffer
    if (!test_scalar_value(std::string_view(R"("hi")"), std::string_view("hi"), false)) { return false; }
    // ... unless the document is big enough to have a big string buffer :)
    if (!test_scalar_value(std::string_view(R"("hi"        )"), std::string_view("hi"))) { return false; }
    TEST_SUCCEED();
  }

  bool numeric_values() {
    TEST_START();
    if (!test_scalar_value<int64_t> (std::string_view("0"),   0)) { return false; }
    if (!test_scalar_value<uint64_t>(std::string_view("0"),   0)) { return false; }
    if (!test_scalar_value<double>  (std::string_view("0"),   0)) { return false; }
    if (!test_scalar_value<int64_t> (std::string_view("1"),   1)) { return false; }
    if (!test_scalar_value<uint64_t>(std::string_view("1"),   1)) { return false; }
    if (!test_scalar_value<double>  (std::string_view("1"),   1)) { return false; }
    if (!test_scalar_value<int64_t> (std::string_view("-1"),  -1)) { return false; }
    if (!test_scalar_value<double>  (std::string_view("-1"),  -1)) { return false; }
    if (!test_scalar_value<double>  (std::string_view("1.1"), 1.1)) { return false; }
    TEST_SUCCEED();
  }

  bool boolean_values() {
    TEST_START();
    if (!test_scalar_value<bool> (std::string_view("true"),  true)) { return false; }
    if (!test_scalar_value<bool> (std::string_view("false"), false)) { return false; }
    TEST_SUCCEED();
  }

  bool null_value() {
    TEST_START();
    auto json = std::string_view("null");
    SUBTEST("ondemand::document", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::document doc;
      ASSERT_SUCCESS( std::move(doc_result).get(doc) );
      ASSERT_EQUAL( doc.is_null(), true );
      return true;
    }));
    SUBTEST("simdjson_result<ondemand::document>", test_ondemand_doc(json, [&](auto doc_result) {
      ASSERT_EQUAL( doc_result.is_null(), true );
      return true;
    }));
    json = std::string_view("[null]");
    SUBTEST("ondemand::value", test_ondemand_doc(json, [&](auto doc_result) {
      int count = 0;
      for (auto value_result : doc_result) {
        ondemand::value value;
        ASSERT_SUCCESS( value_result.get(value) );
        ASSERT_EQUAL( value.is_null(), true );
        count++;
      }
      ASSERT_EQUAL( count, 1 );
      return true;
    }));
    SUBTEST("simdjson_result<ondemand::value>", test_ondemand_doc(json, [&](auto doc_result) {
      int count = 0;
      for (auto value_result : doc_result) {
        ASSERT_EQUAL( value_result.is_null(), true );
        count++;
      }
      ASSERT_EQUAL( count, 1 );
      return true;
    }));
    return true;
  }

#if SIMDJSON_EXCEPTIONS

  template<typename T>
  bool test_scalar_value_exception(std::string_view json, const T &expected) {
    std::cout << "- JSON: " << json << endl;
    SUBTEST( "document", test_ondemand_doc(json, [&](auto doc_result) {
      ondemand::document doc;
      ASSERT_SUCCESS( std::move(doc_result).get(doc) );
      ASSERT_EQUAL( expected, T(doc) );
      return true;
    }));
    auto array_json = std::string("[") + std::string(json) + "]";
    std::cout << "- JSON: " << array_json << endl;
    SUBTEST( "value", test_ondemand_doc(array_json, [&](auto doc_result) {
      int count = 0;
      for (T actual : doc_result) {
        ASSERT_EQUAL( actual, expected );
        count++;
      }
      ASSERT_EQUAL(count, 1);
      return true;
    }));
    TEST_SUCCEED();
  }
  bool string_value_exception() {
    TEST_START();
    return test_scalar_value_exception(std::string_view(R"("hi")"), std::string_view("hi"));
  }

  bool numeric_values_exception() {
    TEST_START();
    if (!test_scalar_value_exception<int64_t> (std::string_view("0"),   0)) { return false; }
    if (!test_scalar_value_exception<uint64_t>(std::string_view("0"),   0)) { return false; }
    if (!test_scalar_value_exception<double>  (std::string_view("0"),   0)) { return false; }
    if (!test_scalar_value_exception<int64_t> (std::string_view("1"),   1)) { return false; }
    if (!test_scalar_value_exception<uint64_t>(std::string_view("1"),   1)) { return false; }
    if (!test_scalar_value_exception<double>  (std::string_view("1"),   1)) { return false; }
    if (!test_scalar_value_exception<int64_t> (std::string_view("-1"),  -1)) { return false; }
    if (!test_scalar_value_exception<double>  (std::string_view("-1"),  -1)) { return false; }
    if (!test_scalar_value_exception<double>  (std::string_view("1.1"), 1.1)) { return false; }
    TEST_SUCCEED();
  }

  bool boolean_values_exception() {
    TEST_START();
    if (!test_scalar_value_exception<bool> (std::string_view("true"),  true)) { return false; }
    if (!test_scalar_value_exception<bool> (std::string_view("false"), false)) { return false; }
    TEST_SUCCEED();
  }

#endif // SIMDJSON_EXCEPTIONS

  bool run() {
    return
           string_value() &&
           numeric_values() &&
           boolean_values() &&
           null_value() &&
#if SIMDJSON_EXCEPTIONS
           string_value_exception() &&
           numeric_values_exception() &&
           boolean_values_exception() &&
#endif // SIMDJSON_EXCEPTIONS
           true;
  }

} // namespace scalar_tests

int main(int argc, char *argv[]) {
  return test_main(argc, argv, scalar_tests::run);
}
