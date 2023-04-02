#ifndef __TOS_BASE_COMMAND_H__
#define __TOS_BASE_COMMAND_H__

typedef enum 
{
    TOS_CMDID_GET_VERSION       = 1,
} TOS_CmdID_T;

typedef struct 
{
    TOS_CmdID_T CmdID;

    void *pCmdPayload;
    long PayloadLength;
    long PayloadBufferSize;

} TOS_BaseCommand_T, *TOS_BaseCommand_pT;

#endif
