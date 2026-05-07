cmake_minimum_required(VERSION 3.20)
project(crosspoint_sim CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)  # device build uses gnu++2a

# Sanitizers help catch the kinds of UB that on-device builds silently mask.
if(NOT DEFINED SIM_SANITIZE)
  set(SIM_SANITIZE ON)
endif()
if(SIM_SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

add_compile_options(
  -Wall -Wno-unused-parameter -Wno-sign-compare -Wno-unused-variable
  -Wno-unused-but-set-variable
)

add_compile_definitions(
  CROSSPOINT_EMULATED=1
  ENABLE_SERIAL_LOG
  LOG_LEVEL=2
  XML_GE=0
)

set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Compat shims (Arduino.h, HardwareSerial.h) and stubs (HalDisplay.h) MUST come
# first on the include path so they shadow the device headers.
include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}/compat
  ${REPO_ROOT}/lib/hal_sim
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
  ${REPO_ROOT}/lib/Logging
  ${REPO_ROOT}/lib/Utf8
  ${REPO_ROOT}/lib/InflateReader
  ${REPO_ROOT}/lib/uzlib/src
  ${REPO_ROOT}/lib/EpdFont
  ${REPO_ROOT}/lib/EpdFont/builtinFonts
  ${REPO_ROOT}/lib/GfxRenderer
  ${REPO_ROOT}/lib/FsHelpers
  ${REPO_ROOT}/lib/ZipFile
  ${REPO_ROOT}/lib/Epub
  ${REPO_ROOT}/lib/Epub/Epub
  ${REPO_ROOT}/lib/Epub/Epub/css
  ${REPO_ROOT}/lib/Epub/Epub/hyphenation
  ${REPO_ROOT}/lib/I18n
  ${REPO_ROOT}/src
  ${REPO_ROOT}/src/components
  ${REPO_ROOT}/src/components/themes
  ${REPO_ROOT}/src/components/themes/lyra
  ${REPO_ROOT}/src/util
)

# uzlib (C)
add_library(uzlib_sim STATIC
  ${REPO_ROOT}/lib/uzlib/src/tinflate.c
  ${CMAKE_CURRENT_SOURCE_DIR}/stub/uzlib_chksum_stubs.c
)
target_include_directories(uzlib_sim PUBLIC ${REPO_ROOT}/lib/uzlib/src)

add_library(crosspoint_core STATIC
  ${REPO_ROOT}/lib/Logging/Logging.cpp
  ${REPO_ROOT}/lib/Utf8/Utf8.cpp
  ${REPO_ROOT}/lib/InflateReader/InflateReader.cpp
  ${REPO_ROOT}/lib/EpdFont/EpdFont.cpp
  ${REPO_ROOT}/lib/EpdFont/EpdFontFamily.cpp
  ${REPO_ROOT}/lib/EpdFont/FontDecompressor.cpp
  ${REPO_ROOT}/lib/GfxRenderer/GfxRenderer.cpp
  ${REPO_ROOT}/lib/GfxRenderer/Bitmap.cpp
  ${REPO_ROOT}/lib/GfxRenderer/BitmapHelpers.cpp
  ${REPO_ROOT}/lib/GfxRenderer/FontCacheManager.cpp
  ${REPO_ROOT}/lib/FsHelpers/FsHelpers.cpp
  ${REPO_ROOT}/lib/ZipFile/ZipFile.cpp
  ${REPO_ROOT}/lib/Epub/Epub/htmlEntities.cpp
  ${REPO_ROOT}/lib/Epub/Epub/css/CssParser.cpp
  ${REPO_ROOT}/lib/Epub/Epub/hyphenation/HyphenationCommon.cpp
  ${REPO_ROOT}/lib/Epub/Epub/hyphenation/LiangHyphenation.cpp
  ${REPO_ROOT}/lib/Epub/Epub/hyphenation/LanguageRegistry.cpp
  ${REPO_ROOT}/lib/Epub/Epub/hyphenation/Hyphenator.cpp
  ${REPO_ROOT}/lib/I18n/I18n.cpp
  ${REPO_ROOT}/lib/I18n/I18nStrings.cpp
  # Real device source brought in (header dependencies satisfied by stubs).
  ${REPO_ROOT}/src/MappedInputManager.cpp
  ${REPO_ROOT}/src/util/ButtonNavigator.cpp
  ${REPO_ROOT}/src/components/themes/BaseTheme.cpp
  ${REPO_ROOT}/src/components/themes/lyra/LyraTheme.cpp
  # Sim-side stubs for state classes (in-memory, no JSON persistence).
  ${CMAKE_CURRENT_SOURCE_DIR}/sim_src/CrossPointSettings.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sim_src/RecentBooksStore.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sim_src/OpdsServerStore.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sim_src/UITheme.cpp
  # Simulator HAL backend (replaces lib/hal/ via include-path priority).
  ${REPO_ROOT}/lib/hal_sim/HalDisplay.cpp
  ${REPO_ROOT}/lib/hal_sim/HalGPIO.cpp
  ${REPO_ROOT}/lib/hal_sim/HalStorage.cpp
  ${REPO_ROOT}/lib/hal_sim/HalPowerManager.cpp
)
target_link_libraries(crosspoint_core PUBLIC uzlib_sim)

find_package(SDL2 REQUIRED)

add_executable(crosspoint_sim
  sim_main.cpp
)
target_link_libraries(crosspoint_sim PRIVATE crosspoint_core ${SDL2_LIBRARIES})
target_include_directories(crosspoint_sim PRIVATE ${SDL2_INCLUDE_DIRS})
