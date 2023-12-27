#include "_PlatIF_IOC.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//===> BEGIN of IMPL in C
#ifdef __cplusplus
extern "C" {
#endif


TOS_Result_T PLT_IOC_initModule(TOS_ModuleObjectID_T *pModObjID, const IOC_ModuleArgs_pT pModArgs)
{
    return TOS_RESULT_NOT_IMPLEMENTED;
}

TOS_Result_T PLT_IOC_deinitModule(TOS_ModuleObjectID_T ModObjID)
{
    return TOS_RESULT_NOT_IMPLEMENTED;
}
#ifdef __cplusplus
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//===> BEGIN of IOC_Module's internal UT in CXX
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

