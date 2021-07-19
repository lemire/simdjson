namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace ondemand {

simdjson_really_inline simdjson_result<value> object::find_field_unordered(const std::string_view key) & noexcept {
  bool has_value;
  SIMDJSON_TRY( iter.find_field_unordered_raw(key).get(has_value) );
  if (!has_value) { return NO_SUCH_FIELD; }
  return value(iter.child());
}
simdjson_really_inline simdjson_result<value> object::find_field_unordered(const std::string_view key) && noexcept {
  bool has_value;
  SIMDJSON_TRY( iter.find_field_unordered_raw(key).get(has_value) );
  if (!has_value) { return NO_SUCH_FIELD; }
  return value(iter.child());
}
simdjson_really_inline simdjson_result<value> object::operator[](const std::string_view key) & noexcept {
  return find_field_unordered(key);
}
simdjson_really_inline simdjson_result<value> object::operator[](const std::string_view key) && noexcept {
  return std::forward<object>(*this).find_field_unordered(key);
}
simdjson_really_inline simdjson_result<value> object::find_field(const std::string_view key) & noexcept {
  bool has_value;
  SIMDJSON_TRY( iter.find_field_raw(key).get(has_value) );
  if (!has_value) { return NO_SUCH_FIELD; }
  return value(iter.child());
}
simdjson_really_inline simdjson_result<value> object::find_field(const std::string_view key) && noexcept {
  bool has_value;
  SIMDJSON_TRY( iter.find_field_raw(key).get(has_value) );
  if (!has_value) { return NO_SUCH_FIELD; }
  return value(iter.child());
}

simdjson_really_inline simdjson_result<object> object::start(value_iterator &iter) noexcept {
  SIMDJSON_TRY( iter.start_object().error() );
  return object(iter);
}
simdjson_really_inline simdjson_result<object> object::start_root(value_iterator &iter) noexcept {
  SIMDJSON_TRY( iter.start_root_object().error() );
  return object(iter);
}
simdjson_really_inline error_code object::consume() noexcept {
  if(iter.is_at_key()) {
    /**
     * whenever you are pointing at a key, calling skip_child() is
     * unsafe because you will hit a string and you will assume that
     * it is string value, and this mistake will lead you to make bad
     * depth computation.
     */
    /**
     * We want to 'consume' the key. We could really
     * just do _json_iter->return_current_and_advance(); at this
     * point, but, for clarity, we will use the high-level API to
     * eat the key. We assume that the compiler optimizes away
     * most of the work.
     */
    simdjson_unused raw_json_string actual_key;
    auto error = iter.field_key().get(actual_key);
    if (error) { iter.abandon(); return error; };
    // Let us move to the value while we are at it.
    if ((error = iter.field_value())) { iter.abandon(); return error; }
  }
  auto error_skip = iter.json_iter().skip_child(iter.depth()-1);
  if(error_skip) { iter.abandon(); }
  return error_skip;
}

simdjson_really_inline simdjson_result<std::string_view> object::raw_json() noexcept {
  const uint8_t * starting_point{iter.peek_start()};
  auto error = consume();
  if(error) { return error; }
  const uint8_t * final_point{iter._json_iter->peek(0)};
  return std::string_view(reinterpret_cast<const char*>(starting_point), size_t(final_point - starting_point));
}

simdjson_really_inline simdjson_result<object> object::started(value_iterator &iter) noexcept {
  SIMDJSON_TRY( iter.started_object().error() );
  return object(iter);
}

simdjson_really_inline object object::resume(const value_iterator &iter) noexcept {
  return iter;
}

simdjson_really_inline object::object(const value_iterator &_iter) noexcept
  : iter{_iter}
{
}

simdjson_really_inline simdjson_result<object_iterator> object::begin() noexcept {
#ifdef SIMDJSON_DEVELOPMENT_CHECKS
  if (!iter.is_at_iterator_start()) { return OUT_OF_ORDER_ITERATION; }
#endif
  return object_iterator(iter);
}
simdjson_really_inline simdjson_result<object_iterator> object::end() noexcept {
  return object_iterator(iter);
}

inline simdjson_result<value> object::at_pointer(std::string_view json_pointer) noexcept {
  if (json_pointer[0] != '/') { return INVALID_JSON_POINTER; }
  json_pointer = json_pointer.substr(1);
  size_t slash = json_pointer.find('/');
  std::string_view key = json_pointer.substr(0, slash);
  // Grab the child with the given key
  simdjson_result<value> child;

  // If there is an escape character in the key, unescape it and then get the child.
  size_t escape = key.find('~');
  if (escape != std::string_view::npos) {
    // Unescape the key
    std::string unescaped(key);
    do {
      switch (unescaped[escape+1]) {
        case '0':
          unescaped.replace(escape, 2, "~");
          break;
        case '1':
          unescaped.replace(escape, 2, "/");
          break;
        default:
          return INVALID_JSON_POINTER; // "Unexpected ~ escape character in JSON pointer");
      }
      escape = unescaped.find('~', escape+1);
    } while (escape != std::string::npos);
    child = find_field(unescaped);  // Take note find_field does not unescape keys when matching
  } else {
    child = find_field(key);
  }
  if(child.error()) {
    return child; // we do not continue if there was an error
  }
  // If there is a /, we have to recurse and look up more of the path
  if (slash != std::string_view::npos) {
    child = child.at_pointer(json_pointer.substr(slash));
  }
  return child;
}

} // namespace ondemand
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson

namespace simdjson {

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::object>::simdjson_result(SIMDJSON_IMPLEMENTATION::ondemand::object &&value) noexcept
    : implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::object>(std::forward<SIMDJSON_IMPLEMENTATION::ondemand::object>(value)) {}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::object>::simdjson_result(error_code error) noexcept
    : implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::object>(error) {}

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::object_iterator> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::object>::begin() noexcept {
  if (error()) { return error(); }
  return first.begin();
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::object_iterator> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::object>::end() noexcept {
  if (error()) { return error(); }
  return first.end();
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::object>::find_field_unordered(std::string_view key) & noexcept {
  if (error()) { return error(); }
  return first.find_field_unordered(key);
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::object>::find_field_unordered(std::string_view key) && noexcept {
  if (error()) { return error(); }
  return std::forward<SIMDJSON_IMPLEMENTATION::ondemand::object>(first).find_field_unordered(key);
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::object>::operator[](std::string_view key) & noexcept {
  if (error()) { return error(); }
  return first[key];
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::object>::operator[](std::string_view key) && noexcept {
  if (error()) { return error(); }
  return std::forward<SIMDJSON_IMPLEMENTATION::ondemand::object>(first)[key];
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::object>::find_field(std::string_view key) & noexcept {
  if (error()) { return error(); }
  return first.find_field(key);
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::object>::find_field(std::string_view key) && noexcept {
  if (error()) { return error(); }
  return std::forward<SIMDJSON_IMPLEMENTATION::ondemand::object>(first).find_field(key);
}

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::object>::at_pointer(std::string_view json_pointer) noexcept {
  if (error()) { return error(); }
  return first.at_pointer(json_pointer);
}

} // namespace simdjson
