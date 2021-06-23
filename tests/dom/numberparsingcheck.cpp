#include <cstring>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#ifndef JSON_TEST_NUMBERS
#define JSON_TEST_NUMBERS
#endif

#if (!(_MSC_VER) && !(__MINGW32__) && !(__MINGW64__))
#include <dirent.h>
#else
#include <dirent_portable.h>
#endif

void found_invalid_number(const uint8_t *buf);
void found_float(double result, const uint8_t *buf);
void found_integer(int64_t result, const uint8_t *buf);
void found_unsigned_integer(uint64_t result, const uint8_t *buf);

#include "simdjson.h"



/**
 * Some systems have bad floating-point parsing. We want to exclude them.
 */
#if defined(SIMDJSON_REGULAR_VISUAL_STUDIO) || defined (__linux__) || defined (__APPLE__) || defined(__FreeBSD__)
// Ok. So under Visual Studio, linux, apple and freebsd systems, we have a good chance of having a decent
// enough strtod. It is not certain, but it is maybe a good enough heuristics. We exclude systems like msys2
// or cygwin.
//
// Finally, we want to exclude legacy 32-bit systems.
#ifndef SIMDJSON_IS_32BITS
// So we only run some of the floating-point tests under 64-bit linux, apple, regular visual studio, freebsd.
#define TEST_FLOATS
// Apple and freebsd need a special header, typically.
#if defined __APPLE__ || defined(__FreeBSD__)
#  include <xlocale.h>
#endif

#endif

#endif



int parse_error;
char *fullpath;
enum { PARSE_WARNING, PARSE_ERROR };

size_t float_count;
size_t int_count;
size_t invalid_count;

// strings that start with these should not be parsed as numbers
const char *really_bad[] = {"013}", "0x14", "0e]", "0e+]", "0e+-1]"};

bool starts_with(const char *pre, const char *str) {
  size_t lenpre = std::strlen(pre);
  return strncmp(pre, str, lenpre) == 0;
}

bool is_in_bad_list(const char *buf) {
  if (buf[0] != '0')
    return false;
  for (size_t i = 0; i < sizeof(really_bad) / sizeof(really_bad[0]); i++)
    if (starts_with(really_bad[i], buf))
      return true;
  return false;
}

#ifndef TEST_FLOATS
// We do not recognize the system, so we do not verify our results.
void found_invalid_number(const uint8_t *) {}
#else
void found_invalid_number(const uint8_t *buf) {
  invalid_count++;
  char *endptr;
#ifdef _WIN32
  static _locale_t c_locale = _create_locale(LC_ALL, "C");
  double expected = _strtod_l((const char *)buf, &endptr, c_locale);
#else
  static locale_t c_locale = newlocale(LC_ALL_MASK, "C", NULL);
  double expected = strtod_l((const char *)buf, &endptr, c_locale);
#endif
  if (endptr != (const char *)buf) {
    if (!is_in_bad_list((const char *)buf)) {
      printf("Warning: found_invalid_number %.32s whereas strtod parses it to "
             "%f, ",
             buf, expected);
      printf(" while parsing %s \n", fullpath);
      parse_error |= PARSE_WARNING;
    }
  }
}
#endif

void found_integer(int64_t result, const uint8_t *buf) {
  int_count++;
  char *endptr;
  long long expected = strtoll((const char *)buf, &endptr, 10);
  if ((endptr == (const char *)buf) || (expected != result)) {
#if (!(__MINGW32__) && !(__MINGW64__))
    fprintf(stderr, "Error: parsed %" PRId64 " out of %.32s, ", result, buf);
#else // mingw is busted since we include #include <inttypes.h> and it will still  not provide PRId64
    fprintf(stderr, "Error: parsed %lld out of %.32s, ", (long long)result, buf);
#endif
    fprintf(stderr, " while parsing %s \n", fullpath);
    parse_error |= PARSE_ERROR;
  }
}

void found_unsigned_integer(uint64_t result, const uint8_t *buf) {
  int_count++;
  char *endptr;
  unsigned long long expected = strtoull((const char *)buf, &endptr, 10);
  if ((endptr == (const char *)buf) || (expected != result)) {
#if (!(__MINGW32__) && !(__MINGW64__))
    fprintf(stderr, "Error: parsed %" PRIu64 " out of %.32s, ", result, buf);
#else // mingw is busted since we include #include <inttypes.h>
    fprintf(stderr, "Error: parsed %llu out of %.32s, ", (unsigned long long)result, buf);
#endif
    fprintf(stderr, " while parsing %s \n", fullpath);
    parse_error |= PARSE_ERROR;
  }
}

