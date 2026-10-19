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

# Native math port prerequisites, independently tested before app integration.
# Explicit file ownership: do not collect an unreviewed port through a glob.
add_library(folia_math STATIC)
target_compile_features(folia_math PUBLIC cxx_std_23)
set_target_properties(folia_math PROPERTIES CXX_MODULE_STD ON)
target_include_directories(folia_math PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/srell)
find_program(FOLIA_MATH_FONT_NODE NAMES node REQUIRED)
set(FOLIA_MATH_BASE_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/math-port/base-generated")
set(FOLIA_MATH_BASE_INITIAL_SNAPSHOT "${CMAKE_CURRENT_BINARY_DIR}/math-port/base-generated/base-mappings-initial.json")
set(FOLIA_MATH_BASE_GENERATED "${FOLIA_MATH_BASE_GENERATED_DIR}/base_mapping_data.inc")
set(FOLIA_MATH_BASE_SOURCE_INPUTS
    "${CMAKE_CURRENT_SOURCE_DIR}/.reference/MathJax-src/ts/input/tex/base/BaseConfiguration.ts"
    "${CMAKE_CURRENT_SOURCE_DIR}/.reference/MathJax-src/ts/input/tex/base/BaseMethods.ts"
    "${CMAKE_CURRENT_SOURCE_DIR}/.reference/MathJax-src/ts/input/tex/base/BaseMappings.ts"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/base/BaseConfiguration.js"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/base/BaseMethods.js"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/base/BaseMappings.js"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/Configuration.js"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/HandlerTypes.js"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/MapHandler.js"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/TokenMap.js"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/Token.js")
add_custom_command(OUTPUT "${FOLIA_MATH_BASE_INITIAL_SNAPSHOT}" "${FOLIA_MATH_BASE_GENERATED}"
    COMMAND "${FOLIA_MATH_FONT_NODE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/math-port/base-mappings-initial-oracle.cjs"
        "${FOLIA_MATH_BASE_INITIAL_SNAPSHOT}"
    COMMAND "${FOLIA_MATH_FONT_NODE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/math-port/generate-base-mapping-data.cjs"
        --input "${FOLIA_MATH_BASE_INITIAL_SNAPSHOT}" --output "${FOLIA_MATH_BASE_GENERATED}"
    DEPENDS tools/math-port/base-mappings-initial-oracle.cjs
        tools/math-port/generate-base-mapping-data.cjs ${FOLIA_MATH_BASE_SOURCE_INPUTS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    VERBATIM)
add_custom_target(folia_math_base_data DEPENDS
    "${FOLIA_MATH_BASE_INITIAL_SNAPSHOT}" "${FOLIA_MATH_BASE_GENERATED}")
add_dependencies(folia_math folia_math_base_data)
set_source_files_properties(src/math/input/tex/base/base_mappings.cpp PROPERTIES
    INCLUDE_DIRECTORIES "${FOLIA_MATH_BASE_GENERATED_DIR}")
set(FOLIA_MATH_FONT_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/math-port/font-generated")
file(GLOB_RECURSE FOLIA_MATH_FONT_INPUTS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/mathjax-bundle/node_modules/@mathjax/mathjax-newcm-font/cjs/*.js"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/mathjax-bundle/node_modules/@mathjax/mathjax-bbm-font-extension/cjs/*.js"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/mathjax-bundle/node_modules/@mathjax/mathjax-bboldx-font-extension/cjs/*.js"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/mathjax-bundle/node_modules/@mathjax/mathjax-dsfont-font-extension/cjs/*.js")
set(FOLIA_MATH_FONT_GENERATED
    "${FOLIA_MATH_FONT_GENERATED_DIR}/newcm_font_data.inc"
    "${FOLIA_MATH_FONT_GENERATED_DIR}/font_extensions.inc"
    "${FOLIA_MATH_FONT_GENERATED_DIR}/newcm_dynamic_setup.inc")
# The frozen provenance manifest enumerates each reviewed Source shard. These
# generated implementation units contain only its transcribed native tables.
set(FOLIA_MATH_FONT_LOCK "${CMAKE_CURRENT_SOURCE_DIR}/docs/math-port/evidence/font-data-source-lock.json")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${FOLIA_MATH_FONT_LOCK}")
file(READ "${FOLIA_MATH_FONT_LOCK}" FOLIA_MATH_FONT_MANIFEST)
string(JSON FOLIA_MATH_FONT_OUTPUT_COUNT LENGTH "${FOLIA_MATH_FONT_MANIFEST}" outputs)
math(EXPR FOLIA_MATH_FONT_OUTPUT_LAST "${FOLIA_MATH_FONT_OUTPUT_COUNT} - 1")
set(FOLIA_MATH_FONT_SHARD_SOURCES)
foreach(FOLIA_MATH_FONT_OUTPUT_INDEX RANGE 0 ${FOLIA_MATH_FONT_OUTPUT_LAST})
    string(JSON FOLIA_MATH_FONT_OUTPUT_PATH GET "${FOLIA_MATH_FONT_MANIFEST}" outputs ${FOLIA_MATH_FONT_OUTPUT_INDEX} path)
    if(FOLIA_MATH_FONT_OUTPUT_PATH MATCHES "^build/math-port/font-generated/(newcm_dynamic_setup_[A-Za-z0-9_]+[.]cpp)$")
        list(APPEND FOLIA_MATH_FONT_SHARD_SOURCES "${FOLIA_MATH_FONT_GENERATED_DIR}/${CMAKE_MATCH_1}")
    endif()
endforeach()
list(LENGTH FOLIA_MATH_FONT_SHARD_SOURCES FOLIA_MATH_FONT_SHARD_COUNT)
if(NOT FOLIA_MATH_FONT_SHARD_COUNT EQUAL 40)
    message(FATAL_ERROR "The pinned NewCM profile requires exactly 40 reviewed dynamic shard implementation units.")
endif()
list(APPEND FOLIA_MATH_FONT_GENERATED ${FOLIA_MATH_FONT_SHARD_SOURCES})
add_custom_command(OUTPUT ${FOLIA_MATH_FONT_GENERATED}
    COMMAND "${FOLIA_MATH_FONT_NODE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/math-port/font-data-generate.cjs"
        --check-lock --output-dir "${FOLIA_MATH_FONT_GENERATED_DIR}"
    DEPENDS tools/math-port/font-data-generate.cjs
        docs/math-port/evidence/font-data-source-lock.json ${FOLIA_MATH_FONT_INPUTS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    VERBATIM)
add_custom_target(folia_math_font_data DEPENDS ${FOLIA_MATH_FONT_GENERATED})
add_dependencies(folia_math folia_math_font_data)
target_sources(folia_math PRIVATE ${FOLIA_MATH_FONT_SHARD_SOURCES})
set_property(GLOBAL APPEND PROPERTY JOB_POOLS folia_math_font_compile=2)
set_source_files_properties(${FOLIA_MATH_FONT_SHARD_SOURCES}
    src/math/fonts/newcm_font_data.cpp src/math/fonts/svg_extensions.cpp
    PROPERTIES JOB_POOL_COMPILE folia_math_font_compile)
target_include_directories(folia_math PRIVATE "${FOLIA_MATH_FONT_GENERATED_DIR}")

set(FOLIA_MATH_ENTITIES_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/math-port/entities")
set(FOLIA_MATH_ENTITIES_GENERATED "${FOLIA_MATH_ENTITIES_GENERATED_DIR}/entities_data.inc")
set(FOLIA_MATH_ENTITIES_INPUTS
    "${CMAKE_CURRENT_SOURCE_DIR}/.reference/MathJax-src/ts/util/Entities.ts"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/mathjax-bundle/node_modules/@mathjax/src/cjs/util/Entities.js")
foreach(FOLIA_MATH_ENTITY_SHARD IN ITEMS
    a b c d e f g h i j k l m n o p q r s t u v w x y z fr opf scr)
    list(APPEND FOLIA_MATH_ENTITIES_INPUTS
        "${CMAKE_CURRENT_SOURCE_DIR}/.reference/MathJax-src/ts/util/entities/${FOLIA_MATH_ENTITY_SHARD}.ts"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/mathjax-bundle/node_modules/@mathjax/src/cjs/util/entities/${FOLIA_MATH_ENTITY_SHARD}.js")
endforeach()
add_custom_command(OUTPUT "${FOLIA_MATH_ENTITIES_GENERATED}"
    COMMAND "${FOLIA_MATH_FONT_NODE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/math-port/entities-data-generate.cjs"
        --output "${FOLIA_MATH_ENTITIES_GENERATED}"
    DEPENDS tools/math-port/entities-data-generate.cjs
        docs/math-port/evidence/entities-source-lock.json ${FOLIA_MATH_ENTITIES_INPUTS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    VERBATIM)
add_custom_target(folia_math_entities_data DEPENDS "${FOLIA_MATH_ENTITIES_GENERATED}")
add_dependencies(folia_math folia_math_entities_data)
set_source_files_properties(src/math/util/entities.cpp PROPERTIES
    INCLUDE_DIRECTORIES "${FOLIA_MATH_ENTITIES_GENERATED_DIR}")

set(FOLIA_MATH_AMS_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/math-port/ams")
set(FOLIA_MATH_AMS_GENERATED "${FOLIA_MATH_AMS_GENERATED_DIR}/ams_mapping_data.inc")
add_custom_command(OUTPUT "${FOLIA_MATH_AMS_GENERATED}"
    COMMAND "${FOLIA_MATH_FONT_NODE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/math-port/generate-ams-mapping-data.cjs"
        --check-lock --output-dir "${FOLIA_MATH_AMS_GENERATED_DIR}"
    DEPENDS tools/math-port/generate-ams-mapping-data.cjs
        tools/math-port/ams-newcommand-reference/ams-newcommand-source.json
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    VERBATIM)
add_custom_target(folia_math_ams_data DEPENDS "${FOLIA_MATH_AMS_GENERATED}")
add_dependencies(folia_math folia_math_ams_data)
set_source_files_properties(src/math/input/tex/ams/mappings.cpp PROPERTIES
    INCLUDE_DIRECTORIES "${FOLIA_MATH_AMS_GENERATED_DIR}")
target_sources(folia_math PUBLIC FILE_SET CXX_MODULES FILES
    src/math/core/dom_node.ixx
    src/math/core/math_item.ixx
    src/math/core/math_location.ixx
    src/math/core/math_list.ixx
    src/math/util/runtime_modules.ixx
    src/math/util/linked_list.ixx
    src/math/core/output_jax.ixx
    src/math/core/jax_filter_record_context.ixx
    src/math/core/math_item_host.ixx
    src/math/core/input_jax.ixx
    src/math/core/input_jax_host.ixx
    src/math/core/find_math.ixx
    src/math/core/find_math_host.ixx
    src/math/util/regexp_host.ixx
    src/math/util/regexp_binding.ixx
    src/math/input/tex/find_tex.ixx
    src/math/input/tex/find_tex_host.ixx
    src/math/util/string_map.ixx
    src/math/util/unicode_normalization.ixx
    src/math/util/unicode_normalization_data.ixx
    src/math/input/tex/parser_types.ixx
    src/math/input/tex/configuration.ixx
    src/math/input/tex/tags.ixx
    src/math/input/tex/base/base_items.ixx
    src/math/input/tex/stack_item_factory_bindings.ixx
    src/math/input/tex/base/base_methods.ixx
    src/math/input/tex/base/base_parse_methods.ixx
    src/math/input/tex/base/base_mappings.ixx
    src/math/input/tex/base/base_configuration.ixx
    src/math/input/tex/ams/items.ixx
    src/math/input/tex/ams/methods.ixx
    src/math/input/tex/ams/mappings.ixx
    src/math/input/tex/ams/configuration.ixx
    src/math/input/tex/newcommand/items.ixx
    src/math/input/tex/newcommand/util.ixx
    src/math/input/tex/newcommand/methods.ixx
    src/math/input/tex/newcommand/mappings.ixx
    src/math/input/tex/newcommand/configuration.ixx
    src/math/input/tex/begingroup/stack.ixx
    src/math/input/tex/begingroup/methods.ixx
    src/math/input/tex/begingroup/configuration.ixx
    src/math/input/tex/action/configuration.ixx
    src/math/input/tex/bbox/configuration.ixx
    src/math/input/tex/braket/items.ixx
    src/math/input/tex/braket/methods.ixx
    src/math/input/tex/braket/mappings.ixx
    src/math/input/tex/braket/configuration.ixx
    src/math/input/tex/cancel/configuration.ixx
    src/math/input/tex/color/constants.ixx
    src/math/input/tex/color/util.ixx
    src/math/input/tex/color/methods.ixx
    src/math/input/tex/color/configuration.ixx
    src/math/input/tex/enclose/configuration.ixx
    src/math/input/tex/noundefined/configuration.ixx
    src/math/input/tex/verb/configuration.ixx
    src/math/input/tex/unicode/configuration.ixx
    src/math/input/tex/upgreek/configuration.ixx
    src/math/input/tex/gensymb/configuration.ixx
    src/math/input/tex/fontsizev3/configuration.ixx
    src/math/input/tex/extpfeil/configuration.ixx
    src/math/input/tex/bbm/configuration.ixx
    src/math/input/tex/bboldx/methods.ixx
    src/math/input/tex/bboldx/mappings.ixx
    src/math/input/tex/bboldx/configuration.ixx
    src/math/input/tex/dsfont/configuration.ixx
    src/math/input/tex/boldsymbol/configuration.ixx
    src/math/input/tex/configmacros/configuration.ixx
    src/math/input/tex/column_parser.ixx
    src/math/input/tex/parse_options.ixx
    src/math/input/tex/tex_parser.ixx
    src/math/input/tex/text_parser.ixx
    src/math/input/tex/textmacros_methods.ixx
    src/math/input/tex/textmacros_mappings.ixx
    src/math/input/tex/textmacros_configuration.ixx
    src/math/util/retries.ixx
    src/math/input/tex/parse_util.ixx
    src/math/input/tex/tex_jax.ixx
    src/math/input/tex/parser_runtime.ixx
    src/math/input/tex/parser_family.ixx
    src/math/util/object_types.ixx
    src/math/util/callable_types.ixx
    src/math/util/argument_expansion.ixx
    src/math/input/tex/token.ixx
    src/math/input/tex/node_util.ixx
    src/math/input/tex/node_factory.ixx
    src/math/input/tex/node_factory_host.ixx
    src/math/input/tex/stack_services.ixx
    src/math/input/tex/stack_property.ixx
    src/math/input/tex/stack_item.ixx
    src/math/input/tex/stack_item_host.ixx
    src/math/input/tex/base_item_host.ixx
    src/math/input/tex/stack_item_factory.ixx
    src/math/input/tex/stack.ixx
    src/math/input/tex/stack_array_access.ixx
    src/math/input/tex/stack_context_bindings.ixx
    src/math/input/tex/tex_parser_context_fields.ixx
    src/math/input/tex/stack_host.ixx
    src/math/input/tex/tex_error.ixx
    src/math/input/tex/tex_error_pattern.ixx
    src/math/input/tex/types.ixx
    src/math/input/tex/tex_parser_context.ixx
    src/math/input/tex/unit_pattern.ixx
    src/math/input/tex/unit_util.ixx
    src/math/input/tex/unit_util_host.ixx
    src/math/input/tex/token_map_types.ixx
    src/math/input/tex/token_map.ixx
    src/math/input/tex/map_handler.ixx
    src/math/util/method_config.ixx
    src/math/input/tex/macro.ixx
    src/math/input/tex/handler_types.ixx
    src/math/input/tex/tex_constants.ixx
    src/math/core/tree/property.ixx
    src/math/core/tree/child_array.ixx
    src/math/core/tree/factory_types.ixx
    src/math/core/tree/factory_class_map_owner.ixx
    src/math/core/tree/factory.ixx
    src/math/core/tree/factory_context_bindings.ixx
    src/math/core/tree/node_factory.ixx
    src/math/core/tree/wrapper_factory.ixx
    src/math/core/tree/node_types.ixx
    src/math/core/tree/node.ixx
    src/math/core/tree/wrapper_types.ixx
    src/math/core/tree/wrapper.ixx
    src/math/core/mml_tree/attributes.ixx
    src/math/core/mml_tree/mml_node_host.ixx
    src/math/core/mml_tree/mml_node.ixx
    src/math/core/mml_tree/mml_node_algorithms.ixx
    src/math/core/mml_tree/mml_node_initialization.ixx
    src/math/core/mml_tree/mml.ixx
    src/math/core/mml_tree/mml_factory.ixx
    src/math/core/mml_tree/action_host.ixx
    src/math/core/mml_tree/mml_nodes/maction.ixx
    src/math/core/mml_tree/html_adaptor.ixx
    src/math/core/mml_tree/mml_nodes/html_node.ixx
    src/math/core/mml_tree/class_binding.ixx
    src/math/core/mml_tree/operator_class.ixx
    src/math/core/mml_tree/mml_nodes/tex_atom.ixx
    src/math/core/mml_tree/mml_nodes/semantics.ixx
    src/math/core/mml_tree/mml_nodes/menclose.ixx
    src/math/core/mml_tree/mml_nodes/mpadded.ixx
    src/math/core/mml_tree/mml_nodes/maligngroup.ixx
    src/math/core/mml_tree/mml_nodes/malignmark.ixx
    src/math/core/mml_tree/mml_nodes/mathchoice.ixx
    src/math/core/mml_tree/mml_nodes/mglyph.ixx
    src/math/core/mml_tree/mml_nodes/mfenced.ixx
    src/math/core/mml_tree/multiscript_host.ixx
    src/math/core/mml_tree/mml_nodes/mmultiscripts.ixx
    src/math/core/mml_tree/table_host.ixx
    src/math/core/mml_tree/mml_nodes/mtable.ixx
    src/math/core/mml_tree/mml_nodes/mtr.ixx
    src/math/core/mml_tree/mml_nodes/mtd.ixx
    src/math/core/mml_tree/arithmetic_host.ixx
    src/math/core/mml_tree/mml_nodes/mfrac.ixx
    src/math/core/mml_tree/mml_nodes/msqrt.ixx
    src/math/core/mml_tree/mml_nodes/mroot.ixx
    src/math/core/mml_tree/leaf_host.ixx
    src/math/core/mml_tree/mo_patterns.ixx
    src/math/core/mml_tree/mo_types.ixx
    src/math/core/mml_tree/mo_host.ixx
    src/math/core/mml_tree/mml_nodes/mo.ixx
    src/math/core/mml_tree/leaf_defaults.ixx
    src/math/core/mml_tree/script_defaults.ixx
    src/math/core/mml_tree/operator_dictionary.ixx
    src/math/core/mml_tree/mml_nodes/msubsup.ixx
    src/math/core/mml_tree/mml_nodes/munderover.ixx
    src/math/core/mml_tree/mml_nodes/mrow.ixx
    src/math/core/mml_tree/mml_nodes/math.ixx
    src/math/core/mml_tree/mml_nodes/mi.ixx
    src/math/core/mml_tree/mml_nodes/mn.ixx
    src/math/core/mml_tree/mml_nodes/mtext.ixx
    src/math/core/mml_tree/mml_nodes/ms.ixx
    src/math/core/mml_tree/mml_nodes/mspace.ixx
    src/math/core/mml_tree/mml_nodes/mstyle.ixx
    src/math/core/mml_tree/mml_nodes/merror.ixx
    src/math/core/mml_tree/mml_nodes/mphantom.ixx
    src/math/util/constants.ixx
    src/math/util/numeric.ixx
    src/math/util/string.ixx
    src/math/util/bbox.ixx
    src/math/util/prioritized_list.ixx
    src/math/util/prioritized_list_context_array.ixx
    src/math/util/prioritized_list_bindings.ixx
    src/math/util/prioritized_list_source_operations.ixx
    src/math/util/ordered_map_storage.ixx
    src/math/util/property_host.ixx
    src/math/util/promise.ixx
    src/math/util/promise_context_bindings.ixx
    src/math/util/function_types.ixx
    src/math/util/function_list.ixx
    src/math/util/function_list_bindings.ixx
    src/math/util/option_functions.ixx
    src/math/util/context_property_bindings.ixx
    src/math/util/styles.ixx
    src/math/util/style_json.ixx
    src/math/util/native_data_fields.ixx
    src/math/output/common/direction.ixx
    src/math/output/common/font_data.ixx
    src/math/output/svg/font_data.ixx
    src/math/fonts/newcm_font_data.ixx
    src/math/fonts/svg_extensions.ixx
    src/math/util/native_identities.ixx
    src/math/util/js_number.ixx
    src/math/util/js_numeric_input.ixx
    src/math/util/js_case.ixx
    src/math/util/utf16_types.ixx
    src/math/util/utf16.ixx
    src/math/util/lengths.ixx
    src/math/util/option_types.ixx
    src/math/util/options.ixx)
  target_sources(folia_math PRIVATE
    src/math/output/common/font_data.cpp
    src/math/output/svg/font_data.cpp
    src/math/fonts/newcm_font_data.cpp
    src/math/fonts/svg_extensions.cpp
    src/math/core/math_item.cpp
    src/math/core/math_location.cpp
    src/math/core/math_list.cpp
    src/math/core/output_jax.cpp
    src/math/core/math_item_state.cpp
    src/math/core/math_item_bindings.cpp
    src/math/core/input_jax_constructor.cpp
    src/math/core/input_jax.cpp
    src/math/core/find_math.cpp
    src/math/input/tex/find_tex.cpp
    src/math/util/regexp_host.cpp
    src/math/input/tex/token.cpp
    src/math/input/tex/node_util.cpp
    src/math/input/tex/node_factory.cpp
    src/math/input/tex/node_factory_bindings.cpp
    src/math/input/tex/stack_item.cpp
    src/math/input/tex/base_item.cpp
    src/math/input/tex/base_item_bindings.cpp
    src/math/input/tex/stack_item_factory_bindings.cpp
    src/math/input/tex/stack_bindings.cpp
    src/math/input/tex/stack.cpp
    src/math/input/tex/tex_error.cpp
    src/math/util/numeric.cpp
    src/math/input/tex/map_handler.cpp
    src/math/input/tex/token_map.cpp
    src/math/input/tex/macro.cpp
    src/math/core/mml_tree/mml.cpp
    src/math/core/mml_tree/mml_factory.cpp
    src/math/core/mml_tree/mml_nodes/maction_bindings.cpp
    src/math/core/mml_tree/mml_nodes/maction_selection.cpp
    src/math/core/mml_tree/mml_nodes/maction_behavior.cpp
    src/math/util/js_case.cpp
    src/math/util/promise.cpp
    src/math/util/function_list.cpp
    src/math/util/function_list_bindings.cpp
    src/math/util/prioritized_list_source_operations.cpp
    src/math/util/argument_expansion.cpp
    src/math/input/tex/parse_callable.cpp
    src/math/input/tex/unit_util.cpp
    src/math/input/tex/unit_util_bindings.cpp
    src/math/util/unicode_normalization.cpp
    src/math/input/tex/tags.cpp
    src/math/util/js_case_data.cpp
    src/math/core/mml_tree/mml_nodes/html_node_bindings.cpp
    src/math/core/mml_tree/mml_nodes/html_node.cpp
    src/math/core/mml_tree/operator_class.cpp
    src/math/core/mml_tree/mml_nodes/tex_atom_bindings.cpp
    src/math/core/mml_tree/mml_nodes/tex_atom.cpp
    src/math/core/mml_tree/mml_nodes/semantics_bindings.cpp
    src/math/core/mml_tree/mml_nodes/semantics.cpp
    src/math/core/mml_tree/mml_nodes/menclose_bindings.cpp
    src/math/core/mml_tree/mml_nodes/menclose.cpp
    src/math/core/mml_tree/mml_nodes/mpadded_bindings.cpp
    src/math/core/mml_tree/mml_nodes/mpadded.cpp
    src/math/core/mml_tree/mml_nodes/maligngroup_bindings.cpp
    src/math/core/mml_tree/mml_nodes/maligngroup.cpp
    src/math/core/mml_tree/mml_nodes/malignmark_bindings.cpp
    src/math/core/mml_tree/mml_nodes/mathchoice_bindings.cpp
    src/math/core/mml_tree/mml_nodes/mathchoice.cpp
    src/math/core/mml_tree/mml_nodes/mglyph_bindings.cpp
    src/math/core/mml_tree/mml_nodes/mglyph.cpp
    src/math/core/mml_tree/mml_nodes/mfenced_bindings.cpp
    src/math/core/mml_tree/mml_nodes/mfenced.cpp
    src/math/core/mml_tree/mml_nodes/mmultiscripts_bindings.cpp
    src/math/core/mml_tree/mml_nodes/mmultiscripts.cpp
    src/math/core/mml_tree/mml_nodes/mtable_bindings.cpp
    src/math/core/mml_tree/mml_nodes/mtable.cpp
    src/math/core/mml_tree/mml_nodes/mtr_bindings.cpp
    src/math/core/mml_tree/mml_nodes/mtr.cpp
    src/math/core/mml_tree/mml_nodes/mtd_bindings.cpp
    src/math/core/mml_tree/mml_nodes/mtd.cpp
    src/math/core/mml_tree/mml_nodes/mfrac_bindings.cpp
    src/math/core/mml_tree/mml_nodes/mfrac.cpp
    src/math/core/mml_tree/mml_nodes/msqrt_bindings.cpp
    src/math/core/mml_tree/mml_nodes/msqrt.cpp
    src/math/core/mml_tree/mml_nodes/mroot_bindings.cpp
    src/math/core/mml_tree/mml_nodes/mroot.cpp
    src/math/core/mml_tree/mo_pattern_data.cpp
    src/math/core/mml_tree/mo_patterns.cpp
    src/math/core/mml_tree/mml_nodes/mo_bindings.cpp
    src/math/core/mml_tree/mml_nodes/mo_lookup.cpp
    src/math/core/mml_tree/mml_nodes/mo_classes.cpp
    src/math/core/mml_tree/mml_nodes/mo_scripts.cpp
    src/math/core/mml_tree/operator_dictionary_data.cpp
    src/math/core/mml_tree/operator_dictionary.cpp
    src/math/util/property_host.cpp
    src/math/core/tree/property_array.cpp
    src/math/util/function_value.cpp
    src/math/util/string.cpp
    src/math/core/mml_tree/mml_nodes/msubsup_bindings.cpp
    src/math/core/mml_tree/mml_nodes/msubsup.cpp
    src/math/core/mml_tree/mml_nodes/munderover_bindings.cpp
    src/math/core/mml_tree/mml_nodes/munderover.cpp
    src/math/core/mml_tree/mml_bindings.cpp
    src/math/core/mml_tree/mml_node.cpp
    src/math/core/mml_tree/mml_branches.cpp
    src/math/core/mml_tree/mml_empty.cpp
    src/math/core/mml_tree/mml_nodes/mrow_bindings.cpp
    src/math/core/mml_tree/mml_nodes/mrow.cpp
    src/math/core/mml_tree/mml_nodes/math_bindings.cpp
    src/math/core/mml_tree/mml_nodes/math.cpp
    src/math/core/mml_tree/mml_nodes/mi_bindings.cpp
    src/math/core/mml_tree/mml_nodes/mi.cpp
    src/math/core/mml_tree/mml_nodes/mn_bindings.cpp
    src/math/core/mml_tree/mml_nodes/mn.cpp
    src/math/core/mml_tree/mml_nodes/mtext_bindings.cpp
    src/math/core/mml_tree/mml_nodes/mtext.cpp
    src/math/core/mml_tree/mml_nodes/ms_bindings.cpp
    src/math/core/mml_tree/mml_nodes/ms.cpp
    src/math/core/mml_tree/mml_nodes/mspace_bindings.cpp
    src/math/core/mml_tree/mml_nodes/mspace.cpp
    src/math/core/mml_tree/mml_nodes/mstyle_bindings.cpp
    src/math/core/mml_tree/mml_nodes/mstyle.cpp
    src/math/core/mml_tree/mml_nodes/merror_bindings.cpp
    src/math/core/mml_tree/mml_nodes/merror.cpp
    src/math/core/mml_tree/mml_nodes/mphantom_bindings.cpp
    src/math/core/mml_tree/mml_nodes/mphantom.cpp
    src/math/util/option_object.cpp
    src/math/util/option_array.cpp)
target_sources(folia_math PRIVATE
    src/math/util/context_property_bindings.cpp
    src/math/util/styles.cpp
    src/math/util/style_json.cpp
    src/math/util/option_array_iteration.cpp
    src/math/util/option_iterator.cpp
    src/math/input/tex/configuration.cpp
    src/math/input/tex/configuration_prioritized_list_projection.cpp
    src/math/input/tex/column_parser.cpp
    src/math/input/tex/parser_runtime.cpp
    src/math/input/tex/stack_array_access.cpp
    src/math/input/tex/stack_context_bindings.cpp
    src/math/input/tex/tex_parser_context_fields.cpp
    src/math/input/tex/parse_options.cpp
    src/math/input/tex/tex_parser.cpp
    src/math/input/tex/tex_jax.cpp
    src/math/input/tex/base_items.cpp
    src/math/input/tex/stack_item_factory_context.cpp
    src/math/input/tex/base/base_methods.cpp
    src/math/input/tex/base/base_parse_methods.cpp
    src/math/input/tex/base/base_mappings.cpp
    src/math/input/tex/base/base_configuration.cpp
    src/math/input/tex/ams/items.cpp
    src/math/input/tex/ams/methods.cpp
    src/math/input/tex/ams/mappings.cpp
    src/math/input/tex/ams/configuration.cpp
    src/math/input/tex/newcommand/items.cpp
    src/math/input/tex/newcommand/util.cpp
    src/math/input/tex/newcommand/methods.cpp
    src/math/input/tex/newcommand/mappings.cpp
    src/math/input/tex/newcommand/configuration.cpp
    src/math/input/tex/begingroup/stack.cpp
    src/math/input/tex/begingroup/methods.cpp
    src/math/input/tex/begingroup/configuration.cpp
    src/math/input/tex/action/configuration.cpp
    src/math/input/tex/bbox/configuration.cpp
    src/math/input/tex/braket/items.cpp
    src/math/input/tex/braket/methods.cpp
    src/math/input/tex/braket/mappings.cpp
    src/math/input/tex/braket/configuration.cpp
    src/math/input/tex/cancel/configuration.cpp
    src/math/input/tex/color/constants.cpp
    src/math/input/tex/color/util.cpp
    src/math/input/tex/color/methods.cpp
    src/math/input/tex/color/configuration.cpp
    src/math/input/tex/enclose/configuration.cpp
    src/math/input/tex/noundefined/configuration.cpp
    src/math/input/tex/verb/configuration.cpp
    src/math/input/tex/unicode/configuration.cpp
    src/math/input/tex/upgreek/configuration.cpp
    src/math/input/tex/gensymb/configuration.cpp
    src/math/input/tex/fontsizev3/configuration.cpp
    src/math/input/tex/extpfeil/configuration.cpp
    src/math/input/tex/bbm/configuration.cpp
    src/math/input/tex/bboldx/methods.cpp
    src/math/input/tex/bboldx/mappings.cpp
    src/math/input/tex/bboldx/configuration.cpp
    src/math/input/tex/dsfont/configuration.cpp
    src/math/input/tex/boldsymbol/configuration.cpp
    src/math/input/tex/configmacros/configuration.cpp
    src/math/input/tex/text_parser.cpp
    src/math/input/tex/textmacros_methods.cpp
    src/math/input/tex/textmacros_mappings.cpp
    src/math/input/tex/textmacros_configuration.cpp
    src/math/util/retries.cpp
    src/math/input/tex/parse_util.cpp)

# First whole native output closure (compile checkpoint; parity remains open).
target_sources(folia_math PUBLIC FILE_SET CXX_MODULES FILES
    src/math/adaptors/lite/document.ixx
    src/math/adaptors/lite/element.ixx
    src/math/adaptors/lite/list.ixx
    src/math/adaptors/lite/lite_module.ixx
    src/math/adaptors/lite/node_store.ixx
    src/math/adaptors/lite/parser.ixx
    src/math/adaptors/lite/text.ixx
    src/math/adaptors/lite/window.ixx
    src/math/adaptors/lite_adaptor.ixx
    src/math/core/dom_adaptor.ixx
    src/math/core/tree/visitor.ixx
    src/math/output/common/common.ixx
    src/math/output/common/common_bindings.ixx
    src/math/output/common/factory_context_fields.ixx
    src/math/output/common/font_data_bindings.ixx
    src/math/output/common/linebreak_visitor.ixx
    src/math/output/common/line_bbox.ixx
    src/math/output/common/output_jax.ixx
    src/math/output/common/types.ixx
    src/math/output/common/wrapper.ixx
    src/math/output/common/wrappers/maction.ixx
    src/math/output/common/wrappers/math.ixx
    src/math/output/common/wrappers/mfenced.ixx
    src/math/output/common/wrappers/mfrac.ixx
    src/math/output/common/wrappers/mglyph.ixx
    src/math/output/common/wrappers/mi.ixx
    src/math/output/common/wrappers/mn.ixx
    src/math/output/common/wrappers/mo.ixx
    src/math/output/common/wrappers/mroot.ixx
    src/math/output/common/wrappers/mrow.ixx
    src/math/output/common/wrappers/ms.ixx
    src/math/output/common/wrappers/mspace.ixx
    src/math/output/common/wrappers/msqrt.ixx
    src/math/output/common/wrappers/mtext.ixx
    src/math/output/common/wrappers/mtr.ixx
    src/math/output/common/wrappers/semantics.ixx
    src/math/output/common/wrappers/text_node.ixx
    src/math/output/common/wrappers/tex_atom.ixx
    src/math/output/common/wrapper_factory.ixx
    src/math/output/scene/builder.ixx
    src/math/output/scene/glyph_paint.ixx
    src/math/output/scene/html_types.ixx
    src/math/output/scene/html_snapshot.ixx
    src/math/output/scene/xml_node.ixx
    src/math/output/scene/transform_types.ixx
    src/math/output/scene/script_support.ixx
    src/math/output/scene/scriptbase.ixx
    src/math/output/scene/msubsup.ixx
    src/math/output/scene/munderover.ixx
    src/math/output/scene/mmultiscripts.ixx
    src/math/output/scene/placements.ixx
    src/math/output/scene/menclose.ixx
    src/math/output/scene/scene.ixx
    src/math/output/scene/svg_paint.ixx
    src/math/output/scene/types.ixx
    src/math/output/scene/wrapper.ixx
    src/math/output/scene/wrappers/inherited_svg_wrapper.ixx
    src/math/output/scene/wrappers/maction.ixx
    src/math/output/scene/wrappers/math.ixx
    src/math/output/scene/wrappers/mfenced.ixx
    src/math/output/scene/wrappers/mfrac.ixx
    src/math/output/scene/wrappers/mglyph.ixx
    src/math/output/scene/wrappers/mo.ixx
    src/math/output/scene/wrappers/mphantom.ixx
    src/math/output/scene/wrappers/mroot.ixx
    src/math/output/scene/wrappers/mrow.ixx
    src/math/output/scene/wrappers/msqrt.ixx
    src/math/output/scene/wrappers/semantics.ixx
    src/math/output/scene/wrappers/text_node.ixx
    src/math/output/scene/wrappers/tex_atom.ixx
    src/math/output/svg/font_cache.ixx
    src/math/output/svg/output_jax.ixx
    src/math/output/svg/serializer.ixx
    src/math/util/entities.ixx)
target_sources(folia_math PRIVATE
    src/math/adaptors/lite/document.cpp
    src/math/adaptors/lite/element.cpp
    src/math/adaptors/lite/list.cpp
    src/math/adaptors/lite/lite_module.cpp
    src/math/adaptors/lite/node_store.cpp
    src/math/adaptors/lite/parser.cpp
    src/math/adaptors/lite/text.cpp
    src/math/adaptors/lite/window.cpp
    src/math/adaptors/lite_adaptor.cpp
    src/math/core/dom_adaptor.cpp
    src/math/output/common/common_bindings.cpp
    src/math/output/common/font_data_bindings.cpp
    src/math/output/common/linebreak_visitor.cpp
    src/math/output/common/line_bbox.cpp
    src/math/output/common/output_jax.cpp
    src/math/output/common/wrapper.cpp
    src/math/output/common/wrappers/maction.cpp
    src/math/output/common/wrappers/math.cpp
    src/math/output/common/wrappers/mfenced.cpp
    src/math/output/common/wrappers/mfrac.cpp
    src/math/output/common/wrappers/mglyph.cpp
    src/math/output/common/wrappers/mi.cpp
    src/math/output/common/wrappers/mn.cpp
    src/math/output/common/wrappers/mo.cpp
    src/math/output/common/wrappers/mroot.cpp
    src/math/output/common/wrappers/mrow.cpp
    src/math/output/common/wrappers/ms.cpp
    src/math/output/common/wrappers/mspace.cpp
    src/math/output/common/wrappers/msqrt.cpp
    src/math/output/common/wrappers/mtext.cpp
    src/math/output/common/wrappers/mtr.cpp
    src/math/output/common/wrappers/semantics.cpp
    src/math/output/common/wrappers/text_node.cpp
    src/math/output/common/wrappers/tex_atom.cpp
    src/math/output/common/wrapper_factory.cpp
    src/math/output/scene/builder.cpp
    src/math/output/scene/glyph_outline.cpp
    src/math/output/scene/html_snapshot.cpp
    src/math/output/scene/xml_node.cpp
    src/math/output/scene/script_support.cpp
    src/math/output/scene/scriptbase.cpp
    src/math/output/scene/msubsup.cpp
    src/math/output/scene/munderover.cpp
    src/math/output/scene/mmultiscripts.cpp
    src/math/output/scene/glyph_paint.cpp
    src/math/output/scene/placements.cpp
    src/math/output/scene/menclose.cpp
    src/math/output/scene/scene.cpp
    src/math/output/scene/svg_paint.cpp
    src/math/output/scene/wrapper.cpp
    src/math/output/scene/wrappers/inherited_svg_wrapper.cpp
    src/math/output/scene/wrappers/maction.cpp
    src/math/output/scene/wrappers/math.cpp
    src/math/output/scene/wrappers/mfenced.cpp
    src/math/output/scene/wrappers/mfrac.cpp
    src/math/output/scene/wrappers/mglyph.cpp
    src/math/output/scene/wrappers/mo.cpp
    src/math/output/scene/wrappers/mphantom.cpp
    src/math/output/scene/wrappers/mroot.cpp
    src/math/output/scene/wrappers/mrow.cpp
    src/math/output/scene/wrappers/msqrt.cpp
    src/math/output/scene/wrappers/semantics.cpp
    src/math/output/scene/wrappers/text_node.cpp
    src/math/output/scene/wrappers/tex_atom.cpp
    src/math/output/svg/font_cache.cpp
    src/math/output/svg/output_jax.cpp
    src/math/output/svg/serializer.cpp
    src/math/util/entities.cpp)

# Additional native Source wrappers and SVG constructor binding.
target_sources(folia_math PUBLIC FILE_SET CXX_MODULES FILES
    src/math/output/common/wrappers/mmultiscripts.ixx
    src/math/output/common/wrappers/msubsup.ixx
    src/math/output/common/wrappers/munderover.ixx
    src/math/output/common/wrappers/scriptbase.ixx
    src/math/output/common/wrapper_class_bindings.ixx
    src/math/output/svg/output_jax_bindings.ixx
    src/math/output/svg/wrapper_factory.ixx
    src/math/output/svg/wrapper_factory_bindings.ixx)
target_sources(folia_math PRIVATE
    src/math/output/common/wrappers/mmultiscripts.cpp
    src/math/output/common/wrappers/msubsup.cpp
    src/math/output/common/wrappers/munderover.cpp
    src/math/output/common/wrappers/scriptbase.cpp
    src/math/output/svg/output_jax_bindings.cpp
    src/math/output/svg/wrapper_factory.cpp
    src/math/output/svg/wrapper_factory_bindings.cpp)

# Required Source merror placement.
target_sources(folia_math PUBLIC FILE_SET CXX_MODULES FILES
    src/math/output/scene/wrappers/merror.ixx)
target_sources(folia_math PRIVATE
    src/math/output/scene/wrappers/merror.cpp)

# Constructor phases and canonical Map compiler packet.
target_sources(folia_math PUBLIC FILE_SET CXX_MODULES FILES
    src/math/output/common/wrappers/mpadded.ixx
    src/math/output/common/wrappers/mtable.ixx
    src/math/output/common/wrappers/mtd.ixx
    src/math/output/common/wrappers/inferred_mrow.ixx
    src/math/output/common/wrappers/xml_node.ixx
    src/math/output/svg/wrapper_classes.ixx
    src/math/output/svg/wrapper_constructors.ixx
    src/math/util/map_bindings.ixx
    src/math/util/factory_context_field_views.ixx
    src/math/util/factory_context_source_map_access.ixx)
target_sources(folia_math PUBLIC FILE_SET CXX_MODULES FILES
    src/math/input/tex/token_map_context_bindings.ixx
    src/math/input/tex/handler_context_bindings.ixx
    src/math/core/mml_tree/attributes_bindings.ixx
    src/math/core/mml_tree/mml_node_context_fields.ixx
    src/math/core/mml_tree/mml_factory_context_values.ixx
    src/math/core/mml_tree/mml_factory_context_fields.ixx
    src/math/core/mml_tree/mml_source_allocator.ixx
    src/math/core/mml_tree/mml_factory_context_methods.ixx
    src/math/core/mml_tree/mml_node_context_module.ixx
    src/math/core/mml_tree/mml_node_context_methods.ixx)
target_sources(folia_math PUBLIC FILE_SET CXX_MODULES FILES
    src/math/core/tree/node_algorithms.ixx
    src/math/core/tree/node_context_bindings.ixx
    src/math/core/tree/node_context_classes.ixx)
target_sources(folia_math PRIVATE
    src/math/util/prioritized_list_bindings.cpp
    src/math/input/tex/token_map_context_bindings.cpp
    src/math/input/tex/handler_context_bindings.cpp
    src/math/core/mml_tree/attributes_bindings.cpp
    src/math/core/mml_tree/mml_node_context_fields.cpp
    src/math/core/mml_tree/mml_factory_context_values.cpp
    src/math/core/mml_tree/mml_factory_context_fields.cpp
    src/math/core/mml_tree/mml_source_allocator.cpp
    src/math/core/mml_tree/mml_factory_context_methods.cpp
    src/math/core/mml_tree/mml_node_context_module.cpp
    src/math/core/mml_tree/mml_node_context_methods.cpp
    src/math/core/tree/node_context_classes.cpp)
target_sources(folia_math PUBLIC FILE_SET CXX_MODULES FILES
    src/math/core/mml_tree/xml_dom_adaptor.ixx)
target_sources(folia_math PRIVATE
    src/math/core/mml_tree/xml_dom_adaptor.cpp)
target_sources(folia_math PUBLIC FILE_SET CXX_MODULES FILES
    src/math/adaptors/lite/source_text_content.ixx)
target_sources(folia_math PRIVATE
    src/math/adaptors/lite/source_text_content.cpp)
target_sources(folia_math PRIVATE
    src/math/output/common/wrappers/mpadded.cpp
    src/math/output/common/wrappers/mtable.cpp
    src/math/output/common/wrappers/mtd.cpp
    src/math/output/common/wrappers/inferred_mrow.cpp
    src/math/output/common/wrappers/xml_node.cpp
    src/math/output/svg/wrapper_classes.cpp
    src/math/output/svg/wrapper_constructors.cpp
    src/math/util/map_bindings.cpp)

# Concrete native SVG service graph and its canonical Source projections.
target_sources(folia_math PUBLIC FILE_SET CXX_MODULES FILES
    src/math/core/math_render_actions.ixx
    src/math/output/scene/foundation.ixx
    src/math/output/scene/menclose_source.ixx
    src/math/output/common/bbox_bindings.ixx
    src/math/output/common/maction_source.ixx
    src/math/output/common/font_data_options.ixx
    src/math/output/common/font_data_surface.ixx
    src/math/output/common/wrappers/menclose.ixx
    src/math/output/svg/font_data_surface.ixx
    src/math/output/svg/menclose_surface.ixx
    src/math/output/svg/maction_surface.ixx
    src/math/output/svg/scene_context_nodes.ixx
    src/math/output/svg/timer_host.ixx
    src/math/output/svg/render_session.ixx
    src/math/output/svg/wrapper_source_methods.ixx
    src/math/fonts/newcm_font_data_surface.ixx)
target_sources(folia_math PUBLIC FILE_SET CXX_MODULES FILES
    src/math/fonts/newcm_font_module_provider.ixx)
target_sources(folia_math PRIVATE
    src/math/core/math_render_actions.cpp
    src/math/output/common/bbox_bindings.cpp
    src/math/output/common/maction_source.cpp
    src/math/output/common/font_data_options.cpp
    src/math/output/common/font_data_surface.cpp
    src/math/output/common/wrappers/menclose.cpp
    src/math/output/svg/font_data_surface.cpp
    src/math/output/svg/menclose_surface.cpp
    src/math/output/svg/maction_surface.cpp
    src/math/output/svg/scene_context_nodes.cpp
    src/math/output/svg/timer_host.cpp
    src/math/output/svg/render_session.cpp
    src/math/output/svg/wrapper_source_methods.cpp
    src/math/fonts/newcm_font_data_surface.cpp)
target_sources(folia_math PRIVATE
    src/math/fonts/newcm_font_module_provider.cpp)

# Source document lifecycle: generic Core, HTML ownership, then SVG assembly.
target_sources(folia_math PUBLIC FILE_SET CXX_MODULES FILES
    src/math/output/svg/wrappers/math.ixx)
target_sources(folia_math PRIVATE
    src/math/output/svg/wrappers/math.cpp)

target_sources(folia_math PUBLIC FILE_SET CXX_MODULES FILES
    src/math/core/math_document.ixx
    src/math/handlers/html/html_math_document.ixx
    src/math/handlers/html/html_math_item.ixx
    src/math/output/svg/native_document.ixx
    src/math/output/svg/xml_wrapper.ixx)
target_sources(folia_math PRIVATE
    src/math/core/math_document.cpp
    src/math/handlers/html/html_math_document.cpp
    src/math/handlers/html/html_math_item.cpp
    src/math/output/svg/native_document.cpp
    src/math/output/svg/xml_wrapper.cpp)

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

# Focused tests link the actual math library without importing unfinished
# parser peers through unrelated tests. This is an additional host gate; the
# complete core/parser acceptance remains FoliaTests plus original comparison.
add_executable(FoliaMathHostTests EXCLUDE_FROM_ALL
    tests/math/context_object_record_retirement.cpp
    tests/math/native_identity_request_capture.cpp
    tests/math/mml_context_allocator_ownership.cpp
    tests/math/native_identities_host.cpp
    tests/math/reference_native_identities.cpp
    tests/math/factory_product_mark.cpp
    tools/math-port/drafts/token_map_context_bindings.cpp
    tests/math/tags_class_binding.cpp
    tests/math/tags_info_request_retirement.cpp
    tests/math/math_cancellation_g147.cpp
    tests/math/context_raw_edge_trace.cpp
    tests/math/token_map_raw_edge_trace.cpp
    tests/math/tags_raw_edge_trace.cpp
    tests/math/parse_symbol_binding.cpp
    tests/math/mml_constructor_identity.cpp
    tests/math/mml_factory_context_fields.cpp
    tests/math/mml_token_walk_tree.cpp
    tests/math/mml_shared_context_module.cpp
    tests/math/mml_foreign_constructor.cpp
    tests/math/mml_tree_source_child_owner.cpp
    tests/math/reference_mo_order.cpp
    tests/math/core_tree_node_context_classes.cpp
    tests/math/tree_ordinary_call.cpp
    tests/math/factory_context_bindings.cpp
    tests/math/native_data_fields.cpp
    tests/math/context_property_bindings.cpp
    tests/math/common_factory_live_fields.cpp
    tests/math/common_factory_context_methods.cpp
    tests/math/factory_context_field_views.cpp
    tests/math/factory_creator_cells.cpp
    tests/math/prioritized_list_source_operations.cpp
    tests/math/parser_configuration_prioritized_list_projection.cpp
    tests/math/parser_configuration_prioritized_list_live_array.cpp
    tests/math/option_array_concat.cpp
    tests/main.cpp
    tests/math/reference_option_array_iteration.cpp
    tests/math/reference_option_array_map.cpp
    tools/math-port/parser-array-host-reference.cpp
    tools/math-port/parser-iterator-reference.cpp)
target_compile_features(FoliaMathHostTests PRIVATE cxx_std_23)
set_target_properties(FoliaMathHostTests PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_include_directories(FoliaMathHostTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(FoliaMathHostTests PRIVATE folia_math Boost::ut_module)

add_executable(FoliaMathPromiseOwnerTests EXCLUDE_FROM_ALL
    tests/main.cpp
    tests/math/promise_context_bindings_owner.cpp
    tests/math/parser_runtime_promise_owner.cpp)
target_compile_features(FoliaMathPromiseOwnerTests PRIVATE cxx_std_23)
set_target_properties(FoliaMathPromiseOwnerTests PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_include_directories(FoliaMathPromiseOwnerTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(FoliaMathPromiseOwnerTests PRIVATE folia_math Boost::ut_module)

add_executable(FoliaMathRetryTests EXCLUDE_FROM_ALL
    tests/main.cpp
    tools/math-port/parser-family-reference/retries-native-candidate.cpp)
target_compile_features(FoliaMathRetryTests PRIVATE cxx_std_23)
set_target_properties(FoliaMathRetryTests PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_include_directories(FoliaMathRetryTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(FoliaMathRetryTests PRIVATE folia_math Boost::ut_module)

add_executable(FoliaMathFontTests EXCLUDE_FROM_ALL
    tests/main.cpp
    tests/math/font_data_source.cpp
    tools/math-port/newcm-font-module-provider-native-tests.cpp)
target_compile_features(FoliaMathFontTests PRIVATE cxx_std_23)
set_target_properties(FoliaMathFontTests PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_include_directories(FoliaMathFontTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(FoliaMathFontTests PRIVATE folia_math Boost::ut_module)

add_executable(FoliaMathFontDump EXCLUDE_FROM_ALL
    tools/math-port/font-data-native-candidate.cpp)
target_compile_features(FoliaMathFontDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathFontDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathFontDump PRIVATE folia_math)

add_executable(FoliaMathFontLifecycleDump EXCLUDE_FROM_ALL
    tools/math-port/font-dynamic-lifecycle-native-candidate.cpp)
target_compile_features(FoliaMathFontLifecycleDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathFontLifecycleDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathFontLifecycleDump PRIVATE folia_math)

add_executable(FoliaMathFontMethodsDump EXCLUDE_FROM_ALL
    tools/math-port/font-methods-native-candidate.cpp)
target_compile_features(FoliaMathFontMethodsDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathFontMethodsDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathFontMethodsDump PRIVATE folia_math)

add_executable(FoliaMathFontTupleDump EXCLUDE_FROM_ALL
    tools/math-port/font-tuple-presence-native-candidate.cpp)
target_compile_features(FoliaMathFontTupleDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathFontTupleDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathFontTupleDump PRIVATE folia_math)

add_executable(FoliaMathFontModuleDump EXCLUDE_FROM_ALL
    tools/math-port/newcm-font-module-provider-native-candidate.cpp)
target_compile_features(FoliaMathFontModuleDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathFontModuleDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathFontModuleDump PRIVATE folia_math)

add_executable(FoliaMathFontInstanceExtensionDump EXCLUDE_FROM_ALL
    tools/math-port/font-instance-extension-native-candidate.cpp)
target_compile_features(FoliaMathFontInstanceExtensionDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathFontInstanceExtensionDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathFontInstanceExtensionDump PRIVATE folia_math)

add_executable(FoliaMathFontDynamicFileRawDump EXCLUDE_FROM_ALL
    tools/math-port/font-dynamic-file-raw-native-candidate.cpp)
target_compile_features(FoliaMathFontDynamicFileRawDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathFontDynamicFileRawDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathFontDynamicFileRawDump PRIVATE folia_math)

add_executable(FoliaMathNativeDocumentProbe EXCLUDE_FROM_ALL
    tools/math-port/native-document-probe.cpp)
target_compile_features(FoliaMathNativeDocumentProbe PRIVATE cxx_std_23)
set_target_properties(FoliaMathNativeDocumentProbe PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathNativeDocumentProbe PRIVATE folia_math)

add_executable(FoliaMathMencloseProbe EXCLUDE_FROM_ALL
    tools/math-port/menclose-native-probe.cpp)
target_compile_features(FoliaMathMencloseProbe PRIVATE cxx_std_23)
set_target_properties(FoliaMathMencloseProbe PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathMencloseProbe PRIVATE folia_math)

add_executable(FoliaMathCommonLayoutDump EXCLUDE_FROM_ALL
    tools/math-port/common-layout-native-dump.cpp
    tools/math-port/common-layout-native-observer.cpp)
target_sources(FoliaMathCommonLayoutDump PRIVATE FILE_SET CXX_MODULES FILES
    tools/math-port/common-layout-native-observer.ixx)
target_compile_features(FoliaMathCommonLayoutDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathCommonLayoutDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathCommonLayoutDump PRIVATE folia_math)

# Source package closure and table/padded Scene providers. The new package
# corpus exercises these providers before production-session registration.
target_sources(folia_math PUBLIC FILE_SET CXX_MODULES FILES
    src/math/input/tex/amscd/configuration.ixx
    src/math/input/tex/amscd/mappings.ixx
    src/math/input/tex/amscd/methods.ixx
    src/math/input/tex/cases/configuration.ixx
    src/math/input/tex/cases/items.ixx
    src/math/input/tex/cases/methods.ixx
    src/math/input/tex/cases/tags.ixx
    src/math/input/tex/centernot/configuration.ixx
    src/math/input/tex/empheq/configuration.ixx
    src/math/input/tex/empheq/util.ixx
    src/math/input/tex/html/configuration.ixx
    src/math/input/tex/html/methods.ixx
    src/math/input/tex/textcomp/configuration.ixx
    src/math/input/tex/textcomp/mappings.ixx
    src/math/input/tex/textcomp/methods.ixx
    src/math/input/tex/units/configuration.ixx
    src/math/output/scene/wrappers/mpadded.ixx
    src/math/output/scene/wrappers/mtd.ixx
    src/math/output/scene/wrappers/mtr.ixx
    src/math/output/scene/wrappers/mtable.ixx)
target_sources(folia_math PRIVATE
    src/math/input/tex/amscd/configuration.cpp
    src/math/input/tex/amscd/mappings.cpp
    src/math/input/tex/amscd/methods.cpp
    src/math/input/tex/cases/configuration.cpp
    src/math/input/tex/cases/items.cpp
    src/math/input/tex/cases/methods.cpp
    src/math/input/tex/cases/tags.cpp
    src/math/input/tex/centernot/configuration.cpp
    src/math/input/tex/empheq/configuration.cpp
    src/math/input/tex/empheq/util.cpp
    src/math/input/tex/html/configuration.cpp
    src/math/input/tex/html/methods.cpp
    src/math/input/tex/textcomp/configuration.cpp
    src/math/input/tex/textcomp/mappings.cpp
    src/math/input/tex/textcomp/methods.cpp
    src/math/input/tex/units/configuration.cpp
    src/math/output/scene/wrappers/mpadded.cpp
    src/math/output/scene/wrappers/mtd.cpp
    src/math/output/scene/wrappers/mtr.cpp
    src/math/output/scene/wrappers/mtable.cpp)

# Public entry point delegates to the same native document and Scene pipeline.
target_sources(folia_math PUBLIC FILE_SET CXX_MODULES FILES
    src/math/math.ixx
    src/math/cancellation.ixx
    src/math/input/tex/physics/items.ixx
    src/math/input/tex/physics/methods.ixx
    src/math/input/tex/physics/mappings.ixx
    src/math/input/tex/physics/configuration.ixx
    src/math/input/tex/colortbl/items.ixx
    src/math/input/tex/colortbl/methods.ixx
    src/math/input/tex/colortbl/configuration.ixx)
target_sources(folia_math PRIVATE
    src/math/math.cpp
    src/math/cancellation.cpp
    src/math/input/tex/physics/items.cpp
    src/math/input/tex/physics/methods.cpp
    src/math/input/tex/physics/mappings.cpp
    src/math/input/tex/physics/configuration.cpp
    src/math/input/tex/colortbl/items.cpp
    src/math/input/tex/colortbl/methods.cpp
    src/math/input/tex/colortbl/configuration.cpp)

add_executable(FoliaMathApiDump EXCLUDE_FROM_ALL
    tools/math-port/math-api-native-candidate.cpp)
target_compile_features(FoliaMathApiDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathApiDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathApiDump PRIVATE folia_math)

add_executable(FoliaMathPhysicsDump EXCLUDE_FROM_ALL
    tools/math-port/physics-native-candidate.cpp)
target_compile_features(FoliaMathPhysicsDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathPhysicsDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathPhysicsDump PRIVATE folia_math)

add_executable(FoliaMathColortblDump EXCLUDE_FROM_ALL
    tools/math-port/colortbl-native-candidate.cpp)
target_compile_features(FoliaMathColortblDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathColortblDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathColortblDump PRIVATE folia_math)

# Complete package implementations are compiled before production registration.
target_sources(folia_math PUBLIC FILE_SET CXX_MODULES FILES
    src/math/dependencies/mhchem_parser/parser.ixx
    src/math/input/tex/mhchem/configuration.ixx)
target_sources(folia_math PRIVATE
    src/math/dependencies/mhchem_parser/parser.cpp
    src/math/dependencies/mhchem_parser/patterns.cpp
    src/math/dependencies/mhchem_parser/actions.cpp
    src/math/dependencies/mhchem_parser/texify.cpp
    src/math/input/tex/mhchem/configuration.cpp)

function(folia_add_math_probe target source)
    add_executable(${target} EXCLUDE_FROM_ALL ${source})
    target_compile_features(${target} PRIVATE cxx_std_23)
    set_target_properties(${target} PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
    target_link_libraries(${target} PRIVATE folia_math)
endfunction()

folia_add_math_probe(FoliaMathMhchemDependencyDump tools/math-port/drafts/mhchem-native-dependency.cpp)
folia_add_math_probe(FoliaMathMhchemPackageDump tools/math-port/drafts/mhchem-package-native-candidate.cpp)
folia_add_math_probe(FoliaMathUnshiftDump tools/math-port/drafts/array-unshift-native-candidate.cpp)
folia_add_math_probe(FoliaMathGetBBoxDump tools/math-port/drafts/common-getbbox-native-candidate.cpp)
folia_add_math_probe(FoliaMathGetBBoxLifetime tools/math-port/drafts/common-getbbox-lifetime-native-candidate.cpp)
folia_add_math_probe(FoliaMathTextTipDump tools/math-port/drafts/maction-texttip-native-candidate.cpp)
folia_add_math_probe(FoliaMathParserProjectionDump tools/math-port/drafts/parser-projection-native-candidate.cpp)
folia_add_math_probe(FoliaMathAttributesDump tools/math-port/drafts/attributes-bindings-native-probe.cpp)
folia_add_math_probe(FoliaMathHandlerDefaultsDump tools/math-port/drafts/handler-context-defaults-native-candidate.cpp)
folia_add_math_probe(FoliaMathHandlerOwnersDump tools/math-port/drafts/subhandlers-live-owner-native-candidate.cpp)
folia_add_math_probe(FoliaMathBussproofsExpandedDump tools/math-port/drafts/bussproofs-expanded-native-candidate.cpp)
folia_add_math_probe(FoliaMathStackOwnersDump tools/math-port/drafts/bussproofs-stack-packet/bussproofs-session-state-native-candidate.cpp)
folia_add_math_probe(FoliaMathParserFieldsDump tools/math-port/drafts/tex-parser-context-fields-native-candidate.cpp)
folia_add_math_probe(FoliaMathTagDefaultsDump tools/math-port/drafts/tag-setoptions-default-methods-native-witness.cpp)

# Whole native package owners; production session registration follows acceptance.
target_sources(folia_math PUBLIC FILE_SET CXX_MODULES FILES
    src/math/input/tex/bussproofs/items.ixx
    src/math/input/tex/bussproofs/methods.ixx
    src/math/input/tex/bussproofs/mappings.ixx
    src/math/input/tex/bussproofs/configuration.ixx
    src/math/input/tex/bussproofs/util.ixx
    src/math/input/tex/mathtools/items.ixx
    src/math/input/tex/mathtools/methods.ixx
    src/math/input/tex/mathtools/mappings.ixx
    src/math/input/tex/mathtools/tags.ixx
    src/math/input/tex/mathtools/util.ixx
    src/math/input/tex/mathtools/configuration.ixx
    src/math/input/tex/tagformat/tag_format_configuration.ixx
    src/math/input/tex/setoptions/set_options_configuration.ixx
    src/math/input/tex/texhtml/tex_html_services.ixx
    src/math/input/tex/texhtml/tex_html_methods.ixx
    src/math/input/tex/texhtml/tex_html_configuration.ixx
    src/math/handlers/html/html_dom_strings.ixx)
target_sources(folia_math PRIVATE
    src/math/input/tex/bussproofs/items.cpp
    src/math/input/tex/bussproofs/methods.cpp
    src/math/input/tex/bussproofs/mappings.cpp
    src/math/input/tex/bussproofs/configuration.cpp
    src/math/input/tex/bussproofs/util.cpp
    src/math/input/tex/mathtools/items.cpp
    src/math/input/tex/mathtools/methods.cpp
    src/math/input/tex/mathtools/mappings.cpp
    src/math/input/tex/mathtools/tags.cpp
    src/math/input/tex/mathtools/util.cpp
    src/math/input/tex/mathtools/configuration.cpp
    src/math/input/tex/tagformat/tag_format_configuration.cpp
    src/math/input/tex/setoptions/set_options_configuration.cpp
    src/math/input/tex/texhtml/tex_html_services.cpp
    src/math/input/tex/texhtml/tex_html_methods.cpp
    src/math/input/tex/texhtml/tex_html_configuration.cpp
    src/math/handlers/html/html_dom_strings.cpp)
folia_add_math_probe(FoliaMathBussproofsDump tools/math-port/bussproofs-native-candidate.cpp)
folia_add_math_probe(FoliaMathBussproofsUtilDump tools/math-port/bussproofs-util-surface-native-candidate.cpp)
folia_add_math_probe(FoliaMathBussproofsSessions tools/math-port/drafts/bussproofs-session-state-native-test.cpp)
folia_add_math_probe(FoliaMathBussproofsPublicRenderDump tools/math-port/candidates/bussproofs-public-render-g143/bussproofs-public-render-native-candidate.cpp)
folia_add_math_probe(FoliaMathProfile75FullSvgDump tools/math-port/candidates/profile75-full-svg-g147/profile75-full-svg-native-candidate.cpp)
folia_add_math_probe(FoliaMathProfile75SceneMeasurementDump tools/math-port/candidates/shared-scene-producer-g153/profile75-scene-measurement-native-candidate.cpp)
folia_add_math_probe(FoliaMathMathtoolsDump tools/math-port/mathtools-native-candidate.cpp)
folia_add_math_probe(FoliaMathTagSetOptionsDump tools/math-port/drafts/tag-setoptions-native-candidate.cpp)
folia_add_math_probe(FoliaMathMapSessionsDump tools/math-port/drafts/map-handler-session-native-candidate.cpp)
folia_add_math_probe(FoliaMathTexHtmlDump tools/math-port/drafts/texhtml-native-candidate.cpp)
folia_add_math_probe(FoliaMathTexHtmlXmlDump tools/math-port/drafts/texhtml-xml-native-candidate.cpp)
folia_add_math_probe(FoliaMathRendererOwnerCountCandidate tools/math-port/drafts/math-native-session-retirement-next/renderer-owner-counts.cpp)
folia_add_math_probe(FoliaMathRendererRequestLifetime tools/math-port/regressions/g176/renderer-request-lifetime.cpp)
folia_add_math_probe(FoliaMathRendererRequestOwnerRetirement tools/math-port/regressions/g188/renderer-request-owner-retirement.cpp)
folia_add_math_probe(FoliaMathBboldxTextSvgProbe tools/math-port/regressions/g198/bboldx-text-svg/native-renderer-probe.cpp)


add_executable(FoliaMathTexNextSixDump EXCLUDE_FROM_ALL
    tools/math-port/tex-next-six-native-candidate.cpp)
target_compile_features(FoliaMathTexNextSixDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathTexNextSixDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathTexNextSixDump PRIVATE folia_math)

add_executable(FoliaMathTexSixPackagesDump EXCLUDE_FROM_ALL
    tools/math-port/tex-six-packages-native-candidate.cpp)
target_compile_features(FoliaMathTexSixPackagesDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathTexSixPackagesDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathTexSixPackagesDump PRIVATE folia_math)

add_executable(FoliaMathTexNextSevenDump EXCLUDE_FROM_ALL
    tools/math-port/tex-next-seven-native-candidate.cpp)
target_compile_features(FoliaMathTexNextSevenDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathTexNextSevenDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathTexNextSevenDump PRIVATE folia_math)

add_executable(FoliaMathTexFontPackagesDump EXCLUDE_FROM_ALL
    tools/math-port/tex-bboldx-font-packages-native-candidate.cpp)
target_compile_features(FoliaMathTexFontPackagesDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathTexFontPackagesDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathTexFontPackagesDump PRIVATE folia_math)

add_executable(FoliaMathConcatDump EXCLUDE_FROM_ALL
    tools/math-port/array-concat-native-dump.cpp)
target_compile_features(FoliaMathConcatDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathConcatDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathConcatDump PRIVATE folia_math)

add_executable(FoliaMathStylesTests EXCLUDE_FROM_ALL
    tests/main.cpp
    tests/math/common_styles_source.cpp)
target_compile_features(FoliaMathStylesTests PRIVATE cxx_std_23)
set_target_properties(FoliaMathStylesTests PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_include_directories(FoliaMathStylesTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(FoliaMathStylesTests PRIVATE folia_math Boost::ut_module)

add_executable(FoliaMathStylesDump EXCLUDE_FROM_ALL
    tools/math-port/common-styles-native-candidate.cpp)
target_compile_features(FoliaMathStylesDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathStylesDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathStylesDump PRIVATE folia_math)

add_executable(FoliaMathOutputJaxTests EXCLUDE_FROM_ALL
    tests/main.cpp
    tools/math-port/output-jax-reference-tests.cpp)
target_compile_features(FoliaMathOutputJaxTests PRIVATE cxx_std_23)
set_target_properties(FoliaMathOutputJaxTests PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_include_directories(FoliaMathOutputJaxTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(FoliaMathOutputJaxTests PRIVATE folia_math Boost::ut_module)

add_executable(FoliaMathOutputJaxDump EXCLUDE_FROM_ALL
    tools/math-port/output-jax-native-candidate.cpp)
target_compile_features(FoliaMathOutputJaxDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathOutputJaxDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathOutputJaxDump PRIVATE folia_math)

add_executable(FoliaMathLinkedListTests EXCLUDE_FROM_ALL
    tests/main.cpp
    tools/math-port/common-layout-reference/linked-list-native-tests.cpp)
target_compile_features(FoliaMathLinkedListTests PRIVATE cxx_std_23)
set_target_properties(FoliaMathLinkedListTests PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_include_directories(FoliaMathLinkedListTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(FoliaMathLinkedListTests PRIVATE folia_math Boost::ut_module)

add_executable(FoliaMathLinkedListDump EXCLUDE_FROM_ALL
    tools/math-port/common-layout-reference/linked-list-native-dump.cpp)
target_compile_features(FoliaMathLinkedListDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathLinkedListDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathLinkedListDump PRIVATE folia_math)

add_executable(FoliaMathOutputFoundationTests EXCLUDE_FROM_ALL
    tests/math/scene_request_owner_retirement.cpp
    tests/math/maction_passive_host_g213.cpp
    tests/main.cpp
    tools/math-port/drafts/scene-context-nodes-test.cpp
    tests/math/scene_attribute_parity_g145.cpp
    tests/math/scene_parent_projection.cpp
    tools/math-port/drafts/scene-layering-source-tests.cpp
    tools/math-port/drafts/texhtml-bridge-native-test.cpp
    tools/math-port/drafts/texhtml-html-dom-strings-native-test.cpp
    tools/math-port/drafts/texhtml-xml-lifetime-test.cpp
    tools/math-port/drafts/texhtml-xml-font-fallback-test.cpp
    tools/math-port/drafts/svg-processmath-packet/maction-context-native-test.cpp
    tools/math-port/drafts/attributes-bindings-native-tests.cpp
    tools/math-port/lite-context-projection-native-test.cpp
    tools/math-port/scene-ownership-native-tests.cpp
    tools/math-port/map-native-candidate.cpp
    tools/math-port/xml-dom-ownership-native-tests.cpp
    tools/math-port/lite-array-reduce-native-tests.cpp
    tools/math-port/native-render-session-tests.cpp
    tools/math-port/font-data-options-native-tests.cpp
    tools/math-port/font-field-replacement-native-tests.cpp
    tools/math-port/context-number-coercion-native-tests.cpp
    tools/math-port/common-jax-extension-native-tests.cpp
    tools/math-port/common-wrapper-length-native-tests.cpp
    tools/math-port/mml-node-context-methods-native-tests.cpp
    tools/math-port/drafts/mml-factory-context-methods-native-tests.cpp
    tests/math/math_render_actions.cpp)
target_compile_features(FoliaMathOutputFoundationTests PRIVATE cxx_std_23)
set_target_properties(FoliaMathOutputFoundationTests PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_include_directories(FoliaMathOutputFoundationTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(FoliaMathOutputFoundationTests PRIVATE folia_math Boost::ut_module)

add_executable(FoliaMathUnicodeRegexpDump EXCLUDE_FROM_ALL
    tools/math-port/unicode-regexp-native-candidate.cpp)
target_compile_features(FoliaMathUnicodeRegexpDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathUnicodeRegexpDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathUnicodeRegexpDump PRIVATE folia_math)

add_executable(FoliaMathBegingroupTests EXCLUDE_FROM_ALL
    tests/main.cpp
    tools/math-port/begingroup-reference/begingroup-native-tests.cpp)
target_compile_features(FoliaMathBegingroupTests PRIVATE cxx_std_23)
set_target_properties(FoliaMathBegingroupTests PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_include_directories(FoliaMathBegingroupTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(FoliaMathBegingroupTests PRIVATE folia_math Boost::ut_module)

add_executable(FoliaMathBoldsymbolConfigmacrosTests EXCLUDE_FROM_ALL
    tests/main.cpp
    tools/math-port/boldsymbol-configmacros-reference/native-candidate.cpp)
target_compile_features(FoliaMathBoldsymbolConfigmacrosTests PRIVATE cxx_std_23)
set_target_properties(FoliaMathBoldsymbolConfigmacrosTests PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_include_directories(FoliaMathBoldsymbolConfigmacrosTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(FoliaMathBoldsymbolConfigmacrosTests PRIVATE folia_math Boost::ut_module)

add_executable(FoliaMathBoldsymbolConfigmacrosDump EXCLUDE_FROM_ALL
    tools/math-port/boldsymbol-configmacros-reference/native-dump.cpp)
target_compile_features(FoliaMathBoldsymbolConfigmacrosDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathBoldsymbolConfigmacrosDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathBoldsymbolConfigmacrosDump PRIVATE folia_math)

add_executable(FoliaMathParserTests EXCLUDE_FROM_ALL
    tests/main.cpp
    tools/math-port/parser-family-reference/base-items-native-tests.cpp
    tools/math-port/parser-family-reference/base-items-reductions-native-tests.cpp
    tools/math-port/parser-family-reference/base-items-position-style-native-tests.cpp
    tools/math-port/parser-family-reference/base-items-prime-subsup-native-tests.cpp
    tools/math-port/parser-family-reference/base-items-constructor-identity-native-tests.cpp
    tools/math-port/parser-family-reference/parse-util-native-candidate.cpp
    tools/math-port/parser-family-reference/textmacros-native-candidate.cpp)
target_compile_features(FoliaMathParserTests PRIVATE cxx_std_23)
set_target_properties(FoliaMathParserTests PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_include_directories(FoliaMathParserTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(FoliaMathParserTests PRIVATE folia_math Boost::ut_module)

add_executable(FoliaMathFrontendDump EXCLUDE_FROM_ALL
    tools/math-port/parser-family-reference/native-frontend-dump.cpp)
target_compile_features(FoliaMathFrontendDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathFrontendDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathFrontendDump PRIVATE folia_math)

add_executable(FoliaMathBaseMethodsDump EXCLUDE_FROM_ALL
    tools/math-port/parser-family-reference/base-methods-native-candidate.cpp)
target_compile_features(FoliaMathBaseMethodsDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathBaseMethodsDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathBaseMethodsDump PRIVATE folia_math)

add_executable(FoliaMathAmsNewcommandDump EXCLUDE_FROM_ALL
    tools/math-port/ams-newcommand-reference/ams-newcommand-native-candidate.cpp)
target_compile_features(FoliaMathAmsNewcommandDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathAmsNewcommandDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathAmsNewcommandDump PRIVATE folia_math)

add_executable(FoliaMathAmsNewcommandBoundaryDump EXCLUDE_FROM_ALL
    tools/math-port/ams-newcommand-reference/ams-newcommand-boundary-native-candidate.cpp)
target_compile_features(FoliaMathAmsNewcommandBoundaryDump PRIVATE cxx_std_23)
set_target_properties(FoliaMathAmsNewcommandBoundaryDump PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_link_libraries(FoliaMathAmsNewcommandBoundaryDump PRIVATE folia_math)

add_executable(FoliaMathRegexpTests EXCLUDE_FROM_ALL
    tests/main.cpp
    tests/math/scanner_regexp_host.cpp
    tests/math/reference_scanner_regexp.cpp
    tests/math/reference_regexp_ignore_case.cpp
    tests/math/context_string_match.cpp)
target_compile_features(FoliaMathRegexpTests PRIVATE cxx_std_23)
set_target_properties(FoliaMathRegexpTests PROPERTIES CXX_MODULE_STD ON CXX_SCAN_FOR_MODULES ON)
target_include_directories(FoliaMathRegexpTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(FoliaMathRegexpTests PRIVATE folia_math Boost::ut_module)

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

target_sources(folia_math PUBLIC FILE_SET CXX_MODULES FILES
    src/math/core/mml_tree/mml_context_bindings.ixx)
target_sources(folia_math PRIVATE
    src/math/core/mml_tree/mml_context_bindings.cpp)
