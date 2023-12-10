
#include "PlatIF_EventTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Interface Call Flow:
 *  1. PLT_EVT_initEvtManger() to init EvtManger.
 *      StateInit -> StateReady
 *  2. PLT_EVT_regOper() to register EvtOper.
 *  3. PLT_EVT_subEvts() to subscribe events.
 *  4. PLT_EVT_pubEvts() to publish events.
 *  5. PLT_EVT_enableEvtManger() to enable EvtManger.
 *      StateReady -> StateRunning
 *  6. PLT_EVT_postEvtSRT() to post events in Soft-Real-Time manner.
 *      <LOOP>StateRunning -> StateRunning
 *  7. PLT_EVT_disableEvtManger() to disable EvtManger.
 *      StateRunning -> StateReady
 *  8. PLT_EVT_unsubEvts() to unsubscribe events.
 *  9. PLT_EVT_unpubEvts() to unpublish events.
 *  10. PLT_EVT_unregOper() to unregister EvtOper.
 *  11. PLT_EVT_deinitEvtManger() to deinit EvtManger.
 *      StateReady -> StateDeinit==StateInit
 * 
 */

/**
 * @brief EvtSuber call this to tell EvtManger what events it want to subscribe.
 *    ONLY EvtManger's in StateReady, EvtSuber's subscribe will be accepted.
 * 
 * @param EvtSuber: who subscribe these events, it is a EvtOperID call from PLT_EVT_regOper() as EvtSuber.
 * @param EvtIDs: what EvtIDs to sub&proc with EvtSubArgs.CbProcEvt_F;
 *          USE indivisual TOS_EVTID_*_*_* in TOS_EventID.h defined by TOS_MAKE_EVTID()
 *          OR USE group define with TOS_MAKE_EVTID_GROUP() in TOS_EventID.h such as:
 *              TOS_MAKE_EVTID_GROUP(TOS_EVENT_CLASS_TEST, TOS_EVENT_ALL_IN_CLASS)
 *              TOS_MAKE_EVTID_GROUP(TOS_EVENT_CLASS_VEHICLE_MAIN 
 *                  | TOS_EVENT_CLASS_VEHICLE_SUB_CHASSIS
 *                  | TOS_EVENT_CLASS_VEHICLE_SUB_PALLET, TOS_EVENT_ALL_IN_CLASS)
 * @param NumIDs: the number of EvtID in param EvtIDs
 * @param pEvtSubArgs: the EvtSubArgs, Refmore: TOS_EvtSubArgs_T in TOS_EventTypes.h
 *
 * @return TOS_Result_T
 *    - TOS_RESULT_SUCCESS: subscribe events successfully, means EvtSuber's CbProcEvt_F will be called when these events posted.
 *    - TOS_RESULT_BAD_STATE: EvtManger's not in StateReady.
 *    - TOS_RESULT_NOT_REGED: the EvtOper is not registered.
 *    - TOS_RESULT_TOO_MANY: the EvtSubers is full.
 */
TOS_Result_T PLT_EVT_subEvts
    ( /*ARG_IN*/TOS_EvtOperID_T EvtSuber, 
      /*ARG_IN*/TOS_EvtID_T EvtIDs[], 
      /*ARG_IN*/uint32_t NumIDs, 
      /*ARG_IN*/const TOS_EvtSubArgs_pT pEvtSubArgs);

//TODO(@W): PLT_EVT_subEvts_byModObjID, PLT_EVT_subEvts_byEvtPuberID


/**
 * @brief EvtPuber publishes events to EvtManger which my subscribe them later by EvtSuber.
 *  ONLY EvtManger's in StateReady, EvtPuber's publish will be accepted.
 *
 * @param EvtPuber who publishe the events. This is an EvtPuberID obtained from PLT_EVT_regOper() as EvtPuber.
 * @param EvtIDs An array of event IDs that the publisher wants to publish. 
 *      These IDs are defined in TOS_EventID.h using the TOS_MAKE_EVTID() or TOS_MAKE_EVTID_GROUP() macros.
 * @param NumIDs The number of event IDs in the EvtIDs array.
 *
 * @return TOS_Result_T which can be one of the following:
 * - TOS_RESULT_SUCCESS: The publication was successful.
 * - TOS_RESULT_BAD_STATE: The event manager is not in the StateReady state.
 * - TOS_RESULT_NOT_REGED: The event operator is not registered.
 * - TOS_RESULT_TOO_MANY: The event publishers are full.
 */
TOS_Result_T PLT_EVT_pubEvts
    (/*ARG_IN*/TOS_EvtOperID_T EvtPuber, /*ARG_IN*/TOS_EvtID_T EvtIDs[], /*ARG_IN*/uint32_t NumIDs);

/**
 * @brief EvtPuber call this method to post an event to EvtManger in 'Soft-Real-Time' manner.
 *  IF EvtManger's in StateReady, new posted EvtDesc will be put into EvtQueue only.
 *  WHEN EvtManger's in StateRunning, ‘old EvtDesc’ in EvtQueue will be processed first,
 *      and new posted EvtDesc will be put into EvtQueue while be processed immediately.
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
    (/*ARG_IN*/TOS_EvtOperID_T EvtOper, /*ARG_IN*/TOS_EvtDesc_pT pEvtDesc);

#ifdef CONFIG_BUILD_FEATURE_HARD_REAL_TIME
TOS_Result_T PLT_EVT_postEvtHRT
    (/*ARG_IN*/TOS_EvtOperID_T, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc);
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TOS_Result_T PLT_EVT_regOper(/*ARG_OUT*/ TOS_EvtOperID_T*, /*ARG_IN*/const TOS_EvtOperArgs_pT);


TOS_Result_T PLT_EVT_initEvtManger(/*ARG_IN*/const TOS_EvtModuleArgs_pT);


TOS_Result_T PLT_EVT_enableEvtManger(void);


//CONFIG_BUILD_WITH_UNIT_TESTING
    void PLT_EVT_unsubEvts(/*ARG_IN*/TOS_EvtOperID_T);
    void PLT_EVT_unpubEvts(/*ARG_IN*/TOS_EvtOperID_T);
    void PLT_EVT_unregOper(/*ARG_IN*/ TOS_EvtOperID_T);
    void PLT_EVT_disableEvtManger(void);
    void PLT_EVT_deinitEvtManger(void);
//CONFIG_BUILD_WITH_UNIT_TESTING

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Helpers:
#define TOS_EVT_defineEvtDesc(_EvtDescName, _EvtID)     \
    TOS_EvtDesc_T _EvtDescName = {                      \
        .EvtID        = _EvtID,                         \
    }

//TOS_EVT_[alloc,free]EvtDesc

#ifdef __cplusplus
}
#endif