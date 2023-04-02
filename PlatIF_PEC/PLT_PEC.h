#include "../ThinkOS/TOS_Base4ALL.h"

typedef TOS_BaseObjectID_T              PEC_ObjectID_T;

typedef PEC_ObjectID_T                  PEC_ServiceObjectID_T;
typedef PEC_ServiceObjectID_T*          PEC_ServiceObjectID_pT;

typedef PEC_ObjectID_T                  PEC_LinkObjectID_T;
typedef PEC_LinkObjectID_T*             PEC_LinkObjectID_pT;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef const char*                     PEC_UriAddr_pT;

typedef enum 
{
    PEC_URI_PROTO_AUTO      = 0,
    PEC_URI_PROTO_API       = 1UL<<0,/*Eg: api:// */
    PEC_URI_PROTO_TCP       = 1UL<<1,/*Eg: tcp:// */
    PEC_URI_PROTO_UDP       = 1UL<<2,/*Eg: udp:// */
    PEC_URI_PROTO_FILE      = 1UL<<3,/*Eg: file:// */
    PEC_URI_PROTO_HTTP      = 1UL<<4,/*Eg: http:// */
    PEC_URI_PROTO_HTTPS     = 1UL<<5,/*Eg: https:// */
} PEC_UriProto_T;

typedef struct 
{
    PEC_UriAddr_pT pUriAddr;//Format following member info in one address line text

    //-------------------------------------------------------------------------
    //PEC_UriProto_T Proto;
    //PEC_UriPRotoVersion_T ProtoVersion;

    //const char *pUsername, *pPassword;
} PEC_UriDesc_T, *PEC_UriDesc_pT;

typedef struct 
{
    PEC_UriDesc_pT pUriDesc;
} PEC_ServiceArgs_T, *PEC_ServiceArgs_pT;

typedef struct 
{
    PEC_UriDesc_pT pUriDesc;
} PEC_ConnArgs_T, *PEC_ConnArgs_pT;


TOS_Result_T PLT_PEC_onlineService(/*ARG_OUT*/PEC_ServiceObjectID_pT, /*ARG_IN*/PEC_ServiceArgs_pT);
TOS_Result_T PLT_PEC_offlineSecvice(/*ARG_IN*/PEC_ServiceObjectID_T);

TOS_Result_T PLT_PEC_acceptLink(/*ARG_IN*/PEC_ServiceObjectID_T, /*ARG_OUT*/PEC_LinkObjectID_pT);

TOS_Result_T PLT_PEC_connectLink(/*ARG_OUT*/PEC_LinkObjectID_pT, /*ARG_IN*/PEC_ConnArgs_pT);
TOS_Result_T PLT_PEC_closeLink(/*ARG_IN*/PEC_LinkObjectID_T);

TOS_Result_T PLT_PEC_execCommand(/*ARG_IN*/PEC_LinkObjectID_T, /*ARG_IN*/TOS_BaseCommand_pT, /*ARG_IN_OPTIONAL*/TOS_BaseOption_pT);

TOS_Result_T PLT_PEC_recvCommand(/*ARG_IN*/PEC_LinkObjectID_T, /*ARG_OUT*/TOS_BaseCommand_pT*, /*ARG_IN_OPTIONAL*/TOS_BaseOption_pT);
TOS_Result_T PLT_PEC_ackCommand(/*ARG_IN*/PEC_LinkObjectID_T, /*ARG_IN*/TOS_BaseCommand_pT);
