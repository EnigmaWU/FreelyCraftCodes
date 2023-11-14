cmake_minimum_required(VERSION 3.20)

project(MyCraftCodes LANGUAGES C)
set(CMAKE_C_COMPILER /usr/bin/clang)

### PlatIF_PEC -> libPlatPEC
#set(LIB_PEC PlatPEC)
#file(GLOB LIB_PEC_SRCFILES PlatIF_PEC/*.c)
#add_library(${LIB_PEC} STATIC ${LIB_PEC_SRCFILES})

#add_subdirectory(PlatIF_PEC_UT_useCmocka)
#add_subdirectory(Tools)

set(CMAKE_VERBOSE_MAKEFILE ON)

option(CONFIG_BUILD_UNIT_TESTING "Build unit test" ON)

### PlatIF_EVT -> libPlatEVT
set(LIB_EVT PlatEVT)
file(GLOB LIB_EVT_SRCFILES PlatIF_EVT/*.c)
add_library(${LIB_EVT} STATIC ${LIB_EVT_SRCFILES})

if(CONFIG_BUILD_UNIT_TESTING)
    target_compile_definitions(${LIB_EVT} PRIVATE CONFIG_BUILD_WITH_UNIT_TESTING)

    enable_testing()
    add_subdirectory(PlatIF_EVT_UT_useCmocka)
endif()