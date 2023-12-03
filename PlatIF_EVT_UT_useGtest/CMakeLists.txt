# Find the googletest headers
find_path(INCGTST gtest/gtest.h)
# Find the googletest library
find_library(LIBGTST gtest)


# Each *.c in current directory will be a standalone executable test case 
#   to verify LIB_EVT use cmocka as framework.

#Write a cmake function to generate test case for each *.c file in current directory
#   and link with LIB_EVT and cmocka framework and add the test case to CTest
function(UT_addEachC_asTestExe)
    file(GLOB UT_C_FILES *.cxx)
    foreach(UT_C_FILE ${UT_C_FILES})
        get_filename_component(UT_EXE_NAME ${UT_C_FILE} NAME_WE)
        add_executable(${UT_EXE_NAME} ${UT_C_FILE})
        set_target_properties(${UT_EXE_NAME} PROPERTIES LINKER_LANGUAGE CXX)
        target_compile_definitions(${UT_EXE_NAME} PRIVATE 
            CONFIG_BUILD_WITH_UNIT_TESTING CONFIG_BUILD_WITH_UTFWK_GTEST)
        target_include_directories(${UT_EXE_NAME} PRIVATE ${INCGTST})
        target_link_libraries(${UT_EXE_NAME} ${LIB_EVT} ${LIBGTST})
        add_test(NAME ${UT_EXE_NAME} COMMAND ${UT_EXE_NAME})
    endforeach()
endfunction()

UT_addEachC_asTestExe()
