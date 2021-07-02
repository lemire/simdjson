#include <algorithm>
#include <limits>
#include <stdexcept>
namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace ondemand {

simdjson_really_inline document_stream::document_stream(
  ondemand::parser &_parser,
  const uint8_t *_buf,
  size_t _len,
  size_t _batch_size
) noexcept
  : parser{&_parser},
    buf{_buf},
    len{_len},
    batch_size{_batch_size <= simdjson::MINIMAL_BATCH_SIZE ? simdjson::MINIMAL_BATCH_SIZE : _batch_size},
    error{SUCCESS}
{}

simdjson_really_inline document_stream::document_stream() noexcept
  : parser{nullptr},
    buf{nullptr},
    len{0},
    batch_size{0},
    error{UNINITIALIZED}
{
}

simdjson_really_inline document_stream::~document_stream() noexcept
{
}

inline size_t document_stream::size_in_bytes() const noexcept {
  return len;
}

simdjson_really_inline document_stream::iterator::iterator() noexcept
  : stream{nullptr}, finished{true} {
}

simdjson_really_inline document_stream::iterator::iterator(document_stream* _stream, bool is_end) noexcept
  : stream{_stream}, finished{is_end} {
}

simdjson_really_inline ondemand::document& document_stream::iterator::operator*() noexcept {
  return stream->doc;
}

simdjson_really_inline document_stream::iterator& document_stream::iterator::operator++() noexcept {
  // If there is an error, then we want the iterator
  // to be finished, no matter what. (E.g., we do not
  // keep generating documents with errors, or go beyond
  // a document with errors.)
  //
  // Users do not have to call "operator*()" when they use operator++,
  // so we need to end the stream in the operator++ function.
  //
  // Note that setting finished = true is essential otherwise
  // we would enter an infinite loop.
  if (stream->error) { finished = true; }
  // Note that stream->error() is guarded against error conditions
  // (it will immediately return if stream->error casts to false).
  // In effect, this next function does nothing when (stream->error)
  // is true (hence the risk of an infinite loop).
  stream->next();
  // If that was the last document, we're finished.
  // It is the only type of error we do not want to appear
  // in operator*.
  if (stream->error == EMPTY) { finished = true; }
  // If we had any other kind of error (not EMPTY) then we want
  // to pass it along to the operator* and we cannot mark the result
  // as "finished" just yet.
  return *this;
}

simdjson_really_inline bool document_stream::iterator::operator!=(const document_stream::iterator &other) const noexcept {
  return finished != other.finished;
}

simdjson_really_inline document_stream::iterator document_stream::begin() noexcept {
  start();
  // If there are no documents, we're finished.
  return iterator(this, error == EMPTY);
}

simdjson_really_inline document_stream::iterator document_stream::end() noexcept {
  return iterator(this, true);
}

inline void document_stream::start() noexcept {
  if (error) { return; }
  error = parser->allocate(batch_size);
  if (error) { return; }
  // Always run the first stage 1 parse immediately
  batch_start = 0;
  error = run_stage1(*parser, batch_start);
  if(error == EMPTY) {
    // In exceptional cases, we may start with an empty block
    batch_start = next_batch_start();
    if (batch_start >= len) { return; }
    error = run_stage1(*parser, batch_start);
  }
  if (error) { return; }
  doc = document(json_iterator(buf, parser));
}

inline void document_stream::next() noexcept {
  // We always enter at once once in an error condition.
  if (error) { return; }
  next_document();
  if (error) { return; }
  doc_index = batch_start + parser->implementation->structural_indexes[doc.iter._root - parser->implementation->structural_indexes.get()];

  // Check if at end of structural indexes (i.e. at end of batch)
  if(doc.iter._root - parser->implementation->structural_indexes.get() >= parser->implementation->n_structural_indexes) {
    error = EMPTY;
    //std::cout << "LOADING NEW BATCH" << std::endl;
    // Load another batch (if available)
    while (error == EMPTY) {
      batch_start = next_batch_start();
      if (batch_start >= len) { break; }

      std::cout << "FINE: " << doc.iter.peek() <<std::endl;
      error = run_stage1(*parser, batch_start);
      std::cout << "BROKEN: " << doc.iter.peek() <<std::endl;

      if (error) { continue; } // If the error was EMPTY, we may want to load another batch.
      doc_index = batch_start;
    }
  }
}

inline void document_stream::next_document() noexcept {
  // Go to next place where depth=0 (document depth)
  error = doc.iter.skip_child(0);
  if (error) { return; }
  // Always set depth=1 at the start of document
  doc.iter._depth = 1;
  // Resets the string buffer at the beginning, thus invalidating the strings.
  doc.iter._string_buf_loc = parser->string_buf.get();
  doc.iter._root = doc.iter.position();
}

inline size_t document_stream::next_batch_start() const noexcept {
  return batch_start + parser->implementation->structural_indexes[parser->implementation->n_structural_indexes];
}

inline error_code document_stream::run_stage1(ondemand::parser &p, size_t _batch_start) noexcept {
  size_t remaining = len - _batch_start;
  if (remaining <= batch_size) {
    return p.implementation->stage1(&buf[_batch_start], remaining, stage1_mode::streaming_final);
  } else {
    return p.implementation->stage1(&buf[_batch_start], batch_size, stage1_mode::streaming_partial);
  }
}

simdjson_really_inline size_t document_stream::iterator::current_index() const noexcept {
  return stream->doc_index;
}

} // namespace ondemand
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson

namespace simdjson {

simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document_stream>::simdjson_result(
  error_code error
) noexcept :
    implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::document_stream>(error)
{
}
simdjson_really_inline simdjson_result<SIMDJSON_IMPLEMENTATION::ondemand::document_stream>::simdjson_result(
  SIMDJSON_IMPLEMENTATION::ondemand::document_stream &&value
) noexcept :
    implementation_simdjson_result_base<SIMDJSON_IMPLEMENTATION::ondemand::document_stream>(
      std::forward<SIMDJSON_IMPLEMENTATION::ondemand::document_stream>(value)
    )
{
}

}