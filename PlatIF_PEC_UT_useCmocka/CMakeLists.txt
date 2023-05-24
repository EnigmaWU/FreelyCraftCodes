enable_testing()

find_path(INCMK cmocka.h)
find_library(LIBCMK cmocka)

# UT依赖cmocka库，如果当前目录下存在cmocka的头文件和库文件，则直接使用
# 否则下载cmocka的源码到当前目录下，编译并安装到当前目录下
if(NOT INCMK OR NOT LIBCMK)
    message(STATUS "cmocka not found, download and install it")
    include(ExternalProject)
    ExternalProject_Add(cmocka
        GIT_REPOSITORY https://git.cryptomilk.org/projects/cmocka.git
        GIT_TAG cmocka-1.1.5
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/cmocka
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    )
    set(INCMK ${CMAKE_CURRENT_BINARY_DIR}/cmocka/include)
    set(LIBCMK ${CMAKE_CURRENT_BINARY_DIR}/cmocka/lib/libcmocka.a)
    add_library(cmocka STATIC IMPORTED)
    set_property(TARGET cmocka PROPERTY IMPORTED_LOCATION ${LIBCMK})
    add_dependencies(cmocka cmocka)
endif(NOT INCMK OR NOT LIBCMK)

# 如果当前目录下存在NewCmocka这个目录，那么它是更新cmocka的源码
# 编译这个目录下的源码，安装到当前目录下，使用新装的cmocka
# 特别注意：新版本使用 make && make install，而不是cmake
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/NewCmocka)
    message(STATUS "cmocka update found, install it")
    execute_process(COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR}/NewCmocka)
    execute_process(COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR}/NewCmocka install)
    set(INCMK ${CMAKE_CURRENT_BINARY_DIR}/cmocka/include)
    set(LIBCMK ${CMAKE_CURRENT_BINARY_DIR}/cmocka/lib/libcmocka.a)
    add_library(cmocka STATIC IMPORTED)
    set_property(TARGET cmocka PROPERTY IMPORTED_LOCATION ${LIBCMK})
    add_dependencies(cmocka cmocka)
endif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/NewCmocka)

function(addNewUT _UT_SRC_FNAME)
    add_executable(${_UT_SRC_FNAME} ${_UT_SRC_FNAME}.c)
    target_include_directories(${_UT_SRC_FNAME} PUBLIC ${INCMK})
    target_link_libraries(${_UT_SRC_FNAME} ${LIBCMK})
    add_test(NAME ${_UT_SRC_FNAME} COMMAND ${_UT_SRC_FNAME})
endfunction(addNewUT _UT_SRC_FNAME)


# 当前目录下的每个C文件，都是独立的UT可执行程序，每个都自动添加到测试中
file(GLOB UT_SRC_FILES *.c)
foreach(UT_SRC_FILE ${UT_SRC_FILES})
    get_filename_component(UT_SRC_FNAME ${UT_SRC_FILE} NAME_WE)
    addNewUT(${UT_SRC_FNAME})
endforeach(UT_SRC_FILE ${UT_SRC_FILES})