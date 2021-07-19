#ifndef SIMDJSON_COMMON_DEFS_H
#define SIMDJSON_COMMON_DEFS_H

#include <cassert>
#include "simdjson/portability.h"

namespace simdjson {

namespace internal {
/**
 * @private
 * Our own implementation of the C++17 to_chars function.
 * Defined in src/to_chars
 */
char *to_chars(char *first, const char *last, double value);
/**
 * @private
 * A number parsing routine.
 * Defined in src/from_chars
 */
double from_chars(const char *first) noexcept;
double from_chars(const char *first, const char* end) noexcept;

}

#ifndef SIMDJSON_EXCEPTIONS
#if __cpp_exceptions
#define SIMDJSON_EXCEPTIONS 1
#else
#define SIMDJSON_EXCEPTIONS 0
#endif
#endif

/** The maximum document size supported by simdjson. */
constexpr size_t SIMDJSON_MAXSIZE_BYTES = 0xFFFFFFFF;

/**
 * The amount of padding needed in a buffer to parse JSON.
 *
 * the input buf should be readable up to buf + SIMDJSON_PADDING
 * this is a stopgap; there should be a better description of the
 * main loop and its behavior that abstracts over this
 * See https://github.com/simdjson/simdjson/issues/174
 */
constexpr size_t SIMDJSON_PADDING = 32;

/**
 * By default, simdjson supports this many nested objects and arrays.
 *
 * This is the default for parser::max_depth().
 */
constexpr size_t DEFAULT_MAX_DEPTH = 1024;

} // namespace simdjson

#if defined(__GNUC__)
  // Marks a block with a name so that MCA analysis can see it.
  #define SIMDJSON_BEGIN_DEBUG_BLOCK(name) __asm volatile("# LLVM-MCA-BEGIN " #name);
  #define SIMDJSON_END_DEBUG_BLOCK(name) __asm volatile("# LLVM-MCA-END " #name);
  #define SIMDJSON_DEBUG_BLOCK(name, block) BEGIN_DEBUG_BLOCK(name); block; END_DEBUG_BLOCK(name);
#else
  #define SIMDJSON_BEGIN_DEBUG_BLOCK(name)
  #define SIMDJSON_END_DEBUG_BLOCK(name)
  #define SIMDJSON_DEBUG_BLOCK(name, block)
#endif

// Align to N-byte boundary
#define SIMDJSON_ROUNDUP_N(a, n) (((a) + ((n)-1)) & ~((n)-1))
#define SIMDJSON_ROUNDDOWN_N(a, n) ((a) & ~((n)-1))

#define SIMDJSON_ISALIGNED_N(ptr, n) (((uintptr_t)(ptr) & ((n)-1)) == 0)

#if defined(SIMDJSON_REGULAR_VISUAL_STUDIO)

  #define simdjson_really_inline __forceinline
  #define simdjson_never_inline __declspec(noinline)

  #define simdjson_unused
  #define simdjson_warn_unused

  #ifndef simdjson_likely
  #define simdjson_likely(x) x
  #endif
  #ifndef simdjson_unlikely
  #define simdjson_unlikely(x) x
  #endif

  #define SIMDJSON_PUSH_DISABLE_WARNINGS __pragma(warning( push ))
  #define SIMDJSON_PUSH_DISABLE_ALL_WARNINGS __pragma(warning( push, 0 ))
  #define SIMDJSON_DISABLE_VS_WARNING(WARNING_NUMBER) __pragma(warning( disable : WARNING_NUMBER ))
  // Get rid of Intellisense-only warnings (Code Analysis)
  // Though __has_include is C++17, it is supported in Visual Studio 2017 or better (_MSC_VER>=1910).
  #ifdef __has_include
  #if __has_include(<CppCoreCheck\Warnings.h>)
  #include <CppCoreCheck\Warnings.h>
  #define SIMDJSON_DISABLE_UNDESIRED_WARNINGS SIMDJSON_DISABLE_VS_WARNING(ALL_CPPCORECHECK_WARNINGS)
  #endif
  #endif

  #ifndef SIMDJSON_DISABLE_UNDESIRED_WARNINGS
  #define SIMDJSON_DISABLE_UNDESIRED_WARNINGS
  #endif

  #define SIMDJSON_DISABLE_DEPRECATED_WARNING SIMDJSON_DISABLE_VS_WARNING(4996)
  #define SIMDJSON_DISABLE_STRICT_OVERFLOW_WARNING
  #define SIMDJSON_POP_DISABLE_WARNINGS __pragma(warning( pop ))

#else // SIMDJSON_REGULAR_VISUAL_STUDIO

  #define simdjson_really_inline inline __attribute__((always_inline))
  #define simdjson_never_inline inline __attribute__((noinline))

  #define simdjson_unused __attribute__((unused))
  #define simdjson_warn_unused __attribute__((warn_unused_result))

  #ifndef simdjson_likely
  #define simdjson_likely(x) __builtin_expect(!!(x), 1)
  #endif
  #ifndef simdjson_unlikely
  #define simdjson_unlikely(x) __builtin_expect(!!(x), 0)
  #endif

  #define SIMDJSON_PUSH_DISABLE_WARNINGS _Pragma("GCC diagnostic push")
  // gcc doesn't seem to disable all warnings with all and extra, add warnings here as necessary
  #define SIMDJSON_PUSH_DISABLE_ALL_WARNINGS SIMDJSON_PUSH_DISABLE_WARNINGS \
    SIMDJSON_DISABLE_GCC_WARNING(-Weffc++) \
    SIMDJSON_DISABLE_GCC_WARNING(-Wall) \
    SIMDJSON_DISABLE_GCC_WARNING(-Wconversion) \
    SIMDJSON_DISABLE_GCC_WARNING(-Wextra) \
    SIMDJSON_DISABLE_GCC_WARNING(-Wattributes) \
    SIMDJSON_DISABLE_GCC_WARNING(-Wimplicit-fallthrough) \
    SIMDJSON_DISABLE_GCC_WARNING(-Wnon-virtual-dtor) \
    SIMDJSON_DISABLE_GCC_WARNING(-Wreturn-type) \
    SIMDJSON_DISABLE_GCC_WARNING(-Wshadow) \
    SIMDJSON_DISABLE_GCC_WARNING(-Wunused-parameter) \
    SIMDJSON_DISABLE_GCC_WARNING(-Wunused-variable)
  #define SIMDJSON_PRAGMA(P) _Pragma(#P)
  #define SIMDJSON_DISABLE_GCC_WARNING(WARNING) SIMDJSON_PRAGMA(GCC diagnostic ignored #WARNING)
  #if defined(SIMDJSON_CLANG_VISUAL_STUDIO)
  #define SIMDJSON_DISABLE_UNDESIRED_WARNINGS SIMDJSON_DISABLE_GCC_WARNING(-Wmicrosoft-include)
  #else
  #define SIMDJSON_DISABLE_UNDESIRED_WARNINGS
  #endif
  #define SIMDJSON_DISABLE_DEPRECATED_WARNING SIMDJSON_DISABLE_GCC_WARNING(-Wdeprecated-declarations)
  #define SIMDJSON_DISABLE_STRICT_OVERFLOW_WARNING SIMDJSON_DISABLE_GCC_WARNING(-Wstrict-overflow)
  #define SIMDJSON_POP_DISABLE_WARNINGS _Pragma("GCC diagnostic pop")



#endif // MSC_VER

#if defined(SIMDJSON_VISUAL_STUDIO)
    /**
     * Windows users need to do some extra work when building
     * or using a dynamic library (DLL). When building, we need
     * to set SIMDJSON_DLLIMPORTEXPORT to __declspec(dllexport).
     * When *using* the DLL, the user needs to set
     * SIMDJSON_DLLIMPORTEXPORT __declspec(dllimport).
     *
     * Static libraries not need require such work.
     *
     * It does not matter here whether you are using
     * the regular visual studio or clang under visual
     * studio, you still need to handle these issues.
     *
     * Non-Windows systems do not have this complexity.
     */
    #if SIMDJSON_BUILDING_WINDOWS_DYNAMIC_LIBRARY
    // We set SIMDJSON_BUILDING_WINDOWS_DYNAMIC_LIBRARY when we build a DLL under Windows.
    // It should never happen that both SIMDJSON_BUILDING_WINDOWS_DYNAMIC_LIBRARY and
    // SIMDJSON_USING_WINDOWS_DYNAMIC_LIBRARY are set.
    #define SIMDJSON_DLLIMPORTEXPORT __declspec(dllexport)
    #elif SIMDJSON_USING_WINDOWS_DYNAMIC_LIBRARY
    // Windows user who call a dynamic library should set SIMDJSON_USING_WINDOWS_DYNAMIC_LIBRARY to 1.
    #define SIMDJSON_DLLIMPORTEXPORT __declspec(dllimport)
    #else
    // We assume by default static linkage
    #define SIMDJSON_DLLIMPORTEXPORT
    #endif

