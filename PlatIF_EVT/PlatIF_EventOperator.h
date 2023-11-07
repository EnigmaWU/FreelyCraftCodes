typedef struct
{
    TOS_ModObjID_T ModObjID;//PuberObjID or SuberObjID
    //TODO: +More...
} TOS_EvtOperArgs_T, *TOS_EvtOperArgs_pT;

TOS_Result_T TOS_registerOperator(/*ARG_OUT*/ TOS_EvtOperID_T*, /*ARG_IN*/TOS_EvtOperArgs_pT);
void TOS_unregisterOperator(/*ARG_IN*/ TOS_EvtOperID_T);
