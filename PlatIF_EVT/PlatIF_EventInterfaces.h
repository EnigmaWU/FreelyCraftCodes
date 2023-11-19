
#include "PlatIF_EventTypes.h"

TOS_Result_T PLT_EVT_subEvts
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuber, /*ARG_IN*/TOS_EvtID_T EvtIDs[], /*ARG_IN*/uint32_t NumIDs, /*ARG_IN*/const TOS_EvtSubArgs_pT pEvtSubArgs);

TOS_Result_T PLT_EVT_pubEvts
    (/*ARG_IN*/TOS_EvtOperID_T EvtPuber, /*ARG_IN*/TOS_EvtID_T EvtIDs[], /*ARG_IN*/uint32_t NumIDs);

void PLT_EVT_unsubEvts(/*ARG_IN*/TOS_EvtOperID_T);
void PLT_EVT_unpubEvts(/*ARG_IN*/TOS_EvtOperID_T);

/**
 * @brief EvtPuber call this method to post an event to EvtManger in 'Soft-Real-Time' manner.
 * 
 * @param EvtOper: who post this event to EvtManger, it is a EvtOperID regist with PLT_EVT_regPuber().
 * @param pEvtDesc: the event description, refmore:
 *                      TOS_EvtDesc_T in TOS_EventTypes.h 
 *                      TOS_EVTID_*_*_* in TOS_EventID.h
 *                      TOS_EVTFLAG_*_*_* in TOS_EventFlags.h
 * @return TOS_Result_T
 *    - TOS_RESULT_SUCCESS: post event to EvtManger successfully, means it is put into EvtQueue successfully.
 *    - TOS_RESULT_TOO_MANY: the EvtQueue is full, means EvtManger or EvtSubers is busy ProcEvts or 
 *                              the EvtQueueDepth is not enough, refmore: TOS_EvtModuleArgs_T::Params::EvtQueueDepth.
 *    - TOS_RESULT_NOT_PUBLISHED: the EvtDesc::EvtID is not published, means it is not publish with PLT_EVT_pubEvts().
 *    - TOS_RESULT_NOT_REGPUBER: the EvtOper is not registered, means it is not regist with PLT_EVT_regPuber().
 *    - TOS_RESULT_NO_SUBSCRIBER: no subscriber for this event, means no EvtSuber is subscribe with PLT_EVT_subEvts().
 */
TOS_Result_T PLT_EVT_postEvtSRT
    (/*ARG_IN*/TOS_EvtOperID_T EvtOper, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc);

#ifdef CONFIG_BUILD_FEATURE_HARD_REAL_TIME
TOS_Result_T PLT_EVT_postEvtHRT
    (/*ARG_IN*/TOS_EvtOperID_T, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc);
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TOS_Result_T PLT_EVT_regOper(/*ARG_OUT*/ TOS_EvtOperID_T*, /*ARG_IN*/const TOS_EvtOperArgs_pT);
void PLT_EVT_unregOper(/*ARG_IN*/ TOS_EvtOperID_T);

TOS_Result_T PLT_EVT_initEvtManger(/*ARG_IN*/const TOS_EvtModuleArgs_pT);
void PLT_EVT_deinitEvtManger(void);

TOS_Result_T PLT_EVT_enableEvtManger(void);
void PLT_EVT_disableEvtManger(void);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Helpers:
#define TOS_EVT_defineEvtDesc(_EvtDescName, _EvtID) \
    TOS_EvtDesc_T _EvtDescName = { \
        .ToModObjID   = TOS_MODOBJID_EVTSUBERS, \
        .EvtID        = _EvtID, \
    }

//TOS_EVT_[alloc,free]EvtDesc