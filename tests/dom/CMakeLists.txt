
link_libraries(simdjson)
include_directories(..)

if(NOT SIMDJSON_LEGACY_VISUAL_STUDIO AND NOT SIMDJSON_WINDOWS_DLL)
  add_cpp_test(random_string_number_tests LABELS dom acceptance per_implementation)
endif()
add_cpp_test(basictests                 LABELS dom acceptance per_implementation)
add_cpp_test(document_stream_tests      LABELS dom acceptance per_implementation)
add_cpp_test(document_tests             LABELS dom acceptance per_implementation)
add_cpp_test(errortests                 LABELS dom acceptance per_implementation)
add_cpp_test(extracting_values_example  LABELS dom acceptance per_implementation)
add_cpp_test(integer_tests              LABELS dom acceptance per_implementation)
add_cpp_test(jsoncheck                  LABELS dom acceptance per_implementation)
add_cpp_test(minefieldcheck             LABELS dom acceptance per_implementation)
add_cpp_test(numberparsingcheck         LABELS dom acceptance per_implementation) # https://tools.ietf.org/html/rfc6901
add_cpp_test(parse_many_test            LABELS dom acceptance per_implementation)
add_cpp_test(pointercheck               LABELS dom acceptance per_implementation) # https://tools.ietf.org/html/rfc6901
add_cpp_test(stringparsingcheck         LABELS dom acceptance per_implementation) # https://tools.ietf.org/html/rfc6901
add_cpp_test(trivially_copyable_test    LABELS dom acceptance per_implementation)



find_program(BASH bash)

# Below we skip anything on Windows, not just visual studio, because running bash under Windows requires you to
# map app paths to their "mounted" equivalent (e.g., /mnt/c/...). So even if you have bash under Windows, extra work would be
# required to make things work robustly. Simply put: bash is not quite portable.

# Script tests
if (BASH AND (NOT WIN32) AND SIMDJSON_BASH AND (TARGET json2json)) # The scripts are not robust enough to run under Windows even if bash is available
  #
  # json2json test
  #
  add_test(
    NAME testjson2json
    COMMAND ${BASH} ${CMAKE_CURRENT_SOURCE_DIR}/testjson2json.sh ${PROJECT_BINARY_DIR}
    WORKING_DIRECTORY $<TARGET_FILE_DIR:minify>
  )
  set_property(TEST testjson2json APPEND PROPERTY DEPENDS minify json2json)
  set_property(TEST testjson2json APPEND PROPERTY LABELS per_implementation)
  add_dependencies(per_implementation_tests minify)

  #
  # Competition parse test
  #
  if ((SIMDJSON_COMPETITION) AND (!SIMDJSON_SANITIZE))
    # It looks like RapidJSON does not pass the sanitizer under some conditions (Clang 10)
    add_executable(allparserscheckfile allparserscheckfile.cpp)
    add_dependencies(competition_tests allparserscheckfile)
    add_dependencies(per_implementation_tests allparserscheckfile)
    target_link_libraries(allparserscheckfile PRIVATE competition-all)

    add_test(issue150 ${BASH} ${CMAKE_CURRENT_SOURCE_DIR}/issue150.sh ${PROJECT_BINARY_DIR})
    set_property(TEST issue150 APPEND PROPERTY LABELS per_implementation competition)
    add_dependencies(all_tests allparserscheckfile)
  endif()

  #
  # json2json tool test: check that json2json can parse twitter.json
  #


  add_test(NAME json2json COMMAND $<TARGET_FILE:json2json> ${EXAMPLE_JSON})
  set_property(TEST json2json APPEND PROPERTY LABELS acceptance per_implementation)
  add_dependencies(acceptance_tests json2json)
  add_dependencies(per_implementation_tests json2json)

  #
  # SIMDJSON_FORCE_IMPLEMENTATION tests: run json2json with SIMDJSON
  #
  if (SIMDJSON_IMPLEMENTATION MATCHES "fallback")
    add_test(
      NAME simdjson_force_implementation
      COMMAND
        ${CMAKE_COMMAND} -E env
        SIMDJSON_FORCE_IMPLEMENTATION=fallback
        $<TARGET_FILE:checkimplementation>
    )
  endif()
  add_test(
    NAME simdjson_force_implementation_error
    COMMAND
      ${CMAKE_COMMAND} -E env
      SIMDJSON_FORCE_IMPLEMENTATION=doesnotexist
      $<TARGET_FILE:json2json> ${EXAMPLE_JSON}
  )
  set_tests_properties(simdjson_force_implementation_error PROPERTIES WILL_FAIL TRUE)
endif()

#
# Compile-only tests with simdjson flags on
#

# Don't add the tests if we're on VS2017 or older; they don't succeed.
if(NOT (MSVC AND MSVC_VERSION LESS 1920))
  if(SIMDJSON_EXCEPTIONS)
    add_cpp_test(readme_examples COMPILE_ONLY LABELS acceptance)
    add_cpp_test(readme_examples11 COMPILE_ONLY LABELS acceptance SOURCES readme_examples.cpp)
    set_target_properties(readme_examples11 PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
  endif()

  add_cpp_test(readme_examples_noexceptions COMPILE_ONLY LABELS acceptance)

  add_cpp_test(readme_examples_noexceptions11 COMPILE_ONLY LABELS acceptance SOURCES readme_examples_noexceptions.cpp)
  set_target_properties(readme_examples_noexceptions11 PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

  # Compile tests that *should fail*
  add_cpp_test(readme_examples_will_fail_with_exceptions_off WILL_FAIL COMPILE_ONLY LABELS acceptance SOURCES readme_examples.cpp)
  target_compile_definitions(readme_examples_will_fail_with_exceptions_off PRIVATE SIMDJSON_EXCEPTIONS=0)
endif()


if(MSVC AND BUILD_SHARED_LIBS)
  add_custom_command(TARGET basictests POST_BUILD        # Adds a post-build event
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake -E copy_if_different..."
        "$<TARGET_FILE:simdjson>"      # <--this is in-file
        "$<TARGET_FILE_DIR:basictests>")                 # <--this is out-file path
endif()

if(NOT MSVC)
  ######
  # This tests is to guard us against ever again removing the soversion
  # number from the library. See Bug
  # https://github.com/simdjson/simdjson/issues/1014
  #####
  get_target_property(REPORTED_SIMDJSON_SOVERSION simdjson SOVERSION)
  if(NOT ${REPORTED_SIMDJSON_SOVERSION} STREQUAL ${SIMDJSON_LIB_SOVERSION})
    message(FATAL_ERROR "The library target does not have the proper soversion information." )
  endif()
  get_target_property(REPORTED_SIMDJSON_VERSION simdjson VERSION)
  if(NOT ${REPORTED_SIMDJSON_VERSION} STREQUAL ${SIMDJSON_LIB_VERSION})
    message(FATAL_ERROR "The library target does not have the proper version information." )
  endif()
endif()