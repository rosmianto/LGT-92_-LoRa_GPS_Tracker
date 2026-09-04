add_executable(lgt92_sim
    wasm_main.cpp
)

target_link_libraries(lgt92_sim PRIVATE
    lgt92_core
)

set_target_properties(lgt92_sim PROPERTIES
    SUFFIX ".html"
)

target_link_options(lgt92_sim PRIVATE
    "-sWASM=1"
    "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString']"
    "-sNO_EXIT_RUNTIME=1"
    "--shell-file" "${CMAKE_CURRENT_SOURCE_DIR}/web/shell_minimal.html"
)

