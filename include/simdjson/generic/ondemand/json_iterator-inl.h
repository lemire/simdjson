namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace ondemand {

simdjson_really_inline json_iterator::json_iterator(json_iterator &&other) noexcept
  : token(std::forward<token_iterator>(other.token)),
    parser{other.parser},
    _string_buf_loc{other._string_buf_loc},
    _depth{other._depth}
{
  other.parser = nullptr;
}
simdjson_really_inline json_iterator &json_iterator::operator=(json_iterator &&other) noexcept {
  token = other.token;
  parser = other.parser;
  _string_buf_loc = other._string_buf_loc;
  _depth = other._depth;
  other.parser = nullptr;
  return *this;
}

simdjson_really_inline json_iterator::json_iterator(const uint8_t *buf, ondemand::parser *_parser) noexcept
  : token(buf, &_parser->implementation->structural_indexes[0]),
    parser{_parser},
    _string_buf_loc{parser->string_buf.get()},
    _depth{1}
{
  logger::log_headers();
  assert_more_tokens(1);
}

inline void json_iterator::rewind() noexcept {
  token.set_position(parser->implementation->structural_indexes.get());
  logger::log_headers(); // We start again
  _string_buf_loc = parser->string_buf.get();
  _depth = 1;
}

// GCC 7 warns when the first line of this function is inlined away into oblivion due to the caller
// relating depth and parent_depth, which is a desired effect. The warning does not show up if the
// skip_child() function is not marked inline).
SIMDJSON_PUSH_DISABLE_WARNINGS
SIMDJSON_DISABLE_STRICT_OVERFLOW_WARNING
simdjson_warn_unused simdjson_really_inline error_code json_iterator::skip_child(depth_t parent_depth) noexcept {
  if (depth() <= parent_depth) { return SUCCESS; }

  SIMDJSON_TRY( require_tokens(1) );
  switch (*advance()) {
    // TODO consider whether matching braces is a requirement: if non-matching braces indicates
    // *missing* braces, then future lookups are not in the object/arrays they think they are,
    // violating the rule "validate enough structure that the user can be confident they are
    // looking at the right values."
    // PERF TODO we can eliminate the switch here with a lookup of how much to add to depth

    // For the first open array/object in a value, we've already incremented depth, so keep it the same
    // We never stop at colon, but if we did, it wouldn't affect depth
    case '[': case '{': case ':':
      logger::log_start_value(*this, "skip");
      break;
    // If there is a comma, we have just finished a value in an array/object, and need to get back in
    case ',':
      logger::log_value(*this, "skip");
      break;
    // ] or } means we just finished a value and need to jump out of the array/object
    case ']': case '}':
      logger::log_end_value(*this, "skip");
      _depth--;
      if (depth() <= parent_depth) { return SUCCESS; }
      break;
    /*case '"':
      if(*peek() == ':') {
        // we are at a key!!! This is
        // only possible if someone searched
        // for a key in an object and the key
        // was not found but our code then
        // decided the consume the separating
        // comma before returning.
        logger::log_value(*this, "key");
        advance(); // eat up the ':'
        break; // important!!!
      }
      simdjson_fallthrough;*/
    // Anything else must be a scalar value
    default:
      // For the first scalar, we will have incremented depth already, so we decrement it here.
      logger::log_value(*this, "skip");
      _depth--;
      if (depth() <= parent_depth) { return SUCCESS; }
      break;
  }

  // Now that we've considered the first value, we only increment/decrement for arrays/objects
  while (position() < end_position()) {
    switch (*advance()) {
      case '[': case '{':
        logger::log_start_value(*this, "skip");
        _depth++;
        break;
      // TODO consider whether matching braces is a requirement: if non-matching braces indicates
      // *missing* braces, then future lookups are not in the object/arrays they think they are,
      // violating the rule "validate enough structure that the user can be confident they are
      // looking at the right values."
      // PERF TODO we can eliminate the switch here with a lookup of how much to add to depth
      case ']': case '}':
        logger::log_end_value(*this, "skip");
        _depth--;
        if (depth() <= parent_depth) { return SUCCESS; }
        break;
      default:
        logger::log_value(*this, "skip", "");
        break;
    }
  }

  return report_error(TAPE_ERROR, "not enough close braces");
}

SIMDJSON_POP_DISABLE_WARNINGS

simdjson_really_inline bool json_iterator::at_root() const noexcept {
  return position() == root_position();
}

simdjson_really_inline token_position json_iterator::root_position() const noexcept {
  return parser->implementation->structural_indexes.get();
}

simdjson_really_inline void json_iterator::assert_at_root() const noexcept {
  SIMDJSON_ASSUME( _depth == 1 );
  // Visual Studio Clang treats unique_ptr.get() as "side effecting."
#ifndef SIMDJSON_CLANG_VISUAL_STUDIO
  SIMDJSON_ASSUME( token._position == parser->implementation->structural_indexes.get() );
#endif
}

simdjson_really_inline void json_iterator::assert_more_tokens(uint32_t required_tokens) const noexcept {
  assert_valid_position(token.position() + required_tokens - 1);
}

simdjson_really_inline void json_iterator::assert_valid_position(token_position position) const noexcept {
#ifndef SIMDJSON_CLANG_VISUAL_STUDIO
  SIMDJSON_ASSUME( position >= &parser->implementation->structural_indexes[0] );
  SIMDJSON_ASSUME( position < &parser->implementation->structural_indexes[parser->implementation->n_structural_indexes] );
#endif
}

simdjson_really_inline bool json_iterator::at_end() const noexcept {
  return position() == end_position();
}
simdjson_really_inline token_position json_iterator::end_position() const noexcept {
  uint32_t n_structural_indexes{parser->implementation->n_structural_indexes};
  return &parser->implementation->structural_indexes[n_structural_indexes];
}
simdjson_really_inline const uint8_t *json_iterator::end() const noexcept {
  return token.buf + parser->implementation->len;
}

inline std::string json_iterator::to_string() const noexcept {
  if( !is_alive() ) { return "dead json_iterator instance"; }
  const char * current_structural = reinterpret_cast<const char *>(token.peek());
  return std::string("json_iterator [ depth : ") + std::to_string(_depth)
          + std::string(", structural : '") + std::string(current_structural,1)
          + std::string("', offset : ") + std::to_string(token.current_offset())
          + std::string(" ]");
}

simdjson_really_inline bool json_iterator::is_alive() const noexcept {
  return parser;
}

simdjson_really_inline void json_iterator::abandon() noexcept {
  parser = nullptr;
  _depth = 0;
}

simdjson_really_inline const uint8_t *json_iterator::advance() noexcept {
  assert_more_tokens();
  return token.advance();
}

simdjson_really_inline simdjson_result<const uint8_t *> json_iterator::try_advance(uint32_t required_tokens) noexcept {
  const uint8_t *json = token.advance();
  // Check this *after* we get the pointer, since getting the pointer is more time-sensitive than the branch.
  // Also resolves nicely to 0 in the common case of required_tokens == 1.
  SIMDJSON_TRY( require_tokens(required_tokens - 1) );
  return json;
}

simdjson_really_inline error_code json_iterator::require_tokens(simdjson_unused uint32_t required_tokens) noexcept {
#if __SIMDJSON_CHECK_EOF
  if (position() + required_tokens > end_position()) {
    return report_error(TAPE_ERROR, "Document ended early");
  }
#endif
  return SUCCESS;
}

simdjson_really_inline const uint8_t *json_iterator::peek(int32_t delta) const noexcept {
  assert_more_tokens(delta+1);
  return token.peek(delta);
}

simdjson_really_inline uint32_t json_iterator::peek_length(int32_t delta) const noexcept {
  assert_more_tokens(delta+1);
  return token.peek_length(delta);
}

simdjson_really_inline const uint8_t *json_iterator::peek(token_position position) const noexcept {
  assert_valid_position(position);
  return token.peek(position);
}

simdjson_really_inline uint32_t json_iterator::peek_length(token_position position) const noexcept {
  assert_valid_position(position);
  return token.peek_length(position);
}

simdjson_really_inline token_position json_iterator::last_position() const noexcept {
  // The following line fails under some compilers...
  // SIMDJSON_ASSUME(parser->implementation->n_structural_indexes > 0);
  // since it has side-effects.
  uint32_t n_structural_indexes{parser->implementation->n_structural_indexes};
  SIMDJSON_ASSUME(n_structural_indexes > 0);
  return &parser->implementation->structural_indexes[n_structural_indexes - 1];
}
simdjson_really_inline const uint8_t *json_iterator::peek_last() const noexcept {
  return token.peek(last_position());
}

simdjson_really_inline void json_iterator::ascend_to(depth_t parent_depth) noexcept {
  SIMDJSON_ASSUME(parent_depth >= 0 && parent_depth < INT32_MAX - 1);
  SIMDJSON_ASSUME(_depth == parent_depth + 1);
  _depth = parent_depth;
}

simdjson_really_inline void json_iterator::descend_to(depth_t child_depth) noexcept {
  SIMDJSON_ASSUME(child_depth >= 1 && child_depth < INT32_MAX);
  SIMDJSON_ASSUME(_depth == child_depth - 1);
  _depth = child_depth;
}

simdjson_really_inline depth_t json_iterator::depth() const noexcept {
  return _depth;
}

simdjson_really_inline uint8_t *&json_iterator::string_buf_loc() noexcept {
  return _string_buf_loc;
}

simdjson_really_inline error_code json_iterator::report_error(error_code _error, const char *message) noexcept {
  SIMDJSON_ASSUME(_error != SUCCESS && _error != UNINITIALIZED && _error != INCORRECT_TYPE && _error != NO_SUCH_FIELD);
  logger::log_error(*this, message);
  error = _error;
  return error;
}

simdjson_really_inline token_position json_iterator::position() const noexcept {
  return token.position();
}

simdjson_really_inline void json_iterator::reenter_child(token_position position, depth_t child_depth) noexcept {
  SIMDJSON_ASSUME(child_depth >= 1 && child_depth < INT32_MAX);
  SIMDJSON_ASSUME(_depth == child_depth - 1);
#ifdef SIMDJSON_DEVELOPMENT_CHECKS
#ifndef SIMDJSON_CLANG_VISUAL_STUDIO
  SIMDJSON_ASSUME(position >= parser->start_positions[child_depth]);
#endif
#endif
  token.set_position(position);
  _depth = child_depth;
}

#ifdef SIMDJSON_DEVELOPMENT_CHECKS

simdjson_really_inline token_position json_iterator::start_position(depth_t depth) const noexcept {
  return parser->start_positions[depth];
}

simdjson_really_inline void json_iterator::set_start_position(depth_t depth, token_position position) noexcept {
  parser->start_positions[depth] = position;
}

#endif


simdjson_really_inline error_code json_iterator::optional_error(error_code _error, const char *message) noexcept {
  SIMDJSON_ASSUME(_error == INCORRECT_TYPE || _error == NO_SUCH_FIELD);
  logger::log_error(*this, message);
  return _error;
}

} // namespace ondemand
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson

namespace simdjson {

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::json_iterator>::simdjson_result(SIMDJSON_IMPLEMENTATION::ondemand::json_iterator &&value) noexcept
    : implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::json_iterator>(std::forward<SIMDJSON_IMPLEMENTATION::ondemand::json_iterator>(value)) {}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::json_iterator>::simdjson_result(error_code error) noexcept
    : implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::json_iterator>(error) {}

} // namespace simdjson