#ifndef SIMDJSON_INTERNAL_DOM_PARSER_IMPLEMENTATION_H
#define SIMDJSON_INTERNAL_DOM_PARSER_IMPLEMENTATION_H

#include "simdjson/common_defs.h"
#include "simdjson/error.h"
#include <memory>

namespace simdjson {

namespace dom {
class document;
} // namespace dom

/**
* This enum is used with the dom_parser_implementation::stage1 function.
* 1) The regular mode expects a fully formed JSON document.
* 2) The streaming_partial mode expects a possibly truncated
* input within a stream on JSON documents.
* 3) The stream_final mode allows us to truncate final
* unterminated strings. It is useful in conjunction with streaming_partial.
*/
enum class stage1_mode { regular, streaming_partial, streaming_final};

/**
 * Returns true if mode == streaming_partial or mode == streaming_final
 */
inline bool is_streaming(stage1_mode mode) {
  // performance note: it is probably faster to check that mode is different
  // from regular than checking that it is either streaming_partial or streaming_final.
  return (mode != stage1_mode::regular);
  // return (mode == stage1_mode::streaming_partial || mode == stage1_mode::streaming_final);
}


namespace internal {

// expectation: sizeof(open_container) = 64/8.
struct open_container {
  uint32_t tape_index; // where, on the tape, does the scope ([,{) begins
  uint32_t count; // how many elements in the scope
}; // struct open_container

static_assert(sizeof(open_container) == 64/8, "Open container must be 64 bits");

/**
 * An implementation of simdjson's DOM parser for a particular CPU architecture.
 *
 * This class is expected to be accessed only by pointer, and never move in memory (though the
 * pointer can move).
 */
class dom_parser_implementation {
public:

  /**
   * @private For internal implementation use
   *
   * Run a full JSON parse on a single document (stage1 + stage2).
   *
   * Guaranteed only to be called when capacity > document length.
   *
   * Overridden by each implementation.
   *
   * @param buf The json document to parse. *MUST* be allocated up to len + SIMDJSON_PADDING bytes.
   * @param len The length of the json document.
   * @return The error code, or SUCCESS if there was no error.
   */
  simdjson_warn_unused virtual error_code parse(const uint8_t *buf, size_t len, dom::document &doc) noexcept = 0;

  /**
   * @private For internal implementation use
   *
   * Stage 1 of the document parser.
   *
   * Guaranteed only to be called when capacity > document length.
   *
   * Overridden by each implementation.
   *
   * @param buf The json document to parse.
   * @param len The length of the json document.
   * @param streaming Whether this is being called by parser::parse_many.
   * @return The error code, or SUCCESS if there was no error.
   */
  simdjson_warn_unused virtual error_code stage1(const uint8_t *buf, size_t len, stage1_mode streaming) noexcept = 0;

  /**
   * @private For internal implementation use
   *
   * Stage 2 of the document parser.
   *
   * Called after stage1().
   *
   * Overridden by each implementation.
   *
   * @param doc The document to output to.
   * @return The error code, or SUCCESS if there was no error.
   */
  simdjson_warn_unused virtual error_code stage2(dom::document &doc) noexcept = 0;

  /**
   * @private For internal implementation use
   *
   * Stage 2 of the document parser for parser::parse_many.
   *
   * Guaranteed only to be called after stage1().
   * Overridden by each implementation.
   *
   * @param doc The document to output to.
   * @return The error code, SUCCESS if there was no error, or EMPTY if all documents have been parsed.
   */
  simdjson_warn_unused virtual error_code stage2_next(dom::document &doc) noexcept = 0;

  /**
   * Change the capacity of this parser.
   *
   * The capacity can never exceed SIMDJSON_MAXSIZE_BYTES (e.g., 4 GB)
   * and an CAPACITY error is returned if it is attempted.
   *
   * Generally used for reallocation.
   *
   * @param capacity The new capacity.
   * @param max_depth The new max_depth.
   * @return The error code, or SUCCESS if there was no error.
   */
  virtual error_code set_capacity(size_t capacity) noexcept = 0;

