cmake_minimum_required(VERSION 3.21)
project(MaenBrowser VERSION 1.0.0 LANGUAGES CXX)

set(PROJECT_ARCH "x86_64")

if(NOT WIN32)
  message(FATAL_ERROR "MaenBrowser Windows foundation currently builds on Windows only.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MAEN_ENABLE_UPDATER "Enable WinSparkle updater integration" OFF)
set(CEF_ROOT "" CACHE PATH "Path to an extracted CEF Standard Distribution")
set(WINSPARKLE_ROOT "" CACHE PATH "Path to WinSparkle SDK (when MAEN_ENABLE_UPDATER=ON)")

if(NOT CEF_ROOT)
  message(FATAL_ERROR "Set -DCEF_ROOT=C:/path/to/cef_binary_xxx_windows64")
endif()

list(APPEND CMAKE_MODULE_PATH "${CEF_ROOT}/cmake")
find_package(CEF REQUIRED)

add_subdirectory("${CEF_LIBCEF_DLL_WRAPPER_PATH}" libcef_dll_wrapper)
ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")

set(MAEN_SOURCES
  src/main_win.cpp
  src/app/maen_app.cpp
  src/app/maen_client.cpp
  src/extensions/extension_manager.cpp
  src/storage/local_profile.cpp
  src/updater/update_manager.cpp
)

set(MAEN_HEADERS
  include/maenbrowser/version.h
  src/app/maen_app.h
  src/app/maen_client.h
  src/extensions/extension_manager.h
  src/storage/local_profile.h
  src/updater/update_manager.h
)

add_executable(MaenBrowser WIN32 ${MAEN_SOURCES} ${MAEN_HEADERS})
SET_EXECUTABLE_TARGET_PROPERTIES(MaenBrowser)

if(CMAKE_GENERATOR MATCHES "Ninja|Makefiles")
  set(MAEN_OUT_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
else()
  set(MAEN_OUT_DIR "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
endif()
set_target_properties(MaenBrowser PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${MAEN_OUT_DIR}")
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
    message(FATAL_ERROR "MAEN_ENABLE_UPDATER=ON requires -DWINSPARKLE_ROOT=...")
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
  ${CEF_STANDARD_LIBS}
  shell32
  user32
)

# CEF helper macros copy required runtime files/resources to the output directory.
COPY_FILES("MaenBrowser" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${MAEN_OUT_DIR}")
COPY_FILES("MaenBrowser" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${MAEN_OUT_DIR}")
