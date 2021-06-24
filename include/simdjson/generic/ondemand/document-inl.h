namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace ondemand {

simdjson_really_inline document::document(ondemand::json_iterator &&_iter) noexcept
  : iter{std::forward<json_iterator>(_iter)}
{
  logger::log_start_value(iter, "document");
}

simdjson_really_inline document document::start(json_iterator &&iter) noexcept {
  return document(std::forward<json_iterator>(iter));
}

inline void document::rewind() noexcept {
  iter.rewind();
}

inline std::string document::to_debug_string() noexcept {
  return iter.to_string();
}

simdjson_really_inline value_iterator document::resume_value_iterator() noexcept {
  return value_iterator(&iter, 1, iter.root_position());
}
simdjson_really_inline value_iterator document::get_root_value_iterator() noexcept {
  return resume_value_iterator();
}
simdjson_really_inline value document::resume_value() noexcept {
  return resume_value_iterator();
}
simdjson_really_inline simdjson_result<array> document::get_array() & noexcept {
  auto value = get_root_value_iterator();
  return array::start_root(value);
}
simdjson_really_inline simdjson_result<object> document::get_object() & noexcept {
  auto value = get_root_value_iterator();
  return object::start_root(value);
}
simdjson_really_inline simdjson_result<uint64_t> document::get_uint64() noexcept {
  return get_root_value_iterator().get_root_uint64();
}
simdjson_really_inline simdjson_result<int64_t> document::get_int64() noexcept {
  return get_root_value_iterator().get_root_int64();
}
simdjson_really_inline simdjson_result<double> document::get_double() noexcept {
  return get_root_value_iterator().get_root_double();
}
simdjson_really_inline simdjson_result<std::string_view> document::get_string() noexcept {
  return get_root_value_iterator().get_root_string();
}
simdjson_really_inline simdjson_result<raw_json_string> document::get_raw_json_string() noexcept {
  return get_root_value_iterator().get_root_raw_json_string();
}
simdjson_really_inline simdjson_result<bool> document::get_bool() noexcept {
  return get_root_value_iterator().get_root_bool();
}
simdjson_really_inline bool document::is_null() noexcept {
  return get_root_value_iterator().is_root_null();
}

template<> simdjson_really_inline simdjson_result<array> document::get() & noexcept { return get_array(); }
template<> simdjson_really_inline simdjson_result<object> document::get() & noexcept { return get_object(); }
template<> simdjson_really_inline simdjson_result<raw_json_string> document::get() & noexcept { return get_raw_json_string(); }
template<> simdjson_really_inline simdjson_result<std::string_view> document::get() & noexcept { return get_string(); }
template<> simdjson_really_inline simdjson_result<double> document::get() & noexcept { return get_double(); }
template<> simdjson_really_inline simdjson_result<uint64_t> document::get() & noexcept { return get_uint64(); }
template<> simdjson_really_inline simdjson_result<int64_t> document::get() & noexcept { return get_int64(); }
template<> simdjson_really_inline simdjson_result<bool> document::get() & noexcept { return get_bool(); }

template<> simdjson_really_inline simdjson_result<raw_json_string> document::get() && noexcept { return get_raw_json_string(); }
template<> simdjson_really_inline simdjson_result<std::string_view> document::get() && noexcept { return get_string(); }
template<> simdjson_really_inline simdjson_result<double> document::get() && noexcept { return std::forward<document>(*this).get_double(); }
template<> simdjson_really_inline simdjson_result<uint64_t> document::get() && noexcept { return std::forward<document>(*this).get_uint64(); }
template<> simdjson_really_inline simdjson_result<int64_t> document::get() && noexcept { return std::forward<document>(*this).get_int64(); }
template<> simdjson_really_inline simdjson_result<bool> document::get() && noexcept { return std::forward<document>(*this).get_bool(); }

template<typename T> simdjson_really_inline error_code document::get(T &out) & noexcept {
  return get<T>().get(out);
}
template<typename T> simdjson_really_inline error_code document::get(T &out) && noexcept {
  return std::forward<document>(*this).get<T>().get(out);
}

#if SIMDJSON_EXCEPTIONS
simdjson_really_inline document::operator array() & noexcept(false) { return get_array(); }
simdjson_really_inline document::operator object() & noexcept(false) { return get_object(); }
simdjson_really_inline document::operator uint64_t() noexcept(false) { return get_uint64(); }
simdjson_really_inline document::operator int64_t() noexcept(false) { return get_int64(); }
simdjson_really_inline document::operator double() noexcept(false) { return get_double(); }
simdjson_really_inline document::operator std::string_view() noexcept(false) { return get_string(); }
simdjson_really_inline document::operator raw_json_string() noexcept(false) { return get_raw_json_string(); }
simdjson_really_inline document::operator bool() noexcept(false) { return get_bool(); }
#endif
simdjson_really_inline simdjson_result<size_t> document::count_elements() & noexcept {
  auto a = get_array();
  simdjson_result<size_t> answer = a.count_elements();
  /* If there was an array, we are now left pointing at its first element. */
  if(answer.error() == SUCCESS) { iter._depth -= 1 ; /* undoing the increment so we go back at the doc depth.*/ }
  return answer;
}
simdjson_really_inline simdjson_result<array_iterator> document::begin() & noexcept {
  return get_array().begin();
}
simdjson_really_inline simdjson_result<array_iterator> document::end() & noexcept {
  return {};
}

simdjson_really_inline simdjson_result<value> document::find_field(std::string_view key) & noexcept {
  return resume_value().find_field(key);
}
simdjson_really_inline simdjson_result<value> document::find_field(const char *key) & noexcept {
  return resume_value().find_field(key);
}
simdjson_really_inline simdjson_result<value> document::find_field_unordered(std::string_view key) & noexcept {
  return resume_value().find_field_unordered(key);
}
simdjson_really_inline simdjson_result<value> document::find_field_unordered(const char *key) & noexcept {
  return resume_value().find_field_unordered(key);
}
simdjson_really_inline simdjson_result<value> document::operator[](std::string_view key) & noexcept {
  return resume_value()[key];
}
simdjson_really_inline simdjson_result<value> document::operator[](const char *key) & noexcept {
  return resume_value()[key];
}

simdjson_really_inline simdjson_result<json_type> document::type() noexcept {
  return get_root_value_iterator().type();
}

simdjson_really_inline simdjson_result<std::string_view> document::raw_json_token() noexcept {
  auto _iter = get_root_value_iterator();
  return std::string_view(reinterpret_cast<const char*>(_iter.peek_start()), _iter.peek_start_length());
}

simdjson_really_inline simdjson_result<value> document::at_pointer(std::string_view json_pointer) noexcept {
  rewind(); // Rewind the document each time at_pointer is called
  if (json_pointer.empty()) {
    return this->resume_value();
  }
  json_type t;
  SIMDJSON_TRY(type().get(t));
  switch (t)
  {
    case json_type::array:
      return (*this).get_array().at_pointer(json_pointer);
    case json_type::object:
      return (*this).get_object().at_pointer(json_pointer);
    default:
      return INVALID_JSON_POINTER;
  }
}

} // namespace ondemand
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson

namespace simdjson {

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::simdjson_result(
  SIMDJSON_IMPLEMENTATION::ondemand::document &&value
) noexcept :
    implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::document>(
      std::forward<SIMDJSON_IMPLEMENTATION::ondemand::document>(value)
    )
{
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::simdjson_result(
  error_code error
) noexcept :
    implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::document>(
      error
    )
{
}
simdjson_really_inline simdjson_result<size_t> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::count_elements() & noexcept {
  if (error()) { return error(); }
  return first.count_elements();
}
simdjson_really_inline error_code simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::rewind() noexcept {
  if (error()) { return error(); }
  first.rewind();
  return SUCCESS;
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::array_iterator> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::begin() & noexcept {
  if (error()) { return error(); }
  return first.begin();
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::array_iterator> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::end() & noexcept {
  return {};
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::find_field_unordered(std::string_view key) & noexcept {
  if (error()) { return error(); }
  return first.find_field_unordered(key);
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::find_field_unordered(const char *key) & noexcept {
  if (error()) { return error(); }
  return first.find_field_unordered(key);
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::operator[](std::string_view key) & noexcept {
  if (error()) { return error(); }
  return first[key];
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::operator[](const char *key) & noexcept {
  if (error()) { return error(); }
  return first[key];
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::find_field(std::string_view key) & noexcept {
  if (error()) { return error(); }
  return first.find_field(key);
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::find_field(const char *key) & noexcept {
  if (error()) { return error(); }
  return first.find_field(key);
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::array> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::get_array() & noexcept {
  if (error()) { return error(); }
  return first.get_array();
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::object> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::get_object() & noexcept {
  if (error()) { return error(); }
  return first.get_object();
}
simdjson_really_inline simdjson_result<uint64_t> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::get_uint64() noexcept {
  if (error()) { return error(); }
  return first.get_uint64();
}
simdjson_really_inline simdjson_result<int64_t> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::get_int64() noexcept {
  if (error()) { return error(); }
  return first.get_int64();
}
simdjson_really_inline simdjson_result<double> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::get_double() noexcept {
  if (error()) { return error(); }
  return first.get_double();
}
simdjson_really_inline simdjson_result<std::string_view> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::get_string() noexcept {
  if (error()) { return error(); }
  return first.get_string();
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::raw_json_string> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::get_raw_json_string() noexcept {
  if (error()) { return error(); }
  return first.get_raw_json_string();
}
simdjson_really_inline simdjson_result<bool> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::get_bool() noexcept {
  if (error()) { return error(); }
  return first.get_bool();
}
simdjson_really_inline bool simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::is_null() noexcept {
  if (error()) { return error(); }
  return first.is_null();
}

template<typename T>
simdjson_really_inline simdjson_result<T> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::get() & noexcept {
  if (error()) { return error(); }
  return first.get<T>();
}
template<typename T>
simdjson_really_inline simdjson_result<T> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::get() && noexcept {
  if (error()) { return error(); }
  return std::forward<SIMDJSON_IMPLEMENTATION::ondemand::document>(first).get<T>();
}
template<typename T>
simdjson_really_inline error_code simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::get(T &out) & noexcept {
  if (error()) { return error(); }
  return first.get<T>(out);
}
template<typename T>
simdjson_really_inline error_code simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::get(T &out) && noexcept {
  if (error()) { return error(); }
  return std::forward<SIMDJSON_IMPLEMENTATION::ondemand::document>(first).get<T>(out);
}

template<> simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::get<SIMDJSON_IMPLEMENTATION::ondemand::document>() & noexcept = delete;
template<> simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::get<SIMDJSON_IMPLEMENTATION::ondemand::document>() && noexcept {
  if (error()) { return error(); }
  return std::forward<SIMDJSON_IMPLEMENTATION::ondemand::document>(first);
}
template<> simdjson_really_inline error_code simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::get<SIMDJSON_IMPLEMENTATION::ondemand::document>(SIMDJSON_IMPLEMENTATION::ondemand::document &out) & noexcept = delete;
template<> simdjson_really_inline error_code simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::get<SIMDJSON_IMPLEMENTATION::ondemand::document>(SIMDJSON_IMPLEMENTATION::ondemand::document &out) && noexcept {
  if (error()) { return error(); }
  out = std::forward<SIMDJSON_IMPLEMENTATION::ondemand::document>(first);
  return SUCCESS;
}

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::json_type> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::type() noexcept {
  if (error()) { return error(); }
  return first.type();
}

#if SIMDJSON_EXCEPTIONS
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::operator SIMDJSON_IMPLEMENTATION::ondemand::array() & noexcept(false) {
  if (error()) { throw simdjson_error(error()); }
  return first;
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::operator SIMDJSON_IMPLEMENTATION::ondemand::object() & noexcept(false) {
  if (error()) { throw simdjson_error(error()); }
  return first;
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::operator uint64_t() noexcept(false) {
  if (error()) { throw simdjson_error(error()); }
  return first;
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::operator int64_t() noexcept(false) {
  if (error()) { throw simdjson_error(error()); }
  return first;
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::operator double() noexcept(false) {
  if (error()) { throw simdjson_error(error()); }
  return first;
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::operator std::string_view() noexcept(false) {
  if (error()) { throw simdjson_error(error()); }
  return first;
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::operator SIMDJSON_IMPLEMENTATION::ondemand::raw_json_string() noexcept(false) {
  if (error()) { throw simdjson_error(error()); }
  return first;
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::operator bool() noexcept(false) {
  if (error()) { throw simdjson_error(error()); }
  return first;
}
#endif

simdjson_really_inline simdjson_result<std::string_view> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::raw_json_token() noexcept {
  if (error()) { return error(); }
  return first.raw_json_token();
}

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document>::at_pointer(std::string_view json_pointer) noexcept {
  if (error()) { return error(); }
  return first.at_pointer(json_pointer);
}

} // namespace simdjson
