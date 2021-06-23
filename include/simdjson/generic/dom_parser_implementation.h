#include "simdjson/base.h"
#include "simdjson/internal/isadetection.h"

namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {

class dom_parser_implementation final : public internal::dom_parser_implementation {
public:
  inline dom_parser_implementation() noexcept;
  inline dom_parser_implementation(dom_parser_implementation &&other) noexcept;
  inline dom_parser_implementation &operator=(dom_parser_implementation &&other) noexcept;
  dom_parser_implementation(const dom_parser_implementation &) = delete;
  dom_parser_implementation &operator=(const dom_parser_implementation &) = delete;

  simdjson_warn_unused error_code parse(const uint8_t *buf, size_t len, dom::document &doc) noexcept final;
  simdjson_warn_unused error_code stage1(const uint8_t *buf, size_t len, stage1_mode partial) noexcept final;
  simdjson_warn_unused error_code stage2(dom::document &doc) noexcept final;
  simdjson_warn_unused error_code stage2_next(dom::document &doc) noexcept final;
  inline simdjson_warn_unused error_code set_capacity(size_t capacity) noexcept final;
  inline simdjson_warn_unused error_code set_max_depth(size_t max_depth) noexcept final;
private:
  simdjson_really_inline simdjson_warn_unused error_code set_capacity_stage1(size_t capacity);

};

} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson

namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {

inline dom_parser_implementation::dom_parser_implementation() noexcept = default;
inline dom_parser_implementation::dom_parser_implementation(dom_parser_implementation &&other) noexcept = default;
inline dom_parser_implementation &dom_parser_implementation::operator=(dom_parser_implementation &&other) noexcept = default;

// Leaving these here so they can be inlined if so desired
inline simdjson_warn_unused error_code dom_parser_implementation::set_capacity(size_t capacity) noexcept {
  if(capacity > SIMDJSON_MAXSIZE_BYTES) { return CAPACITY; }
  // Stage 1 index output
  size_t max_structures = SIMDJSON_ROUNDUP_N(capacity, 64) + 2 + 7;
  structural_indexes.reset( new (std::nothrow) uint32_t[max_structures] );
  if (!structural_indexes) { _capacity = 0; return MEMALLOC; }
  structural_indexes[0] = 0;
  n_structural_indexes = 0;

  _capacity = capacity;
  return SUCCESS;
}

inline simdjson_warn_unused error_code dom_parser_implementation::set_max_depth(size_t max_depth) noexcept {
  // Stage 2 stacks
  open_containers.reset(new (std::nothrow) internal::open_container[max_depth]);
  is_array.reset(new (std::nothrow) bool[max_depth]);
  if (!is_array || !open_containers) { _max_depth = 0; return MEMALLOC; }

  _max_depth = max_depth;
  return SUCCESS;
}

} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson
