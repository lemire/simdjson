#include <array>
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <string_view>

#ifdef _MSC_VER
#define SIMDJSON_FUNC_NAME __FUNCSIG__
#else
#ifndef __PRETTY_FUNCTION__
#define SIMDJSON_FUNC_NAME __func__
#else
#define SIMDJSON_FUNC_NAME __PRETTY_FUNCTION__
#endif
#endif // _MSC_VER

#define RETURN_ON_FALSE(x_, msg_) \
do {                                                                         \
    if (false == (x_)) {                                                     \
      fprintf(stderr, "\n " SIMDJSON_FUNC_NAME " -- %s", msg_);              \
      return false;                                                          \
    }                                                                        \
  } while (0)

namespace simdjson::lib {
using namespace std;

constexpr size_t max_native_stack_char_array_size{ BUFSIZ * 4 };

// VS2017 std lib or other non conformant std libs
// are C++17 but not having string_view literals
inline namespace literals {
constexpr string_view operator"" _sv(const char *_Str, size_t _Len) noexcept {
  return string_view(_Str, _Len);
}

constexpr wstring_view operator"" _sv(const wchar_t *_Str,
                                      size_t _Len) noexcept {
  return wstring_view(_Str, _Len);
}

#ifdef __cpp_char8_t
constexpr basic_string_view<char8_t> operator"" _sv(const char8_t *_Str,
                                                    size_t _Len) noexcept {
  return basic_string_view<char8_t>(_Str, _Len);
}
#endif // __cpp_char8_t

constexpr u16string_view operator"" _sv(const char16_t *_Str,
                                        size_t _Len) noexcept {
  return u16string_view(_Str, _Len);
}

constexpr u32string_view operator"" _sv(const char32_t *_Str,
                                        size_t _Len) noexcept {
  return u32string_view(_Str, _Len);
}
} // namespace literals

/*
----------------------------------------------------------------------------
compile time string equal, returns true or false

static_assert( simdjson::lib::str_equal("A", "A") ) ;

*/
constexpr inline bool str_equal(char const *lhs, char const *rhs) noexcept {
  while (*lhs || *rhs)
    if (*lhs++ != *rhs++)
      return false;
  return true;
}

/*
----------------------------------------------------------------------------
Return string literal copy with white spaces removed.

    1. This is compile time function.
       It uses stack space, thus be carefull of overdose

    What is returned is std::array, thus:

    2. result size equals the input size
    3. result strlen is less than it's size
    4. on all white spaces input result strlen is 0

Synopsis:

    constexpr auto clean_ = simdjson::lib::remove_ws("A \t\v\rB\n C");
    printf("\n%s\n", clean_.data());

*/
template <size_t N> constexpr auto remove_ws(const char (&literal_)[N]) {

    static_assert(N < max_native_stack_char_array_size);

  array<char, N> rezult_{{}}; // zero it
  size_t rezult_idx_{};

  for (size_t j{}; j < N; ++j) {
    switch (literal_[j]) {
    case ' ':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
      continue;
      break;
    default:
      rezult_[rezult_idx_++] = literal_[j];
    };
  }
  return rezult_;
}

//----------------------------------------------------------------------------
// ulp distance
// Marc B. Reynolds, 2016-2019
// Public Domain under http://unlicense.org, see link for details.
// adapted by D. Lemire
inline uint64_t f64_ulp_dist(double a, double b) {
  uint64_t ua, ub;
  memcpy(&ua, &a, sizeof(ua));
  memcpy(&ub, &b, sizeof(ub));
  if ((int64_t)(ub ^ ua) >= 0)
    return (int64_t)(ua - ub) >= 0 ? (ua - ub) : (ub - ua);
  return ua + ub + 0x80000000;
}

/*
//----------------------------------------------------------------------------
char only safe simdjson::lib::s_printf version

note: it is not allowed to use protected names even if they are in namespaces
thus we can not use 'simdjson::lib::s_printf' here

returns int by the same logic as simdjson::lib::s_printf()
thus on error -1 is returned amnd errno is set

   char buf[0xFF]{};
   simdjson::lib::s_printf( buf, "%s%s%s", "A", "B", "C") ;
   assert( simdjson::lib::str_equal( "ABC", buf) ) ;
*/
template <size_t N, typename... A>
inline int s_printf(char (&buf_)[N], const char *fmt_, A... args_) {

  // check if buf sent is big enough
  int check_ = std::snprintf(nullptr, 0, fmt_, args_...);

  if (check_ >= N) {
    errno = EINVAL;
    perror(__FILE__ " buffer sent to s_printf is not large enough ");
    return -1;
  }

  // +1 for terminator
  // std::snprintf() is always zero terminating
  check_ = std::snprintf(buf_, N + 1, fmt_, args_...);

  if (check_ < 0) {
    errno = EINVAL;
    perror(__FILE__ " std::snprintf failed ");
    return -1;
  }
  return check_;
}

} // namespace simdjson::lib