  /**
   * Change the max depth of this parser.
   *
   * Generally used for reallocation.
   *
   * @param capacity The new capacity.
   * @param max_depth The new max_depth.
   * @return The error code, or SUCCESS if there was no error.
   */
  virtual error_code set_max_depth(size_t max_depth) noexcept = 0;

  /**
   * Deallocate this parser.
   */
  virtual ~dom_parser_implementation() = default;

  /** Number of structural indices passed from stage 1 to stage 2 */
  uint32_t n_structural_indexes{0};
  /** Structural indices passed from stage 1 to stage 2 */
  std::unique_ptr<uint32_t[]> structural_indexes{};
  /** Next structural index to parse */
  uint32_t next_structural_index{0};

  /**
   * The largest document this parser can support without reallocating.
   *
   * @return Current capacity, in bytes.
   */
  simdjson_really_inline size_t capacity() const noexcept;

  /**
   * The maximum level of nested object and arrays supported by this parser.
   *
   * @return Maximum depth, in bytes.
   */
  simdjson_really_inline size_t max_depth() const noexcept;

  /**
   * Ensure this parser has enough memory to process JSON documents up to `capacity` bytes in length
   * and `max_depth` depth.
   *
   * @param capacity The new capacity.
   * @param max_depth The new max_depth. Defaults to DEFAULT_MAX_DEPTH.
   * @return The error, if there is one.
   */
  simdjson_warn_unused inline error_code allocate(size_t capacity, size_t max_depth) noexcept;

protected:
  /**
   * The maximum document length this parser supports.
   *
   * Buffers are large enough to handle any document up to this length.
   */
  size_t _capacity{0};

  /**
   * The maximum depth (number of nested objects and arrays) supported by this parser.
   *
   * Defaults to DEFAULT_MAX_DEPTH.
   */
  size_t _max_depth{0};

public:
  /** Tape location of each open { or [ */
  std::unique_ptr<open_container[]> open_containers{};
  /** Whether each open container is a [ or { */
  std::unique_ptr<bool[]> is_array{};
  /** Buffer passed to stage 1 */
  const uint8_t *buf{};
  /** Length passed to stage 1 */
  size_t len{0};
  /** Document passed to stage 2 */
  dom::document *doc{};

protected:
  // Declaring these so that subclasses can use them to implement their constructors.
  simdjson_really_inline dom_parser_implementation() noexcept;
  simdjson_really_inline dom_parser_implementation(dom_parser_implementation &&other) noexcept;
  simdjson_really_inline dom_parser_implementation &operator=(dom_parser_implementation &&other) noexcept;

  simdjson_really_inline dom_parser_implementation(const dom_parser_implementation &) noexcept = delete;
  simdjson_really_inline dom_parser_implementation &operator=(const dom_parser_implementation &other) noexcept = delete;
}; // class dom_parser_implementation

simdjson_really_inline dom_parser_implementation::dom_parser_implementation() noexcept = default;
simdjson_really_inline dom_parser_implementation::dom_parser_implementation(dom_parser_implementation &&other) noexcept = default;
simdjson_really_inline dom_parser_implementation &dom_parser_implementation::operator=(dom_parser_implementation &&other) noexcept = default;

simdjson_really_inline size_t dom_parser_implementation::capacity() const noexcept {
  return _capacity;
}

simdjson_really_inline size_t dom_parser_implementation::max_depth() const noexcept {
  return _max_depth;
}

simdjson_warn_unused
inline error_code dom_parser_implementation::allocate(size_t capacity, size_t max_depth) noexcept {
  if (this->max_depth() != max_depth) {
    error_code err = set_max_depth(max_depth);
    if (err) { return err; }
  }
  if (_capacity != capacity) {
    error_code err = set_capacity(capacity);
    if (err) { return err; }
  }
  return SUCCESS;
}

} // namespace internal
} // namespace simdjson

#endif // SIMDJSON_INTERNAL_DOM_PARSER_IMPLEMENTATION_H
