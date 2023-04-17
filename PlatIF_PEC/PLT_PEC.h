#include "../ThinkOS/TOS_Base4ALL.h"

#ifdef __cplusplus
extern "C" {
#endif

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

typedef enum 
{
    PEC_URI_PROTO_VERSION_AUTO  = 0,
    PEC_URI_PROTO_VERSION_1     = 1UL<<0,
    //PEC_URI_PROTO_VERSION_2     = 1UL<<1,
} PEC_UriProtoVersion_T;

typedef struct 
{
    const char *pKey, *pValue;
} PEC_UriQuery_T, *PEC_UriQuery_pT;

//Reference URI address format: https://en.wikipedia.org/wiki/Uniform_Resource_Identifier
//  Eg: proto://username:password@host:port/path?query#fragment
typedef struct 
{
    //---------------------------------------------------------------------------------------------------------
    //[1]Formatted URI address in single text line, or NULL use UriAddr info to build URI address
    PEC_UriAddr_pT pUriAddrLine;

    //---------------------------------------------------------------------------------------------------------
    //[2]IF pURIAddr is NULL, then use following info to build URI address
    struct 
    {
        PEC_UriProto_T Proto;
        PEC_UriProtoVersion_T ProtoVersion;
        const char *pUsername, *pPassword;//NULL for no username or password
        const char *pHost, *pPath;
        uint16_t Port;

        uint16_t QueryCount;//0 for no query
        PEC_UriQuery_pT pQuery;//NULL for no query
    }UriAddr;

} PEC_UriDesc_T, *PEC_UriDesc_pT;

typedef struct 
{
    PEC_UriDesc_pT pUriDesc;
} PEC_ServiceArgs_T, *PEC_ServiceArgs_pT;

typedef struct 
{
    PEC_UriDesc_pT pUriDesc;
} PEC_ConnArgs_T, *PEC_ConnArgs_pT;

/**
 * @brief 某个实体（Entity）将自己上线为一个服务，即所谓服务对象；
 *          以便其它实体（Entity）调用连接（Connect）方法连接到该服务；
 *          并进行命令（Command）执行或事件（Event）投递类通讯。
 *      SrvObjID: 成功上线的服务对象ID, 用于下线服务和接受连接.
 *      SrvArgs:  服务参数, 包括服务的URI地址.
 *      pOption:  可选参数, NULL为默认参数.
 * 
 * @return TOS_Result_T
 *      TOS_RESULT_SUCCESS: 成功上线服务.
 *      TOS_RESULT_ERROR:   上线服务失败.
 *      TOS_RESULT_INVALID: 无效的服务参数.
 *          SrvArgs.pUriDesc->pUriAddr: 无效的服务URI地址.
 * 
 */
TOS_Result_T PLT_PEC_onlineService
    ( /*ARG_OUT*/PEC_ServiceObjectID_pT, 
      /*ARG_IN*/PEC_ServiceArgs_pT, 
      /*ARG_IN_OPTIONAL*/TOS_BaseOption_pT );
TOS_Result_T PLT_PEC_offlineSecvice(/*ARG_IN*/PEC_ServiceObjectID_T);

TOS_Result_T PLT_PEC_acceptLink(/*ARG_IN*/PEC_ServiceObjectID_T, /*ARG_OUT*/PEC_LinkObjectID_pT, /*ARG_IN_OPTIONAL*/TOS_BaseOption_pT);

TOS_Result_T PLT_PEC_connectLink(/*ARG_OUT*/PEC_LinkObjectID_pT, /*ARG_IN*/PEC_ConnArgs_pT, /*ARG_IN_OPTIONAL*/TOS_BaseOption_pT);
TOS_Result_T PLT_PEC_closeLink(/*ARG_IN*/PEC_LinkObjectID_T);

TOS_Result_T PLT_PEC_execCommand(/*ARG_IN*/PEC_LinkObjectID_T, /*ARG_IN*/TOS_BaseCommand_pT, /*ARG_IN_OPTIONAL*/TOS_BaseOption_pT);

TOS_Result_T PLT_PEC_recvCommand(/*ARG_IN*/PEC_LinkObjectID_T, /*ARG_OUT*/TOS_BaseCommand_pT*, /*ARG_IN_OPTIONAL*/TOS_BaseOption_pT);
TOS_Result_T PLT_PEC_ackCommand(/*ARG_IN*/PEC_LinkObjectID_T, /*ARG_IN*/TOS_BaseCommand_pT);

#if 0
TOS_Result_T PLT_PEC_sendData(/*ARG_IN*/PEC_LinkObjectID_T, /*ARG_IN*/TOS_BaseData_pT, /*ARG_IN_OPTIONAL*/TOS_BaseOption_pT);
TOS_Result_T PLT_PEC_recvData(/*ARG_IN*/PEC_LinkObjectID_T, /*ARG_OUT*/TOS_BaseData_pT*, /*ARG_IN_OPTIONAL*/TOS_BaseOption_pT);

TOS_Result_T PLT_PEC_sendFile(/*ARG_IN*/PEC_LinkObjectID_T, /*ARG_IN*/TOS_BaseFile_pT, /*ARG_IN_OPTIONAL*/TOS_BaseOption_pT);
TOS_Result_T PLT_PEC_recvFile(/*ARG_IN*/PEC_LinkObjectID_T, /*ARG_OUT*/TOS_BaseFile_pT*, /*ARG_IN_OPTIONAL*/TOS_BaseOption_pT);
#endif

#ifdef __cplusplus
}
#endif
