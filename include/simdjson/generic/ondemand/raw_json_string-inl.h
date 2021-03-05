namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace ondemand {

simdjson_really_inline raw_json_string::raw_json_string(const uint8_t * _buf) noexcept : buf{_buf} {}

simdjson_really_inline const char * raw_json_string::raw() const noexcept { return reinterpret_cast<const char *>(buf); }
simdjson_really_inline simdjson_warn_unused simdjson_result<std::string_view> raw_json_string::unescape(uint8_t *&dst) const noexcept {
  uint8_t *end = stringparsing::parse_string(buf, dst);
  if (!end) { return STRING_ERROR; }
  std::string_view result(reinterpret_cast<const char *>(dst), end-dst);
  dst = end;
  return result;
}

simdjson_really_inline bool raw_json_string::unsafe_is_equal(size_t length, std::string_view target) const noexcept {
  // If we are going to call memcmp, then we must know something about the length of the raw_json_string.
  return (length >= target.size()) && (raw()[target.size()] == '"') && !memcmp(raw(), target.data(), target.size());
}

simdjson_really_inline bool raw_json_string::unsafe_is_equal(std::string_view target) const noexcept {
  // Assumptions: does not contain unescaped quote characters, and
  // the raw content is quote terminated within a valid JSON string.
  const char * r{raw()};
  size_t pos{0};
  for(;pos < target.size();pos++) {
    if(r[pos] != target[pos]) { return false; }
  }
  if(r[pos] != '"') { return false; }
  return true;
}

simdjson_really_inline bool raw_json_string::is_equal(std::string_view target) const noexcept {
  const char * r{raw()};
  size_t pos{0};
  bool escaping{false};
  for(;pos < target.size();pos++) {
    if(r[pos] != target[pos]) { return false; }
    if((r[pos] == '"') && !escaping) {
      // We have reached the end of the raw_json_string but
      // the target is not done.
      return false;
    } else if(r[pos] == '\\') {
      escaping = !escaping;
    } else {
      escaping = false;
    }
  }
  if(r[pos] != '"') { return false; }
  return true;
}


simdjson_really_inline bool raw_json_string::unsafe_is_equal(const char * target) const noexcept {
  // Assumptions: 'target' does not contain unescaped quote characters, is null terminated and
  // the raw content is quote terminated within a valid JSON string.
  const char * r{raw()};
  size_t pos{0};
  for(;target[pos];pos++) {
    if(r[pos] != target[pos]) { return false; }
  }
  if(r[pos] != '"') { return false; }
  return true;
}

simdjson_really_inline bool raw_json_string::is_equal(const char* target) const noexcept {
  // Assumptions: does not contain unescaped quote characters, and
  // the raw content is quote terminated within a valid JSON string.
  const char * r{raw()};
  size_t pos{0};
  bool escaping{false};
  for(;target[pos];pos++) {
    if(r[pos] != target[pos]) { return false; }
    if((r[pos] == '"') && !escaping) {
      // We have reached the end of the raw_json_string but
      // the target is not done.
      return false;
    } else if(r[pos] == '\\') {
      escaping = !escaping;
    } else {
      escaping = false;
    }
  }
  if(r[pos] != '"') { return false; }
  return true;
}

simdjson_unused simdjson_really_inline bool operator==(const raw_json_string &a, const char *c) noexcept {
  return a.unsafe_is_equal(c);
}

simdjson_unused simdjson_really_inline bool operator==(const char *c, const raw_json_string &a) noexcept {
  return a == c;
}

simdjson_unused simdjson_really_inline bool operator!=(const raw_json_string &a, const char *c) noexcept {
  return !(a == c);
}

simdjson_unused simdjson_really_inline bool operator!=(const char *c, const raw_json_string &a) noexcept {
  return !(a == c);
}

simdjson_really_inline simdjson_warn_unused simdjson_result<std::string_view> raw_json_string::unescape(json_iterator &iter) const noexcept {
  return unescape(iter.string_buf_loc());
}


simdjson_unused simdjson_really_inline std::ostream &operator<<(std::ostream &out, const raw_json_string &str) noexcept {
  bool in_escape = false;
  const char *s = str.raw();
  while (true) {
    switch (*s) {
      case '\\': in_escape = !in_escape; break;
      case '"': if (in_escape) { in_escape = false; } else { return out; } break;
      default: if (in_escape) { in_escape = false; }
    }
    out << *s;
    s++;
  }
}

} // namespace ondemand
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson

namespace simdjson {

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::raw_json_string>::simdjson_result(SIMDJSON_IMPLEMENTATION::ondemand::raw_json_string &&value) noexcept
    : implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::raw_json_string>(std::forward<SIMDJSON_IMPLEMENTATION::ondemand::raw_json_string>(value)) {}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::raw_json_string>::simdjson_result(error_code error) noexcept
    : implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::raw_json_string>(error) {}

simdjson_really_inline simdjson_result<const char *> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::raw_json_string>::raw() const noexcept {
  if (error()) { return error(); }
  return first.raw();
}
simdjson_really_inline simdjson_warn_unused simdjson_result<std::string_view> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::raw_json_string>::unescape(uint8_t *&dst) const noexcept {
  if (error()) { return error(); }
  return first.unescape(dst);
}
simdjson_really_inline simdjson_warn_unused simdjson_result<std::string_view> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::raw_json_string>::unescape(SIMDJSON_IMPLEMENTATION::ondemand::json_iterator &iter) const noexcept {
  if (error()) { return error(); }
  return first.unescape(iter);
}

} // namespace simdjson