#include "simdjson/jsonparser.h"
#include "simdjson/jsonstream.h"

// -------------------------------------------------------------------------------------------------

bool number_test_small_integers() {

  char buf[1024]{};
  simdjson::ParsedJson pj{};

  using simdjson::lib::s_printf;

  if (!pj.allocate_capacity(1024)) {
    printf("allocation failure in number_test_small_integers\n");
    return false;
  }
  for (int m = 10; m < 20; m++) {
    for (int i = -1024; i < 1024; i++) {

      auto n = s_printf(buf, "%*d", m, i);

      auto ok1 = json_parse(buf, n, pj);
      if (ok1 != 0 || !pj.is_valid()) {
        printf("Could not parse: %s.\n", buf);
        return false;
      }
      simdjson::ParsedJson::Iterator pjh(pj);

      RETURN_ON_FALSE (pjh.is_number(), "Root should be number");

      RETURN_ON_FALSE (pjh.is_integer(), "Root should be an integer");

      int64_t x = pjh.get_integer();
      if (x != i) {
        printf("failed to parse %s. \n", buf);
        return false;
      }
    }
  }
  printf("Small integers can be parsed.\n");
  return true;
}

// -------------------------------------------------------------------------------------------------

bool number_test_powers_of_two() {

  using simdjson::lib::s_printf;
  char buf[1024]{};
  simdjson::ParsedJson pj;

  RETURN_ON_FALSE (pj.allocate_capacity(1024), "allocation failure in number_test");

  int maxulp = 0;
  for (int i = -1075; i < 1024; ++i) { // large negative values should be zero.
    double expected = pow(2, i);
    auto n = s_printf(buf, "%.*e",
                      std::numeric_limits<double>::max_digits10 - 1, expected);

    auto ok1 = json_parse(buf, n, pj);
    if (ok1 != 0 || !pj.is_valid()) {
      printf("Could not parse: %s.\n", buf);
      return false;
    }
    simdjson::ParsedJson::Iterator pjh(pj);

    RETURN_ON_FALSE(pjh.is_number(), "Root should be number");

    if (pjh.is_integer()) {
      int64_t x = pjh.get_integer();
      int power = 0;
      while (x > 1) {
        if ((x % 2) != 0) {
          printf("failed to parse %s. \n", buf);
          return false;
        }
        x = x / 2;
        power++;
      }
      if (power != i) {
        printf("failed to parse %s. \n", buf);
        return false;
      }
    } else if (pjh.is_unsigned_integer()) {
      uint64_t x = pjh.get_unsigned_integer();
      int power = 0;
      while (x > 1) {
        if ((x % 2) != 0) {
          printf("failed to parse %s. \n", buf);
          return false;
        }
        x = x / 2;
        power++;
      }
      if (power != i) {
        printf("failed to parse %s. \n", buf);
        return false;
      }
    } else {
      double x = pjh.get_double();
      int ulp = simdjson::lib::f64_ulp_dist(x, expected);
      if (ulp > maxulp)
        maxulp = ulp;
      if (ulp > 3) {
        printf("failed to parse %s. ULP = %d i = %d \n", buf, ulp, i);
        return false;
      }
    }
  }
  printf("Powers of 2 can be parsed, maxulp = %d.\n", maxulp);
  return true;
}

// -------------------------------------------------------------------------------------------------

