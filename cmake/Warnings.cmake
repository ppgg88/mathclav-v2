# Applies the project's standard warning set to a target.
#
# core/ targets additionally get -Werror (Linux/macOS) or /WX (MSVC) via
# mathclav_enable_strict_warnings, since the tree/cursor/evaluator logic is
# exactly where exhaustiveness (-Wswitch-enum) is meant to catch missed
# NodeKind/GraphKind cases at compile time.

function(mathclav_enable_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wswitch-enum)
  endif()
endfunction()

function(mathclav_enable_strict_warnings target)
  mathclav_enable_warnings(${target})
  if(MSVC)
    target_compile_options(${target} PRIVATE /WX)
  else()
    target_compile_options(${target} PRIVATE -Werror)
  endif()
endfunction()
