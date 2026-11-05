cmake_minimum_required(VERSION 3.21)
project(MaenBrowser VERSION 1.11.0 LANGUAGES CXX)

if(MSVC)
  add_compile_options(/utf-8)
endif()

if(NOT WIN32)
  message(FATAL_ERROR "MaenBrowser desktop build currently targets Windows.")
endif()

set(PROJECT_ARCH "x86_64")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CEF_ROOT "" CACHE PATH "Path to extracted CEF binary distribution")
option(MAEN_ENABLE_UPDATER "Enable WinSparkle updater" OFF)
set(WINSPARKLE_ROOT "" CACHE PATH "Path to WinSparkle SDK")
set(WEBVIEW2_SDK_ROOT "" CACHE PATH "Path to extracted Microsoft.Web.WebView2 NuGet package")

if(NOT CEF_ROOT)
  message(FATAL_ERROR "CEF_ROOT is required. Run tools/fetch-cef.ps1 first.")
endif()

if(NOT WEBVIEW2_SDK_ROOT)
  message(FATAL_ERROR "WEBVIEW2_SDK_ROOT is required for embedded WhatsApp/media support.")
endif()
set(WEBVIEW2_INCLUDE "${WEBVIEW2_SDK_ROOT}/build/native/include")
set(WEBVIEW2_STATIC_LOADER "${WEBVIEW2_SDK_ROOT}/build/native/x64/WebView2LoaderStatic.lib")
if(NOT EXISTS "${WEBVIEW2_INCLUDE}/WebView2.h")
  message(FATAL_ERROR "WebView2.h missing under WEBVIEW2_SDK_ROOT")
endif()
if(NOT EXISTS "${WEBVIEW2_STATIC_LOADER}")
  message(FATAL_ERROR "WebView2 x64 static loader missing under WEBVIEW2_SDK_ROOT")
endif()

# Verify the selected archive identity and extracted files before compiling.
# A successful C++ build alone does not establish H.264 support.
find_package(Python3 3.12 REQUIRED COMPONENTS Interpreter)
execute_process(
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/media-runtime.py" --verify "${CEF_ROOT}"
  RESULT_VARIABLE MAEN_MEDIA_VERIFY_RESULT
)
if(NOT MAEN_MEDIA_VERIFY_RESULT EQUAL 0)
  message(FATAL_ERROR "CEF media provenance/layout verification failed. Run tools/fetch-cef.ps1.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CEF_ROOT}/cmake")
find_package(CEF REQUIRED)
if(NOT USE_SANDBOX)
  message(FATAL_ERROR "MaenBrowser distribution requires USE_SANDBOX=ON")
endif()

add_subdirectory("${CEF_LIBCEF_DLL_WRAPPER_PATH}" libcef_dll_wrapper)
ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")

set(MAEN_SOURCES
  src/main_win.cpp
  src/app/maen_app.cpp
  src/app/maen_client.cpp
  src/app/resource_mode.cpp
  src/app/browser_preferences.cpp
  src/app/start_page.cpp
  src/security/windows_hardening.cpp
  src/media/windows_media_foundation.cpp
  src/media/webview2_media_router.cpp
  src/media/media_window.cpp
  src/media/webview2_embedded.cpp
  src/win/maenbrowser.rc
  src/extensions/extension_manager.cpp
  src/storage/local_profile.cpp
  src/updater/update_manager.cpp
  src/protection/protection.cpp
)

set(MAEN_HEADERS
  include/maenbrowser/version.h
  src/app/maen_app.h
  src/app/maen_client.h
  src/app/resource_mode.h
  src/app/browser_preferences.h
  src/app/start_page.h
  src/security/windows_hardening.h
  src/media/windows_media_foundation.h
  src/media/webview2_media_router.h
  src/media/media_window.h
  src/media/webview2_embedded.h
  src/win/resource.h
  src/extensions/extension_manager.h
  src/storage/local_profile.h
  src/updater/update_manager.h
)

