#include "../ThinkOS/TOS_Base4ALL.h"

#ifndef __PLAT_IF_EVENT_TYPES_H__
#define __PLAT_IF_EVENT_TYPES_H__

//ModObjID=[ModID+ModInstID], ModID=0~1023, ModInstID=0~63
//  IF Module's Functional Object is singleton, ModInstID=0, OTHERWISE ModInstID=1~63
typedef uint16_t TOS_ModObjID_T;//RefMore: TOS_ModID_T

enum TOS_ModObjID_Reserved_enum
{
    TOS_MODOBJID_EVTSUBERS      = 0x0000UL,
    TOS_MODOBJID_ALL            = 0xFFFFUL,

    //ModObjID_UT=ModID_UT(=1023)+ModInstID_UT(=0~63)
    TOS_MODOBJID_UT_A           = 0xFFFEUL,
    TOS_MODOBJID_UT_B           = 0xFFFDUL,
    TOS_MODOBJID_UT_C           = 0xFFFCUL,
    TOS_MODOBJID_UT_D           = 0xFFFBUL,
    TOS_MODOBJID_UT_E           = 0xFFFAUL,

};

typedef uint32_t TOS_EvtID_T;//RefMore: TOS_MAKE_EVTID and TOS_EVTID_*_*_* in PlatIF_EventID.h

typedef uint32_t TOS_EvtFlags_T;

typedef uint32_t TOS_EvtOperID_T;

typedef struct {
    TOS_ModObjID_T FromModObjID;
    TOS_ModObjID_T ToModObjID;

    TOS_EvtID_T    EvtID;//RefMore: TOS_EVTID_*_*_* which made by TOS_MAKE_EVTID in PlatIF_EventID.h
    TOS_EvtFlags_T EvtFlags;//DEFAULT is '0' and it always is ENOUGH; RefMore: TOS_EVTFLAG_*_*_* in PlatIF_EventFlags.h
    uint32_t       SeqID;//PerEvtPuber: 0~0xFFFFFFFF

    union 
    {
        uint8_t   U8[32];
        uint16_t U16[16];
        uint32_t U32[8];
        //void *pMemAddr;
    } EvtData;
} TOS_EvtDesc_T, *TOS_EvtDesc_pT;//RefHelper: TOS_EVT_defineEvtDesc, TOS_EVT_[alloc,free]EvtDesc in PlatIF_EventInterfaces.h

typedef TOS_Result_T (*TOS_EvtSubCallback_ProcEvt_F)
    (/*ARG_IN*/TOS_EvtOperID_T, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToObjPriv);

typedef struct 
{
    void *ToObjPriv;

    TOS_EvtSubCallback_ProcEvt_F CbProcEvtSRT_F;
#ifdef CONFIG_BUILD_FEATURE_HARD_REAL_TIME
    TOS_EvtSubCallback_ProcEvt_F CbProcEvtHRT_F;
#endif
} TOS_EvtSubArgs_T, *TOS_EvtSubArgs_pT;


typedef struct
{
    TOS_ModObjID_T ModObjID;//PuberObjID or SuberObjID
    //TODO(@W): +More...
} TOS_EvtOperArgs_T, *TOS_EvtOperArgs_pT;

typedef struct 
{
    //TODO(@W): LogLevel, OpLogWriter_F

    struct 
    {
        uint16_t EvtQueueDepth;
        uint16_t OperatorCount;
        uint16_t PuberEvtMaxRate;//0=Default=100EPS
        //TODO(@W): +More...
    } Params;
} TOS_EvtModuleArgs_T, *TOS_EvtModuleArgs_pT;

#endif//__PLAT_IF_EVENT_TYPES_H__
