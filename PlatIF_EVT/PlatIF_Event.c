#include "PlatIF_Event.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum 
{
    _EVTMGR_STATE_UNINITED = 0,
    _EVTMGR_STATE_READY,
    _EVTMGR_STATE_RUNNING,

} _EvtMangerState_T;
static _EvtMangerState_T _mEvtMangerState = _EVTMGR_STATE_UNINITED;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TOS_Result_T PLT_EVT_regOper(/*ARG_OUT*/ TOS_EvtOperID_T* pEvtOperID, /*ARG_IN*/const TOS_EvtOperArgs_pT pEvtOperArgs)
{
    if(_mEvtMangerState == _EVTMGR_STATE_READY)
    {
        //TODO(@W): doRegOper
        return TOS_RESULT_SUCCESS;
    }
    else
    {
        return TOS_RESULT_NOT_SUPPORTED;
    }
}

#ifdef CONFIG_BUILD_WITH_UNIT_TESTING
void PLT_EVT_unregOper(/*ARG_IN*/ TOS_EvtOperID_T EvtOperID)
{
    return;
}
#endif//CONFIG_BUILD_WITH_UNIT_TESTING

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TOS_Result_T PLT_EVT_enableEvtManger(void)
{
    return TOS_RESULT_NOT_SUPPORTED;
}

#ifdef CONFIG_BUILD_WITH_UNIT_TESTING
void PLT_EVT_disableEvtManger(void)
{
    return;
}
#endif//CONFIG_BUILD_WITH_UNIT_TESTING

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TOS_Result_T PLT_EVT_initEvtManger(/*ARG_IN*/const TOS_EvtModuleArgs_pT pEvtModArgs)
{
    if( _mEvtMangerState == _EVTMGR_STATE_UNINITED )
    {
        _mEvtMangerState = _EVTMGR_STATE_READY;
        return TOS_RESULT_SUCCESS;
    }
    else
    {
        return TOS_RESULT_BUG;
    }
}

#ifdef CONFIG_BUILD_WITH_UNIT_TESTING
void PLT_EVT_deinitEvtManger(void)
{
    return;
}
#endif//CONFIG_BUILD_WITH_UNIT_TESTING

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TOS_Result_T PLT_EVT_subEvts
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/TOS_EvtID_T EvtIDs[], /*ARG_IN*/uint32_t NumIDs, 
     /*ARG_IN*/const TOS_EvtSubArgs_pT pEvtSubArgs)
{
    return TOS_RESULT_NOT_SUPPORTED;
}

TOS_Result_T PLT_EVT_pubEvts
    (/*ARG_IN*/TOS_EvtOperID_T EvtPuberID, /*ARG_IN*/TOS_EvtID_T EvtIDs[], /*ARG_IN*/uint32_t NumIDs)
{
    return TOS_RESULT_NOT_SUPPORTED;
}

#ifdef CONFIG_BUILD_WITH_UNIT_TESTING
void PLT_EVT_unsubEvts(/*ARG_IN*/TOS_EvtOperID_T)
{

}
void PLT_EVT_unpubEvts(/*ARG_IN*/TOS_EvtOperID_T)
{

}
#endif//CONFIG_BUILD_WITH_UNIT_TESTING

TOS_Result_T PLT_EVT_postEvtSRT
    (/*ARG_IN*/TOS_EvtOperID_T EvtPuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc)
{
    return TOS_RESULT_NOT_SUPPORTED;
}

#ifdef CONFIG_BUILD_FEATURE_HARD_REAL_TIME
TOS_Result_T PLT_EVT_postEvtHRT
    (/*ARG_IN*/TOS_EvtOperID_T, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc);
#endif
