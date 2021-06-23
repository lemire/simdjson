namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace ondemand {

simdjson_really_inline value::value(const value_iterator &_iter) noexcept
  : iter{_iter}
{
}
simdjson_really_inline value value::start(const value_iterator &iter) noexcept {
  return iter;
}
simdjson_really_inline value value::resume(const value_iterator &iter) noexcept {
  return iter;
}

simdjson_really_inline simdjson_result<array> value::get_array() noexcept {
  return array::start(iter);
}
simdjson_really_inline simdjson_result<object> value::get_object() noexcept {
  return object::start(iter);
}
simdjson_really_inline simdjson_result<object> value::start_or_resume_object() noexcept {
  if (iter.at_start()) {
    return get_object();
  } else {
    return object::resume(iter);
  }
}

simdjson_really_inline simdjson_result<raw_json_string> value::get_raw_json_string() noexcept {
  return iter.get_raw_json_string();
}
simdjson_really_inline simdjson_result<std::string_view> value::get_string() noexcept {
  return iter.get_string();
}
simdjson_really_inline simdjson_result<double> value::get_double() noexcept {
  return iter.get_double();
}
simdjson_really_inline simdjson_result<uint64_t> value::get_uint64() noexcept {
  return iter.get_uint64();
}
simdjson_really_inline simdjson_result<int64_t> value::get_int64() noexcept {
  return iter.get_int64();
}
simdjson_really_inline simdjson_result<bool> value::get_bool() noexcept {
  return iter.get_bool();
}
simdjson_really_inline bool value::is_null() noexcept {
  return iter.is_null();
}

template<> simdjson_really_inline simdjson_result<array> value::get() noexcept { return get_array(); }
template<> simdjson_really_inline simdjson_result<object> value::get() noexcept { return get_object(); }
template<> simdjson_really_inline simdjson_result<raw_json_string> value::get() noexcept { return get_raw_json_string(); }
template<> simdjson_really_inline simdjson_result<std::string_view> value::get() noexcept { return get_string(); }
template<> simdjson_really_inline simdjson_result<double> value::get() noexcept { return get_double(); }
template<> simdjson_really_inline simdjson_result<uint64_t> value::get() noexcept { return get_uint64(); }
template<> simdjson_really_inline simdjson_result<int64_t> value::get() noexcept { return get_int64(); }
template<> simdjson_really_inline simdjson_result<bool> value::get() noexcept { return get_bool(); }

template<typename T> simdjson_really_inline error_code value::get(T &out) noexcept {
  return get<T>().get(out);
}

#if SIMDJSON_EXCEPTIONS
simdjson_really_inline value::operator array() noexcept(false) {
  return get_array();
}
simdjson_really_inline value::operator object() noexcept(false) {
  return get_object();
}
simdjson_really_inline value::operator uint64_t() noexcept(false) {
  return get_uint64();
}
simdjson_really_inline value::operator int64_t() noexcept(false) {
  return get_int64();
}
simdjson_really_inline value::operator double() noexcept(false) {
  return get_double();
}
simdjson_really_inline value::operator std::string_view() noexcept(false) {
  return get_string();
}
simdjson_really_inline value::operator raw_json_string() noexcept(false) {
  return get_raw_json_string();
}
simdjson_really_inline value::operator bool() noexcept(false) {
  return get_bool();
}
#endif

simdjson_really_inline simdjson_result<array_iterator> value::begin() & noexcept {
  return get_array().begin();
}
simdjson_really_inline simdjson_result<array_iterator> value::end() & noexcept {
  return {};
}
simdjson_really_inline simdjson_result<size_t> value::count_elements() & noexcept {
  simdjson_result<size_t> answer;
  auto a = get_array();
  answer = a.count_elements();
  // count_elements leaves you pointing inside the array, at the first element.
  // We need to move back so that the user can create a new array (which requires that
  // we point at '[').
  iter.move_at_start();
  return answer;
}

simdjson_really_inline simdjson_result<value> value::find_field(std::string_view key) noexcept {
  return start_or_resume_object().find_field(key);
}
simdjson_really_inline simdjson_result<value> value::find_field(const char *key) noexcept {
  return start_or_resume_object().find_field(key);
}

simdjson_really_inline simdjson_result<value> value::find_field_unordered(std::string_view key) noexcept {
  return start_or_resume_object().find_field_unordered(key);
}
simdjson_really_inline simdjson_result<value> value::find_field_unordered(const char *key) noexcept {
  return start_or_resume_object().find_field_unordered(key);
}

simdjson_really_inline simdjson_result<value> value::operator[](std::string_view key) noexcept {
  return start_or_resume_object()[key];
}
simdjson_really_inline simdjson_result<value> value::operator[](const char *key) noexcept {
  return start_or_resume_object()[key];
}

simdjson_really_inline simdjson_result<json_type> value::type() noexcept {
  return iter.type();
}

simdjson_really_inline std::string_view value::raw_json_token() noexcept {
  return std::string_view(reinterpret_cast<const char*>(iter.peek_start()), iter.peek_start_length());
}

simdjson_really_inline simdjson_result<value> value::at_pointer(std::string_view json_pointer) noexcept {
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

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::simdjson_result(
  SIMDJSON_IMPLEMENTATION::ondemand::value &&value
) noexcept :
    implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::value>(
      std::forward<SIMDJSON_IMPLEMENTATION::ondemand::value>(value)
    )
{
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::simdjson_result(
  error_code error
) noexcept :
    implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::value>(error)
{
}
simdjson_really_inline simdjson_result<size_t> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::count_elements() & noexcept {
  if (error()) { return error(); }
  return first.count_elements();
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::array_iterator> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::begin() & noexcept {
  if (error()) { return error(); }
  return first.begin();
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::array_iterator> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::end() & noexcept {
  if (error()) { return error(); }
  return {};
}

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::find_field(std::string_view key) noexcept {
  if (error()) { return error(); }
  return first.find_field(key);
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::find_field(const char *key) noexcept {
  if (error()) { return error(); }
  return first.find_field(key);
}

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::find_field_unordered(std::string_view key) noexcept {
  if (error()) { return error(); }
  return first.find_field_unordered(key);
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::find_field_unordered(const char *key) noexcept {
  if (error()) { return error(); }
  return first.find_field_unordered(key);
}

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::operator[](std::string_view key) noexcept {
  if (error()) { return error(); }
  return first[key];
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::operator[](const char *key) noexcept {
  if (error()) { return error(); }
  return first[key];
}

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::array> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::get_array() noexcept {
  if (error()) { return error(); }
  return first.get_array();
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::object> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::get_object() noexcept {
  if (error()) { return error(); }
  return first.get_object();
}
simdjson_really_inline simdjson_result<uint64_t> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::get_uint64() noexcept {
  if (error()) { return error(); }
  return first.get_uint64();
}
simdjson_really_inline simdjson_result<int64_t> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::get_int64() noexcept {
  if (error()) { return error(); }
  return first.get_int64();
}
simdjson_really_inline simdjson_result<double> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::get_double() noexcept {
  if (error()) { return error(); }
  return first.get_double();
}
simdjson_really_inline simdjson_result<std::string_view> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::get_string() noexcept {
  if (error()) { return error(); }
  return first.get_string();
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::raw_json_string> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::get_raw_json_string() noexcept {
  if (error()) { return error(); }
  return first.get_raw_json_string();
}
simdjson_really_inline simdjson_result<bool> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::get_bool() noexcept {
  if (error()) { return error(); }
  return first.get_bool();
}
simdjson_really_inline bool simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::is_null() noexcept {
  if (error()) { return false; }
  return first.is_null();
}

template<typename T> simdjson_really_inline simdjson_result<T> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::get() noexcept {
  if (error()) { return error(); }
  return first.get<T>();
}
template<typename T> simdjson_really_inline error_code simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::get(T &out) noexcept {
  if (error()) { return error(); }
  return first.get<T>(out);
}

template<> simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::get<SIMDJSON_IMPLEMENTATION::ondemand::value>() noexcept  {
  if (error()) { return error(); }
  return std::move(first);
}
template<> simdjson_really_inline error_code simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::get<SIMDJSON_IMPLEMENTATION::ondemand::value>(SIMDJSON_IMPLEMENTATION::ondemand::value &out) noexcept {
  if (error()) { return error(); }
  out = first;
  return SUCCESS;
}

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::json_type> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::type() noexcept {
  if (error()) { return error(); }
  return first.type();
}

#if SIMDJSON_EXCEPTIONS
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::operator SIMDJSON_IMPLEMENTATION::ondemand::array() noexcept(false) {
  if (error()) { throw simdjson_error(error()); }
  return first;
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::operator SIMDJSON_IMPLEMENTATION::ondemand::object() noexcept(false) {
  if (error()) { throw simdjson_error(error()); }
  return first;
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::operator uint64_t() noexcept(false) {
  if (error()) { throw simdjson_error(error()); }
  return first;
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::operator int64_t() noexcept(false) {
  if (error()) { throw simdjson_error(error()); }
  return first;
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::operator double() noexcept(false) {
  if (error()) { throw simdjson_error(error()); }
  return first;
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::operator std::string_view() noexcept(false) {
  if (error()) { throw simdjson_error(error()); }
  return first;
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::operator SIMDJSON_IMPLEMENTATION::ondemand::raw_json_string() noexcept(false) {
  if (error()) { throw simdjson_error(error()); }
  return first;
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::operator bool() noexcept(false) {
  if (error()) { throw simdjson_error(error()); }
  return first;
}
#endif

simdjson_really_inline simdjson_result<std::string_view> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::raw_json_token() noexcept {
  if (error()) { return error(); }
  return first.raw_json_token();
}

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value>::at_pointer(std::string_view json_pointer) noexcept {
  if (error()) { return error(); }
  return first.at_pointer(json_pointer);
}

} // namespace simdjson
