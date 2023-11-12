#include "PlatIF_EventTypes.h"

typedef struct
{
    TOS_ModObjID_T ModObjID;//PuberObjID or SuberObjID
    //TODO: +More...
} TOS_EvtOperArgs_T, *TOS_EvtOperArgs_pT;

TOS_Result_T PLT_EVT_registerOperator(/*ARG_OUT*/ TOS_EvtOperID_T*, /*ARG_IN*/TOS_EvtOperArgs_pT);
void PLT_EVT_unregisterOperator(/*ARG_IN*/ TOS_EvtOperID_T);
