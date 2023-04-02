#ifndef __TOS_BASE_OPTION_H__
#define __TOS_BASE_OPTION_H__

typedef enum 
{
    TOS_OPTFLAG_NONBLOCK    = 1UL<<0,

} TOS_OptionFlag_T;

typedef enum
{
    TOS_OPTID_TIMEOUT   = 1,
} TOS_OptionID_T;

typedef struct 
{
    TOS_OptionFlag_T Flags;
    
    TOS_OptionID_T  OptID;
    void *pOptPayload;
    
} TOS_BaseOption_T, *TOS_BaseOption_pT;


#endif