bool number_test_powers_of_ten() {
  char buf[1024]{};
  simdjson::ParsedJson pj;

  RETURN_ON_FALSE(pj.allocate_capacity(1024), "allocation failure in number_test");

  for (int i = -1000000; i <= 308;
       ++i) { // large negative values should be zero.
    auto n = simdjson::lib::s_printf(buf, "1e%d", i);

    auto ok1 = json_parse(buf, n, pj);
    if (ok1 != 0 || !pj.is_valid()) {
      printf("Could not parse: %s.\n", buf);
      return false;
    }
    simdjson::ParsedJson::Iterator pjh(pj);
    if (!pjh.is_number()) {
      printf("Root should be number\n");
      return false;
    }
    if (pjh.is_integer()) {
      int64_t x = pjh.get_integer();
      int power = 0;
      while (x > 1) {
        if ((x % 10) != 0) {
          printf("failed to parse %s. \n", buf);
          return false;
        }
        x = x / 10;
        power++;
      }
      if (power != i) {
        printf("failed to parse %s. \n", buf);
        return false;
      }
    } else if (pjh.is_unsigned_integer()) {
      uint64_t x = pjh.get_unsigned_integer();
      int power = 0;
      while (x > 1) {
        if ((x % 10) != 0) {
          printf("failed to parse %s. \n", buf);
          return false;
        }
        x = x / 10;
        power++;
      }
      if (power != i) {
        printf("failed to parse %s. \n", buf);
        return false;
      }
    } else {
      double x = pjh.get_double();
      double expected = std::pow(10, i);
      int ulp = (int)simdjson::lib::f64_ulp_dist(x, expected);
      if (ulp > 1) {
        printf("failed to parse %s. \n", buf);
        printf("actual: %.20g expected: %.20g \n", x, expected);
        printf("ULP: %d \n", ulp);
        return false;
      }
    }
  }
  printf("Powers of 10 can be parsed.\n");
  return true;
}

// -------------------------------------------------------------------------------------------------
// adversarial example that once triggred overruns, see
// https://github.com/lemire/simdjson/issues/345
bool bad_example() {

  using namespace std;
  using namespace simdjson::lib::literals;

  constexpr auto badjson =
      "[7,7,7,7,6,7,7,7,6,7,7,6,[7,7,7,7,6,7,7,7,6,7,7,6,7,7,7,7,7,7,6"_sv;

  simdjson::ParsedJson pj = simdjson::build_parsed_json(badjson);
  if (pj.is_valid()) {
    printf("This json should not be valid %s.\n", badjson.data());
    return false;
  }
  return true;
}
// -------------------------------------------------------------------------------------------------
// returns true if successful
bool stable_test() {

   using namespace std;
  using namespace simdjson::lib::literals;

  constexpr auto json =
      R"({"Image":{"Width":800,"Height":600,"Title":"Viewfromthe15thFloor","Thumbnail":{"Url":"http://www.example.com/image/481989943","Height":125,"Width":100},"Animated":false,"IDs":[116,943.3,234,38793]}})"_sv;

  simdjson::ParsedJson pj = simdjson::build_parsed_json(json);
  ostringstream myStream{};

  RETURN_ON_FALSE(pj.print_json(myStream), "cannot print it out? ");

  string newjson = myStream.str();
  
  RETURN_ON_FALSE(simdjson::lib::str_equal(json.data(), newjson.data()),
                  "nSerialized json differs from the input!");
  return true;
}

// -------------------------------------------------------------------------------------------------

// returns true if successful
bool navigate_test() {
  using namespace simdjson::lib::literals;
  constexpr auto json =
      R"({
      "Image": {
      "Width":  800,
      "Height": 600,
      "Title":  "View from 15th Floor",
      "Thumbnail": {
          "Url":    "http://www.example.com/image/481989943",
          "Height": 125,
          "Width":  100
      },
      "Animated" : false,
      "IDs": [116, 943, 234, 38793]
      }
})"_sv;

  simdjson::ParsedJson pj =
      simdjson::build_parsed_json(json.data(), json.size());

  if (!pj.is_valid()) {
    printf("Something is wrong in navigate: %s.\n", json.data());
    return false;
  }
  simdjson::ParsedJson::Iterator pjh(pj);

  RETURN_ON_FALSE( pjh.is_object(), "Root should be object\n");
  
  RETURN_ON_FALSE( ! pjh.move_to_key("bad key"), "We should not move to a non-existing key");

  RETURN_ON_FALSE(pjh.is_object(), "We should have remained at the object.");

  RETURN_ON_FALSE (! pjh.move_to_key_insensitive("bad key"), "We should not move to a non-existing key");
  
  RETURN_ON_FALSE( pjh.is_object(), "We should have remained at the object.");
  
  RETURN_ON_FALSE( ! pjh.move_to_key("bad key", 7), "We should not move to a non-existing key");

  RETURN_ON_FALSE(pjh.is_object(), "We should have remained at the object.");

  RETURN_ON_FALSE (pjh.down(), "Root should not be emtpy");

  RETURN_ON_FALSE( pjh.is_string(), "Object should start with string key\n");

  RETURN_ON_FALSE( ! pjh.prev(), "We should not be able to go back from the start of the scope.");

  RETURN_ON_FALSE (simdjson::lib::str_equal(pjh.get_string(), "Image"), "There should be a single key, image.");

  pjh.move_to_value();

  RETURN_ON_FALSE (pjh.is_object(), "Value of image should be object\n");

  RETURN_ON_FALSE (pjh.down(), "Image key should not be emtpy");

  RETURN_ON_FALSE(pjh.next(), "key should have a value");
  
  RETURN_ON_FALSE (pjh.prev(), "We should go back to the key.");

  if (! simdjson::lib::str_equal(pjh.get_string(), "Width") ) {
    printf("There should be a  key Width.\n");
    return false;
  }
  
  RETURN_ON_FALSE(pjh.up(), "Could not move up");

  RETURN_ON_FALSE(pjh.move_to_key("IDs"), "We should be able to move to an existing key");
  
  if (!pjh.is_array()) {
    printf("Value of IDs should be array, it is %c \n", pjh.get_type());
    return false;
  }

  RETURN_ON_FALSE( ! pjh.move_to_index(4), "We should not be able to move to a non-existing index\n");

  RETURN_ON_FALSE(pjh.is_array(), "We should have remained at the array");

  return true;
}

