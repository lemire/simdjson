
#include <cassert>
#include <iostream>
#include <limits>

#include "simdjson/jsonparser.h"

using namespace simdjson;

static const std::string make_json(const std::string value) {
  const std::string s = "{\"key\": ";
  return s + value + "}";
}

// e.g. make_json(123) => {"key": 123} as string
template <typename T> static const std::string make_json(T value) {
  return make_json(std::to_string(value));
}

template <typename T>
static void parse_and_validate(const std::string src, T expected) {
  std::cout << "src: " << src << ", ";
  const padded_string pstr{src};
  auto json = build_parsed_json(pstr);

  assert(json.is_valid());
  ParsedJson::Iterator it{json};
  assert(it.down());
  assert(it.next());
  bool result;
  if constexpr (std::is_same<int64_t, T>::value) {
    const auto actual = it.get_integer();
    result = expected == actual;
  } else {
    const auto actual = it.get_unsigned_integer();
    result = expected == actual;
  }
  std::cout << std::boolalpha << "test: " << result << std::endl;
  assert(result);
}

int main() {
  using std::numeric_limits;
  constexpr auto int64_max = numeric_limits<int64_t>::max();
  constexpr auto int64_min = numeric_limits<int64_t>::min();
  constexpr auto uint64_max = numeric_limits<uint64_t>::max();
  constexpr auto uint64_min = numeric_limits<uint64_t>::min();
  parse_and_validate(make_json(int64_max), int64_max);
  parse_and_validate(make_json(int64_min), int64_min);
  parse_and_validate(make_json(uint64_max), uint64_max);
  parse_and_validate(make_json(uint64_min), uint64_min);

  constexpr auto int64_max_plus1 = static_cast<uint64_t>(int64_max) + 1;
  parse_and_validate(make_json(int64_max_plus1), int64_max_plus1);
}

