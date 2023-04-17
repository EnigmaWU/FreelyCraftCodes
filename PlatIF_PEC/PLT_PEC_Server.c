#include "PLT_PEC.h"

TOS_Result_T PLT_PEC_onlineService
    ( /*ARG_OUT*/PEC_ServiceObjectID_pT pSrvObjID, 
      /*ARG_IN*/PEC_ServiceArgs_pT pSrvArgs, 
      /*ARG_IN_OPTIONAL*/TOS_BaseOption_pT pOption )
{
    if (pSrvObjID == NULL || pSrvArgs == NULL || pOption == NULL)
    {
        return TOS_RESULT_INVALID;
    }

    static bool isOnline = false;

    if (isOnline)
    {
        return TOS_RESULT_ALREADY;
    }

    isOnline = true;

    return TOS_RESULT_SUCCESS;

}
TOS_Result_T PLT_PEC_offlineSecvice(/*ARG_IN*/PEC_ServiceObjectID_T);

TOS_Result_T PLT_PEC_acceptLink(/*ARG_IN*/PEC_ServiceObjectID_T, /*ARG_OUT*/PEC_LinkObjectID_pT, /*ARG_IN_OPTIONAL*/TOS_BaseOption_pT);

