# Folia (C++) — native self-rendered WYSIWYG Markdown editor.
#
# Build design:
#   - C++23 with C++ modules (CMake FILE_SET CXX_MODULES) + `import std;`.
#   - Core library `folia_core` is PURE C++: no Windows / DirectX / WinUI deps.
#   - Platform library `folia_platform` is the ONLY layer touching Win32 /
#     Direct2D / DirectWrite / DXGI / TSF / Clipboard (Windows headers stay
#     in that translation unit's global module fragment so no macros leak).
#   - App `Folia` is a WinUI 3 shell hosting a self-drawn EditorView surface.
#
# `import std;` is experimental in CMake 4.3; the UUID below is the official
# activation token for the CxxImportStd experimental feature in CMake 4.3.
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")

cmake_minimum_required(VERSION 4.0)

project(folia
    VERSION 0.2.1
    LANGUAGES C CXX
    DESCRIPTION "Folia: native self-rendered WYSIWYG Markdown editor (C++)")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

# Require Ninja (needed for CMake's C++ module dependency scanning with MSVC).
if(NOT CMAKE_GENERATOR MATCHES "Ninja")
    message(FATAL_ERROR "Folia requires the Ninja generator for C++ module support. Reconfigure with -G Ninja.")
endif()

if(MSVC)
    # /utf-8: source and execution charset UTF-8 (we embed box-drawing chars).
    # /permissive-: strict standards conformance.
    add_compile_options(/utf-8 /permissive- /W3 /Zc:preprocessor /Zc:__cplusplus)
    add_compile_definitions(WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNDEFINED _UNICODE)
    if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        add_compile_options(/O2)
    endif()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

option(FOLIA_BUILD_WINUI "Build the WinUI 3 C++/WinRT application shell" OFF)

if(FOLIA_BUILD_WINUI)
    message(FATAL_ERROR "The WinUI 3 shell is built with MSBuild at src/app-winui/Folia.vcxproj so XAML/IDL/C++WinRT generation runs through the Windows App SDK targets. Keep CMake for folia_core, folia_platform, and tests.")
endif()

# ---------------------------------------------------------------------------
# Core library — pure C++, portable. Acceptance gate: no Windows dependency.
# ---------------------------------------------------------------------------
add_library(folia_tree_sitter_html STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tree-sitter/runtime/lib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tree-sitter/grammars/html_parser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tree-sitter/grammars/html_scanner.c)
target_include_directories(folia_tree_sitter_html PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tree-sitter/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tree-sitter/runtime)
set_target_properties(folia_tree_sitter_html PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON)
if(MSVC)
    target_compile_options(folia_tree_sitter_html PRIVATE /wd4244 /wd4996)
endif()

add_library(folia_core STATIC)
target_compile_features(folia_core PUBLIC cxx_std_23)
set_target_properties(folia_core PROPERTIES CXX_MODULE_STD ON)
target_include_directories(folia_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tree-sitter/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/srell)
target_link_libraries(folia_core PRIVATE folia_tree_sitter_html)

file(GLOB_RECURSE FOLIA_CORE_MODULE CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.ixx)
file(GLOB_RECURSE FOLIA_CORE_SOURCE CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp)
target_sources(folia_core PUBLIC
    FILE_SET CXX_MODULES FILES ${FOLIA_CORE_MODULE})
target_sources(folia_core PRIVATE ${FOLIA_CORE_SOURCE})

# Native math reconstruction: only reviewed typed C++ owners are active.
# Parser, package families, common placement and Renderer are incomplete.
# No withdrawn Context runtime, app bridge, or fallback is linked here.
add_library(folia_math STATIC)
target_compile_features(folia_math PUBLIC cxx_std_23)
set_target_properties(folia_math PROPERTIES CXX_MODULE_STD ON)
target_include_directories(folia_math PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/srell)
# Font tables are generated offline from the pinned Source package data. The
# generator verifies every locked input and preserves unchanged output mtimes.
find_program(FOLIA_NODE_EXECUTABLE NAMES node REQUIRED)
execute_process(
    COMMAND ${FOLIA_NODE_EXECUTABLE} tools/math-port/font-data-generate-native.cjs
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE FOLIA_FONT_GENERATE_RESULT
    OUTPUT_VARIABLE FOLIA_FONT_GENERATE_OUTPUT
    ERROR_VARIABLE FOLIA_FONT_GENERATE_ERROR)
