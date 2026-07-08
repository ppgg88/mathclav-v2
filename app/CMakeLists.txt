find_package(Qt6 REQUIRED COMPONENTS Widgets Charts Network)

# MOC/RCC/UIC generated code is only needed by this Qt target, not by the
# Qt-free mathclav_core library.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(FetchMicroTeX)

add_executable(mathclav
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/FormulaView.cpp
    src/FormulaView.h
    src/microtex/MicroTexContext.cpp
    src/microtex/MicroTexContext.h
    src/microtex/FormulaRenderer.cpp
    src/microtex/FormulaRenderer.h
    src/input/ModeState.h
    src/input/PhysicalKey.cpp
    src/input/PhysicalKey.h
    src/input/KeymapTables.cpp
    src/input/KeymapTables.h
    src/input/KeyDispatcher.cpp
    src/input/KeyDispatcher.h
    src/widgets/MultiChoicePopup.cpp
    src/widgets/MultiChoicePopup.h
    src/widgets/GrapherDialog.cpp
    src/widgets/GrapherDialog.h
    src/widgets/HistoryDialog.cpp
    src/widgets/HistoryDialog.h
    src/widgets/CreditsDialog.cpp
    src/widgets/CreditsDialog.h
    src/widgets/RaptorEasterEgg.cpp
    src/widgets/RaptorEasterEgg.h
    src/widgets/HelpDialog.cpp
    src/widgets/HelpDialog.h
    src/widgets/UpdateCheckDialog.cpp
    src/widgets/UpdateCheckDialog.h
    src/graphing/ChartBuilder.cpp
    src/graphing/ChartBuilder.h
    src/graphing/AsymptoteOverlay.cpp
    src/graphing/AsymptoteOverlay.h
    resources/mathclav.qrc
)

target_include_directories(mathclav PRIVATE src)

target_link_libraries(mathclav PRIVATE
    mathclav_core
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network
    LaTeX
)
mathclav_enable_warnings(mathclav)

set_target_properties(mathclav PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    OUTPUT_NAME "MathClav"
)

# Deploy MicroTeX's res/ (fonts, symbol XML tables) next to the built
# binary so MicroTexContext can find it at runtime via
# QCoreApplication::applicationDirPath() -- same relative layout in dev
# builds and in packaging (windeployqt/AppImage both preserve the
# executable's own directory).
add_custom_command(TARGET mathclav POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${MATHCLAV_MICROTEX_RES_DIR}"
        "$<TARGET_FILE_DIR:mathclav>/res"
    COMMENT "Deploying MicroTeX res/ next to the mathclav binary"
)

install(TARGETS mathclav RUNTIME DESTINATION .)
install(DIRECTORY "${MATHCLAV_MICROTEX_RES_DIR}" DESTINATION .)

# Bundle Qt's own DLLs/plugins next to the binary on Windows, both for a
# dev build (so it's runnable straight from the build tree without Qt on
# PATH) and for packaging (packaging/windows/mathclav.nsi installs
# whatever's sitting next to the .exe as-is, unmodified). windeployqt
# isn't exposed as a CMake target; it's found the standard way, next to
# qmake (Qt6::qmake's IMPORTED_LOCATION).
if(WIN32)
    get_target_property(_qt_qmake_location Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qt_qmake_location}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET mathclav POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                "$<TARGET_FILE:mathclav>"
            COMMENT "Deploying Qt runtime next to the mathclav binary (windeployqt)"
        )
        # Also run for `cmake --install`: its own install(TARGETS/DIRECTORY)
        # rules above only copy what CMake was explicitly told to install,
        # not whatever windeployqt separately drops in the *build* tree --
        # without this, an installed-but-not-packaged-via-NSIS tree would
        # be missing every Qt DLL.
        install(CODE "
            execute_process(COMMAND \"${WINDEPLOYQT_EXECUTABLE}\"
                --no-translations --no-system-d3d-compiler --no-opengl-sw
                \"\${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:mathclav>\")
        ")
    else()
        message(WARNING "windeployqt not found next to qmake (${_qt_bin_dir}) -- "
                         "the built .exe won't be able to find its Qt DLLs unless "
                         "Qt's bin/ is on PATH.")
    endif()
endif()
