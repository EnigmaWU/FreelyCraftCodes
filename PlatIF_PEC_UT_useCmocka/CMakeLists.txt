enable_testing()

find_path(INCMK cmocka.h)
find_library(LIBCMK cmocka)

function(addNewUT _UT_SRC_FNAME)
    add_executable(${_UT_SRC_FNAME} ${_UT_SRC_FNAME}.c)
    target_include_directories(${_UT_SRC_FNAME} PUBLIC ${INCMK})
    target_link_libraries(${_UT_SRC_FNAME} ${LIBCMK})
    add_test(NAME ${_UT_SRC_FNAME} COMMAND ${_UT_SRC_FNAME})
endfunction(addNewUT _UT_SRC_FNAME)


addNewUT(UT_T1_Server)