if(NOT FOLIA_FONT_GENERATE_RESULT EQUAL 0)
    message(FATAL_ERROR "Native font data generation failed: ${FOLIA_FONT_GENERATE_ERROR}")
endif()
set(FOLIA_FONT_GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/math-port/font-generated-native-raii")
file(READ "${FOLIA_FONT_GENERATED_DIR}/font-data-source-dependencies.json" FOLIA_FONT_DEPENDENCIES)
string(JSON FOLIA_FONT_DEPENDENCY_COUNT LENGTH "${FOLIA_FONT_DEPENDENCIES}" source_dependencies)
math(EXPR FOLIA_FONT_DEPENDENCY_LAST "${FOLIA_FONT_DEPENDENCY_COUNT} - 1")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/math-port/font-data-generate-native.cjs"
    "${CMAKE_CURRENT_SOURCE_DIR}/docs/math-port/evidence/font-data-source-lock.json")
foreach(FOLIA_FONT_INDEX RANGE 0 ${FOLIA_FONT_DEPENDENCY_LAST})
    string(JSON FOLIA_FONT_DEPENDENCY GET "${FOLIA_FONT_DEPENDENCIES}" source_dependencies ${FOLIA_FONT_INDEX} path)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/${FOLIA_FONT_DEPENDENCY}")
endforeach()
string(JSON FOLIA_FONT_GENERATED_COUNT LENGTH "${FOLIA_FONT_DEPENDENCIES}" generated_cpp)
math(EXPR FOLIA_FONT_GENERATED_LAST "${FOLIA_FONT_GENERATED_COUNT} - 1")
set(FOLIA_FONT_GENERATED_CPP)
foreach(FOLIA_FONT_INDEX RANGE 0 ${FOLIA_FONT_GENERATED_LAST})
    string(JSON FOLIA_FONT_GENERATED_FILE GET "${FOLIA_FONT_DEPENDENCIES}" generated_cpp ${FOLIA_FONT_INDEX})
    if(FOLIA_FONT_GENERATED_FILE MATCHES "[.]cpp$")
        list(APPEND FOLIA_FONT_GENERATED_CPP "${CMAKE_CURRENT_SOURCE_DIR}/${FOLIA_FONT_GENERATED_FILE}")
    endif()
endforeach()
target_include_directories(folia_math PRIVATE ${FOLIA_FONT_GENERATED_DIR})
set_property(GLOBAL APPEND PROPERTY JOB_POOLS folia_math_compile=2)
set_property(TARGET folia_math PROPERTY JOB_POOL_COMPILE folia_math_compile)
target_sources(folia_math PUBLIC FILE_SET CXX_MODULES FILES
    src/math/core/mml_tree/node_kind.ixx
    src/math/core/mml_tree/attributes.ixx
    src/math/core/mml_tree/node_properties.ixx
    src/math/core/mml_tree/mml_node.ixx
    src/math/core/mml_tree/mml_document.ixx
    src/math/core/mml_tree/node_traits.ixx
    src/math/core/mml_tree/node_defaults.ixx
    src/math/core/mml_tree/mml_factory.ixx
    src/math/input/tex/session.ixx
    src/math/input/tex/environment.ixx
    src/math/input/tex/tex_error.ixx
    src/math/input/tex/tex_scanner.ixx
    src/math/input/tex/parse_util.ixx
    src/math/input/tex/node_factory.ixx
    src/math/output/scene/types.ixx
    src/math/output/scene/scene.ixx
    src/math/output/scene/builder.ixx
    src/math/output/svg/writer.ixx
    src/math/output/svg/font_data.ixx
    src/math/output/common/font_data.ixx
    src/math/output/common/font_path.ixx
    src/math/output/common/common.ixx
    src/math/output/common/wrappers/mfrac.ixx
    src/math/output/common/wrappers/msqrt.ixx
    src/math/output/common/wrappers/mroot.ixx
    src/math/fonts/newcm_font_data.ixx
    src/math/fonts/newcm_font_module_provider.ixx
    src/math/util/constants.ixx
    src/math/util/bbox.ixx
    src/math/util/lengths.ixx
    src/math/util/js_number.ixx
    src/math/util/styles.ixx
    src/math/util/regexp_host.ixx)
