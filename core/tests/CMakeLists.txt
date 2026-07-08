include(FetchCatch2)

add_executable(mathclav_core_tests
    test_version.cpp
    test_node_catalog.cpp
    test_latex_serializer.cpp
    test_cursor_ops.cpp
    test_multichoice_cycle.cpp
    test_caret_splice.cpp
    test_graph_ast_builder.cpp
    test_evaluator.cpp
    test_json_node_codec.cpp
    test_session_store.cpp
    test_history_store.cpp
    test_settings_schema.cpp
)

target_link_libraries(mathclav_core_tests PRIVATE
    mathclav_core
    Catch2::Catch2WithMain
)
mathclav_enable_warnings(mathclav_core_tests)

include(Catch)
catch_discover_tests(mathclav_core_tests)
