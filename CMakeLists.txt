cmake_minimum_required(VERSION 3.20)

project(MyCraftCodes LANGUAGES C)
set(CMAKE_C_COMPILER /usr/bin/clang)

### PlatIF_PEC -> libPlatPEC
set(LIB_PEC PlatPEC)
file(GLOB LIB_PEC_SRCFILES PlatIF_PEC/*.c)
add_library(${LIB_PEC} STATIC ${LIB_PEC_SRCFILES})

add_subdirectory(PlatIF_PEC_UT_useCmocka)
add_subdirectory(Tools)
