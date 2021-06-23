namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace ondemand {

simdjson_really_inline parser::parser(size_t max_capacity) noexcept
  : _max_capacity{max_capacity} {
}

simdjson_warn_unused simdjson_really_inline error_code parser::allocate(size_t new_capacity, size_t new_max_depth) noexcept {
  if (new_capacity > max_capacity()) { return CAPACITY; }
  if (string_buf && new_capacity == capacity() && new_max_depth == max_depth()) { return SUCCESS; }

  // string_capacity copied from document::allocate
  _capacity = 0;
  size_t string_capacity = SIMDJSON_ROUNDUP_N(5 * new_capacity / 3 + SIMDJSON_PADDING, 64);
  string_buf.reset(new (std::nothrow) uint8_t[string_capacity]);
#ifdef SIMDJSON_DEVELOPMENT_CHECKS
  start_positions.reset(new (std::nothrow) token_position[new_max_depth]);
#endif
  if (implementation) {
    SIMDJSON_TRY( implementation->set_capacity(new_capacity) );
    SIMDJSON_TRY( implementation->set_max_depth(new_max_depth) );
  } else {
    SIMDJSON_TRY( simdjson::active_implementation->create_dom_parser_implementation(new_capacity, new_max_depth, implementation) );
  }
  _capacity = new_capacity;
  _max_depth = new_max_depth;
  return SUCCESS;
}

simdjson_warn_unused simdjson_really_inline simdjson_result<document> parser::iterate(std::string_view json) & noexcept {
  // Allocate if needed
  if (capacity() < json.length() || !string_buf) {
    SIMDJSON_TRY( allocate(json.length(), max_depth()) );
  }

  // Run stage 1.
  SIMDJSON_TRY( implementation->stage1(reinterpret_cast<const uint8_t *>(json.data()), json.length(), stage1_mode::regular) );
  return document::start({ reinterpret_cast<const uint8_t *>(json.data()), this });
}

simdjson_warn_unused simdjson_really_inline simdjson_result<document> parser::iterate(const char *json, size_t len) & noexcept {
  return iterate(std::string_view(json, len));
}

simdjson_warn_unused simdjson_really_inline simdjson_result<document> parser::iterate(const uint8_t *json, size_t len) & noexcept {
  return iterate(std::string_view(reinterpret_cast<const char *>(json), len));
}

simdjson_warn_unused simdjson_really_inline simdjson_result<document> parser::iterate(const std::string &json) & noexcept {
  return iterate(std::string_view(json));
}

simdjson_warn_unused simdjson_really_inline simdjson_result<document> parser::iterate(const simdjson_result<padded_string_view> &result) & noexcept {
  // We don't presently have a way to temporarily get a const T& from a simdjson_result<T> without throwing an exception
  SIMDJSON_TRY( result.error() );
  padded_string_view json = result.value_unsafe();
  return iterate(json);
}

simdjson_warn_unused simdjson_really_inline simdjson_result<document> parser::iterate(const simdjson_result<padded_string> &result) & noexcept {
  // We don't presently have a way to temporarily get a const T& from a simdjson_result<T> without throwing an exception
  SIMDJSON_TRY( result.error() );
  const padded_string &json = result.value_unsafe();
  return iterate(json);
}

simdjson_warn_unused simdjson_really_inline simdjson_result<json_iterator> parser::iterate_raw(std::string_view json) & noexcept {
  // Allocate if needed
  if (capacity() < json.length()) {
    SIMDJSON_TRY( allocate(json.length(), max_depth()) );
  }

  // Run stage 1.
  SIMDJSON_TRY( implementation->stage1(reinterpret_cast<const uint8_t *>(json.data()), json.length(), stage1_mode::regular) );
  return json_iterator(reinterpret_cast<const uint8_t *>(json.data()), this);
}

simdjson_really_inline size_t parser::capacity() const noexcept {
  return _capacity;
}
simdjson_really_inline size_t parser::max_capacity() const noexcept {
  return _max_capacity;
}
simdjson_really_inline size_t parser::max_depth() const noexcept {
  return _max_depth;
}

simdjson_really_inline void parser::set_max_capacity(size_t max_capacity) noexcept {
  size_t MINIMAL_DOCUMENT_CAPACITY = 32;
  if(max_capacity < MINIMAL_DOCUMENT_CAPACITY) {
    _max_capacity = max_capacity;
  } else {
    _max_capacity = MINIMAL_DOCUMENT_CAPACITY;
  }
}

} // namespace ondemand
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson

namespace simdjson {

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::parser>::simdjson_result(SIMDJSON_IMPLEMENTATION::ondemand::parser &&value) noexcept
    : implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::parser>(std::forward<SIMDJSON_IMPLEMENTATION::ondemand::parser>(value)) {}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::parser>::simdjson_result(error_code error) noexcept
    : implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::parser>(error) {}

} // namespace simdjson
