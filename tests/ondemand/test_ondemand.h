#ifndef ONDEMAND_TEST_ONDEMAND_H
#define ONDEMAND_TEST_ONDEMAND_H

#include <unistd.h>
#include "simdjson.h"
#include "cast_tester.h"
#include "test_macros.h"

template<typename T, typename F>
bool test_ondemand(simdjson::ondemand::parser &parser, std::string_view json, const F& f) {
  auto doc = parser.iterate(json);
  T val;
  ASSERT_SUCCESS( doc.get(val) );
  return f(val);
}
template<typename T, typename F>
bool test_ondemand(std::string_view json, const F& f) {
  simdjson::ondemand::parser parser;
  return test_ondemand<T, F>(parser, json, f);
}

template<typename F>
bool test_ondemand_doc(simdjson::ondemand::parser &parser, std::string_view json, const F& f) {
  return f(parser.iterate(json));
}
template<typename F>
bool test_ondemand_doc(std::string_view json, const F& f) {
  simdjson::ondemand::parser parser;
  return test_ondemand_doc(parser, json, f);
}

#define ONDEMAND_SUBTEST(NAME, JSON, TEST) \
{ \
  std::cout << "- Subtest " << NAME << " - JSON: " << (JSON) << " ..." << std::endl; \
  if (!test_ondemand_doc(std::string_view(JSON), [&](auto doc) { \
    return (TEST); \
  })) { \
    return false; \
  } \
}

const size_t AMAZON_CELLPHONES_NDJSON_DOC_COUNT = 793;
#define SIMDJSON_SHOW_DEFINE(x) printf("%s=%s\n", #x, STRINGIFY(x))

template<typename F>
int test_main(int argc, char *argv[], const F& test_function) {
  std::cout << std::unitbuf;
  int c;
  while ((c = getopt(argc, argv, "a:")) != -1) {
    switch (c) {
    case 'a': {
      const simdjson::implementation *impl = simdjson::available_implementations[optarg];
      if (!impl) {
        std::fprintf(stderr, "Unsupported architecture value -a %s\n", optarg);
        return EXIT_FAILURE;
      }
      simdjson::active_implementation = impl;
      break;
    }
    default:
      std::fprintf(stderr, "Unexpected argument %c\n", c);
      return EXIT_FAILURE;
    }
  }

  // this is put here deliberately to check that the documentation is correct (README),
  // should this fail to compile, you should update the documentation:
  if (simdjson::active_implementation->name() == "unsupported") {
    std::printf("unsupported CPU\n");
    std::abort();
  }
  // We want to know what we are testing.
  // Next line would be the runtime dispatched implementation but that's not necessarily what gets tested.
  // std::cout << "Running tests against this implementation: " << simdjson::active_implementation->name();
  // Rather, we want to display builtin_implementation()->name().
  // In practice, by default, we often end up testing against fallback.
  std::cout << "builtin_implementation -- " << simdjson::builtin_implementation()->name() << std::endl;
  std::cout << "------------------------------------------------------------" << std::endl;

  std::cout << "Running tests." << std::endl;
  if (test_function()) {
    std::cout << "Success!" << std::endl;
    return EXIT_SUCCESS;
  } else {
    std::cerr << "FAILED." << std::endl;
    return EXIT_FAILURE;
  }
}

#endif // ONDEMAND_TEST_ONDEMAND_H
