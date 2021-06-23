#include "simdjson/error.h"

namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace ondemand {

class parser;
class array;
class object;
class value;
class raw_json_string;
class array_iterator;

/**
 * A JSON document iteration.
 *
 * Used by tokens to get text, and string buffer location.
 *
 * You must keep the document around during iteration.
 */
class document {
public:
  /**
   * Create a new invalid document.
   *
   * Exists so you can declare a variable and later assign to it before use.
   */
  simdjson_really_inline document() noexcept = default;
  simdjson_really_inline document(const document &other) noexcept = delete;
  simdjson_really_inline document(document &&other) noexcept = default;
  simdjson_really_inline document &operator=(const document &other) noexcept = delete;
  simdjson_really_inline document &operator=(document &&other) noexcept = default;

  /**
   * Cast this JSON value to an array.
   *
   * @returns An object that can be used to iterate the array.
   * @returns INCORRECT_TYPE If the JSON value is not an array.
   */
  simdjson_really_inline simdjson_result<array> get_array() & noexcept;
  /**
   * Cast this JSON value to an object.
   *
   * @returns An object that can be used to look up or iterate fields.
   * @returns INCORRECT_TYPE If the JSON value is not an object.
   */
  simdjson_really_inline simdjson_result<object> get_object() & noexcept;
  /**
   * Cast this JSON value to an unsigned integer.
   *
   * @returns A signed 64-bit integer.
   * @returns INCORRECT_TYPE If the JSON value is not a 64-bit unsigned integer.
   */
  simdjson_really_inline simdjson_result<uint64_t> get_uint64() noexcept;
  /**
   * Cast this JSON value to a signed integer.
   *
   * @returns A signed 64-bit integer.
   * @returns INCORRECT_TYPE If the JSON value is not a 64-bit integer.
   */
  simdjson_really_inline simdjson_result<int64_t> get_int64() noexcept;
  /**
   * Cast this JSON value to a double.
   *
   * @returns A double.
   * @returns INCORRECT_TYPE If the JSON value is not a valid floating-point number.
   */
  simdjson_really_inline simdjson_result<double> get_double() noexcept;
  /**
   * Cast this JSON value to a string.
   *
   * The string is guaranteed to be valid UTF-8.
   *
   * @returns An UTF-8 string. The string is stored in the parser and will be invalidated the next
   *          time it parses a document or when it is destroyed.
   * @returns INCORRECT_TYPE if the JSON value is not a string.
   */
  simdjson_really_inline simdjson_result<std::string_view> get_string() noexcept;
  /**
   * Cast this JSON value to a raw_json_string.
   *
   * The string is guaranteed to be valid UTF-8, and may have escapes in it (e.g. \\ or \n).
   *
   * @returns A pointer to the raw JSON for the given string.
   * @returns INCORRECT_TYPE if the JSON value is not a string.
   */
  simdjson_really_inline simdjson_result<raw_json_string> get_raw_json_string() noexcept;
  /**
   * Cast this JSON value to a bool.
   *
   * @returns A bool value.
   * @returns INCORRECT_TYPE if the JSON value is not true or false.
   */
  simdjson_really_inline simdjson_result<bool> get_bool() noexcept;
  /**
   * Checks if this JSON value is null.
   *
   * @returns Whether the value is null.
   */
  simdjson_really_inline bool is_null() noexcept;

  /**
   * Get this value as the given type.
   *
   * Supported types: object, array, raw_json_string, string_view, uint64_t, int64_t, double, bool
   *
   * You may use get_double(), get_bool(), get_uint64(), get_int64(),
   * get_object(), get_array(), get_raw_json_string(), or get_string() instead.
   *
   * @returns A value of the given type, parsed from the JSON.
   * @returns INCORRECT_TYPE If the JSON value is not the given type.
   */
  template<typename T> simdjson_really_inline simdjson_result<T> get() & noexcept {
    // Unless the simdjson library provides an inline implementation, calling this method should
    // immediately fail.
    static_assert(!sizeof(T), "The get method with given type is not implemented by the simdjson library.");
  }
  /** @overload template<typename T> simdjson_result<T> get() & noexcept */
  template<typename T> simdjson_really_inline simdjson_result<T> get() && noexcept {
    // Unless the simdjson library provides an inline implementation, calling this method should
    // immediately fail.
    static_assert(!sizeof(T), "The get method with given type is not implemented by the simdjson library.");
  }

  /**
   * Get this value as the given type.
   *
   * Supported types: object, array, raw_json_string, string_view, uint64_t, int64_t, double, bool
   *
   * @param out This is set to a value of the given type, parsed from the JSON. If there is an error, this may not be initialized.
   * @returns INCORRECT_TYPE If the JSON value is not an object.
   * @returns SUCCESS If the parse succeeded and the out parameter was set to the value.
   */
  template<typename T> simdjson_really_inline error_code get(T &out) & noexcept;
  /** @overload template<typename T> error_code get(T &out) & noexcept */
  template<typename T> simdjson_really_inline error_code get(T &out) && noexcept;

#if SIMDJSON_EXCEPTIONS
  /**
   * Cast this JSON value to an array.
   *
   * @returns An object that can be used to iterate the array.
   * @exception simdjson_error(INCORRECT_TYPE) If the JSON value is not an array.
   */
  simdjson_really_inline operator array() & noexcept(false);
  /**
   * Cast this JSON value to an object.
   *
   * @returns An object that can be used to look up or iterate fields.
   * @exception simdjson_error(INCORRECT_TYPE) If the JSON value is not an object.
   */
  simdjson_really_inline operator object() & noexcept(false);
  /**
   * Cast this JSON value to an unsigned integer.
   *
   * @returns A signed 64-bit integer.
   * @exception simdjson_error(INCORRECT_TYPE) If the JSON value is not a 64-bit unsigned integer.
   */
  simdjson_really_inline operator uint64_t() noexcept(false);
  /**
   * Cast this JSON value to a signed integer.
   *
   * @returns A signed 64-bit integer.
   * @exception simdjson_error(INCORRECT_TYPE) If the JSON value is not a 64-bit integer.
   */
  simdjson_really_inline operator int64_t() noexcept(false);
  /**
   * Cast this JSON value to a double.
   *
   * @returns A double.
   * @exception simdjson_error(INCORRECT_TYPE) If the JSON value is not a valid floating-point number.
   */
  simdjson_really_inline operator double() noexcept(false);
  /**
   * Cast this JSON value to a string.
   *
   * The string is guaranteed to be valid UTF-8.
   *
   * @returns An UTF-8 string. The string is stored in the parser and will be invalidated the next
   *          time it parses a document or when it is destroyed.
   * @exception simdjson_error(INCORRECT_TYPE) if the JSON value is not a string.
   */
  simdjson_really_inline operator std::string_view() noexcept(false);
  /**
   * Cast this JSON value to a raw_json_string.
   *
   * The string is guaranteed to be valid UTF-8, and may have escapes in it (e.g. \\ or \n).
   *
   * @returns A pointer to the raw JSON for the given string.
   * @exception simdjson_error(INCORRECT_TYPE) if the JSON value is not a string.
   */
  simdjson_really_inline operator raw_json_string() noexcept(false);
  /**
   * Cast this JSON value to a bool.
   *
   * @returns A bool value.
   * @exception simdjson_error(INCORRECT_TYPE) if the JSON value is not true or false.
   */
  simdjson_really_inline operator bool() noexcept(false);
#endif
  /**
   * This method scans the array and counts the number of elements.
   * The count_elements method should always be called before you have begun
   * iterating through the array: it is expected that you are pointing at
   * the beginning of the array.
   * The runtime complexity is linear in the size of the array. After
   * calling this function, if successful, the array is 'rewinded' at its
   * beginning as if it had never been accessed. If the JSON is malformed (e.g.,
   * there is a missing comma), then an error is returned and it is no longer
   * safe to continue.
   */
  simdjson_really_inline simdjson_result<size_t> count_elements() & noexcept;
  /**
   * Begin array iteration.
   *
   * Part of the std::iterable interface.
   */
  simdjson_really_inline simdjson_result<array_iterator> begin() & noexcept;
  /**
   * Sentinel representing the end of the array.
   *
   * Part of the std::iterable interface.
   */
  simdjson_really_inline simdjson_result<array_iterator> end() & noexcept;

  /**
   * Look up a field by name on an object (order-sensitive).
   *
   * The following code reads z, then y, then x, and thus will not retrieve x or y if fed the
   * JSON `{ "x": 1, "y": 2, "z": 3 }`:
   *
   * ```c++
   * simdjson::ondemand::parser parser;
   * auto obj = parser.parse(R"( { "x": 1, "y": 2, "z": 3 } )");
   * double z = obj.find_field("z");
   * double y = obj.find_field("y");
   * double x = obj.find_field("x");
   * ```
   *
   * **Raw Keys:** The lookup will be done against the *raw* key, and will not unescape keys.
   * e.g. `object["a"]` will match `{ "a": 1 }`, but will *not* match `{ "\u0061": 1 }`.
   *
   * @param key The key to look up.
   * @returns The value of the field, or NO_SUCH_FIELD if the field is not in the object.
   */
  simdjson_really_inline simdjson_result<value> find_field(std::string_view key) & noexcept;
  /** @overload simdjson_really_inline simdjson_result<value> find_field(std::string_view key) & noexcept; */
  simdjson_really_inline simdjson_result<value> find_field(const char *key) & noexcept;

  /**
   * Look up a field by name on an object, without regard to key order.
   *
   * **Performance Notes:** This is a bit less performant than find_field(), though its effect varies
   * and often appears negligible. It starts out normally, starting out at the last field; but if
   * the field is not found, it scans from the beginning of the object to see if it missed it. That
   * missing case has a non-cache-friendly bump and lots of extra scanning, especially if the object
   * in question is large. The fact that the extra code is there also bumps the executable size.
   *
   * It is the default, however, because it would be highly surprising (and hard to debug) if the
   * default behavior failed to look up a field just because it was in the wrong order--and many
   * APIs assume this. Therefore, you must be explicit if you want to treat objects as out of order.
   *
   * Use find_field() if you are sure fields will be in order (or are willing to treat it as if the
   * field wasn't there when they aren't).
   *
   * @param key The key to look up.
   * @returns The value of the field, or NO_SUCH_FIELD if the field is not in the object.
   */
  simdjson_really_inline simdjson_result<value> find_field_unordered(std::string_view key) & noexcept;
  /** @overload simdjson_really_inline simdjson_result<value> find_field_unordered(std::string_view key) & noexcept; */
  simdjson_really_inline simdjson_result<value> find_field_unordered(const char *key) & noexcept;
  /** @overload simdjson_really_inline simdjson_result<value> find_field_unordered(std::string_view key) & noexcept; */
  simdjson_really_inline simdjson_result<value> operator[](std::string_view key) & noexcept;
  /** @overload simdjson_really_inline simdjson_result<value> find_field_unordered(std::string_view key) & noexcept; */
  simdjson_really_inline simdjson_result<value> operator[](const char *key) & noexcept;

  /**
   * Get the type of this JSON value.
   *
   * NOTE: If you're only expecting a value to be one type (a typical case), it's generally
   * better to just call .get_double, .get_string, etc. and check for INCORRECT_TYPE (or just
   * let it throw an exception).
   *
   * @error TAPE_ERROR when the JSON value is a bad token like "}" "," or "alse".
   */
  simdjson_really_inline simdjson_result<json_type> type() noexcept;

  /**
   * Get the raw JSON for this token.
   *
   * The string_view will always point into the input buffer.
   *
   * The string_view will start at the beginning of the token, and include the entire token
   * *as well as all spaces until the next token (or EOF).* This means, for example, that a
   * string token always begins with a " and is always terminated by the final ", possibly
   * followed by a number of spaces.
   *
   * The string_view is *not* null-terminated. If this is a scalar (string, number,
   * boolean, or null), the character after the end of the string_view may be past the end of
   * the buffer.
   *
   * Tokens include:
   * - {
   * - [
   * - "a string (possibly with UTF-8 or backslashed characters like \\\")".
   * - -1.2e-100
   * - true
   * - false
   * - null
   */
  simdjson_really_inline simdjson_result<std::string_view> raw_json_token() noexcept;

  /**
   * Reset the iterator inside the document instance so we are pointing back at the
   * beginning of the document, as if it had just been created.
   */
  inline void rewind() noexcept;
  /**
   * Returns debugging information.
   */
  inline std::string to_debug_string() noexcept;

  /**
   * Get the value associated with the given JSON pointer.  We use the RFC 6901
   * https://tools.ietf.org/html/rfc6901 standard.
   *
   *   ondemand::parser parser;
   *   auto json = R"({ "foo": { "a": [ 10, 20, 30 ] }})"_padded;
   *   auto doc = parser.iterate(json);
   *   doc.at_pointer("/foo/a/1") == 20
   *
   * It is allowed for a key to be the empty string:
   *
   *   ondemand::parser parser;
   *   auto json = R"({ "": { "a": [ 10, 20, 30 ] }})"_padded;
   *   auto doc = parser.iterate(json);
   *   doc.at_pointer("//a/1") == 20
   *
   * Note that at_pointer() does not automatically rewind between each call (call rewind() to reset).
   * Also note that at_pointer() relies on find_field() which implies that we do not unescape keys when matching
   *
   * @return The value associated with the given JSON pointer, or:
   *         - NO_SUCH_FIELD if a field does not exist in an object
   *         - INDEX_OUT_OF_BOUNDS if an array index is larger than an array length
   *         - INCORRECT_TYPE if a non-integer is used to access an array
   *         - INVALID_JSON_POINTER if the JSON pointer is invalid and cannot be parsed
   */
  simdjson_really_inline simdjson_result<value> at_pointer(std::string_view json_pointer) noexcept;

protected:
  simdjson_really_inline document(ondemand::json_iterator &&iter) noexcept;
  simdjson_really_inline const uint8_t *text(uint32_t idx) const noexcept;

  simdjson_really_inline value_iterator resume_value_iterator() noexcept;
  simdjson_really_inline value_iterator get_root_value_iterator() noexcept;
  simdjson_really_inline value resume_value() noexcept;
  static simdjson_really_inline document start(ondemand::json_iterator &&iter) noexcept;

  //
  // Fields
  //
  json_iterator iter{}; ///< Current position in the document
  static constexpr depth_t DOCUMENT_DEPTH = 0; ///< document depth is always 0

  friend class array_iterator;
  friend class value;
  friend class ondemand::parser;
  friend class object;
  friend class array;
  friend class field;
  friend class token;
};

} // namespace ondemand
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson

