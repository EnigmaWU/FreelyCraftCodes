
#include "PlatIF_EventTypes.h"

TOS_Result_T PLT_EVT_subEvts
    (/*ARG_IN*/TOS_EvtOperID_T, /*ARG_IN*/TOS_EvtID_T EvtIDs[], /*ARG_IN*/uint32_t NumIDs, /*ARG_IN*/TOS_EvtSubArgs_pT);

TOS_Result_T PLT_EVT_pubEvts
    (/*ARG_IN*/TOS_EvtOperID_T, /*ARG_IN*/TOS_EvtID_T EvtIDs[], /*ARG_IN*/uint32_t NumIDs);

TOS_Result_T PLT_EVT_postEvtSRT
    (/*ARG_IN*/TOS_EvtOperID_T, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc);

#ifdef CONFIG_BUILD_FEATURE_HARD_REAL_TIME
TOS_Result_T PLT_EVT_postEvtHRT
    (/*ARG_IN*/TOS_EvtOperID_T, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc);
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TOS_Result_T PLT_EVT_registerOperator(/*ARG_OUT*/ TOS_EvtOperID_T*, /*ARG_IN*/const TOS_EvtOperArgs_pT);
void PLT_EVT_unregisterOperator(/*ARG_IN*/ TOS_EvtOperID_T);

TOS_Result_T PLT_EVT_initModule(/*ARG_IN*/const TOS_EvtModuleArgs_pT);
void PLT_EVT_deinitModule(void);

TOS_Result_T PLT_EVT_enableModule(void);
void PLT_EVT_disableModule(void);