target_sources(folia_math PRIVATE
    src/math/core/mml_tree/attributes.cpp
    src/math/core/mml_tree/node_properties.cpp
    src/math/core/mml_tree/mml_node.cpp
    src/math/core/mml_tree/mml_document.cpp
    src/math/core/mml_tree/mml_factory.cpp
    src/math/core/mml_tree/node_defaults.cpp
    src/math/input/tex/session.cpp
    src/math/input/tex/environment.cpp
    src/math/input/tex/tex_error.cpp
    src/math/input/tex/tex_scanner.cpp
    src/math/input/tex/parse_util.cpp
    src/math/input/tex/node_factory.cpp
    src/math/output/scene/scene.cpp
    src/math/output/scene/builder.cpp
    src/math/output/svg/writer.cpp
    src/math/output/svg/node_metadata.cpp
    src/math/output/svg/font_data.cpp
    src/math/output/common/font_data.cpp
    src/math/output/common/font_path.cpp
    src/math/output/common/wrappers/mfrac.cpp
    src/math/output/common/wrappers/msqrt.cpp
    src/math/output/common/wrappers/mroot.cpp
    src/math/fonts/newcm_font_data.cpp
    src/math/fonts/newcm_font_module_provider.cpp
    ${FOLIA_FONT_GENERATED_CPP}
    src/math/util/lengths.cpp
    src/math/util/styles.cpp
    src/math/util/regexp_host.cpp)