#ifndef TEST_FLOATS
// We do not recognize the system, so we do not verify our results.
void found_float(double , const uint8_t *) {}
#else
void found_float(double result, const uint8_t *buf) {
  char *endptr;
  float_count++;
#ifdef _WIN32
  static _locale_t c_locale = _create_locale(LC_ALL, "C");
  double expected = _strtod_l((const char *)buf, &endptr, c_locale);
#else
  static locale_t c_locale = newlocale(LC_ALL_MASK, "C", NULL);
  double expected = strtod_l((const char *)buf, &endptr, c_locale);
#endif
  if (endptr == (const char *)buf) {
    fprintf(stderr,
            "parsed %f from %.32s whereas strtod refuses to parse a float, ",
            result, buf);
    fprintf(stderr, " while parsing %s \n", fullpath);
    parse_error |= PARSE_ERROR;
  }
  if (std::fpclassify(expected) != std::fpclassify(result)) {
    fprintf(stderr,
            "floats not in the same category expected: %f observed: %f \n",
            expected, result);
    fprintf(stderr, "%.32s\n", buf);
    parse_error |= PARSE_ERROR;
    return;
  }
  if (expected != result) {
    fprintf(stderr, "parsed %.128e from \n", result);
    fprintf(stderr, "       %.32s whereas strtod gives\n", buf);
    fprintf(stderr, "       %.128e,", expected);
    fprintf(stderr, " while parsing %s \n", fullpath);
    parse_error |= PARSE_ERROR;
  }
}
#endif

#include "simdjson.h"

/**
 * Does the file filename ends with the given extension.
 */
static bool has_extension(const char *filename, const char *extension) {
  const char *ext = strrchr(filename, '.');
  return (ext && !strcmp(ext, extension));
}

bool validate(const char *dirname) {
  parse_error = 0;
  size_t total_count = 0;
  const char *extension = ".json";
  size_t dirlen = std::strlen(dirname);
  struct dirent **entry_list;
  int c = scandir(dirname, &entry_list, 0, alphasort);
  if (c < 0) {
    printf("error accessing %s \n", dirname);
    return false;
  }
  if (c == 0) {
    printf("nothing in dir %s \n", dirname);
    return false;
  }
  bool needsep = (strlen(dirname) > 1) && (dirname[strlen(dirname) - 1] != '/');
  for (int i = 0; i < c; i++) {
    const char *name = entry_list[i]->d_name;
    if (has_extension(name, extension)) {
      size_t filelen = std::strlen(name);
      fullpath = (char *)malloc(dirlen + filelen + 1 + 1);
      strcpy(fullpath, dirname);
      if (needsep) {
        fullpath[dirlen] = '/';
        strcpy(fullpath + dirlen + 1, name);
      } else {
        strcpy(fullpath + dirlen, name);
      }
      simdjson::padded_string p;
      auto error = simdjson::padded_string::load(fullpath).get(p);
      if (error) {
        std::cerr << "Could not load the file " << fullpath << std::endl;
        return EXIT_FAILURE;
      }
      // terrible hack but just to get it working
      float_count = 0;
      int_count = 0;
      invalid_count = 0;
      total_count += float_count + int_count + invalid_count;
      simdjson::dom::parser parser;
      auto err = parser.parse(p).error();
      bool isok = (err == simdjson::error_code::SUCCESS);
      if (int_count + float_count + invalid_count > 0) {
        printf("File %40s %s --- integers: %10zu floats: %10zu invalid: %10zu "
               "total numbers: %10zu \n",
               name, isok ? " is valid     " : " is not valid ", int_count,
               float_count, invalid_count,
               int_count + float_count + invalid_count);
      }
      free(fullpath);
    }
  }
  if ((parse_error & PARSE_ERROR) != 0) {
    printf("NUMBER PARSING FAILS?\n");
  } else {
    printf("All ok.\n");
  }
  for (int i = 0; i < c; ++i)
    free(entry_list[i]);
  free(entry_list);
  return ((parse_error & PARSE_ERROR) == 0);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <directorywithjsonfiles>"
              << std::endl;
#if defined(SIMDJSON_TEST_DATA_DIR) && defined(SIMDJSON_BENCHMARK_DATA_DIR)
    std::cout << "We are going to assume you mean to use the '"
              << SIMDJSON_TEST_DATA_DIR << "'  and  '"
              << SIMDJSON_BENCHMARK_DATA_DIR << "'directories." << std::endl;
    return validate(SIMDJSON_TEST_DATA_DIR) &&
                   validate(SIMDJSON_BENCHMARK_DATA_DIR)
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
#else
    std::cout << "We are going to assume you mean to use the 'jsonchecker' and "
                 "'jsonexamples' directories."
              << std::endl;
    return validate("jsonchecker/") && validate("jsonexamples/") ? EXIT_SUCCESS
                                                                 : EXIT_FAILURE;
#endif
  }
  return validate(argv[1]) ? EXIT_SUCCESS : EXIT_FAILURE;
}
