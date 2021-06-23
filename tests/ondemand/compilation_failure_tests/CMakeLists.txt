#
# This directory contains files aimed to verify that constructs that
# are supposed to fail at compile time, indeed do so.
# To prevent bit rot, the same source file is compiled twice with
# the macro COMPILATION_TEST_USE_FAILING_CODE set to 0 or 1.
#

# adds a compilation test. Two targets are created, one expected to
# succeed compilation and one that is expected to fail.
function(add_dual_compile_test TEST_NAME)
  add_cpp_test(${TEST_NAME}_should_compile SOURCES ${TEST_NAME}.cpp COMPILE_ONLY LABELS ondemand no_mingw)
  add_cpp_test(${TEST_NAME}_should_not_compile SOURCES ${TEST_NAME}.cpp COMPILE_ONLY WILL_FAIL LABELS ondemand acceptance no_mingw)
  target_compile_definitions(${TEST_NAME}_should_not_compile PRIVATE COMPILATION_TEST_USE_FAILING_CODE=1)
endfunction(add_dual_compile_test)


add_dual_compile_test(iterate_char_star)
add_dual_compile_test(iterate_string_view)
add_dual_compile_test(iterate_temporary_buffer)
add_dual_compile_test(padded_string_view_char_star_no_capacity)
