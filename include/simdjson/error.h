#ifndef SIMDJSON_ERROR_H
#define SIMDJSON_ERROR_H

#include "simdjson/common_defs.h"
#include <string>

namespace simdjson {

/**
 * All possible errors returned by simdjson.
 */
enum error_code {
  SUCCESS = 0,                ///< No error
  CAPACITY,                   ///< This parser can't support a document that big
  MEMALLOC,                   ///< Error allocating memory, most likely out of memory
  TAPE_ERROR,                 ///< Something went wrong while writing to the tape (stage 2), this is a generic error
  DEPTH_ERROR,                ///< Your document exceeds the user-specified depth limitation
  STRING_ERROR,               ///< Problem while parsing a string
  T_ATOM_ERROR,               ///< Problem while parsing an atom starting with the letter 't'
  F_ATOM_ERROR,               ///< Problem while parsing an atom starting with the letter 'f'
  N_ATOM_ERROR,               ///< Problem while parsing an atom starting with the letter 'n'
  NUMBER_ERROR,               ///< Problem while parsing a number
  UTF8_ERROR,                 ///< the input is not valid UTF-8
  UNINITIALIZED,              ///< unknown error, or uninitialized document
  EMPTY,                      ///< no structural element found
  UNESCAPED_CHARS,            ///< found unescaped characters in a string.
  UNCLOSED_STRING,            ///< missing quote at the end
  UNSUPPORTED_ARCHITECTURE,   ///< unsupported architecture
  INCORRECT_TYPE,             ///< JSON element has a different type than user expected
  NUMBER_OUT_OF_RANGE,        ///< JSON number does not fit in 64 bits
  INDEX_OUT_OF_BOUNDS,        ///< JSON array index too large
  NO_SUCH_FIELD,              ///< JSON field not found in object
  IO_ERROR,                   ///< Error reading a file
  INVALID_JSON_POINTER,       ///< Invalid JSON pointer reference
  INVALID_URI_FRAGMENT,       ///< Invalid URI fragment
  UNEXPECTED_ERROR,           ///< indicative of a bug in simdjson
  PARSER_IN_USE,              ///< parser is already in use.
  OUT_OF_ORDER_ITERATION,     ///< tried to iterate an array or object out of order
  INSUFFICIENT_PADDING,       ///< The JSON doesn't have enough padding for simdjson to safely parse it.
  INCOMPLETE_ARRAY_OR_OBJECT, ///< The document ends early.
  NUM_ERROR_CODES
};

/**
 * Get the error message for the given error code.
 *
 *   dom::parser parser;
 *   dom::element doc;
 *   auto error = parser.parse("foo",3).get(doc);
 *   if (error) { printf("Error: %s\n", error_message(error)); }
 *
 * @return The error message.
 */
inline const char *error_message(error_code error) noexcept;

/**
 * Write the error message to the output stream
 */
inline std::ostream& operator<<(std::ostream& out, error_code error) noexcept;

/**
 * Exception thrown when an exception-supporting simdjson method is called
 */
struct simdjson_error : public std::exception {
  /**
   * Create an exception from a simdjson error code.
   * @param error The error code
   */
  simdjson_error(error_code error) noexcept : _error{error} { }
  /** The error message */
  const char *what() const noexcept { return error_message(error()); }
  /** The error code */
  error_code error() const noexcept { return _error; }
private:
  /** The error code that was used */
  error_code _error;
};

namespace internal {

/**
 * The result of a simdjson operation that could fail.
 *
 * Gives the option of reading error codes, or throwing an exception by casting to the desired result.
 *
 * This is a base class for implementations that want to add functions to the result type for
 * chaining.
 *
 * Override like:
 *
 *   struct simdjson_result<T> : public internal::simdjson_result_base<T> {
 *     simdjson_result() noexcept : internal::simdjson_result_base<T>() {}
 *     simdjson_result(error_code error) noexcept : internal::simdjson_result_base<T>(error) {}
 *     simdjson_result(T &&value) noexcept : internal::simdjson_result_base<T>(std::forward(value)) {}
 *     simdjson_result(T &&value, error_code error) noexcept : internal::simdjson_result_base<T>(value, error) {}
 *     // Your extra methods here
 *   }
 *
 * Then any method returning simdjson_result<T> will be chainable with your methods.
 */
template<typename T>
struct simdjson_result_base : protected std::pair<T, error_code> {

  /**
   * Create a new empty result with error = UNINITIALIZED.
   */
  simdjson_really_inline simdjson_result_base() noexcept;

  /**
   * Create a new error result.
   */
  simdjson_really_inline simdjson_result_base(error_code error) noexcept;

  /**
   * Create a new successful result.
   */
  simdjson_really_inline simdjson_result_base(T &&value) noexcept;

  /**
   * Create a new result with both things (use if you don't want to branch when creating the result).
   */
  simdjson_really_inline simdjson_result_base(T &&value, error_code error) noexcept;

  /**
   * Move the value and the error to the provided variables.
   *
   * @param value The variable to assign the value to. May not be set if there is an error.
   * @param error The variable to assign the error to. Set to SUCCESS if there is no error.
   */
  simdjson_really_inline void tie(T &value, error_code &error) && noexcept;

