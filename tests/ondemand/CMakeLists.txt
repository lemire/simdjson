# All remaining tests link with simdjson proper
link_libraries(simdjson)
include_directories(..)
add_subdirectory(compilation_failure_tests)

add_cpp_test(ondemand_tostring_tests         LABELS ondemand acceptance per_implementation)
add_cpp_test(ondemand_active_tests           LABELS ondemand acceptance per_implementation)
add_cpp_test(ondemand_array_tests            LABELS ondemand acceptance per_implementation)
add_cpp_test(ondemand_array_error_tests      LABELS ondemand acceptance per_implementation)
add_cpp_test(ondemand_compilation_tests      LABELS ondemand acceptance per_implementation)
add_cpp_test(ondemand_error_tests            LABELS ondemand acceptance per_implementation)
add_cpp_test(ondemand_json_pointer_tests     LABELS ondemand acceptance per_implementation)
add_cpp_test(ondemand_key_string_tests       LABELS ondemand acceptance per_implementation)
add_cpp_test(ondemand_misc_tests             LABELS ondemand acceptance per_implementation)
add_cpp_test(ondemand_number_tests           LABELS ondemand acceptance per_implementation)
add_cpp_test(ondemand_object_tests           LABELS ondemand acceptance per_implementation)
add_cpp_test(ondemand_object_error_tests     LABELS ondemand acceptance per_implementation)
add_cpp_test(ondemand_ordering_tests         LABELS ondemand acceptance per_implementation)
add_cpp_test(ondemand_parse_api_tests        LABELS ondemand acceptance per_implementation)
add_cpp_test(ondemand_readme_examples        LABELS ondemand acceptance per_implementation)
add_cpp_test(ondemand_scalar_tests           LABELS ondemand acceptance per_implementation)
add_cpp_test(ondemand_twitter_tests          LABELS ondemand acceptance per_implementation)
add_cpp_test(ondemand_wrong_type_error_tests LABELS ondemand acceptance per_implementation)

if(HAVE_POSIX_FORK AND HAVE_POSIX_WAIT) # assert tests use fork and wait, which aren't on MSVC
  add_cpp_test(ondemand_assert_out_of_order_values LABELS assert per_implementation explicitonly ondemand)
endif()

# Copy the simdjson dll into the tests directory
if(MSVC AND BUILD_SHARED_LIBS)
  add_custom_command(TARGET ondemand_parse_api_tests POST_BUILD        # Adds a post-build event
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake -E copy_if_different..."
        "$<TARGET_FILE:simdjson>"      # <--this is in-file
        "$<TARGET_FILE_DIR:ondemand_parse_api_tests>")                 # <--this is out-file path
endif(MSVC AND BUILD_SHARED_LIBS)
