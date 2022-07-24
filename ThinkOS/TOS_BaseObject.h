#include <stdint.h>

typedef int64_t TOS_Result_T;
typedef uint64_t TOS_BaseObjectID_T;

typedef struct 
{

}TOS_BaseObjectConfig_T, *TOS_BaseObjectConfig_pT;

typedef struct 
{
    TOS_BaseObjectID_T ID_Self;

    struct
    {
        TOS_Result_T (*OpSetConfig_F)( TOS_BaseObjectConfig_pT, TOS_BaseOption_pT );
        TOS_Result_T (*OpGetConfig_F)( TOS_BaseObjectConfig_pT, TOS_BaseOption_pT );
    }Methods;
}TOS_BaseObject_T, *TOS_BaseObject_pT;
