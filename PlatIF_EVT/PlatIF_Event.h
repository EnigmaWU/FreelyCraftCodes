#include "../ThinkOS/TOS_Base4ALL.h"

#ifndef __PLATIF_EVT_H__
#define __PLATIF_EVT_H__

//ModObjID=[ModID+ModInstID], ModID=0~1023, ModInstID=0~63
//  IF Module's Functional Object is singleton, ModInstID=0, OTHERWISE ModInstID=1~63
typedef uint16_t TOS_ModObjID_T;//RefMore: TOS_ModID_T

typedef uint32_t TOS_EvtID_T;//RefMore: TOS_MAKE_EVTID and TOS_EVTID_*_*_* in PlatIF_EventID.h8
#include "PlatIF_EventMetaID.h"
#include "PlatIF_EventID.h"

typedef uint32_t TOS_EvtFlags_T;
#include "PlatIF_EventFlags.h"

typedef struct {
    TOS_ModObjID_T FromObjID, ToObjID;

    TOS_EvtID_T    EvtID;
    TOS_EvtFlags_T EvtFlags;

    union 
    {
        uint8_t   U8[32];
        uint16_t U16[16];
        uint32_t U32[8];
    } EvtData;
} TOS_EvtDesc_T, *TOS_EvtDesc_pT;

typedef struct 
{
    TOS_ModObjID_T ToObjID;//SuberObjID
    void *ToObjPriv;

    TOS_Result_T (*OpProcessEvent_F)(TOS_EvtDesc_pT pEvtDesc, void* pToObjPriv);
} TOS_EvtSubArgs_T, *TOS_EvtSubArgs_pT;

TOS_Result_T PLT_EVT_subscribeEvents(TOS_EvtID_T EvtID, TOS_EvtSubArgs_pT);

TOS_Result_T PLT_EVT_publishEvents(TOS_EvtID_T EvtID);

TOS_Result_T PLT_EVT_postEvent(TOS_EvtDesc_pT pEvtDesc);

#endif // __PLATIF_EVT_H__