// -------------------------------------------------------------------------------------------------
bool stream_utf8_test() {

  const size_t n_records = 10000;
  std::string data{};
  char buf[1024]{};

  for (size_t i = 0; i < n_records; ++i) {
    auto n = simdjson::lib::s_printf(
        buf,
        R"({"id": %zu, "name": "name%zu", "gender": "%s", "été": {"id": %zu, "name": "éventail%zu"}})",
        i, i, (i % 2) ? "⺃" : "⺕", i % 10, i % 10);
    data += std::string(buf, n);
  }

  for (size_t i = 1000; i < 2000; i += (i > 1050 ? 10 : 1)) {
    
      printf(".");

    simdjson::JsonStream js{data.c_str(), data.size(), i};
    int parse_res = simdjson::SUCCESS_AND_HAS_MORE;
    size_t count = 0;
    simdjson::ParsedJson pj{};

    while (parse_res == simdjson::SUCCESS_AND_HAS_MORE) {
      parse_res = js.json_parse(pj);
      simdjson::ParsedJson::Iterator pjh(pj);
      
      RETURN_ON_FALSE (pjh.is_object(), "Root should be object");

      RETURN_ON_FALSE(pjh.down(), "Root should not be emtpy");

      RETURN_ON_FALSE( pjh.is_string(), "Object should start with string key");

      if (strcmp(pjh.get_string(), "id") != 0) {
        printf("There should a single key, id.\n");
        return false;
      }
      pjh.move_to_value();

      RETURN_ON_FALSE(pjh.is_integer(), "Value of image should be integer");

      int64_t keyid = pjh.get_integer();
      if (keyid != (int64_t)count) {
        printf("key does not match %d, expected %d\n", (int)keyid, (int)count);
        return false;
      }
      count++;
    }
    if (count != n_records) {
      printf("Something is wrong in stream_test at window size = %zu.\n", i);
      return false;
    }
  }
  return true;
}

