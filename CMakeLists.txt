cmake_minimum_required(VERSION 3.20)

project(MyCraftCodes LANGUAGES C CXX)

#set(CMAKE_C_COMPILER /usr/bin/clang)
#set(CMAKE_CXX_COMPILER /usr/bin/clang++)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_VERBOSE_NINJA ON)

option(CONFIG_BUILD_UNIT_TESTING "Build unit test" ON)

if(CMAKE_BUILD_TYPE STREQUAL "DiagASAN")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
elseif(CMAKE_BUILD_TYPE STREQUAL "DiagTSAN")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=thread -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")
elseif(CMAKE_BUILD_TYPE STREQUAL "DiagUBSAN")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=undefined -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
elseif(CMAKE_BUILD_TYPE STREQUAL "DiagMSAN")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=memory -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=memory")
elseif(CMAKE_BUILD_TYPE STREQUAL "DiagLSAN")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=leak -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=leak")
endif()

###############################################################################
###===> PlatIF_PEC -> libPlatPEC
#set(LIB_PEC PlatPEC)
#file(GLOB LIB_PEC_SRCFILES PlatIF_PEC/*.c)
#add_library(${LIB_PEC} STATIC ${LIB_PEC_SRCFILES})

#add_subdirectory(PlatIF_PEC_UT_useCmocka)
#add_subdirectory(Tools)

###############################################################################
### PlatIF_EVT -> libPlatEVT
set(LIB_EVT PlatEVT)
file(GLOB LIB_EVT_SRCFILES PlatIF_EVT/*.c)
add_library(${LIB_EVT} STATIC ${LIB_EVT_SRCFILES})

if(CONFIG_BUILD_UNIT_TESTING)
    target_compile_definitions(${LIB_EVT} PRIVATE CONFIG_BUILD_WITH_UNIT_TESTING)

    enable_testing()

    if(USE_UTFWK_CMOCKA)
        add_subdirectory(PlatIF_EVT_UT_useCmocka)
    endif()

    if(USE_UTFWK_CRITERION)
        add_subdirectory(PlatIF_EVT_UT_useCriterion)
    endif()

    if(USE_UTFWK_GTEST)
        add_subdirectory(PlatIF_EVT_UT_useGtest)
    endif()

endif()