namespace simdjson {

template<>
struct simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document> : public SIMDJSON_IMPLEMENTATION::implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::document> {
public:
  simdjson_really_inline simdjson_result(SIMDJSON_IMPLEMENTATION::ondemand::document &&value) noexcept; ///< @private
  simdjson_really_inline simdjson_result(error_code error) noexcept; ///< @private
  simdjson_really_inline simdjson_result() noexcept = default;
  simdjson_really_inline error_code rewind() noexcept;

  simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::array> get_array() & noexcept;
  simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::object> get_object() & noexcept;
  simdjson_really_inline simdjson_result<uint64_t> get_uint64() noexcept;
  simdjson_really_inline simdjson_result<int64_t> get_int64() noexcept;
  simdjson_really_inline simdjson_result<double> get_double() noexcept;
  simdjson_really_inline simdjson_result<std::string_view> get_string() noexcept;
  simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::raw_json_string> get_raw_json_string() noexcept;
  simdjson_really_inline simdjson_result<bool> get_bool() noexcept;
  simdjson_really_inline bool is_null() noexcept;

  template<typename T> simdjson_really_inline simdjson_result<T> get() & noexcept;
  template<typename T> simdjson_really_inline simdjson_result<T> get() && noexcept;

  template<typename T> simdjson_really_inline error_code get(T &out) & noexcept;
  template<typename T> simdjson_really_inline error_code get(T &out) && noexcept;

#if SIMDJSON_EXCEPTIONS
  simdjson_really_inline operator SIMDJSON_IMPLEMENTATION::ondemand::array() & noexcept(false);
  simdjson_really_inline operator SIMDJSON_IMPLEMENTATION::ondemand::object() & noexcept(false);
  simdjson_really_inline operator uint64_t() noexcept(false);
  simdjson_really_inline operator int64_t() noexcept(false);
  simdjson_really_inline operator double() noexcept(false);
  simdjson_really_inline operator std::string_view() noexcept(false);
  simdjson_really_inline operator SIMDJSON_IMPLEMENTATION::ondemand::raw_json_string() noexcept(false);
  simdjson_really_inline operator bool() noexcept(false);
#endif
  simdjson_really_inline simdjson_result<size_t> count_elements() & noexcept;
  simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::array_iterator> begin() & noexcept;
  simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::array_iterator> end() & noexcept;
  simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> find_field(std::string_view key) & noexcept;
  simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> find_field(const char *key) & noexcept;
  simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> operator[](std::string_view key) & noexcept;
  simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> operator[](const char *key) & noexcept;
  simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> find_field_unordered(std::string_view key) & noexcept;
  simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> find_field_unordered(const char *key) & noexcept;

  simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::json_type> type() noexcept;

  /** @copydoc simdjson_really_inline std::string_view document::raw_json_token() const noexcept */
  simdjson_really_inline simdjson_result<std::string_view> raw_json_token() noexcept;

  simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::value> at_pointer(std::string_view json_pointer) noexcept;
};

} // namespace simdjson
