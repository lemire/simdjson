#include "simdjson/base.h"

namespace simdjson {
namespace internal {

  SIMDJSON_DLLIMPORTEXPORT const error_code_info error_codes[] {
    { SUCCESS, "No error" },
    { CAPACITY, "This parser can't support a document that big" },
    { MEMALLOC, "Error allocating memory, we're most likely out of memory" },
    { TAPE_ERROR, "The JSON document has an improper structure: missing or superfluous commas, braces, missing keys, etc." },
    { DEPTH_ERROR, "The JSON document was too deep (too many nested objects and arrays)" },
    { STRING_ERROR, "Problem while parsing a string" },
    { T_ATOM_ERROR, "Problem while parsing an atom starting with the letter 't'" },
    { F_ATOM_ERROR, "Problem while parsing an atom starting with the letter 'f'" },
    { N_ATOM_ERROR, "Problem while parsing an atom starting with the letter 'n'" },
    { NUMBER_ERROR, "Problem while parsing a number" },
    { UTF8_ERROR, "The input is not valid UTF-8" },
    { UNINITIALIZED, "Uninitialized" },
    { EMPTY, "Empty: no JSON found" },
    { UNESCAPED_CHARS, "Within strings, some characters must be escaped, we found unescaped characters" },
    { UNCLOSED_STRING, "A string is opened, but never closed." },
    { UNSUPPORTED_ARCHITECTURE, "simdjson does not have an implementation supported by this CPU architecture (perhaps it's a non-SIMD CPU?)." },
    { INCORRECT_TYPE, "The JSON element does not have the requested type." },
    { NUMBER_OUT_OF_RANGE, "The JSON number is too large or too small to fit within the requested type." },
    { INDEX_OUT_OF_BOUNDS, "Attempted to access an element of a JSON array that is beyond its length." },
    { NO_SUCH_FIELD, "The JSON field referenced does not exist in this object." },
    { IO_ERROR, "Error reading the file." },
    { INVALID_JSON_POINTER, "Invalid JSON pointer syntax." },
    { INVALID_URI_FRAGMENT, "Invalid URI fragment syntax." },
    { UNEXPECTED_ERROR, "Unexpected error, consider reporting this problem as you may have found a bug in simdjson" },
    { PARSER_IN_USE, "Cannot parse a new document while a document is still in use." },
    { OUT_OF_ORDER_ITERATION, "Objects and arrays can only be iterated when they are first encountered." },
    { INSUFFICIENT_PADDING, "simdjson requires the input JSON string to have at least SIMDJSON_PADDING extra bytes allocated, beyond the string's length." },
    { INCOMPLETE_ARRAY_OR_OBJECT, "JSON document ended early in the middle of an object or array." }
  }; // error_messages[]

} // namespace internal
} // namespace simdjson