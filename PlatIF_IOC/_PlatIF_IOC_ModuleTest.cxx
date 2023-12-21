#include "_PlatIF_IOC_Module.c"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//===> BEGIN of UT in CXX
#ifdef CONFIG_BUILD_INFILE_UNIT_TESTING_USE_UTFWK_GTEST
#include <gtest/gtest.h>
TEST(IOC_Module, initWithNullArgs)
{
    EXPECT_EQ(TOS_RESULT_INVALID, PLT_IOC_initModule(NULL, NULL));
}
TEST(IOC_Module, deinitWithZeroArgs)
{
    EXPECT_EQ(TOS_RESULT_INVALID, PLT_IOC_deinitModule(0));
}
#endif /* CONFIG_BUILD_INFILE_UNIT_TESTING_USE_UTFWK_GTEST */

