#ifndef SIMDJSON_ARM64_STRINGPARSING_H
#define SIMDJSON_ARM64_STRINGPARSING_H

#include "simdjson/base.h"
#include "simdjson/arm64/simd.h"
#include "simdjson/arm64/bitmanipulation.h"

namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace {

using namespace simd;

// Holds backslashes and quotes locations.
struct backslash_and_quote {
public:
  static constexpr uint32_t BYTES_PROCESSED = 32;
  simdjson_really_inline static backslash_and_quote
  copy_and_find(const uint8_t *src, uint8_t *dst, const uint8_t *last_full_buf);
  simdjson_really_inline static backslash_and_quote
  copy_and_find(const uint8_t *src, uint8_t *dst);

  simdjson_really_inline bool has_quote_first() { return ((bs_bits - 1) & quote_bits) != 0; }
  simdjson_really_inline bool has_backslash() { return bs_bits != 0; }
  simdjson_really_inline int quote_index() { return trailing_zeroes(quote_bits); }
  simdjson_really_inline int backslash_index() { return trailing_zeroes(bs_bits); }

  uint32_t bs_bits;
  uint32_t quote_bits;
}; // struct backslash_and_quote

simdjson_really_inline backslash_and_quote
backslash_and_quote::copy_and_find(const uint8_t *src, uint8_t *dst, const uint8_t *last_full_buf) {
  // If we don't have enough memory left to load a whole simd register, copy it first.
  uint8_t tmpbuf[BYTES_PROCESSED];
  if (simdjson_unlikely(src > last_full_buf)) {
    SIMDJSON_ASSUME(last_full_buf+BYTES_PROCESSED > src);
    std::memset(tmpbuf, 0, BYTES_PROCESSED);
    std::memcpy(tmpbuf, src, last_full_buf+BYTES_PROCESSED-src);
    src = tmpbuf;
  }
  return copy_and_find(src, dst);
}

simdjson_really_inline backslash_and_quote
backslash_and_quote::copy_and_find(const uint8_t *src, uint8_t *dst) {
  simd8<uint8_t> v0(src);
  simd8<uint8_t> v1(src + sizeof(v0));
  v0.store(dst);
  v1.store(dst + sizeof(v0));

  // Getting a 64-bit bitmask is much cheaper than multiple 16-bit bitmasks on ARM; therefore, we
  // smash them together into a 64-byte mask and get the bitmask from there.
  uint64_t bs_and_quote = simd8x64<bool>(v0 == '\\', v1 == '\\', v0 == '"', v1 == '"').to_bitmask();
  return {
    uint32_t(bs_and_quote),      // bs_bits
    uint32_t(bs_and_quote >> 32) // quote_bits
  };
}

} // unnamed namespace
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson

#include "simdjson/generic/stringparsing.h"

#endif // SIMDJSON_ARM64_STRINGPARSING_H
