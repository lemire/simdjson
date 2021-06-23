#include <cassert>
#include <climits>
#include <cstring>
#include <cinttypes>
#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#ifndef JSON_TEST_STRINGS
#define JSON_TEST_STRINGS
#endif

#if (!(_MSC_VER) && !(__MINGW32__) && !(__MINGW64__))
#include <dirent.h>
#else
#include <dirent_portable.h>
#endif
#include "simdjson.h"

char *fullpath;

size_t bad_string;
size_t good_string;
size_t empty_string;

size_t total_string_length;
bool probable_bug;
// borrowed code (sajson?)

static inline bool read_hex(const char *p, unsigned &u) {
  unsigned v = 0;
  int i = 4;
  while (i--) {
    unsigned char c = *p++;
    if (c >= '0' && c <= '9') {
      c = static_cast<unsigned char>(c - '0');
    } else if (c >= 'a' && c <= 'f') {
      c = static_cast<unsigned char>(c - 'a' + 10);
    } else if (c >= 'A' && c <= 'F') {
      c = static_cast<unsigned char>(c - 'A' + 10);
    } else {
      return false;
    }
    v = (v << 4) + c;
  }

  u = v;
  return true;
}

static inline void write_utf8(unsigned codepoint, char *&end) {
  if (codepoint < 0x80) {
    *end++ = static_cast<char>(codepoint);
  } else if (codepoint < 0x800) {
    *end++ = static_cast<char>(0xC0 | (codepoint >> 6));
    *end++ = static_cast<char>(0x80 | (codepoint & 0x3F));
  } else if (codepoint < 0x10000) {
    *end++ = static_cast<char>(0xE0 | (codepoint >> 12));
    *end++ = static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
    *end++ = static_cast<char>(0x80 | (codepoint & 0x3F));
  } else {
    assert(codepoint < 0x200000);
    *end++ = static_cast<char>(0xF0 | (codepoint >> 18));
    *end++ = static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
    *end++ = static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
    *end++ = static_cast<char>(0x80 | (codepoint & 0x3F));
  }
}

static bool parse_string(const char *p, char *output, char **end) {
  if (*p != '"')
    return false;
  p++;

  for (;;) {
#if (CHAR_MIN < 0) || (!defined(CHAR_MIN)) // the '!defined' is just paranoia
    // in this path, char is *signed*
    if ((*p >= 0 && *p < 0x20)) {
      return false; // unescaped
    }
#else
    // we have unsigned chars
    if (*p < 0x20) {
      return false; // unescaped
    }
#endif

    switch (*p) {
    case '"':
      *output = '\0'; // end
      *end = output;
      return true;
    case '\\':
      ++p;

      char replacement;
      switch (*p) {
      case '"':
        replacement = '"';
        goto replace;
      case '\\':
        replacement = '\\';
        goto replace;
      case '/':
        replacement = '/';
        goto replace;
      case 'b':
        replacement = '\b';
        goto replace;
      case 'f':
        replacement = '\f';
        goto replace;
      case 'n':
        replacement = '\n';
        goto replace;
      case 'r':
        replacement = '\r';
        goto replace;
      case 't':
        replacement = '\t';
        goto replace;
      replace:
        *output++ = replacement;
        ++p;
        break;
      case 'u': {
        ++p;
        unsigned u;
        if (!read_hex(p, u))
          return false;

        p += 4;
        if (u >= 0xD800 && u <= 0xDBFF) {
          char p0 = p[0];
          char p1 = p[1];
          if (p0 != '\\' || p1 != 'u') {
            return false;
          }
          p += 2;
          unsigned v;
          if (!read_hex(p, v))
            return false;

          p += 4;

          if (v < 0xDC00 || v > 0xDFFF) {
            return false;
          }
          u = 0x10000 + (((u - 0xD800) << 10) | (v - 0xDC00));
        }
        write_utf8(u, output);
        break;
      }
      default:
        return false;
      }
      break;

    default:
      // validate UTF-8
      unsigned char c0 = p[0];
      if (c0 < 128) {
        *output++ = *p++;
      } else if (c0 < 224) {
        unsigned char c1 = p[1];
        if (c1 < 128 || c1 >= 192) {
          return false;
        }
        output[0] = c0;
        output[1] = c1;
        output += 2;
        p += 2;
      } else if (c0 < 240) {
        unsigned char c1 = p[1];
        if (c1 < 128 || c1 >= 192) {
          return false;
        }
        unsigned char c2 = p[2];
        if (c2 < 128 || c2 >= 192) {
          return false;
        }
        output[0] = c0;
        output[1] = c1;
        output[2] = c2;
        output += 3;
        p += 3;
      } else if (c0 < 248) {
        unsigned char c1 = p[1];
        if (c1 < 128 || c1 >= 192) {
          return false;
        }
        unsigned char c2 = p[2];
        if (c2 < 128 || c2 >= 192) {
          return false;
        }
        unsigned char c3 = p[3];
        if (c3 < 128 || c3 >= 192) {
          return false;
        }
        output[0] = c0;
        output[1] = c1;
        output[2] = c2;
        output[3] = c3;
        output += 4;
        p += 4;
      } else {
        return false;
      }
      break;
    }
  }
}
// end of borrowed code
char *big_buffer; // global variable

