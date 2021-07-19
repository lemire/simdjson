namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace ondemand {

simdjson_really_inline array_iterator::array_iterator(const value_iterator &_iter) noexcept
  : iter{_iter}
{}

simdjson_really_inline simdjson_result<value> array_iterator::operator*() noexcept {
  if (iter.error()) { iter.abandon(); return iter.error(); }
  return value(iter.child());
}
simdjson_really_inline bool array_iterator::operator==(const array_iterator &other) const noexcept {
  return !(*this != other);
}
simdjson_really_inline bool array_iterator::operator!=(const array_iterator &) const noexcept {
  return iter.is_open();
}
simdjson_really_inline array_iterator &array_iterator::operator++() noexcept {
  error_code error;
  // PERF NOTE this is a safety rail ... users should exit loops as soon as they receive an error, so we'll never get here.
  // However, it does not seem to make a perf difference, so we add it out of an abundance of caution.
  if (( error = iter.error() )) { return *this; }
  if (( error = iter.skip_child() )) { return *this; }
  if (( error = iter.has_next_element().error() )) { return *this; }
  return *this;
}

} // namespace ondemand
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson

namespace simdjson {

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::array_iterator>::simdjson_result(
  SIMDJSON_IMPLEMENTATION::ondemand::array_iterator &&value
) noexcept
  : SIMDJSON_IMPLEMENTATION::implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::array_iterator>(std::forward<SIMDJSON_IMPLEMENTATION::ondemand::array_iterator>(value))
{
  first.iter.assert_is_valid();
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::array_iterator>::simdjson_result(error_code error) noexcept
  : SIMDJSON_IMPLEMENTATION::implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::array_iterator>({}, error)
{
}

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::array_iterator>::operator*() noexcept {
  if (error()) { return error(); }
  return *first;
}
simdjson_really_inline bool simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::array_iterator>::operator==(const simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::array_iterator> &other) const noexcept {
  if (!first.iter.is_valid()) { return !error(); }
  return first == other.first;
}
simdjson_really_inline bool simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::array_iterator>::operator!=(const simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::array_iterator> &other) const noexcept {
  if (!first.iter.is_valid()) { return error(); }
  return first != other.first;
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::array_iterator> &simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::array_iterator>::operator++() noexcept {
  // Clear the error if there is one, so we don't yield it twice
  if (error()) { second = SUCCESS; return *this; }
  ++(first);
  return *this;
}

} // namespace simdjson
