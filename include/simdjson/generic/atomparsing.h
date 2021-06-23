namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace {
/// @private
namespace atomparsing {

// The string_to_uint32 is exclusively used to map literal strings to 32-bit values.
// We use memcpy instead of a pointer cast to avoid undefined behaviors since we cannot
// be certain that the character pointer will be properly aligned.
// You might think that using memcpy makes this function expensive, but you'd be wrong.
// All decent optimizing compilers (GCC, clang, Visual Studio) will compile string_to_uint32("false");
// to the compile-time constant 1936482662.
simdjson_really_inline uint32_t string_to_uint32(const char* str) { uint32_t val; std::memcpy(&val, str, sizeof(uint32_t)); return val; }


// Again in str4ncmp we use a memcpy to avoid undefined behavior. The memcpy may appear expensive.
// Yet all decent optimizing compilers will compile memcpy to a single instruction, just about.
simdjson_warn_unused
simdjson_really_inline uint32_t str4ncmp(const uint8_t *src, const char* atom) {
  uint32_t srcval; // we want to avoid unaligned 32-bit loads (undefined in C/C++)
  std::memcpy(&srcval, src, sizeof(uint32_t));
  return srcval ^ string_to_uint32(atom);
}

simdjson_warn_unused
simdjson_really_inline bool is_valid_true_atom(const uint8_t *src) {
  return (str4ncmp(src, "true") | jsoncharutils::is_not_structural_or_whitespace(src[4])) == 0;
}

simdjson_warn_unused
simdjson_really_inline bool is_valid_true_atom(const uint8_t *src, size_t len) {
  if (len > 4) { return is_valid_true_atom(src); }
  else if (len == 4) { return !str4ncmp(src, "true"); }
  else { return false; }
}

simdjson_warn_unused
simdjson_really_inline bool is_valid_false_atom(const uint8_t *src) {
  return (str4ncmp(src+1, "alse") | jsoncharutils::is_not_structural_or_whitespace(src[5])) == 0;
}

simdjson_warn_unused
simdjson_really_inline bool is_valid_false_atom(const uint8_t *src, size_t len) {
  if (len > 5) { return is_valid_false_atom(src); }
  else if (len == 5) { return !str4ncmp(src+1, "alse"); }
  else { return false; }
}

simdjson_warn_unused
simdjson_really_inline bool is_valid_null_atom(const uint8_t *src) {
  return (str4ncmp(src, "null") | jsoncharutils::is_not_structural_or_whitespace(src[4])) == 0;
}

simdjson_warn_unused
simdjson_really_inline bool is_valid_null_atom(const uint8_t *src, size_t len) {
  if (len > 4) { return is_valid_null_atom(src); }
  else if (len == 4) { return !str4ncmp(src, "null"); }
  else { return false; }
}

} // namespace atomparsing
} // unnamed namespace
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson
