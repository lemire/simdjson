#
# Flags used by exes and by the simdjson library (project-wide flags)
#
add_library(simdjson-internal-flags INTERFACE)

option(SIMDJSON_NO_FORCE_INLINING "Do not attempt to force function inlining" OFF)
if(SIMDJSON_NO_FORCE_INLINING)
  add_compile_definitions(SIMDJSON_NO_FORCE_INLINING=1)
endif()

option(SIMDJSON_SANITIZE_UNDEFINED "Sanitize undefined behavior" OFF)
if(SIMDJSON_SANITIZE_UNDEFINED)
  add_compile_options(-fsanitize=undefined -fno-sanitize-recover=all)
  add_link_options(-fsanitize=undefined -fno-sanitize-recover=all)
endif()

option(SIMDJSON_SANITIZE "Sanitize addresses" OFF)
if(SIMDJSON_SANITIZE)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    message(STATUS "The address sanitizer under Apple's clang appears to be \
incompatible with the undefined-behavior sanitizer.")
    message(STATUS "You may set SIMDJSON_SANITIZE_UNDEFINED to sanitize \
undefined behavior.")
    add_compile_options(
        -fsanitize=address -fno-omit-frame-pointer -fno-sanitize-recover=all
    )
    add_compile_definitions(ASAN_OPTIONS=detect_leaks=1)
    link_libraries(
        -fsanitize=address -fno-omit-frame-pointer -fno-sanitize-recover=all
    )
  else()
    message(
        STATUS
        "Setting both the address sanitizer and the undefined sanitizer."
    )
    add_compile_options(
        -fsanitize=address -fno-omit-frame-pointer
        -fsanitize=undefined -fno-sanitize-recover=all
    )
    link_libraries(
        -fsanitize=address -fno-omit-frame-pointer
        -fsanitize=undefined -fno-sanitize-recover=all
    )
  endif()

  # Ubuntu bug for GCC 5.0+ (safe for all versions)
  if(CMAKE_COMPILER_IS_GNUCC)
    link_libraries(-fuse-ld=gold)
  endif()
endif()

if(SIMDJSON_SANITIZE_THREADS)
  message(STATUS "Setting both the thread sanitizer \
and the undefined-behavior sanitizer.")
  add_compile_options(
      -fsanitize=thread -fsanitize=undefined -fno-sanitize-recover=all
  )
  link_libraries(
      -fsanitize=thread -fsanitize=undefined -fno-sanitize-recover=all
  )

  # Ubuntu bug for GCC 5.0+ (safe for all versions)
  if(CMAKE_COMPILER_IS_GNUCC)
    link_libraries(-fuse-ld=gold)
  endif()
endif()

get_cmake_property(is_multi_config GENERATOR_IS_MULTI_CONFIG)
if(NOT is_multi_config AND NOT CMAKE_BUILD_TYPE)
  # Deliberately not including SIMDJSON_SANITIZE_THREADS since thread behavior
  # depends on the build type.
  if(SIMDJSON_SANITIZE OR SIMDJSON_SANITIZE_UNDEFINED)
    message(STATUS "No build type selected and you have enabled the sanitizer, \
default to Debug. Consider setting CMAKE_BUILD_TYPE.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
  else()
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  endif()
endif()

if(NOT MSVC)
  option(SIMDJSON_USE_LIBCPP "Use the libc++ library" OFF)
endif()

if(MSVC AND BUILD_SHARED_LIBS)
  # This will require special handling.
  set(SIMDJSON_WINDOWS_DLL TRUE)
endif()

# We compile tools, tests, etc. with C++ 17. Override yourself if you need on a
# target.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MACOSX_RPATH OFF)
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# LTO seems to create all sorts of fun problems. Let us
# disable temporarily.
#include(CheckIPOSupported)
#check_ipo_supported(RESULT ltoresult)
#if(ltoresult)
#  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
#endif()

option(SIMDJSON_VISUAL_STUDIO_BUILD_WITH_DEBUG_INFO_FOR_PROFILING "\
Under Visual Studio, add Zi to the compile flag and DEBUG to the link file to \
add debugging information to the release build for easier profiling inside \
tools like VTune" OFF)
if(MSVC)
  if(MSVC_TOOLSET_VERSION STRLESS "142")
    set(SIMDJSON_LEGACY_VISUAL_STUDIO TRUE)
    message (STATUS "A legacy Visual Studio version was detected. \
We recommend Visual Studio 2019 or better on a 64-bit system.")
  endif()
  if(MSVC_TOOLSET_VERSION STREQUAL "140")
    # Visual Studio 2015 issues warnings and we tolerate it
    # cmake -G "Visual Studio 14" ..
    target_compile_options(simdjson-internal-flags INTERFACE /W0 /sdl)
  else()
    # Recent version of Visual Studio expected (2017, 2019...). Prior versions
    # are unsupported.
    # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4714?view=vs-2019
    target_compile_options(simdjson-internal-flags INTERFACE /WX /W3 /sdl /w34714)
    if(MSVC_VERSION GREATER 1910)
      target_compile_options(simdjson-internal-flags INTERFACE /permissive-)
    endif()
  endif()
  if(SIMDJSON_VISUAL_STUDIO_BUILD_WITH_DEBUG_INFO_FOR_PROFILING)
    add_link_options(/DEBUG)
    add_compile_options(/Zi)
  endif()
else()
  if(NOT WIN32)
    target_compile_options(simdjson-internal-flags INTERFACE -fPIC)
  endif()
  target_compile_options(
      simdjson-internal-flags INTERFACE
      -Werror -Wall -Wextra -Weffc++ -Wsign-compare -Wshadow -Wwrite-strings
      -Wpointer-arith -Winit-self -Wconversion -Wno-sign-conversion
  )
endif()

#
# Other optional flags
#
option(SIMDJSON_BASH "Allow usage of bash within CMake" ON)

option(
    SIMDJSON_VERBOSE_LOGGING
    "Enable verbose logging for internal simdjson library development."
    OFF
)
if(SIMDJSON_VERBOSE_LOGGING)
  add_compile_definitions(SIMDJSON_VERBOSE_LOGGING=1
  )
endif()

if(SIMDJSON_USE_LIBCPP)
  link_libraries(-stdlib=libc++ -lc++abi)
  # instead of the above line, we could have used
  # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++
  # -lc++abi")
  # The next line is needed empirically.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  # we update CMAKE_SHARED_LINKER_FLAGS, this gets updated later as well
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lc++abi")
endif()

# prevent shared libraries from depending on Intel provided libraries
if(CMAKE_C_COMPILER_ID MATCHES "Intel")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-intel")
endif()

include(CheckSymbolExists)
check_symbol_exists(fork unistd.h HAVE_POSIX_FORK)
check_symbol_exists(wait sys/wait.h HAVE_POSIX_WAIT)
