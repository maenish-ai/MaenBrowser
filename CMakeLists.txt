cmake_minimum_required(VERSION 3.21)
project(MaenBrowser VERSION 1.6.0 LANGUAGES CXX)

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

list(APPEND CMAKE_MODULE_PATH "${CEF_ROOT}/cmake")
find_package(CEF REQUIRED)

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
  src/win/maenbrowser.rc
  src/extensions/extension_manager.cpp
  src/storage/local_profile.cpp
  src/updater/update_manager.cpp
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
  ${CEF_STANDARD_LIBS}
)

# Copy CEF runtime DLLs/resources beside MaenBrowser.
COPY_FILES("MaenBrowser" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${MAEN_OUT_DIR}")
COPY_FILES("MaenBrowser" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${MAEN_OUT_DIR}")


# WhatsApp media host: use Microsoft's supported Edge WebView2 Runtime for the
# one web app that requires proprietary media codecs unavailable in stock CEF.
# The main browser remains native C++ + CEF and keeps the existing lightweight
# resource/security policy. WebView2 Evergreen is shared with Windows/apps.
if(WEBVIEW2_SDK_ROOT)
  set(WEBVIEW2_INCLUDE "${WEBVIEW2_SDK_ROOT}/build/native/include")
  set(WEBVIEW2_STATIC_LOADER "${WEBVIEW2_SDK_ROOT}/build/native/x64/WebView2LoaderStatic.lib")
  if(NOT EXISTS "${WEBVIEW2_INCLUDE}/WebView2.h")
    message(FATAL_ERROR "WebView2.h missing under WEBVIEW2_SDK_ROOT")
  endif()
  if(NOT EXISTS "${WEBVIEW2_STATIC_LOADER}")
    message(FATAL_ERROR "WebView2 x64 static loader missing under WEBVIEW2_SDK_ROOT")
  endif()

  add_executable(MaenMediaHost WIN32
    src/media/webview2_media_host.cpp
    src/win/maenbrowser.rc
  )
  target_include_directories(MaenMediaHost PRIVATE "${WEBVIEW2_INCLUDE}")
  target_compile_definitions(MaenMediaHost PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
  target_link_libraries(MaenMediaHost PRIVATE
    "${WEBVIEW2_STATIC_LOADER}"
    user32 shell32 ole32 oleaut32 runtimeobject shlwapi version
  )
  set_target_properties(MaenMediaHost PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${MAEN_OUT_DIR}")
else()
  message(FATAL_ERROR "WEBVIEW2_SDK_ROOT is required for the WhatsApp media host.")
endif()