void found_bad_string(const uint8_t *buf) {
  bad_string++;
  char *end;
  if (parse_string((const char *)buf, big_buffer, &end)) {
    printf("WARNING: Sajson-like parser seems to think that the string is "
           "valid %32s \n",
           buf);
    probable_bug = true;
  }
}

void print_hex(const char *s, size_t len) {
  for (size_t i = 0; i < len; i++) {
    printf("%02x ", s[i] & 0xFF);
  }
}

void print_cmp_hex(const char *s1, const char *s2, size_t len) {
  for (size_t i = 0; i < len; i++) {
    printf("%02x ", (s1[i] ^ s2[i]) & 0xFF);
  }
}

void found_string(const uint8_t *buf, const uint8_t *parsed_begin,
                  const uint8_t *parsed_end) {
  size_t this_len = parsed_end - parsed_begin;
  total_string_length += this_len;
  good_string++;
  char *end = NULL;
  if (!parse_string((const char *)buf, big_buffer, &end)) {
    printf("WARNING: reference parser seems to think that the string is NOT "
           "valid %32s \n",
           buf);
  }
  if (end == big_buffer) {
    // we have a zero-length string
    if (parsed_begin != parsed_end) {
      printf("WARNING: We have a zero-length but gap is %zu \n",
             (size_t)(parsed_end - parsed_begin));
      probable_bug = true;
    }
    empty_string++;
    return;
  }
  size_t len = end - big_buffer;
  if (len != this_len) {
    printf("WARNING: lengths on parsed strings disagree %zu %zu \n", this_len,
           len);
    printf("\nour parsed string  : '%*s'\n\n", (int)this_len,
           (const char *)parsed_begin);
    print_hex((const char *)parsed_begin, this_len);
    printf("\n");

    printf("reference parsing   :'%*s'\n\n", (int)len, big_buffer);
    print_hex((const char *)big_buffer, len);
    printf("\n");

    probable_bug = true;
  }
  if (memcmp(big_buffer, parsed_begin, this_len) != 0) {
    printf("WARNING: parsed strings disagree  \n");
    printf("Lengths %zu %zu  \n", this_len, len);

    printf("\nour parsed string  : '%*s'\n", (int)this_len,
           (const char *)parsed_begin);
    print_hex((const char *)parsed_begin, this_len);
    printf("\n");

    printf("reference parsing   :'%*s'\n", (int)len, big_buffer);
    print_hex((const char *)big_buffer, len);
    printf("\n");

    print_cmp_hex((const char *)parsed_begin, big_buffer, this_len);

    probable_bug = true;
  }
}

#include "simdjson.h"

/**
 * Does the file filename ends with the given extension.
 */
static bool has_extension(const char *filename, const char *extension) {
  const char *ext = strrchr(filename, '.');
  return (ext && !strcmp(ext, extension));
}

bool starts_with(const char *pre, const char *str) {
  size_t lenpre = std::strlen(pre), lenstr = std::strlen(str);
  return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

bool validate(const char *dirname) {
  size_t total_strings = 0;
  probable_bug = false;
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
      big_buffer = (char *)malloc(p.size());
      if (big_buffer == NULL) {
        std::cerr << "can't allocate memory" << std::endl;
        return false;
      }
      bad_string = 0;
      good_string = 0;
      total_string_length = 0;
      empty_string = 0;
      simdjson::dom::parser parser;
      auto err = parser.parse(p).error();
      bool isok = (err == simdjson::error_code::SUCCESS);
      free(big_buffer);
      if (good_string > 0) {
        printf("File %40s %s --- bad strings: %10zu \tgood strings: %10zu\t "
               "empty strings: %10zu "
               "\taverage string length: %.1f \n",
               name, isok ? " is valid     " : " is not valid ", bad_string,
               good_string, empty_string,
               static_cast<double>(total_string_length) / static_cast<double>(good_string));
      } else if (bad_string > 0) {
        printf("File %40s %s --- bad strings: %10zu  \n", name,
               isok ? " is valid     " : " is not valid ", bad_string);
      }
      total_strings += bad_string + good_string;
      free(fullpath);
    }
  }
  printf("%zu strings checked.\n", total_strings);
  if (probable_bug) {
    fprintf(stderr, "STRING PARSING FAILS?\n");
  } else {
    printf("All ok.\n");
  }
  for (int i = 0; i < c; ++i)
    free(entry_list[i]);
  free(entry_list);
  return probable_bug == false;
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