if(CMAKE_GENERATOR MATCHES "Ninja|Makefiles")
  set(MAEN_OUT_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
else()
  set(MAEN_OUT_DIR "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
endif()

if(USE_SANDBOX)
  # CEF 152+ recommended Windows bootstrap model. The output DLL is loaded by
  # bootstrap.exe, which is copied/renamed to MaenBrowser.exe.
  add_library(MaenBrowser SHARED ${MAEN_SOURCES} ${MAEN_HEADERS})
  SET_LIBRARY_TARGET_PROPERTIES(MaenBrowser)
  COPY_SINGLE_FILE(MaenBrowser
    "${CEF_BINARY_DIR}/bootstrap.exe"
    "${MAEN_OUT_DIR}/MaenBrowser.exe")
else()
  add_executable(MaenBrowser WIN32 ${MAEN_SOURCES} ${MAEN_HEADERS})
  SET_EXECUTABLE_TARGET_PROPERTIES(MaenBrowser)
endif()

set_target_properties(MaenBrowser PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${MAEN_OUT_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${MAEN_OUT_DIR}"
)

target_include_directories(MaenBrowser PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CEF_ROOT}"
  "${WEBVIEW2_INCLUDE}"
)

target_compile_definitions(MaenBrowser PRIVATE
  UNICODE
  _UNICODE
  NOMINMAX
  WIN32_LEAN_AND_MEAN
  MAEN_BROWSER_VERSION="${PROJECT_VERSION}"
)

if(MAEN_ENABLE_UPDATER)
  if(NOT WINSPARKLE_ROOT)
    message(FATAL_ERROR "MAEN_ENABLE_UPDATER=ON requires WINSPARKLE_ROOT.")
  endif()
  target_compile_definitions(MaenBrowser PRIVATE MAEN_ENABLE_UPDATER=1)
  target_include_directories(MaenBrowser PRIVATE "${WINSPARKLE_ROOT}/include")
  target_link_directories(MaenBrowser PRIVATE "${WINSPARKLE_ROOT}/lib")
  target_link_libraries(MaenBrowser PRIVATE WinSparkle)
else()
  target_compile_definitions(MaenBrowser PRIVATE MAEN_ENABLE_UPDATER=0)
endif()

target_link_libraries(MaenBrowser PRIVATE
  libcef_lib
  libcef_dll_wrapper
  comctl32
  shlwapi
  user32
  shell32
  mfplat
  mf
  mfuuid
  "${WEBVIEW2_STATIC_LOADER}"
  ole32
  oleaut32
  runtimeobject
  version
  advapi32
  bcrypt
  psapi
  crypt32
  ${CEF_STANDARD_LIBS}
)

# Copy CEF runtime DLLs/resources beside MaenBrowser.
COPY_FILES("MaenBrowser" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${MAEN_OUT_DIR}")
COPY_FILES("MaenBrowser" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${MAEN_OUT_DIR}")

add_custom_command(TARGET MaenBrowser POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "${MAEN_OUT_DIR}/notices"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/config/agent-capabilities.json" "${MAEN_OUT_DIR}/maen-agent-capabilities.json"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${MAEN_OUT_DIR}/tools"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/tools/start-agent-browser.ps1" "${MAEN_OUT_DIR}/tools/start-agent-browser.ps1"
  COMMAND ${CMAKE_COMMAND} -E copy "${CEF_ROOT}/LICENSE.txt" "${MAEN_OUT_DIR}/notices/CEF-LICENSE.txt"
  COMMAND ${CMAKE_COMMAND} -E copy "${CEF_ROOT}/MAEN_MEDIA_PROVENANCE.json" "${MAEN_OUT_DIR}/notices/MAEN_MEDIA_PROVENANCE.json"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/assets/companion" "${MAEN_OUT_DIR}/companion"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/assets/filters" "${MAEN_OUT_DIR}/filters"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/assets/shims" "${MAEN_OUT_DIR}/shims"
)

enable_testing()
add_executable(MaenDomainTests tests/domain_rules_test.cpp)
target_include_directories(MaenDomainTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
add_test(NAME domain_rules COMMAND MaenDomainTests)
add_executable(MaenMediaTests tests/media_policy_test.cpp)
target_include_directories(MaenMediaTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
add_test(NAME media_policy COMMAND MaenMediaTests)



# WebView2 is linked into the main browser as a lazy embedded surface. It is
# instantiated only for routed media sites (WhatsApp Web and direct HTTPS MP4/M4V/M4A/AAC files), preserving
# the full CEF Chrome Runtime UI while avoiding a second top-level application.

add_executable(MaenNavigationTests tests/internal_navigation_test.cpp)
target_include_directories(MaenNavigationTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
add_test(NAME internal_navigation COMMAND MaenNavigationTests)

add_executable(MaenExternalTests tests/external_protocol_test.cpp)
target_include_directories(MaenExternalTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
add_test(NAME external_protocol COMMAND MaenExternalTests)

add_executable(MaenFilterTests tests/filter_snapshot_test.cpp)
target_include_directories(MaenFilterTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
add_test(NAME filter_snapshot COMMAND MaenFilterTests)

add_executable(MaenResourceTests tests/resource_policy_test.cpp)
target_include_directories(MaenResourceTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
add_test(NAME resource_policy COMMAND MaenResourceTests)

# Release acceptance tests must execute their assertions even with NDEBUG.
foreach(test_target MaenDomainTests MaenMediaTests MaenNavigationTests MaenExternalTests MaenFilterTests MaenResourceTests)
  if(MSVC)
    target_compile_options(${test_target} PRIVATE /UNDEBUG)
  else()
    target_compile_options(${test_target} PRIVATE -UNDEBUG)
  endif()
endforeach()

if(EXISTS "${CEF_ROOT}/CREDITS.html")
  add_custom_command(TARGET MaenBrowser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CEF_ROOT}/CREDITS.html" "${MAEN_OUT_DIR}/notices/CEF-CREDITS.html"
  )
endif()

add_executable(MaenControlOriginTests tests/control_origin_test.cpp)
target_include_directories(MaenControlOriginTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
if(MSVC)
  target_compile_options(MaenControlOriginTests PRIVATE /UNDEBUG)
else()
  target_compile_options(MaenControlOriginTests PRIVATE -UNDEBUG)
endif()
add_test(NAME control_origin COMMAND MaenControlOriginTests)

foreach(pair "MaenAgentProfileTests;tests/agent_profile_test.cpp" "MaenPopupPolicyTests;tests/popup_policy_test.cpp" "MaenSettingsStampTests;tests/settings_stamp_test.cpp" "MaenAdShimTests;tests/ad_shim_policy_test.cpp")
  list(GET pair 0 test_target)
  list(GET pair 1 test_source)
  add_executable(${test_target} ${test_source})
  target_include_directories(${test_target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
  if(MSVC)
    target_compile_options(${test_target} PRIVATE /UNDEBUG)
  else()
    target_compile_options(${test_target} PRIVATE -UNDEBUG)
  endif()
  add_test(NAME ${test_target} COMMAND ${test_target})
endforeach()
