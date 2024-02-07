#include <stdint.h>

#include "TOS_BaseObjectTypes.h"

typedef struct 
{
  long RZVD;
}TOS_BaseObjectConfig_T, *TOS_BaseObjectConfig_pT;

typedef struct 
{
    TOS_BaseObjectID_T ID_Self;

    struct
    {
        //TOS_Result_T (*OpSetConfig_F)( TOS_BaseObjectConfig_pT, TOS_BaseOption_pT );
        //TOS_Result_T (*OpGetConfig_F)( TOS_BaseObjectConfig_pT, TOS_BaseOption_pT );
    }Methods;
}TOS_BaseObject_T, *TOS_BaseObject_pT;
