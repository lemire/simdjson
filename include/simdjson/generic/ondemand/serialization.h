

#include "simdjson/dom/serialization.h"
namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace ondemand {

template <class formatter = simdjson::internal::mini_formatter>
class string_builder {
public:
  /** Append an document to the builder (to be printed), numbers are
   * assumed to be 64-bit floating-point numbers.
   **/
  inline void append(document& value);
  /** Append an element to the builder (to be printed) **/
  inline void append(value element);
  /** Append an array to the builder (to be printed) **/
  inline void append(array value);
  /** Append an objet to the builder (to be printed) **/
  inline void append(object value);
  /** Append a field to the builder (to be printed) **/
  inline void append(field value);
  /** Reset the builder (so that it would print the empty string) **/
  simdjson_really_inline void clear();
  /**
   * Get access to the string. The string_view is owned by the builder
   * and it is invalid to use it after the string_builder has been
   * destroyed.
   * However you can make a copy of the string_view on memory that you
   * own.
   */
  simdjson_really_inline std::string_view str() const;
private: 
  formatter format{};
};

} // namespace ondemand
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson

namespace simdjson {

std::string to_string(simdjson::SIMDJSON_IMPLEMENTATION::ondemand::document& x)   {
    simdjson::SIMDJSON_IMPLEMENTATION::ondemand::string_builder<> sb;
    sb.append(x);
    std::string_view answer = sb.str();
    return std::string(answer.data(), answer.size());
}

std::string to_string(simdjson::SIMDJSON_IMPLEMENTATION::ondemand::value& x)   {
    simdjson::SIMDJSON_IMPLEMENTATION::ondemand::string_builder<> sb;
    sb.append(x);
    std::string_view answer = sb.str();
    return std::string(answer.data(), answer.size());
}

std::string to_string(simdjson::SIMDJSON_IMPLEMENTATION::ondemand::object& x)   {
    simdjson::SIMDJSON_IMPLEMENTATION::ondemand::string_builder<> sb;
    sb.append(x);
    std::string_view answer = sb.str();
    return std::string(answer.data(), answer.size());
}

std::string to_string(simdjson::SIMDJSON_IMPLEMENTATION::ondemand::array& x)   {
    simdjson::SIMDJSON_IMPLEMENTATION::ondemand::string_builder<> sb;
    sb.append(x);
    std::string_view answer = sb.str();
    return std::string(answer.data(), answer.size());
}


}// namespace simdjson
