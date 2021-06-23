#ifndef TEST_MACROS_H
#define TEST_MACROS_H

#ifndef SIMDJSON_BENCHMARK_DATA_DIR
#define SIMDJSON_BENCHMARK_DATA_DIR "jsonexamples/"
#endif
const char *TWITTER_JSON = SIMDJSON_BENCHMARK_DATA_DIR "twitter.json";
const char *TWITTER_TIMELINE_JSON = SIMDJSON_BENCHMARK_DATA_DIR "twitter_timeline.json";
const char *REPEAT_JSON = SIMDJSON_BENCHMARK_DATA_DIR "repeat.json";
const char *CANADA_JSON = SIMDJSON_BENCHMARK_DATA_DIR "canada.json";
const char *MESH_JSON = SIMDJSON_BENCHMARK_DATA_DIR "mesh.json";
const char *APACHE_JSON = SIMDJSON_BENCHMARK_DATA_DIR "apache_builds.json";
const char *GSOC_JSON = SIMDJSON_BENCHMARK_DATA_DIR "gsoc-2018.json";

const char *AMAZON_CELLPHONES_NDJSON = SIMDJSON_BENCHMARK_DATA_DIR "amazon_cellphones.ndjson";

#define SIMDJSON_BENCHMARK_SMALLDATA_DIR SIMDJSON_BENCHMARK_DATA_DIR "small/"

const char *ADVERSARIAL_JSON = SIMDJSON_BENCHMARK_SMALLDATA_DIR  "adversarial.json";
const char *FLATADVERSARIAL_JSON = SIMDJSON_BENCHMARK_SMALLDATA_DIR  "flatadversarial.json";
const char *DEMO_JSON = SIMDJSON_BENCHMARK_SMALLDATA_DIR  "demo.json";
const char *SMALLDEMO_JSON = SIMDJSON_BENCHMARK_SMALLDATA_DIR  "smalldemo.json";
const char *TRUENULL_JSON = SIMDJSON_BENCHMARK_SMALLDATA_DIR  "truenull.json";

// For the ASSERT_EQUAL macro
template<typename A, typename E>
simdjson_really_inline bool equals_expected(A actual, E expected) {
  return actual == A(expected);
}
template<>
simdjson_really_inline bool equals_expected<const char *, const char *>(const char *actual, const char *expected) {
  return !strcmp(actual, expected);
}
template<>
simdjson_really_inline bool equals_expected<simdjson::ondemand::raw_json_string, const char *>(simdjson::ondemand::raw_json_string actual, const char * expected) {
  return actual == expected;
}

simdjson_really_inline simdjson::error_code to_error_code(simdjson::error_code error) {
  return error;
}
template<typename T>
simdjson_really_inline simdjson::error_code to_error_code(const simdjson::simdjson_result<T> &result) {
  return result.error();
}

template<typename T>
simdjson_really_inline bool assert_success(const T &actual, const char *operation = "result") {
  simdjson::error_code error = to_error_code(actual);
  if (error) {
    std::cerr << "FAIL: " << operation << " returned error: " << error << " (" << int(error) << ")" << std::endl;
    return false;
  }
  return true;
}
template<typename A, typename E=A>
simdjson_really_inline bool assert_equal(const A &actual, const E &expected, const char *operation = "result") {
  if (!equals_expected(actual, expected)) {
    std::cerr << "FAIL: " << operation << " returned " << actual << " (expected " << expected << ")" << std::flush;
    std::cerr << std::endl;
    return false;
  }
  return true;
}
template<typename T>
simdjson_really_inline bool assert_error(const T &actual_result, simdjson::error_code expected, const char *operation = "result") {
  simdjson::error_code actual = to_error_code(actual_result);
  if (actual != expected) {
    if (actual) {
      std::cerr << "FAIL: " << operation << " failed with error \"" << actual << "\"";
    } else {
      std::cerr << "FAIL: " << operation << " succeeded";
    }
    std::cerr << " (expected error \"" << expected << "\")" << std::endl;
    return false;
  }
  return true;
}
template<typename A, typename E>
simdjson_really_inline bool assert_result(simdjson::simdjson_result<A> &&actual_result, const E &expected, const char *operation = "result") {
  E actual;
  return assert_success(std::forward<simdjson::simdjson_result<A>>(actual_result).get(actual))
      && assert_equal(actual, expected, operation);
}
simdjson_really_inline bool assert_true(bool value, const char *operation = "result") {
  if (!value) {
    std::cerr << "FAIL: " << operation << " was false!" << std::endl;
    return false;
  }
  return true;
}
simdjson_really_inline bool assert_false(bool value, const char *operation = "result") {
  if (value) {
    std::cerr << "FAIL: " << operation << " was true (expected it to be false)!" << std::endl;
    return false;
  }
  return true;
}
template<typename T>
simdjson_really_inline bool assert_iterate_error(T &arr, simdjson::error_code expected, const char *operation = "result") {
  int count = 0;
  for (auto element : arr) {
    count++;
    if (!assert_error( element, expected, operation )) {
      return false;
    }
  }
  return assert_equal( count, 1, operation );
}
#define TEST_START()                    do { std::cout << "> Running " << __func__ << " ..." << std::endl; } while(0);
#define SUBTEST(NAME, TEST)             do { std::cout << " - Subtest " << (NAME) << " ..." << std::endl; if (!(TEST)) { return false; } } while (0);
#define ASSERT_EQUAL(ACTUAL, EXPECTED)  do { if (!::assert_equal  ((ACTUAL), (EXPECTED), #ACTUAL)) { return false; } } while (0);
#define ASSERT_RESULT(ACTUAL, EXPECTED) do { if (!::assert_result ((ACTUAL), (EXPECTED), #ACTUAL)) { return false; } } while (0);
#define ASSERT_SUCCESS(ACTUAL)          do { if (!::assert_success((ACTUAL),             #ACTUAL)) { return false; } } while (0);
#define ASSERT_ERROR(ACTUAL, EXPECTED)  do { if (!::assert_error  ((ACTUAL), (EXPECTED), #ACTUAL)) { return false; } } while (0);
#define ASSERT_TRUE(ACTUAL)             do { if (!::assert_true   ((ACTUAL),             #ACTUAL)) { return false; } } while (0);
#define ASSERT_FALSE(ACTUAL)            do { if (!::assert_false  ((ACTUAL),             #ACTUAL)) { return false; } } while (0);
#define ASSERT(ACTUAL, MESSAGE)         do { if (!::assert_true   ((ACTUAL),           (MESSAGE))) { return false; } } while (0);
#define ASSERT_ITERATE_ERROR(ACTUAL, EXPECTED)  do { if (!::assert_iterate_error((ACTUAL), (EXPECTED), #ACTUAL)) { return false; } } while (0);
#define RUN_TEST(ACTUAL)                do { if (!(ACTUAL)) { return false; } } while (0);
#define TEST_FAIL(MESSAGE)              do { std::cerr << "FAIL: " << (MESSAGE) << std::endl; return false; } while (0);
#define TEST_SUCCEED()                  do { return true; } while (0);

#endif // TEST_MACROS_H