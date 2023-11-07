enable_testing()

find_path(INCMK cmocka.h)
find_library(LIBCMK cmocka)

# Each *.c in current directory will be a standalone executable test case 
#   to verify LIB_EVT use cmocka as framework.

#Write a cmake function to generate test case for each *.c file in current directory
#   and link with LIB_EVT and cmocka framework and add the test case to CTest
function(UT_addEachC_asTestExe)
    file(GLOB UT_C_FILES *.c)
    foreach(UT_C_FILE ${UT_C_FILES})
        get_filename_component(UT_EXE_NAME ${UT_C_FILE} NAME_WE)
        add_executable(${UT_EXE_NAME} ${UT_C_FILE})
        target_include_directories(${UT_EXE_NAME} PRIVATE ${INCMK})
        target_link_libraries(${UT_EXE_NAME} ${LIB_EVT} ${LIBCMK})
        add_test(NAME ${UT_EXE_NAME} COMMAND ${UT_EXE_NAME})
    endforeach()
endfunction()

UT_addEachC_asTestExe()