/**
 * Workaround for the vcpkg package manager. Only vcpkg should
 * ever touch the next line. The SIMDJSON_USING_LIBRARY macro is otherwise unused.
 */
#if SIMDJSON_USING_LIBRARY
#define SIMDJSON_DLLIMPORTEXPORT __declspec(dllimport)
#endif
/**
 * End of workaround for the vcpkg package manager.
 */
#else
    #define SIMDJSON_DLLIMPORTEXPORT
#endif

// C++17 requires string_view.
#if SIMDJSON_CPLUSPLUS17
#define SIMDJSON_HAS_STRING_VIEW
#include <string_view> // by the standard, this has to be safe.
#endif

// This macro (__cpp_lib_string_view) has to be defined
// for C++17 and better, but if it is otherwise defined,
// we are going to assume that string_view is available
// even if we do not have C++17 support.
#ifdef __cpp_lib_string_view
#define SIMDJSON_HAS_STRING_VIEW
#endif

// Some systems have string_view even if we do not have C++17 support,
// and even if __cpp_lib_string_view is undefined, it is the case
// with Apple clang version 11.
// We must handle it. *This is important.*
#ifndef SIMDJSON_HAS_STRING_VIEW
#if defined __has_include
// do not combine the next #if with the previous one (unsafe)
#if __has_include (<string_view>)
// now it is safe to trigger the include
#include <string_view> // though the file is there, it does not follow that we got the implementation
#if defined(_LIBCPP_STRING_VIEW)
// Ah! So we under libc++ which under its Library Fundamentals Technical Specification, which preceded C++17,
// included string_view.
// This means that we have string_view *even though* we may not have C++17.
#define SIMDJSON_HAS_STRING_VIEW
#endif // _LIBCPP_STRING_VIEW
#endif // __has_include (<string_view>)
#endif // defined __has_include
#endif // def SIMDJSON_HAS_STRING_VIEW
// end of complicated but important routine to try to detect string_view.

//
// Backfill std::string_view using nonstd::string_view on systems where
// we expect that string_view is missing. Important: if we get this wrong,
// we will end up with two string_view definitions and potential trouble.
// That is why we work so hard above to avoid it.
//
#ifndef SIMDJSON_HAS_STRING_VIEW
SIMDJSON_PUSH_DISABLE_ALL_WARNINGS
#include "simdjson/nonstd/string_view.hpp"
SIMDJSON_POP_DISABLE_WARNINGS

namespace std {
  using string_view = nonstd::string_view;
}
#endif // SIMDJSON_HAS_STRING_VIEW
#undef SIMDJSON_HAS_STRING_VIEW // We are not going to need this macro anymore.

/// If EXPR is an error, returns it.
#define SIMDJSON_TRY(EXPR) { auto _err = (EXPR); if (_err) { return _err; } }

#ifndef SIMDJSON_DEVELOPMENT_CHECKS
#ifndef NDEBUG
#define SIMDJSON_DEVELOPMENT_CHECKS
#endif
#endif

// Feature flag for partially-implemented "don't require padding" feature
// TODO remove once feature complete.
#ifndef __SIMDJSON_CHECK_EOF
# define __SIMDJSON_CHECK_EOF 1
#endif

#if SIMDJSON_CPLUSPLUS17
// if we have C++, then fallthrough is a default attribute
# define simdjson_fallthrough [[fallthrough]]
// check if we have __attribute__ support
#elif defined(__has_attribute)
// check if we have the __fallthrough__ attribute
#if __has_attribute(__fallthrough__)
// we are good to go:
# define simdjson_fallthrough                    __attribute__((__fallthrough__))
#endif
#endif
// on some systems, we simply do not have support for fallthrough, so use a default:
#ifndef simdjson_fallthrough
# define simdjson_fallthrough do {} while (0)  /* fallthrough */
#endif

#endif // SIMDJSON_COMMON_DEFS_H
