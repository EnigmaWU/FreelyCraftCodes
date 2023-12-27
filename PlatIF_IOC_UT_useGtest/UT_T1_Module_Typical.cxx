#include "_UT_IOC_Common.h"

//This is IOC_Module's typical UT from API user's perspective
//===>Case[01]: init with typical default args and deinit
//===>Case[02]: 
TEST(ModuleTypical, initAndDeinit)
{
    TOS_ModuleObjectID_T modObjID = 0;
    IOC_ModuleArgs_T modArgs = {0};
    EXPECT_EQ(TOS_RESULT_SUCCESS, PLT_IOC_initModule(&modObjID, &modArgs));
    EXPECT_EQ(TOS_RESULT_SUCCESS, PLT_IOC_deinitModule(modObjID));
}

