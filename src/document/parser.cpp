#include "simdjson/document.h"
#include "simdjson/jsonparser.h"

namespace simdjson {

WARN_UNUSED
bool document::parser::allocate_capacity(size_t len, size_t max_depth) {
  if (len <= 0) {
    len = 64; // allocating 0 bytes is wasteful.
  }
  if (len > SIMDJSON_MAXSIZE_BYTES) {
    return false;
  }
  if ((len <= byte_capacity) && (max_depth <= depth_capacity)) {
    return true;
  }
  if (max_depth <= 0) {
    max_depth = 1; // don't let the user allocate nothing
  }

  deallocate();

  //
  // Initialize the document
  //
  // a pathological input like "[[[[..." would generate len tape elements, so
  // need a capacity of at least len + 1, but it is also possible to do
  // worse with "[7,7,7,7,6,7,7,7,6,7,7,6,[7,7,7,7,6,7,7,7,6,7,7,6,7,7,7,7,7,7,6" 
  //where len + 1 tape elements are
  // generated, see issue https://github.com/lemire/simdjson/issues/345
  size_t local_tape_capacity = ROUNDUP_N(len + 2, 64);
  // a document with only zero-length strings... could have len/3 string
  // and we would need len/3 * 5 bytes on the string buffer
  size_t local_string_capacity = ROUNDUP_N(5 * len / 3 + 32, 64);
  bool doc_allocated = allocate_document(local_tape_capacity, local_string_capacity);

  //
  // Initialize stage 1 output
  //
  uint32_t max_structures = ROUNDUP_N(len, 64) + 2 + 7;
  structural_indexes.reset( new (std::nothrow) uint32_t[max_structures]);

  //
  // Initialize stage 2 state
  //
  containing_scope_offset.reset(new (std::nothrow) uint32_t[max_depth]);
#ifdef SIMDJSON_USE_COMPUTED_GOTO
  ret_address.reset(new (std::nothrow) void *[max_depth]);
#else
  ret_address.reset(new (std::nothrow) char[max_depth]);
#endif
  if (!doc_allocated || !ret_address || !structural_indexes || !containing_scope_offset) {
    // Could not allocate memory
    return false;
  }

  /*
  // We do not need to initialize this content for parsing, though we could
  // need to initialize it for safety.
  memset(string_buf, 0 , local_string_capacity);
  memset(structural_indexes, 0, max_structures * sizeof(uint32_t));
  memset(tape, 0, local_tape_capacity * sizeof(uint64_t));
  */
  byte_capacity = len;
  tape_capacity = local_tape_capacity;
  depth_capacity = max_depth;
  string_capacity = local_string_capacity;
  return true;
}

ErrorValues document::parser::try_parse(const uint8_t *buf, size_t len, const document *& dst, bool realloc_if_needed) noexcept {
  auto result = (ErrorValues)json_parse(buf, len, *this, realloc_if_needed);
  dst = result ? nullptr : &this->doc;
  return result;
}

ErrorValues document::parser::try_parse_into(const uint8_t *buf, size_t len, document & dst, bool realloc_if_needed) noexcept {
  auto result = (ErrorValues)json_parse(buf, len, *this, realloc_if_needed);
  if (result) {
    return result;
  }
  // Take the document and allocate a new one for next time
  dst = (document&&)doc;
  if (!allocate_document(tape_capacity, string_capacity)) {
    // May as well put it back if we couldn't allocate a new one and aren't giving it back to the caller ...
    doc = (document&&)dst;
    return MEMALLOC;
  }
  return SUCCESS;
}

const document &document::parser::parse(const uint8_t *buf, size_t len, bool realloc_if_needed) {
  const document *dst;
  ErrorValues result = try_parse(buf, len, dst, realloc_if_needed);
  if (result) {
    throw invalid_json(result);
  }
  return *dst;
}

document document::parser::parse_new(const uint8_t *buf, size_t len, bool realloc_if_needed) {
  document dst;
  ErrorValues result = try_parse_into(buf, len, dst, realloc_if_needed);
  if (result) {
    throw invalid_json(result);
  }
  return dst;
}

bool document::parser::allocate_document(size_t local_tape_capacity, size_t local_string_capacity) {
  doc.deallocate();
  doc.string_buf.reset( new (std::nothrow) uint8_t[local_string_capacity]);
  doc.tape.reset(new (std::nothrow) uint64_t[local_tape_capacity]);
  if (!doc.string_buf || !doc.tape) {
    deallocate();
  }
  return doc.string_buf && doc.tape;
}

void document::parser::deallocate() {
  byte_capacity = 0;
  tape_capacity = 0;
  depth_capacity = 0;
  string_capacity = 0;
  ret_address.reset();
  containing_scope_offset.reset();
  doc.deallocate();
}

void document::parser::init() {
  current_string_buf_loc = doc.string_buf.get();
  current_loc = 0;
  valid = false;
  error_code = UNINITIALIZED;
}

bool document::parser::is_valid() const { return valid; }

int document::parser::get_error_code() const { return error_code; }

std::string document::parser::get_error_message() const {
  return error_message(error_code);
}

WARN_UNUSED
bool document::parser::print_json(std::ostream &os) const {
  return doc.print_json(os);
}

WARN_UNUSED
bool document::parser::dump_raw_tape(std::ostream &os) const {
  return doc.dump_raw_tape(os);
}

} // namespace simdjson
