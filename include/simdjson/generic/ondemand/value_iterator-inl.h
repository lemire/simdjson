namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace ondemand {

simdjson_really_inline value_iterator::value_iterator(
  json_iterator *json_iter,
  depth_t depth,
  token_position start_position
) noexcept : _json_iter{json_iter}, _depth{depth}, _start_position{start_position}
{
}

simdjson_warn_unused simdjson_really_inline simdjson_result<bool> value_iterator::start_object() noexcept {
  SIMDJSON_TRY( start_container('{', "Not an object", "object") );
  return started_object();
}

simdjson_warn_unused simdjson_really_inline simdjson_result<bool> value_iterator::start_root_object() noexcept {
  SIMDJSON_TRY( start_container('{', "Not an object", "object") );
  return started_root_object();
}

simdjson_warn_unused simdjson_really_inline simdjson_result<bool> value_iterator::started_object() noexcept {
  assert_at_container_start();
#ifdef SIMDJSON_DEVELOPMENT_CHECKS
  _json_iter->set_start_position(_depth, start_position());
#endif
  if (*_json_iter->peek() == '}') {
    logger::log_value(*_json_iter, "empty object");
    _json_iter->return_current_and_advance();
    end_container();
    return false;
  }
  return true;
}

simdjson_warn_unused simdjson_really_inline simdjson_result<bool> value_iterator::started_root_object() noexcept {
  if (*_json_iter->peek_last() != '}') {
    return report_error(INCOMPLETE_ARRAY_OR_OBJECT, "missing } at end");
  }
  return started_object();
}

simdjson_warn_unused simdjson_really_inline error_code value_iterator::end_container() noexcept {
#if __SIMDJSON_CHECK_EOF
    if (depth() > 1 && at_end()) { return report_error(INCOMPLETE_ARRAY_OR_OBJECT, "missing parent ] or }"); }
    // if (depth() <= 1 && !at_end()) { return report_error(INCOMPLETE_ARRAY_OR_OBJECT, "missing [ or { at start"); }
#endif // __SIMDJSON_CHECK_EOF
    _json_iter->ascend_to(depth()-1);
    return SUCCESS;
}

simdjson_warn_unused simdjson_really_inline simdjson_result<bool> value_iterator::has_next_field() noexcept {
  assert_at_next();

  // It's illegal to call this unless there are more tokens: anything that ends in } or ] is
  // obligated to verify there are more tokens if they are not the top level.
  switch (*_json_iter->return_current_and_advance()) {
    case '}':
      logger::log_end_value(*_json_iter, "object");
      SIMDJSON_TRY( end_container() );
      return false;
    case ',':
      return true;
    default:
      return report_error(TAPE_ERROR, "Missing comma between object fields");
  }
}

simdjson_warn_unused simdjson_really_inline simdjson_result<bool> value_iterator::find_field_raw(const std::string_view key) noexcept {
  error_code error;
  bool has_value;
  //
  // Initially, the object can be in one of a few different places:
  //
  // 1. The start of the object, at the first field:
  //
  //    ```
  //    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
  //      ^ (depth 2, index 1)
  //    ```
  if (at_first_field()) {
    has_value = true;

  //
  // 2. When a previous search did not yield a value or the object is empty:
  //
  //    ```
  //    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
  //                                     ^ (depth 0)
  //    { }
  //        ^ (depth 0, index 2)
  //    ```
  //
  } else if (!is_open()) {
#ifdef SIMDJSON_DEVELOPMENT_CHECKS
    // If we're past the end of the object, we're being iterated out of order.
    // Note: this isn't perfect detection. It's possible the user is inside some other object; if so,
    // this object iterator will blithely scan that object for fields.
    if (_json_iter->depth() < depth() - 1) { return OUT_OF_ORDER_ITERATION; }
#endif
    return false;

  // 3. When a previous search found a field or an iterator yielded a value:
  //
  //    ```
  //    // When a field was not fully consumed (or not even touched at all)
  //    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
  //           ^ (depth 2)
  //    // When a field was fully consumed
  //    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
  //                   ^ (depth 1)
  //    // When the last field was fully consumed
  //    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
  //                                   ^ (depth 1)
  //    ```
  //
  } else {
    if ((error = skip_child() )) { abandon(); return error; }
    if ((error = has_next_field().get(has_value) )) { abandon(); return error; }
#ifdef SIMDJSON_DEVELOPMENT_CHECKS
    if (_json_iter->start_position(_depth) != start_position()) { return OUT_OF_ORDER_ITERATION; }
#endif
  }
  while (has_value) {
    // Get the key and colon, stopping at the value.
    raw_json_string actual_key;
    // size_t max_key_length = _json_iter->peek_length() - 2; // -2 for the two quotes
    // field_key() advances the pointer and checks that '"' is found (corresponding to a key).
    // The depth is left unchanged by field_key().
    if ((error = field_key().get(actual_key) )) { abandon(); return error; };
    // field_value() will advance and check that we find a ':' separating the
    // key and the value. It will also increment the depth by one.
    if ((error = field_value() )) { abandon(); return error; }
    // If it matches, stop and return
    // We could do it this way if we wanted to allow arbitrary
    // key content (including escaped quotes).
    //if (actual_key.unsafe_is_equal(max_key_length, key)) {
    // Instead we do the following which may trigger buffer overruns if the
    // user provides an adversarial key (containing a well placed unescaped quote
    // character and being longer than the number of bytes remaining in the JSON
    // input).
    if (actual_key.unsafe_is_equal(key)) {
      logger::log_event(*this, "match", key, -2);
      // If we return here, then we return while pointing at the ':' that we just checked.
      return true;
    }

    // No match: skip the value and see if , or } is next
    logger::log_event(*this, "no match", key, -2);
    // The call to skip_child is meant to skip over the value corresponding to the key.
    // After skip_child(), we are right before the next comma (',') or the final brace ('}').
    SIMDJSON_TRY( skip_child() ); // Skip the value entirely
    // The has_next_field() advances the pointer and check that either ',' or '}' is found.
    // It returns true if ',' is found, false otherwise. If anything other than ',' or '}' is found,
    // then we are in error and we abort.
    if ((error = has_next_field().get(has_value) )) { abandon(); return error; }
  }

  // If the loop ended, we're out of fields to look at.
  return false;
}

simdjson_warn_unused simdjson_really_inline simdjson_result<bool> value_iterator::find_field_unordered_raw(const std::string_view key) noexcept {
  /**
   * When find_field_unordered_raw is called, we can either be pointing at the
   * first key, pointing outside (at the closing brace) or if a key was matched
   * we can be either pointing right afterthe ':' right before the value (that we need skip),
   * or we may have consumed the value and we might be at a comma or at the
   * final brace (ready for a call to has_next_field()).
   */
  error_code error;
  bool has_value;

  // First, we scan from that point to the end.
  // If we don't find a match, we may loop back around, and scan from the beginning to that point.
  token_position search_start = _json_iter->position();

  // We want to know whether we need to go back to the beginning.
  bool at_first = at_first_field();
  ///////////////
  // Initially, the object can be in one of a few different places:
  //
  // 1. At the first key:
  //
  //    ```
  //    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
  //      ^ (depth 2, index 1)
  //    ```
  //
  if (at_first) {
    has_value = true;

  // 2. When a previous search did not yield a value or the object is empty:
  //
  //    ```
  //    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
  //                                     ^ (depth 0)
  //    { }
  //        ^ (depth 0, index 2)
  //    ```
  //
  } else if (!is_open()) {

#ifdef SIMDJSON_DEVELOPMENT_CHECKS
    // If we're past the end of the object, we're being iterated out of order.
    // Note: this isn't perfect detection. It's possible the user is inside some other object; if so,
    // this object iterator will blithely scan that object for fields.
    if (_json_iter->depth() < depth() - 1) { return OUT_OF_ORDER_ITERATION; }
#endif
    SIMDJSON_TRY(reset_object().get(has_value));
    at_first = true;
  // 3. When a previous search found a field or an iterator yielded a value:
  //
  //    ```
  //    // When a field was not fully consumed (or not even touched at all)
  //    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
  //           ^ (depth 2)
  //    // When a field was fully consumed
  //    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
  //                   ^ (depth 1)
  //    // When the last field was fully consumed
  //    { "a": [ 1, 2 ], "b": [ 3, 4 ] }
  //                                   ^ (depth 1)
  //    ```
  //
  } else {
    // If someone queried a key but they not did access the value, then we are left pointing
    // at the ':' and we need to move forward through the value... If the value was
    // processed then skip_child() does not move the iterator (but may adjust the depth).
    if ((error = skip_child() )) { abandon(); return error; }
    search_start = _json_iter->position();
    if ((error = has_next_field().get(has_value) )) { abandon(); return error; }
#ifdef SIMDJSON_DEVELOPMENT_CHECKS
    if (_json_iter->start_position(_depth) != start_position()) { return OUT_OF_ORDER_ITERATION; }
#endif
  }

  // After initial processing, we will be in one of two states:
  //
  // ```
  // // At the beginning of a field
  // { "a": [ 1, 2 ], "b": [ 3, 4 ] }
  //   ^ (depth 1)
  // { "a": [ 1, 2 ], "b": [ 3, 4 ] }
  //                  ^ (depth 1)
  // // At the end of the object
  // { "a": [ 1, 2 ], "b": [ 3, 4 ] }
  //                                  ^ (depth 0)
  // ```
  //
  // Next, we find a match starting from the current position.
  while (has_value) {
    SIMDJSON_ASSUME( _json_iter->_depth == _depth ); // We must be at the start of a field

    // Get the key and colon, stopping at the value.
    raw_json_string actual_key;
    // size_t max_key_length = _json_iter->peek_length() - 2; // -2 for the two quotes
    // field_key() advances the pointer and checks that '"' is found (corresponding to a key).
    // The depth is left unchanged by field_key().
    if ((error = field_key().get(actual_key) )) { abandon(); return error; };
    // field_value() will advance and check that we find a ':' separating the
    // key and the value. It will also increment the depth by one.
    if ((error = field_value() )) { abandon(); return error; }

    // If it matches, stop and return
    // We could do it this way if we wanted to allow arbitrary
    // key content (including escaped quotes).
    // if (actual_key.unsafe_is_equal(max_key_length, key)) {
    // Instead we do the following which may trigger buffer overruns if the
    // user provides an adversarial key (containing a well placed unescaped quote
    // character and being longer than the number of bytes remaining in the JSON
    // input).
    if (actual_key.unsafe_is_equal(key)) {
      logger::log_event(*this, "match", key, -2);
      // If we return here, then we return while pointing at the ':' that we just checked.
      return true;
    }

    // No match: skip the value and see if , or } is next
    logger::log_event(*this, "no match", key, -2);
    // The call to skip_child is meant to skip over the value corresponding to the key.
    // After skip_child(), we are right before the next comma (',') or the final brace ('}').
    SIMDJSON_TRY( skip_child() );
    // The has_next_field() advances the pointer and check that either ',' or '}' is found.
    // It returns true if ',' is found, false otherwise. If anything other than ',' or '}' is found,
    // then we are in error and we abort.
    if ((error = has_next_field().get(has_value) )) { abandon(); return error; }
  }
  // Performance note: it maybe wasteful to rewind to the beginning when there might be
  // no other query following. Indeed, it would require reskipping the whole object.
  // Instead, you can just stay where you are. If there is a new query, there is always time
  // to rewind.
  if(at_first) { return false; }

  // If we reach the end without finding a match, search the rest of the fields starting at the
  // beginning of the object.
  // (We have already run through the object before, so we've already validated its structure. We
  // don't check errors in this bit.)
  SIMDJSON_TRY(reset_object().get(has_value));
  while (true) {
    SIMDJSON_ASSUME(has_value); // we should reach search_start before ever reaching the end of the object
    SIMDJSON_ASSUME( _json_iter->_depth == _depth ); // We must be at the start of a field

    // Get the key and colon, stopping at the value.
    raw_json_string actual_key;
    // size_t max_key_length = _json_iter->peek_length() - 2; // -2 for the two quotes
    // field_key() advances the pointer and checks that '"' is found (corresponding to a key).
    // The depth is left unchanged by field_key().
    error = field_key().get(actual_key); SIMDJSON_ASSUME(!error);
    // field_value() will advance and check that we find a ':' separating the
    // key and the value.  It will also increment the depth by one.
    error = field_value(); SIMDJSON_ASSUME(!error);

    // If it matches, stop and return
    // We could do it this way if we wanted to allow arbitrary
    // key content (including escaped quotes).
    // if (actual_key.unsafe_is_equal(max_key_length, key)) {
    // Instead we do the following which may trigger buffer overruns if the
    // user provides an adversarial key (containing a well placed unescaped quote
    // character and being longer than the number of bytes remaining in the JSON
    // input).
    if (actual_key.unsafe_is_equal(key)) {
      logger::log_event(*this, "match", key, -2);
      // If we return here, then we return while pointing at the ':' that we just checked.
      return true;
    }

    // No match: skip the value and see if , or } is next
    logger::log_event(*this, "no match", key, -2);
    // The call to skip_child is meant to skip over the value corresponding to the key.
    // After skip_child(), we are right before the next comma (',') or the final brace ('}').
    SIMDJSON_TRY( skip_child() );
    // If we reached the end of the key-value pair we started from, then we know
    // that the key is not there so we return false. We are either right before
    // the next comma or the final brace.
    if(_json_iter->position() == search_start) { return false; }
    // The has_next_field() advances the pointer and check that either ',' or '}' is found.
    // It returns true if ',' is found, false otherwise. If anything other than ',' or '}' is found,
    // then we are in error and we abort.
    error = has_next_field().get(has_value); SIMDJSON_ASSUME(!error);
    // If we make the mistake of exiting here, then we could be left pointing at a key
    // in the middle of an object. That's not an allowable state.
  }
  // If the loop ended, we're out of fields to look at. The program should
  // never reach this point.
  return false;
}

simdjson_warn_unused simdjson_really_inline simdjson_result<raw_json_string> value_iterator::field_key() noexcept {
  assert_at_next();

  const uint8_t *key = _json_iter->return_current_and_advance();
  if (*(key++) != '"') { return report_error(TAPE_ERROR, "Object key is not a string"); }
  return raw_json_string(key);
}

simdjson_warn_unused simdjson_really_inline error_code value_iterator::field_value() noexcept {
  assert_at_next();

  if (*_json_iter->return_current_and_advance() != ':') { return report_error(TAPE_ERROR, "Missing colon in object field"); }
  _json_iter->descend_to(depth()+1);
  return SUCCESS;
}

simdjson_warn_unused simdjson_really_inline simdjson_result<bool> value_iterator::start_array() noexcept {
  SIMDJSON_TRY( start_container('[', "Not an array", "array") );
  return started_array();
}

simdjson_warn_unused simdjson_really_inline simdjson_result<bool> value_iterator::start_root_array() noexcept {
  SIMDJSON_TRY( start_container('[', "Not an array", "array") );
  return started_root_array();
}

inline std::string value_iterator::to_string() const noexcept {
  auto answer = std::string("value_iterator [ depth : ") + std::to_string(_depth) + std::string(", ");
  if(_json_iter != nullptr) { answer +=  _json_iter->to_string(); }
  answer += std::string(" ]");
  return answer;
}

simdjson_warn_unused simdjson_really_inline simdjson_result<bool> value_iterator::started_array() noexcept {
  assert_at_container_start();
  if (*_json_iter->peek() == ']') {
    logger::log_value(*_json_iter, "empty array");
    _json_iter->return_current_and_advance();
    SIMDJSON_TRY( end_container() );
    return false;
  }
  _json_iter->descend_to(depth()+1);
#ifdef SIMDJSON_DEVELOPMENT_CHECKS
  _json_iter->set_start_position(_depth, start_position());
#endif
  return true;
}

simdjson_warn_unused simdjson_really_inline simdjson_result<bool> value_iterator::started_root_array() noexcept {
  if (*_json_iter->peek_last() != ']') {
    return report_error(INCOMPLETE_ARRAY_OR_OBJECT, "missing ] at end");
  }
  return started_array();
}

simdjson_warn_unused simdjson_really_inline simdjson_result<bool> value_iterator::has_next_element() noexcept {
  assert_at_next();

  logger::log_event(*this, "has_next_element");
  switch (*_json_iter->return_current_and_advance()) {
    case ']':
      logger::log_end_value(*_json_iter, "array");
      SIMDJSON_TRY( end_container() );
      return false;
    case ',':
      _json_iter->descend_to(depth()+1);
      return true;
    default:
      return report_error(TAPE_ERROR, "Missing comma between array elements");
  }
}

simdjson_warn_unused simdjson_really_inline simdjson_result<bool> value_iterator::parse_bool(const uint8_t *json) const noexcept {
  auto not_true = atomparsing::str4ncmp(json, "true");
  auto not_false = atomparsing::str4ncmp(json, "fals") | (json[4] ^ 'e');
  bool error = (not_true && not_false) || jsoncharutils::is_not_structural_or_whitespace(json[not_true ? 5 : 4]);
  if (error) { return incorrect_type_error("Not a boolean"); }
  return simdjson_result<bool>(!not_true);
}
simdjson_really_inline bool value_iterator::parse_null(const uint8_t *json) const noexcept {
  return !atomparsing::str4ncmp(json, "null") && jsoncharutils::is_structural_or_whitespace(json[4]);
}

simdjson_warn_unused simdjson_really_inline simdjson_result<std::string_view> value_iterator::get_string() noexcept {
  return get_raw_json_string().unescape(_json_iter->string_buf_loc());
}
simdjson_warn_unused simdjson_really_inline simdjson_result<raw_json_string> value_iterator::get_raw_json_string() noexcept {
  auto json = peek_scalar("string");
  if (*json != '"') { return incorrect_type_error("Not a string"); }
  advance_scalar("string");
  return raw_json_string(json+1);
}
simdjson_warn_unused simdjson_really_inline simdjson_result<uint64_t> value_iterator::get_uint64() noexcept {
  auto result = numberparsing::parse_unsigned(peek_non_root_scalar("uint64"));
  if(result.error() != INCORRECT_TYPE) { advance_non_root_scalar("uint64"); }
  return result;
}
simdjson_warn_unused simdjson_really_inline simdjson_result<int64_t> value_iterator::get_int64() noexcept {
  auto result = numberparsing::parse_integer(peek_non_root_scalar("int64"));
  if(result.error() != INCORRECT_TYPE) { advance_non_root_scalar("int64"); }
  return result;
}
simdjson_warn_unused simdjson_really_inline simdjson_result<double> value_iterator::get_double() noexcept {
  auto result = numberparsing::parse_double(peek_non_root_scalar("double"));
  if(result.error() != INCORRECT_TYPE) { advance_non_root_scalar("double"); }
  return result;
}
simdjson_warn_unused simdjson_really_inline simdjson_result<bool> value_iterator::get_bool() noexcept {
  auto result = parse_bool(peek_non_root_scalar("bool"));
  if(result.error() != INCORRECT_TYPE) { advance_non_root_scalar("bool"); }
  return result;
}
simdjson_really_inline bool value_iterator::is_null() noexcept {
  auto result = parse_null(peek_non_root_scalar("null"));
  if(result) { advance_non_root_scalar("null"); }
  return result;
}

constexpr const uint32_t MAX_INT_LENGTH = 1024;

simdjson_warn_unused simdjson_really_inline simdjson_result<std::string_view> value_iterator::get_root_string() noexcept {
  return get_string();
}
simdjson_warn_unused simdjson_really_inline simdjson_result<raw_json_string> value_iterator::get_root_raw_json_string() noexcept {
  return get_raw_json_string();
}
simdjson_warn_unused simdjson_really_inline simdjson_result<uint64_t> value_iterator::get_root_uint64() noexcept {
  auto max_len = peek_start_length();
  auto json = peek_root_scalar("uint64");
  uint8_t tmpbuf[20+1]; // <20 digits> is the longest possible unsigned integer
  if (!_json_iter->copy_to_buffer(json, max_len, tmpbuf)) {
    logger::log_error(*_json_iter, start_position(), depth(), "Root number more than 20 characters");
    return NUMBER_ERROR;
  }
  auto result = numberparsing::parse_unsigned(tmpbuf);
  if(result.error() != INCORRECT_TYPE) { advance_root_scalar("uint64"); }
  return result;
}
simdjson_warn_unused simdjson_really_inline simdjson_result<int64_t> value_iterator::get_root_int64() noexcept {
  auto max_len = peek_start_length();
  auto json = peek_root_scalar("int64");
  uint8_t tmpbuf[20+1]; // -<19 digits> is the longest possible integer
  if (!_json_iter->copy_to_buffer(json, max_len, tmpbuf)) {
    logger::log_error(*_json_iter, start_position(), depth(), "Root number more than 20 characters");
    return NUMBER_ERROR;
  }

  auto result = numberparsing::parse_integer(tmpbuf);
  if(result.error() != INCORRECT_TYPE) { advance_root_scalar("int64"); }
  return result;
}
simdjson_warn_unused simdjson_really_inline simdjson_result<double> value_iterator::get_root_double() noexcept {
  auto max_len = peek_start_length();
  auto json = peek_root_scalar("double");
  // Per https://www.exploringbinary.com/maximum-number-of-decimal-digits-in-binary-floating-point-numbers/,
  // 1074 is the maximum number of significant fractional digits. Add 8 more digits for the biggest
  // number: -0.<fraction>e-308.
  uint8_t tmpbuf[1074+8+1];
  if (!_json_iter->copy_to_buffer(json, max_len, tmpbuf)) {
    logger::log_error(*_json_iter, start_position(), depth(), "Root number more than 1082 characters");
    return NUMBER_ERROR;
  }
  auto result = numberparsing::parse_double(tmpbuf);
  if(result.error() != INCORRECT_TYPE) { advance_root_scalar("double"); }
  return result;
}
simdjson_warn_unused simdjson_really_inline simdjson_result<bool> value_iterator::get_root_bool() noexcept {
  auto max_len = peek_start_length();
  auto json = peek_root_scalar("bool");
  uint8_t tmpbuf[5+1];
  if (!_json_iter->copy_to_buffer(json, max_len, tmpbuf)) { return incorrect_type_error("Not a boolean"); }
  advance_root_scalar("bool");
  return parse_bool(tmpbuf);
}
simdjson_really_inline bool value_iterator::is_root_null() noexcept {
  auto max_len = peek_start_length();
  auto json = peek_root_scalar("null");
  auto result = (max_len >= 4 && !atomparsing::str4ncmp(json, "null") &&
         (max_len == 4 || jsoncharutils::is_structural_or_whitespace(json[5])));
  if(result) { advance_root_scalar("null"); }
  return result;
}

simdjson_warn_unused simdjson_really_inline error_code value_iterator::skip_child() noexcept {
  SIMDJSON_ASSUME( _json_iter->token._position > _start_position );
  SIMDJSON_ASSUME( _json_iter->_depth >= _depth );

  return _json_iter->skip_child(depth());
}

simdjson_really_inline value_iterator value_iterator::child() const noexcept {
  assert_at_child();
  return { _json_iter, depth()+1, _json_iter->token.position() };
}

// GCC 7 warns when the first line of this function is inlined away into oblivion due to the caller
// relating depth and iterator depth, which is a desired effect. It does not happen if is_open is
// marked non-inline.
SIMDJSON_PUSH_DISABLE_WARNINGS
SIMDJSON_DISABLE_STRICT_OVERFLOW_WARNING
simdjson_really_inline bool value_iterator::is_open() const noexcept {
  return _json_iter->depth() >= depth();
}
SIMDJSON_POP_DISABLE_WARNINGS

simdjson_really_inline bool value_iterator::at_end() const noexcept {
  return _json_iter->at_end();
}

simdjson_really_inline bool value_iterator::at_start() const noexcept {
  return _json_iter->token.position() == start_position();
}

simdjson_really_inline bool value_iterator::at_first_field() const noexcept {
  SIMDJSON_ASSUME( _json_iter->token._position > _start_position );
  return _json_iter->token.position() == start_position() + 1;
}

simdjson_really_inline void value_iterator::abandon() noexcept {
  _json_iter->abandon();
}

simdjson_warn_unused simdjson_really_inline depth_t value_iterator::depth() const noexcept {
  return _depth;
}
simdjson_warn_unused simdjson_really_inline error_code value_iterator::error() const noexcept {
  return _json_iter->error;
}
simdjson_warn_unused simdjson_really_inline uint8_t *&value_iterator::string_buf_loc() noexcept {
  return _json_iter->string_buf_loc();
}
simdjson_warn_unused simdjson_really_inline const json_iterator &value_iterator::json_iter() const noexcept {
  return *_json_iter;
}
simdjson_warn_unused simdjson_really_inline json_iterator &value_iterator::json_iter() noexcept {
  return *_json_iter;
}

simdjson_really_inline const uint8_t *value_iterator::peek_start() const noexcept {
  return _json_iter->peek(start_position());
}
simdjson_really_inline uint32_t value_iterator::peek_start_length() const noexcept {
  return _json_iter->peek_length(start_position());
}

simdjson_really_inline const uint8_t *value_iterator::peek_scalar(const char *type) noexcept {
  logger::log_value(*_json_iter, start_position(), depth(), type);
  // If we're not at the position anymore, we don't want to advance the cursor.
  if (!is_at_start()) { return peek_start(); }

  // Get the JSON and advance the cursor, decreasing depth to signify that we have retrieved the value.
  assert_at_start();
  return _json_iter->peek();
}

simdjson_really_inline void value_iterator::advance_scalar(const char *type) noexcept {
  logger::log_value(*_json_iter, start_position(), depth(), type);
  // If we're not at the position anymore, we don't want to advance the cursor.
  if (!is_at_start()) { return; }

  // Get the JSON and advance the cursor, decreasing depth to signify that we have retrieved the value.
  assert_at_start();
  _json_iter->return_current_and_advance();
  _json_iter->ascend_to(depth()-1);
}

simdjson_really_inline error_code value_iterator::start_container(uint8_t start_char, const char *incorrect_type_message, const char *type) noexcept {
  logger::log_start_value(*_json_iter, start_position(), depth(), type);
  // If we're not at the position anymore, we don't want to advance the cursor.
  const uint8_t *json;
  if (!is_at_start()) {
#ifdef SIMDJSON_DEVELOPMENT_CHECKS
    if (!is_at_iterator_start()) { return OUT_OF_ORDER_ITERATION; }
#endif
    json = peek_start();
    if (*json != start_char) { return incorrect_type_error(incorrect_type_message); }
  } else {
    assert_at_start();
    /**
     * We should be prudent. Let us peek. If it is not the right type, we
     * return an error. Only once we have determined that we have the right
     * type are we allowed to advance!
     */
    json = _json_iter->peek();
    if (*json != start_char) { return incorrect_type_error(incorrect_type_message); }
    _json_iter->return_current_and_advance();
  }


  return SUCCESS;
}


simdjson_really_inline const uint8_t *value_iterator::peek_root_scalar(const char *type) noexcept {
  logger::log_value(*_json_iter, start_position(), depth(), type);
  if (!is_at_start()) { return peek_start(); }

  assert_at_root();
  return _json_iter->peek();
}
simdjson_really_inline const uint8_t *value_iterator::peek_non_root_scalar(const char *type) noexcept {
  logger::log_value(*_json_iter, start_position(), depth(), type);
  if (!is_at_start()) { return peek_start(); }

  assert_at_non_root_start();
  return _json_iter->peek();
}

simdjson_really_inline void value_iterator::advance_root_scalar(const char *type) noexcept {
  logger::log_value(*_json_iter, start_position(), depth(), type);
  if (!is_at_start()) { return; }

  assert_at_root();
  _json_iter->return_current_and_advance();
  _json_iter->ascend_to(depth()-1);
}
simdjson_really_inline void value_iterator::advance_non_root_scalar(const char *type) noexcept {
  logger::log_value(*_json_iter, start_position(), depth(), type);
  if (!is_at_start()) { return; }

  assert_at_non_root_start();
  _json_iter->return_current_and_advance();
  _json_iter->ascend_to(depth()-1);
}

simdjson_really_inline error_code value_iterator::incorrect_type_error(const char *message) const noexcept {
  logger::log_error(*_json_iter, start_position(), depth(), message);
  return INCORRECT_TYPE;
}

simdjson_really_inline bool value_iterator::is_at_start() const noexcept {
  return position() == start_position();
}

simdjson_really_inline bool value_iterator::is_at_key() const noexcept {
  // Keys are at the same depth as the object.
  // Note here that we could be safer and check that we are within an object,
  // but we do not.
  return _depth == _json_iter->_depth && *_json_iter->peek() == '"';
}

simdjson_really_inline bool value_iterator::is_at_iterator_start() const noexcept {
  // We can legitimately be either at the first value ([1]), or after the array if it's empty ([]).
  auto delta = position() - start_position();
  return delta == 1 || delta == 2;
}

inline void value_iterator::assert_at_start() const noexcept {
  SIMDJSON_ASSUME( _json_iter->token._position == _start_position );
  SIMDJSON_ASSUME( _json_iter->_depth == _depth );
  SIMDJSON_ASSUME( _depth > 0 );
}

inline void value_iterator::assert_at_container_start() const noexcept {
  SIMDJSON_ASSUME( _json_iter->token._position == _start_position + 1 );
  SIMDJSON_ASSUME( _json_iter->_depth == _depth );
  SIMDJSON_ASSUME( _depth > 0 );
}

inline void value_iterator::assert_at_next() const noexcept {
  SIMDJSON_ASSUME( _json_iter->token._position > _start_position );
  SIMDJSON_ASSUME( _json_iter->_depth == _depth );
  SIMDJSON_ASSUME( _depth > 0 );
}

simdjson_really_inline void value_iterator::move_at_start() noexcept {
  _json_iter->_depth = _depth;
  _json_iter->token.set_position(_start_position);
}

simdjson_really_inline void value_iterator::move_at_container_start() noexcept {
  _json_iter->_depth = _depth;
  _json_iter->token.set_position(_start_position + 1);
}

simdjson_really_inline simdjson_result<bool> value_iterator::reset_array() noexcept {
  move_at_container_start();
  return started_array();
}

simdjson_really_inline simdjson_result<bool> value_iterator::reset_object() noexcept {
  move_at_container_start();
  return started_object();
}

inline void value_iterator::assert_at_child() const noexcept {
  SIMDJSON_ASSUME( _json_iter->token._position > _start_position );
  SIMDJSON_ASSUME( _json_iter->_depth == _depth + 1 );
  SIMDJSON_ASSUME( _depth > 0 );
}

inline void value_iterator::assert_at_root() const noexcept {
  assert_at_start();
  SIMDJSON_ASSUME( _depth == 1 );
}

inline void value_iterator::assert_at_non_root_start() const noexcept {
  assert_at_start();
  SIMDJSON_ASSUME( _depth > 1 );
}

inline void value_iterator::assert_is_valid() const noexcept {
  SIMDJSON_ASSUME( _json_iter != nullptr );
}

simdjson_really_inline bool value_iterator::is_valid() const noexcept {
  return _json_iter != nullptr;
}

simdjson_really_inline simdjson_result<json_type> value_iterator::type() const noexcept {
  switch (*peek_start()) {
    case '{':
      return json_type::object;
    case '[':
      return json_type::array;
    case '"':
      return json_type::string;
    case 'n':
      return json_type::null;
    case 't': case 'f':
      return json_type::boolean;
    case '-':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      return json_type::number;
    default:
      return TAPE_ERROR;
  }
}

simdjson_really_inline token_position value_iterator::start_position() const noexcept {
  return _start_position;
}

simdjson_really_inline token_position value_iterator::position() const noexcept {
  return _json_iter->position();
}

simdjson_really_inline token_position value_iterator::end_position() const noexcept {
  return _json_iter->end_position();
}

simdjson_really_inline token_position value_iterator::last_position() const noexcept {
  return _json_iter->last_position();
}

simdjson_really_inline error_code value_iterator::report_error(error_code error, const char *message) noexcept {
  return _json_iter->report_error(error, message);
}

} // namespace ondemand
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson

namespace simdjson {

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value_iterator>::simdjson_result(SIMDJSON_IMPLEMENTATION::ondemand::value_iterator &&value) noexcept
    : implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::value_iterator>(std::forward<SIMDJSON_IMPLEMENTATION::ondemand::value_iterator>(value)) {}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value_iterator>::simdjson_result(error_code error) noexcept
    : implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::value_iterator>(error) {}

} // namespace simdjson
