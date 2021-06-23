#ifndef SIMDJSON_FALLBACK_STRINGPARSING_H
#define SIMDJSON_FALLBACK_STRINGPARSING_H

#include "simdjson/base.h"

namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace {

// Holds backslashes and quotes locations.
struct backslash_and_quote {
public:
  static constexpr uint32_t BYTES_PROCESSED = 1;
  simdjson_really_inline static backslash_and_quote
  copy_and_find(const uint8_t *src, uint8_t *dst, const uint8_t *last_full_buf);
  simdjson_really_inline static backslash_and_quote
  copy_and_find(const uint8_t *src, uint8_t *dst);

  simdjson_really_inline bool has_quote_first() { return c == '"'; }
  simdjson_really_inline bool has_backslash() { return c == '\\'; }
  simdjson_really_inline int quote_index() { return c == '"' ? 0 : 1; }
  simdjson_really_inline int backslash_index() { return c == '\\' ? 0 : 1; }

  uint8_t c;
}; // struct backslash_and_quote

simdjson_really_inline backslash_and_quote backslash_and_quote::copy_and_find(const uint8_t *src, uint8_t *dst, const uint8_t *) {
  return copy_and_find(src, dst);
}

simdjson_really_inline backslash_and_quote backslash_and_quote::copy_and_find(const uint8_t *src, uint8_t *dst) {
  // store to dest unconditionally - we can overwrite the bits we don't like later
  dst[0] = src[0];
  return { src[0] };
}

} // unnamed namespace
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson

#include "simdjson/generic/stringparsing.h"

#endif // SIMDJSON_FALLBACK_STRINGPARSING_H
