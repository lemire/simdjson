set(SIMDJSON_ALL_IMPLEMENTATIONS "fallback;westmere;haswell;arm64")

set(SIMDJSON_IMPLEMENTATION "" CACHE STRING "Semicolon-separated list of implementations to include (${SIMDJSON_ALL_IMPLEMENTATIONS}). If this is not set, any implementations that are supported at compile time and may be selected at runtime will be included.")
set(SIMDJSON_EXCLUDE_IMPLEMENTATION "" CACHE STRING "Semicolon-separated list of implementations to exclude (haswell/westmere/arm64/fallback). By default, excludes any implementations that are unsupported at compile time or cannot be selected at runtime.")

foreach(var IN ITEMS SIMDJSON_IMPLEMENTATION SIMDJSON_EXCLUDE_IMPLEMENTATION)
  foreach(implementation IN LISTS ${var})
    if(NOT implementation IN_LIST SIMDJSON_ALL_IMPLEMENTATIONS)
      message(ERROR "Implementation ${implementation} not supported by simdjson. Possible implementations: ${SIMDJSON_ALL_IMPLEMENTATIONS}")
    endif()
  endforeach()
endforeach()

foreach(implementation IN LISTS SIMDJSON_ALL_IMPLEMENTATIONS)
  string(TOUPPER ${implementation} implementation_upper)
  if(implementation IN_LIST SIMDJSON_EXCLUDE_IMPLEMENTATION)
    message(STATUS "Excluding implementation ${implementation} due to SIMDJSON_EXCLUDE_IMPLEMENTATION=${SIMDJSON_EXCLUDE_IMPLEMENTATION}")
    target_compile_definitions(simdjson PUBLIC "SIMDJSON_IMPLEMENTATION_${implementation_upper}=0")
  elseif(implementation IN_LIST SIMDJSON_IMPLEMENTATION)
    message(STATUS "Including implementation ${implementation} due to SIMDJSON_IMPLEMENTATION=${SIMDJSON_IMPLEMENTATION}")
    target_compile_definitions(simdjson PUBLIC "SIMDJSON_IMPLEMENTATION_${implementation_upper}=1")
  elseif(SIMDJSON_IMPLEMENTATION)
    message(STATUS "Excluding implementation ${implementation} due to SIMDJSON_IMPLEMENTATION=${SIMDJSON_IMPLEMENTATION}")
    target_compile_definitions(simdjson PUBLIC "SIMDJSON_IMPLEMENTATION_${implementation_upper}=0")
  endif()
endforeach()

# TODO make it so this generates the necessary compiler flags to select the given implementation as the builtin automatically!
option(SIMDJSON_BUILTIN_IMPLEMENTATION "Select the implementation that will be used for user code. Defaults to the most universal implementation in SIMDJSON_IMPLEMENTATION (in the order ${SIMDJSON_ALL_IMPLEMENTATIONS}) if specified; otherwise, by default the compiler will pick the best implementation that can always be selected given the compiler flags." "")
if(SIMDJSON_BUILTIN_IMPLEMENTATION)
  target_compile_definitions(simdjson PUBLIC "SIMDJSON_BUILTIN_IMPLEMENTATION=${SIMDJSON_BUILTIN_IMPLEMENTATION}")
else()
  # Pick the most universal implementation out of the selected implementations (if any)
  foreach(implementation IN LISTS SIMDJSON_ALL_IMPLEMENTATIONS)
    if(implementation IN_LIST SIMDJSON_IMPLEMENTATION AND NOT (implementation IN_LIST SIMDJSON_EXCLUDE_IMPLEMENTATION))
      message(STATUS "Selected implementation ${implementation} as builtin implementation based on ${SIMDJSON_IMPLEMENTATION}.")
      target_compile_definitions(simdjson PUBLIC "SIMDJSON_BUILTIN_IMPLEMENTATION=${implementation}")
      break()
    endif()
  endforeach()
endif(SIMDJSON_BUILTIN_IMPLEMENTATION)

option(SIMDJSON_IMPLEMENTATION_HASWELL "Include the haswell implementation" ON)
if(NOT SIMDJSON_IMPLEMENTATION_HASWELL)
  message(DEPRECATION "SIMDJSON_IMPLEMENTATION_HASWELL is deprecated. Use SIMDJSON_IMPLEMENTATION=-haswell instead.")
  target_compile_definitions(simdjson PUBLIC SIMDJSON_IMPLEMENTATION_HASWELL=0)
endif()

option(SIMDJSON_IMPLEMENTATION_WESTMERE "Include the westmere implementation" ON)
if(NOT SIMDJSON_IMPLEMENTATION_WESTMERE)
  message(DEPRECATION "SIMDJSON_IMPLEMENTATION_WESTMERE is deprecated. SIMDJSON_IMPLEMENTATION=-westmere instead.")
  target_compile_definitions(simdjson PUBLIC SIMDJSON_IMPLEMENTATION_WESTMERE=0)
endif()

option(SIMDJSON_IMPLEMENTATION_ARM64 "Include the arm64 implementation" ON)
if(NOT SIMDJSON_IMPLEMENTATION_ARM64)
  message(DEPRECATION "SIMDJSON_IMPLEMENTATION_ARM64 is deprecated. Use SIMDJSON_IMPLEMENTATION=-arm64 instead.")
  target_compile_definitions(simdjson PUBLIC SIMDJSON_IMPLEMENTATION_ARM64=0)
endif()

option(SIMDJSON_IMPLEMENTATION_FALLBACK "Include the fallback implementation" ON)
if(NOT SIMDJSON_IMPLEMENTATION_FALLBACK)
  message(DEPRECATION "SIMDJSON_IMPLEMENTATION_FALLBACK is deprecated. Use SIMDJSON_IMPLEMENTATION=-fallback instead.")
  target_compile_definitions(simdjson PUBLIC SIMDJSON_IMPLEMENTATION_FALLBACK=0)
endif()