// -------------------------------------------------------------------------------------------------
// returns true if successful
bool stream_test() {

  const size_t n_records = 10000;
  std::string data{};
  char buf[1024]{};

  // generate the input json
  for (size_t i = 0; i < n_records; ++i) {
    auto n = simdjson::lib::s_printf(
        buf,
        R"({"id": %zu, "name": "name%zu", "gender": "%s","ete": {"id": %zu, "name": "eventail%zu"}})",
        i, i, (i % 2) ? "homme" : "femme", i % 10, i % 10);
    data += std::string(buf, n);
  }

  // main loop
  for (size_t i = 1000; i < 2000; i += (i > 1050 ? 10 : 1)) {
    printf(".");

    simdjson::JsonStream js{data.c_str(), data.size(), i};
    int parse_res = simdjson::SUCCESS_AND_HAS_MORE;
    size_t count{};
    simdjson::ParsedJson pj{};

    while (parse_res == simdjson::SUCCESS_AND_HAS_MORE) {
      parse_res = js.json_parse(pj);
      simdjson::ParsedJson::Iterator pjh(pj);

      RETURN_ON_FALSE(pjh.is_object(), "Root should be object");

      RETURN_ON_FALSE(pjh.down(), "Root should not be emtpy");

      RETURN_ON_FALSE( pjh.is_string(), "Object should start with string key");

      RETURN_ON_FALSE (simdjson::lib::str_equal(pjh.get_string(), "id"),
                      "There should a single key, id.");

      pjh.move_to_value();

      RETURN_ON_FALSE( pjh.is_integer(), "Value of image should be integer");

      int64_t keyid = pjh.get_integer();

      if (keyid != (int64_t)count) {
        printf("key does not match %d, expected %d\n", (int)keyid, (int)count);
        return false;
      }

      count++;
    }
    if (count != n_records) {
      printf("Something is wrong in stream_test at window size = %zu.\n", i);
      return false;
    }
  } // for()
  return true;
}
// -------------------------------------------------------------------------------------------------
// returns true if successful
bool skyprophet_test() {
  using namespace std;
  constexpr size_t n_records = 100000;
  // minimze the need for vector stretch
  vector<string> data{};
  char buf[1024]{}; // zero it on declaration
  // first n records
  for (size_t i = 0; i < n_records; ++i) {
    auto n = simdjson::lib::s_printf(
        buf,
        R"({"id": %zu, "name": "name%zu", "gender": "%s", "school": {"id": %zu, "name": "school%zu"}})",
        i, i, (i % 2) ? "male" : "female", i % 10, i % 10);
    data.emplace_back(string{buf, uint64_t(n)});
  }
  // next n records
  for (size_t i = 0; i < n_records; ++i) {
    auto n = simdjson::lib::s_printf(buf, R"({"counter": %f, "array": ["%s"]})",
                                     i * 3.1416, (i % 2) ? "true" : "false");
    data.emplace_back(string(buf, n));
  }
  // next n records
  // this is 3 * N = 300000 records
  for (size_t i = 0; i < n_records; ++i) {
    auto n =
        simdjson::lib::s_printf(buf, R"({"number": %e})", i * 10000.31321321);
    data.emplace_back(string(buf, n));
  }
  // and 4 more on top of that
  data.emplace_back(string("true"));
  data.emplace_back(string("false"));
  data.emplace_back(string("null"));
  data.emplace_back(string("0.1"));

  //
  size_t maxsize = 0;
  // in essence collect the largest string size
  // now in the vector
  for (auto &s : data) {
    if (maxsize < s.size())
      maxsize = s.size();
  }
  simdjson::ParsedJson pj;

  RETURN_ON_FALSE(pj.allocate_capacity(maxsize), "allocation failure in skyprophet_test\n");
  
  size_t counter = 0;

  for (auto &rec : data) {
    // print the dot for each 10000-th
    if ((counter % 10000) == 0) {
      printf(".");
    }
    counter++;
    auto ok1 = json_parse(rec.c_str(), rec.length(), pj);
    if (ok1 != 0 || !pj.is_valid()) {
      printf("Something is wrong in the skyprophet_test, json: %s\n",
             rec.c_str());
      return false;
    }
    auto ok2 = json_parse(rec, pj);
    if (ok2 != 0 || !pj.is_valid()) {
      printf("Something is wrong in the skyprophet_test, json: %s\n",
             rec.c_str());
      return false;
    }
  }
  return true;
}

/*
-----------------------------------------------------------------------------------------------------------------
*/
#undef RUN
#define RUN(x) runner(x, STRINGIFY(x))

int main(int, char **argv) {

  auto runner = [&](bool (*test_unit)(), const char *prompt_) {
    try {
      fprintf(
          stdout,
          "\n----------------------------------------------\nRunning %s\n\n",
          prompt_);

      if (test_unit())
        printf("\n\nTEST OK\n\n");
      else
        printf("\n\nTEST FAILED\n\n");

    } catch (simdjson::ParsedJson::InvalidJSON &simdjson_x_) {
      fprintf(stderr, "\n\nSIMDJSON Exception caught in %s, message: %s",
              prompt_, simdjson_x_.what());

    } catch (std::exception &std_x_) {
      // simdjson can throw std exceptions or offsprings of it on
      // few ocassions
      fprintf(stderr, "\n\nException caught in %s, message: %s", prompt_,
              std_x_.what());
    } catch (...) {
      fprintf(stderr, "\n\nUnknown Exception caught in %s", prompt_);
    }
  };

  #ifndef NDEBUG
  fprintf(stdout, "\n\nBasic tests -- DEBUG build [%s]\n", __TIMESTAMP__);
  #else
  fprintf(stdout, "\n\nBasic tests -- RELSEASE build [%s]\n", __TIMESTAMP__);
#endif

  RUN(stream_test);
  RUN(stream_utf8_test);
  RUN(number_test_small_integers);
  RUN(stable_test);
  RUN(bad_example);
  RUN(number_test_powers_of_two);
  RUN(number_test_powers_of_ten);
  RUN(navigate_test);
  RUN(skyprophet_test);

  fprintf(stdout, "\n\nBasic tests -- %s\n", "DONE");

  return EXIT_SUCCESS;
}

#undef RUN