  /**
   * Move the value to the provided variable.
   *
   * @param value The variable to assign the value to. May not be set if there is an error.
   */
  simdjson_really_inline error_code get(T &value) && noexcept;

  /**
   * Move the value to the provided variable.
   *
   * @param value The variable to assign the value to. May not be set if there is an error.
   */
  simdjson_really_inline const T &value(error_code &error) const & noexcept;

  /**
   * The error.
   */
  simdjson_really_inline error_code error() const noexcept;

#if SIMDJSON_EXCEPTIONS

  /**
   * Get the result value.
   *
   * @throw simdjson_error if there was an error.
   */
  simdjson_really_inline T& value() & noexcept(false);

  /**
   * Take the result value (move it).
   *
   * @throw simdjson_error if there was an error.
   */
  simdjson_really_inline T&& value() && noexcept(false);

  /**
   * Take the result value (move it).
   *
   * @throw simdjson_error if there was an error.
   */
  simdjson_really_inline T&& take_value() && noexcept(false);

  /**
   * Cast to the value (will throw on error).
   *
   * @throw simdjson_error if there was an error.
   */
  simdjson_really_inline operator T&&() && noexcept(false);
#endif // SIMDJSON_EXCEPTIONS

  /**
   * Get the result value. This function is safe if and only
   * the error() method returns a value that evaluates to false.
   */
  simdjson_really_inline const T& value_unsafe() const& noexcept;

  /**
   * Take the result value (move it). This function is safe if and only
   * the error() method returns a value that evaluates to false.
   */
  simdjson_really_inline T&& value_unsafe() && noexcept;

}; // struct simdjson_result_base

} // namespace internal

/**
 * The result of a simdjson operation that could fail.
 *
 * Gives the option of reading error codes, or throwing an exception by casting to the desired result.
 */
template<typename T>
struct simdjson_result : public internal::simdjson_result_base<T> {
  /**
   * @private Create a new empty result with error = UNINITIALIZED.
   */
  simdjson_really_inline simdjson_result() noexcept;
  /**
   * @private Create a new error result.
   */
  simdjson_really_inline simdjson_result(T &&value) noexcept;
  /**
   * @private Create a new successful result.
   */
  simdjson_really_inline simdjson_result(error_code error_code) noexcept;
  /**
   * @private Create a new result with both things (use if you don't want to branch when creating the result).
   */
  simdjson_really_inline simdjson_result(T &&value, error_code error) noexcept;

  /**
   * Move the value and the error to the provided variables.
   *
   * @param value The variable to assign the value to. May not be set if there is an error.
   * @param error The variable to assign the error to. Set to SUCCESS if there is no error.
   */
  simdjson_really_inline void tie(T &value, error_code &error) && noexcept;

  /**
   * Move the value to the provided variable.
   *
   * @param value The variable to assign the value to. May not be set if there is an error.
   */
  simdjson_warn_unused simdjson_really_inline error_code get(T &value) && noexcept;

  /**
   * The error.
   */
  simdjson_really_inline error_code error() const noexcept;

#if SIMDJSON_EXCEPTIONS

  /**
   * Get the result value.
   *
   * @throw simdjson_error if there was an error.
   */
  simdjson_really_inline T& value() & noexcept(false);

  /**
   * Take the result value (move it).
   *
   * @throw simdjson_error if there was an error.
   */
  simdjson_really_inline T&& value() && noexcept(false);

  /**
   * Take the result value (move it).
   *
   * @throw simdjson_error if there was an error.
   */
  simdjson_really_inline T&& take_value() && noexcept(false);

  /**
   * Cast to the value (will throw on error).
   *
   * @throw simdjson_error if there was an error.
   */
  simdjson_really_inline operator T&&() && noexcept(false);
#endif // SIMDJSON_EXCEPTIONS

  /**
   * Get the result value. This function is safe if and only
   * the error() method returns a value that evaluates to false.
   */
  simdjson_really_inline const T& value_unsafe() const& noexcept;

  /**
   * Take the result value (move it). This function is safe if and only
   * the error() method returns a value that evaluates to false.
   */
  simdjson_really_inline T&& value_unsafe() && noexcept;

}; // struct simdjson_result

#if SIMDJSON_EXCEPTIONS

template<typename T>
inline std::ostream& operator<<(std::ostream& out, simdjson_result<T> value) noexcept { return out << value.value(); }
#endif // SIMDJSON_EXCEPTIONS

#ifndef SIMDJSON_DISABLE_DEPRECATED_API
/**
 * @deprecated This is an alias and will be removed, use error_code instead
 */
using ErrorValues [[deprecated("This is an alias and will be removed, use error_code instead")]] = error_code;

/**
 * @deprecated Error codes should be stored and returned as `error_code`, use `error_message()` instead.
 */
[[deprecated("Error codes should be stored and returned as `error_code`, use `error_message()` instead.")]]
inline const std::string error_message(int error) noexcept;
#endif // SIMDJSON_DISABLE_DEPRECATED_API
} // namespace simdjson

#endif // SIMDJSON_ERROR_H
