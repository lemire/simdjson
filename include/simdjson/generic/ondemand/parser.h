#include "simdjson/error.h"

namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace ondemand {

class array;
class object;
class value;
class raw_json_string;

/**
 * A JSON fragment iterator.
 *
 * This holds the actual iterator as well as the buffer for writing strings.
 */
class parser {
public:
  /**
   * Create a JSON parser.
   *
   * The new parser will have zero capacity.
   */
  inline explicit parser(size_t max_capacity = SIMDJSON_MAXSIZE_BYTES) noexcept;

  inline parser(parser &&other) noexcept = default;
  simdjson_really_inline parser(const parser &other) = delete;
  simdjson_really_inline parser &operator=(const parser &other) = delete;

  /** Deallocate the JSON parser. */
  inline ~parser() noexcept = default;

  /**
   * Start iterating an on-demand JSON document.
   *
   *   ondemand::parser parser;
   *   document doc = parser.iterate(json);
   *
   * ### IMPORTANT: Buffer Lifetime
   *
   * Because parsing is done while you iterate, you *must* keep the JSON buffer around at least as
   * long as the document iteration.
   *
   * ### IMPORTANT: Document Lifetime
   *
   * Only one iteration at a time can happen per parser, and the parser *must* be kept alive during
   * iteration to ensure intermediate buffers can be accessed. Any document must be destroyed before
   * you call parse() again or destroy the parser.
   *
   * @param json The JSON to parse.
   * @param len The length of the JSON.
   *
   * @return The document, or an error:
   *         - MEMALLOC if realloc_if_needed the parser does not have enough capacity, and memory
   *           allocation fails.
   *         - EMPTY if the document is all whitespace.
   *         - UTF8_ERROR if the document is not valid UTF-8.
   *         - UNESCAPED_CHARS if a string contains control characters that must be escaped
   *         - UNCLOSED_STRING if there is an unclosed string in the document.
   */
  simdjson_warn_unused simdjson_result<document> iterate(std::string_view json) & noexcept;
  /** @overload simdjson_result<document> iterate(std::string_view json) & noexcept */
  simdjson_warn_unused simdjson_result<document> iterate(const char *json, size_t len) & noexcept;
  /** @overload simdjson_result<document> iterate(std::string_view json) & noexcept */
  simdjson_warn_unused simdjson_result<document> iterate(const uint8_t *json, size_t len) & noexcept;
  /** @overload simdjson_result<document> iterate(std::string_view json) & noexcept */
  simdjson_warn_unused simdjson_result<document> iterate(const std::string &json) & noexcept;
  /** @overload simdjson_result<document> iterate(std::string_view json) & noexcept */
  simdjson_warn_unused simdjson_result<document> iterate(const simdjson_result<padded_string> &json) & noexcept;
  /** @overload simdjson_result<document> iterate(std::string_view json) & noexcept */
  simdjson_warn_unused simdjson_result<document> iterate(const simdjson_result<padded_string_view> &json) & noexcept;
  // Make sure people pass capacity in some capacity
  simdjson_warn_unused simdjson_result<document> iterate(const char *json) & noexcept = delete;
  simdjson_warn_unused simdjson_result<document> iterate(const uint8_t *json) & noexcept = delete;

  /**
   * @private
   *
   * Start iterating an on-demand JSON document.
   *
   *   ondemand::parser parser;
   *   json_iterator doc = parser.iterate(json);
   *
   * ### IMPORTANT: Buffer Lifetime
   *
   * Because parsing is done while you iterate, you *must* keep the JSON buffer around at least as
   * long as the document iteration.
   *
   * ### IMPORTANT: Document Lifetime
   *
   * Only one iteration at a time can happen per parser, and the parser *must* be kept alive during
   * iteration to ensure intermediate buffers can be accessed. Any document must be destroyed before
   * you call parse() again or destroy the parser.
   *
   * @param json The JSON to parse.
   *
   * @return The iterator, or an error:
   *         - MEMALLOC if realloc_if_needed the parser does not have enough capacity, and memory
   *           allocation fails.
   *         - EMPTY if the document is all whitespace.
   *         - UTF8_ERROR if the document is not valid UTF-8.
   *         - UNESCAPED_CHARS if a string contains control characters that must be escaped
   *         - UNCLOSED_STRING if there is an unclosed string in the document.
   */
  simdjson_warn_unused simdjson_result<json_iterator> iterate_raw(std::string_view json) & noexcept;

  /** The capacity of this parser (the largest document it can process). */
  simdjson_really_inline size_t capacity() const noexcept;
  /** The maximum capacity of this parser (the largest document it is allowed to process). */
  simdjson_really_inline size_t max_capacity() const noexcept;
  simdjson_really_inline void set_max_capacity(size_t max_capacity) noexcept;
  /** The maximum depth of this parser (the most deeply nested objects and arrays it can process). */
  simdjson_really_inline size_t max_depth() const noexcept;

  /**
   * Ensure this parser has enough memory to process JSON documents up to `capacity` bytes in length
   * and `max_depth` depth.
   *
   * @param capacity The new capacity.
   * @param max_depth The new max_depth. Defaults to DEFAULT_MAX_DEPTH.
   * @return The error, if there is one.
   */
  simdjson_warn_unused error_code allocate(size_t capacity, size_t max_depth=DEFAULT_MAX_DEPTH) noexcept;

private:
  /** @private [for benchmarking access] The implementation to use */
  std::unique_ptr<internal::dom_parser_implementation> implementation{};
  size_t _capacity{0};
  size_t _max_capacity;
  size_t _max_depth{DEFAULT_MAX_DEPTH};
  std::unique_ptr<uint8_t[]> string_buf{};
#ifdef SIMDJSON_DEVELOPMENT_CHECKS
  std::unique_ptr<token_position[]> start_positions{};
#endif

  friend class json_iterator;
};

} // namespace ondemand
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson

namespace simdjson {

template<>
struct simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::parser> : public SIMDJSON_IMPLEMENTATION::implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::parser> {
public:
  simdjson_really_inline simdjson_result(SIMDJSON_IMPLEMENTATION::ondemand::parser &&value) noexcept; ///< @private
  simdjson_really_inline simdjson_result(error_code error) noexcept; ///< @private
  simdjson_really_inline simdjson_result() noexcept = default;
};

} // namespace simdjson
