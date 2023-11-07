#include "../ThinkOS/TOS_Base4ALL.h"

#ifndef __PLATIF_EVT_H__
#define __PLATIF_EVT_H__

//ModObjID=[ModID+ModInstID], ModID=0~1023, ModInstID=0~63
//  IF Module's Functional Object is singleton, ModInstID=0, OTHERWISE ModInstID=1~63
typedef uint16_t TOS_ModObjID_T;//RefMore: TOS_ModID_T

typedef uint32_t TOS_EvtID_T;//RefMore: TOS_MAKE_EVTID and TOS_EVTID_*_*_* in PlatIF_EventID.h
#include "PlatIF_EventMetaID.h"
#include "PlatIF_EventID.h"

typedef uint32_t TOS_EvtFlags_T;
#include "PlatIF_EventFlags.h"

typedef uint32_t TOS_EvtOperID_T;
#include "PlatIF_EventOperator.h"

typedef struct {
    TOS_ModObjID_T FromModObjID, ToModObjID/*Specific or All-Subers*/;

    TOS_EvtID_T    EvtID;
    TOS_EvtFlags_T EvtFlags;

    union 
    {
        uint8_t   U8[32];
        uint16_t U16[16];
        uint32_t U32[8];
        //void *pMemAddr;
    } EvtData;
} TOS_EvtDesc_T, *TOS_EvtDesc_pT;

typedef struct 
{
    void *ToObjPriv;

    TOS_Result_T (*OpProcessEventSRT_F)
        (/*ARG_IN*/TOS_EvtOperID_T, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToObjPriv);
#ifdef CONFIG_BUILD_WITH_HARD_REAL_TIME
    TOS_Result_T (*OpProcessEventHRT_F)
        (/*ARG_IN*/TOS_EvtOperID_T, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToObjPriv);
#endif
} TOS_EvtSubArgs_T, *TOS_EvtSubArgs_pT;

TOS_Result_T PLT_EVT_subscribeEvents
    (/*ARG_IN*/TOS_EvtOperID_T, /*ARG_IN*/TOS_EvtID_T EvtIDs[], /*ARG_IN*/uint32_t NumIDs, /*ARG_IN*/TOS_EvtSubArgs_pT);

TOS_Result_T PLT_EVT_publishEvents
    (/*ARG_IN*/TOS_EvtOperID_T, /*ARG_IN*/TOS_EvtID_T EvtIDs[], /*ARG_IN*/uint32_t NumIDs);

TOS_Result_T PLT_EVT_postEventSRT
    (/*ARG_IN*/TOS_EvtOperID_T, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc);

#ifdef CONFIG_BUILD_WITH_HARD_REAL_TIME
TOS_Result_T PLT_EVT_postEventHRT
    (/*ARG_IN*/TOS_EvtOperID_T, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc);
#endif


#endif // __PLATIF_EVT_H__