# ---------------------------------------------------------------------------
# Platform library — the ONLY Windows-touching layer.
# ---------------------------------------------------------------------------
if(WIN32)
    # Locate the Windows 10/11 SDK so platform .ixx files can #include
    # <dwrite.h>/<d2d1.h>/... without relying on a globally-injected env.
    set(_win_sdk_root "C:/Program Files (x86)/Windows Kits/10")
    file(GLOB _win_sdk_inc_dirs LIST_DIRECTORIES true "${_win_sdk_root}/Include/10.0.*")
    if(_win_sdk_inc_dirs)
        list(SORT _win_sdk_inc_dirs ORDER DESCENDING)
        list(GET _win_sdk_inc_dirs 0 _win_sdk_inc)
        set(FOLIA_WINSDK_INC "${_win_sdk_inc}" CACHE PATH "Windows SDK Include dir")
    endif()

    file(GLOB_RECURSE FOLIA_PLATFORM_MODULE CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/*.ixx)
    file(GLOB_RECURSE FOLIA_PLATFORM_SRC CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/*.cpp)

    if(FOLIA_PLATFORM_MODULE OR FOLIA_PLATFORM_SRC)
        add_library(folia_platform STATIC)
        target_compile_features(folia_platform PUBLIC cxx_std_23)
        set_target_properties(folia_platform PROPERTIES CXX_MODULE_STD ON)
        target_sources(folia_platform PUBLIC
            FILE_SET CXX_MODULES FILES ${FOLIA_PLATFORM_MODULE})
        target_sources(folia_platform PRIVATE ${FOLIA_PLATFORM_SRC})
        target_link_libraries(folia_platform PUBLIC folia_core)
        target_include_directories(folia_platform PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/src/platform)
        if(FOLIA_WINSDK_INC)
            target_include_directories(folia_platform PUBLIC
                ${FOLIA_WINSDK_INC}/um
                ${FOLIA_WINSDK_INC}/shared
                ${FOLIA_WINSDK_INC}/ucrt
                ${FOLIA_WINSDK_INC}/cppwinrt)
        endif()
        target_link_libraries(folia_platform PUBLIC
            dwrite d2d1 d3d11 dxgi windowscodecs dwmapi uxtheme shlwapi
            ole32 oleaut32 uuid user32 gdi32 comctl32
            shell32 advapi32 imm32 propsys)

    endif()
endif()

# ---------------------------------------------------------------------------
# Tests — pure core tests, no Windows dependency, portable.
#
# Boost.UT is consumed through its C++ module. The vendored fork fixes MSVC
# module linkage and command-line initialization; main passes argc/argv
# explicitly so no test translation unit depends on MSVC's private CRT globals.
# ---------------------------------------------------------------------------
set(BOOST_UT_DISABLE_MODULE OFF CACHE BOOL "Build Boost.UT's C++ module" FORCE)
set(BOOST_UT_BUILD_EXAMPLES OFF CACHE BOOL "Do not build Boost.UT examples" FORCE)
set(BOOST_UT_BUILD_TESTS OFF CACHE BOOL "Do not build Boost.UT self-tests" FORCE)
set(BOOST_UT_ENABLE_INSTALL OFF CACHE BOOL "Do not install vendored Boost.UT" FORCE)
add_subdirectory(third_party/ut EXCLUDE_FROM_ALL)

enable_testing()
add_executable(FoliaTests)
target_compile_features(FoliaTests PRIVATE cxx_std_23)
set_target_properties(FoliaTests PROPERTIES
    CXX_MODULE_STD ON
    CXX_SCAN_FOR_MODULES ON)
file(GLOB_RECURSE FOLIA_TEST_SRC CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
list(FILTER FOLIA_TEST_SRC EXCLUDE REGEX "/tests/platform/")
list(FILTER FOLIA_TEST_SRC EXCLUDE REGEX "/tests/encoding/")
list(FILTER FOLIA_TEST_SRC EXCLUDE REGEX "/tests/math/")
target_sources(FoliaTests PRIVATE
    ${FOLIA_TEST_SRC})
target_include_directories(FoliaTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(FoliaTests PRIVATE folia_core folia_math Boost::ut_module)
if(MSVC)
    # Native math recursion tests must reach the explicit 512-call resource
    # guard even with Debug variant/callback frames. App math threads must
    # satisfy the same stack requirement before the native integration gate.
    target_link_options(FoliaTests PRIVATE /STACK:16777216)
endif()
# Boost.UT parses positional test-name patterns and CLI options. For example,
# `FoliaTests.exe "*name*"` filters by name; `-t` lists tags.
add_test(NAME FoliaTests COMMAND FoliaTests)

# Native foundation tests are explicit. Historical mathematical suites remain
# tracked for typed migration; they are not reported as passing this rewrite.
add_executable(FoliaMathTests EXCLUDE_FROM_ALL
    tests/main.cpp
    tests/math/mml_native_raii_foundation.cpp
    tests/math/native_raii_scene.cpp
    tests/math/lengths_native.cpp
    tests/math/native_raii_regexp.cpp
    tests/math/native_tex_session.cpp
    tests/math/native_raii_tex_environment.cpp
    tests/math/native_svg_writer.cpp
    tests/math/native_tex_error_source.cpp
    tests/math/native_tex_error_utf8.cpp
    tests/math/native_tex_scanner.cpp
    tests/math/parse_util_native.cpp
    tests/math/mml_native_factory.cpp
    tests/math/mml_defaults.cpp
    tests/math/mml_node_state.cpp
    tests/math/native_raii_fonts.cpp
    tests/math/common_styles_source.cpp
    tests/math/fraction_native.cpp
    tests/math/radical_native.cpp)
target_compile_features(FoliaMathTests PRIVATE cxx_std_23)
set_target_properties(FoliaMathTests PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_include_directories(FoliaMathTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_compile_definitions(FoliaMathTests PRIVATE
    FOLIA_MATH_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/math/fixtures")
target_link_libraries(FoliaMathTests PRIVATE folia_math Boost::ut_module)
add_test(NAME FoliaMathTests COMMAND FoliaMathTests)
add_executable(FoliaMathFontDump EXCLUDE_FROM_ALL
    tools/math-port/font-data-dump.cpp)
target_compile_features(FoliaMathFontDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathFontDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathFontDump PRIVATE folia_math)
if(MSVC)
    target_link_options(FoliaTests PRIVATE /DEBUG:NONE /INCREMENTAL:NO)
    target_link_options(FoliaMathTests PRIVATE /DEBUG:NONE /INCREMENTAL:NO)
    target_link_options(FoliaMathFontDump PRIVATE /DEBUG:NONE /INCREMENTAL:NO)
endif()

if(TARGET folia_platform)
    add_executable(FoliaWindowsEditorTests)
    target_compile_features(FoliaWindowsEditorTests PRIVATE cxx_std_23)
    set_target_properties(FoliaWindowsEditorTests PROPERTIES
        CXX_MODULE_STD ON
        CXX_SCAN_FOR_MODULES ON)
    file(GLOB_RECURSE FOLIA_PLATFORM_TEST_SRC CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/platform/*.cpp)
    target_sources(FoliaWindowsEditorTests PRIVATE
        ${FOLIA_PLATFORM_TEST_SRC})
    target_include_directories(FoliaWindowsEditorTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
        ${CMAKE_CURRENT_SOURCE_DIR}/src/platform)
    target_link_libraries(FoliaWindowsEditorTests PRIVATE
        folia_platform Boost::ut_module)
    add_test(NAME FoliaWindowsEditorTests COMMAND FoliaWindowsEditorTests)
endif()

if(WIN32)
    set(FOLIA_ICU_ROOT
        "${CMAKE_CURRENT_SOURCE_DIR}/build/dependencies/icu-78.3/root"
        CACHE PATH "ICU4C root used by document encoding tests")
    if(EXISTS "${FOLIA_ICU_ROOT}/include/unicode/ucsdet.h")
        add_executable(FoliaEncodingTests
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/encoding/main.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/encoding/test_document_encoding.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/app-winui/storage/DocumentEncodingService.cpp)
        target_compile_features(FoliaEncodingTests PRIVATE cxx_std_23)
        set_target_properties(FoliaEncodingTests PROPERTIES
            CXX_MODULE_STD ON
            CXX_SCAN_FOR_MODULES ON)
        target_include_directories(FoliaEncodingTests PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/tests
            ${CMAKE_CURRENT_SOURCE_DIR}/src/app-winui
            ${FOLIA_ICU_ROOT}/include)
        target_sources(FoliaEncodingTests PRIVATE
            FILE_SET CXX_MODULES FILES
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/encoding/document_encoding_test_support.ixx)
        target_link_libraries(FoliaEncodingTests PRIVATE
            Boost::ut_module
            ${FOLIA_ICU_ROOT}/lib64/icuin.lib
            ${FOLIA_ICU_ROOT}/lib64/icuuc.lib
            ${FOLIA_ICU_ROOT}/lib64/icudt.lib)
        add_custom_command(TARGET FoliaEncodingTests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${FOLIA_ICU_ROOT}/bin64/icuin78.dll
                ${FOLIA_ICU_ROOT}/bin64/icuuc78.dll
                ${FOLIA_ICU_ROOT}/bin64/icudt78.dll
                $<TARGET_FILE_DIR:FoliaEncodingTests>)
        add_test(NAME FoliaEncodingTests COMMAND FoliaEncodingTests)
    endif()
endif()

# These CMake executables are console tests/probes; the WinUI app uses MSBuild.
# Keep CRT reports on stderr, including failures before main().
if(WIN32 AND MSVC)
    add_library(folia_console_test_runtime OBJECT tools/test-support/windows_console_runtime.cpp)
    target_compile_features(folia_console_test_runtime PRIVATE cxx_std_23)
    set_target_properties(folia_console_test_runtime PROPERTIES
        CXX_MODULE_STD OFF CXX_SCAN_FOR_MODULES OFF)
    get_property(folia_console_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
    foreach(folia_console_target IN LISTS folia_console_targets)
        get_target_property(folia_console_type ${folia_console_target} TYPE)
        if(folia_console_type STREQUAL "EXECUTABLE" AND folia_console_target MATCHES "^Folia")
            target_link_libraries(${folia_console_target} PRIVATE folia_console_test_runtime)
        endif()
    endforeach()
endif()
