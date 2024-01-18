//THIS FILE IS PlatIF_IOC's PUBLIC HEADER FILE.
#include "../ThinkOS/TOS_Base4ALL.h"

/**
 * @brief IOC = Inter-Object Communication
 *  This module is used to communicate between Objects.
 *  Object here is a abstract concept, it is a struct/class instance in a System/Process/Thread.
 *  Object will be instanced as ObjX/ObjY/ObjZ, who will call IOC's API to communicate with other in Message(a.k.a MSG),
 *      such as ObjX send a MSG to ObjY, who processed MSG and acked to ObjX, then send a new MSG to ObjZ.
 *
 *:->Communicate has Connect or Connectless Mode(a.k.a ConetMode ConlesMode).
 *     @ConetMode@:
 *       [1] ObjX MUST call PLT_IOC_onlineService to online a service with $SrvArgs and identfied as $SrvID.
 *       [2] ObjY MUST call PLT_IOC_connectService to that service, and both ObjX/Y will get a $LinkID,
 *       [3.1] ObjY call PLT_IOC_execCMD with $LinkID to ask ObjX execute commands and get result,
 *          or ObjX call PLT_IOC_execCMD.
 *       [3.2] ObjX call PLT_IOC_postEVT with $LinkID to notify ObjY something happened,
 *          or ObjY call PLT_IOC_postEVT.
 *       [3.3] ObjX call PLT_IOC_sendDAT with $LinkID to send data to ObjY, or ObjY call PLT_IOC_sendDAT.
 *     @ConlesMode@: ObjX call PLT_IOC_postEVT with pre-defined $AutoLinkID to notify all ObjYZs,
 *          who call PLT_IOC_listenEVT or PLT_IOC_subEVT, without PLT_IOC_onlineService and PLT_IOC_connectService.
 *
 *      In ConetMode service has dynamic or static online mode:
 *          [D] Dynamic: ObjX call PLT_IOC_onlineService in its context to online a service and identfied as $SrvID,
 *          [S] Static: ObjX use PLT_IOC_defineService in its source to define and identfied by $SrvArgs::SrvURL.
 *
 *:->MSG is a Command(a.k.a CMD) or Event(a.k.a EVT) or Data(a.k.a DAT).
 *      CMD is SYNC and DGRAM defined by IOC identified by CmdID;
 *      EVT is ASYNC and DGRAM defined by IOC identified by EvtID;
 *      DAT is ASNYC and STREAM defined by IOC knowns only by object pair;
 *
 *:->Link+MSG has Single or Hybrid Mode(a.k.a S-Mode vs H-Mode).
 *      S-Mode: means a $LinkID ONLY used by ONE of execCMD or postEVT or sendDAT.
 *          This S-Mode is enabled by default, equals to CONFIG_BUILD_ENABLE_IOC_LINK_SINGLE_MODE.
 *      H-Mode: means a $LinkID MAY used by ANY execCMD or postEVT or sendDAT.
 *          This H-Mode is enabled by CONFIG_BUILD_ENABLE_IOC_LINK_HYBRID_MODE.
 *
 *:->Notes:
 *      ...
 *
 *:->Call flow typical examples of ConetMode:
 *  [1]: ObjX as service, accept connection from ObjY, and ObjX ask ObjY to execute commands.
 *      ObjX: PLT_IOC_onlineService($SrvArgs::SrvURL) -> $SrvID_atObjX
 *      ObjY: PLT_IOC_connectService($LinkArgs::SrvURL) -> $LinkID_atObjY
 *        |~> ObjX: PLT_IOC_acceptService($SrvID_atObjX) -> $LinkID_atObjX
 *      ObjX: PLT_IOC_execCMD($LinkID_atObjX, $CmdID) -> ...
 *        |~> ObjY: PLT_IOC_recvCMD($LinkID_atObjY) -> $CmdID ... $CmdResult -> PLT_IOC_ackCMD($LinkID_atObjY)
 *          |~> ObjX: <resume>PLT_IOC_execCMD() -> $CmdResult
 *      ObjX: PLT_IOC_closeLink($LinkID_atObjX)
 *      ObjY: PLT_IOC_closeLink($LinkID_atObjY)
 *      ObjX: PLT_IOC_offlineService($SrvID_atObjX)
 *  [2]: TODO(@W)
 *  [3]: TODO(@W)
 *      ...
 *
 *:->Call flow typical examples of ConlesMode:
 *  [1]: ObjX as EvtSuber subscribe EVT, ObjY as EvtPuber post EVT, ObjX process EVT posted by ObjY.
 *      ObjX: PLT_IOC_subEVT($AutoLinkID, $EvtDesc::[EvtID, $CbProcEvt_F])
 *      ObjY: PLT_IOC_postEVT($AutoLinkID, $EvtDesc::EvtID)
 *        |~> ObjX: $CbPrcEvt_F($EvtID)
 *      ObjX: PLT_IOC_unsubEVT($AutoLinkID)
 *
 *  [2]: Obj[A-E], ObjA is MainStateObject, ObjB is RemoteCommandObject, ObjC is VerticalMoveObject,
 *          ObjD is HorizontalMoveObject, ObjE is CollisionDetectObject.
 *      ObjA: PLT_IOC_subEvt from ObjB/E of EvtID_KEEPALIVE,
 *          from ObjC of EvtID_KEEPALIVE/EvtID_MOVE_VERTICAL_ACK/EvtID_COLLISION_DETECT_ACK,
 *          from ObjD of EvtID_KEEPALIVE/EvtID_MOVE_HORIZONTAL_ACK/EvtID_COLLISION_DETECT_ACK.
 *      ObjB: PLT_IOC_postEVT to ObjA of EvtID_KEEPALIVE, to ObjC of EvtID_MOVE_VERTICAL,
 *          to ObjD of EvtID_MOVE_HORIZONTAL.
 *      ObjC: PLT_IOC_subEvt from ObjB of EvtID_MOVE_VERTICAL, from ObjE of EvtID_COLLISION_DETECT,
 *          PLT_IOC_postEVT to ObjA of EvtID_KEEPALIVE/EvtID_MOVE_VERTICAL_ACK/EvtID_COLLISION_DETECT_ACK.
 *      ObjD: PLT_IOC_subEvt from ObjB of EvtID_MOVE_HORIZONTAL, from ObjE of EvtID_COLLISION_DETECT,
 *          PLT_IOC_postEVT to ObjA of EvtID_KEEPALIVE/EvtID_MOVE_HORIZONTAL_ACK/EvtID_COLLISION_DETECT_ACK.
 *      ObjE: PLT_IOC_postEVT to ObjA of EvtID_KEEPALIVE, to ObjC/D of EvtID_COLLISION_DETECT.
 *
 *  [3]: TODO(@W)
 *      ...
 */

#ifndef __PLATIF_IOC_H__
#define __PLATIF_IOC_H__
#ifdef __cplusplus
extern "C" {
#endif

//===> Types


 /**
  * @brief FSM of IOC Module from Manager's view
  *     <ACT:initModuel> -> [<T>STATE:ModuleStateIniting]
  *         |-> <EVT:initModuleSuccess> -> [STATE:ModuleStateReady]
  *         |-> <EVT:initModuleFailed>  -> [STATE:ModuleStateDeinited]
  *         |-> <EVT:initModuleExcepted>-> [STATE:ModuleStateExcepted]
  * 
  *     [STATE:ModuleStateReady] -> <ACT:deinitModule> -> [<T>STATE:ModuleStateDeiniting]
  *         |-> <EVT:deinitModuleSuccess> -> [STATE:ModuleStateDeinited]
  *         |-> <EVT:deinitModuleFailed>  -> [STATE:ModuleStateReady]
  *         |-> <EVT:deinitModuleExcepted>-> [STATE:ModuleStateExcepted]
  *
  *     [STATE:ModuleStateReady] -> <ACT:online/connectService> -(IF:RefCnt==0)-> [<T>STATE:ModuleStateReady2Busy]
  *         |-> <EVT:online/connectServiceSuccess> -> [STATE:ModuleStateBusy(RefCnt=1)]
  *         |-> <EVT:online/connectServiceFailed>  -> [STATE:ModuleStateReady]
  *         |-> <EVT:online/connectServiceExcepted>-> [STATE:ModuleStateExcepted]
  *     
  *     [STATE:ModuleStateBusy] -> <ACT:offlineService/closeLink> -(IF:RefCnt==1)-> [<T>STATE:ModuleStateBusy2Ready]
  *         |-> <EVT:offlineService/closeLinkSuccess> -> [STATE:ModuleStateReady(RefCnt=0)]
  *         |-> <EVT:offlineService/closeLinkFailed>  -> [STATE:ModuleStateBusy]
  *         |-> <EVT:offlineService/closeLinkExcepted>-> [STATE:ModuleStateExcepted]
  *
  *     [STATE:ModuleStateBusy] -> <ACT:online/connectService> -(IF:RefCnt>=1)-> [<T>STATE:ModuleStateBusy]
  *         |-> <EVT:online/connectServiceSuccess> -> [STATE:ModuleStateBusy(RefCnt+=1)]
  *         |-> <EVT:online/connectServiceFailed>  -> [STATE:ModuleStateBusy]
  *         |-> <EVT:online/connectServiceExcepted>-> [STATE:ModuleStateExcepted]
  *
  *     [STATE:ModuleStateBusy] -> <ACT:offlineService/closeLink> -(IF:RefCnt>1)-> [<T>STATE:ModuleStateBusy]
  *         |-> <EVT:offlineService/closeLinkSuccess> -> [STATE:ModuleStateBusy(RefCnt-=1)]
  *         |-> <EVT:offlineService/closeLinkFailed>  -> [STATE:ModuleStateBusy]
  *         |-> <EVT:offlineService/closeLinkExcepted>-> [STATE:ModuleStateExcepted]
  */

typedef enum 
{
    IOC_ModuleStateIniting = 1,
    IOC_ModuleStateReady,
    IOC_ModuleStateReady2Busy,
    IOC_ModuleStateBusy,
    IOC_ModuleStateBusy2Ready,
    IOC_ModuleStateDeiniting,
    IOC_ModuleStateDeinited,
    IOC_ModuleStateExcepted,
} IOC_ModuleStateValue_T;

/**
 * @brief FSM of Service in IOC from User such as ObjXYZ's view
 *  TODO(@W)
 */
typedef enum 
{
    RZVD1,
} IOC_ServiceStateValue_T;

/**
 * @brief FSM of Link in IOC from User such as ObjXYZ's view
 *  TODO(@W)
 */
typedef enum 
{
    RZVD2,
} IOC_LinkStateValue_T;

enum IOC_PreDefinedConst {
  IOC_CONLESMODE_AUTO_LINK_ID = 0x20231227UL,
};

typedef struct 
{
    //TOS_BaseModuleArgs_T BaseArgs;

    // TODO: ConlesModeSuberMaxNum;
    long RZVD;
} IOC_ModuleArgs_T, *IOC_ModuleArgs_pT;

typedef TOS_BaseObjectID_T      IOC_ServiceID_T;
typedef TOS_BaseObjectID_T      IOC_LinkID_T;
// #define AutoLinkID             ((IOC_LinkID_T)0x20231221UL)

typedef TOS_BaseID64_T          IOC_CmdID_T;
// #include "PlatIF_IOC_CmdID.h"

typedef TOS_BaseID64_T          IOC_EvtID_T;
#include "PlatIF_IOC_EvtID.h"

typedef struct 
{
    const char *pSrvURL;
} IOC_SrvArgs_T, *IOC_SrvArgs_pT;

typedef struct 
{
    const char *pSrvURL;
} IOC_LinkArgs_T, *IOC_LinkArgs_pT;

typedef struct
{
    unsigned long SeqID;
} IOC_MsgDesc_T, *IOC_MsgDesc_pT;
typedef struct 
{
    //MsgCommon
    IOC_MsgDesc_T MsgDesc;

    //CmdSpecific
    IOC_CmdID_T CmdID;
    TOS_Result_T CmdResult;
    //TOOD(@W): +More...
} IOC_CmdDesc_T, *IOC_CmdDesc_pT;

typedef struct 
{
    //MsgCommon
    IOC_MsgDesc_T MsgDesc;

    //EvtSpecific
    IOC_EvtID_T EvtID;
    //TOOD(@W): +More...
} IOC_EvtDesc_T, *IOC_EvtDesc_pT;

typedef struct 
{
    long RZVD;
} IOC_DatDesc_T, *IOC_DatDesc_pT;

typedef struct 
{
    long RZVD;
} IOC_EvtPubArgs_T, *IOC_EvtPubArgs_pT;

typedef TOS_Result_T (*IOC_CbProcEvt_F)(IOC_EvtDesc_pT pEvtDesc, void *pCbPriv);
typedef struct 
{
    IOC_CbProcEvt_F CbProcEvt_F;
    void *pCbPriv;
    unsigned long EvtNum;
    IOC_EvtID_T *pEvtIDs;
} IOC_EvtSubArgs_T, *IOC_EvtSubArgs_pT;

typedef struct {
    unsigned long EvtNum;
    IOC_EvtID_T *pEvtIDs;
} IOC_EvtUnsubArgs_T, *IOC_EvtUnsubArgs_pT;

typedef struct 
{
    //OPT: Timeout,NonBlock,MayDrop, ...
    long RZVD;
} IOC_Options_T, *IOC_Options_pT;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//===> APIs for ConetMode only
TOS_Result_T PLT_IOC_initModule(
    /*ARG_OUT*/TOS_ModuleObjectID_T *pModObjID, 
    /*ARG_IN*/const IOC_ModuleArgs_pT pModArgs);
TOS_Result_T PLT_IOC_deinitModule(/*ARG_IN*/TOS_ModuleObjectID_T ModObjID);

//---> ConetMode: Dynamic Service
TOS_Result_T PLT_IOC_onlineService(
    /*ARG_OUT*/IOC_ServiceID_T *pSrvID, 
    /*ARG_IN*/const IOC_SrvArgs_pT pSrvArgs);
TOS_Result_T PLT_IOC_acceptService(
    /*ARG_IN*/IOC_ServiceID_T SrvID, 
    /*ARG_OUT*/IOC_LinkID_T *pLinkID,
    /*ARG_IN_OPTIONAL*/IOC_Options_pT pOptions);
TOS_Result_T PLT_IOC_offlineService(/*ARG_IN*/IOC_ServiceID_T SrvID);

//---> ConetMode: Static Service
// TODO(@W): #define PLT_IOC_defineService(pSrvArgs)

TOS_Result_T PLT_IOC_connectService(
    /*ARG_OUT*/IOC_LinkID_T *pLinkID, 
    /*ARG_IN*/const IOC_LinkArgs_pT pLinkArgs,
    /*ARG_IN_OPTIONAL*/IOC_Options_pT pOptions);
TOS_Result_T PLT_IOC_closeLink(/*ARG_IN*/IOC_LinkID_T LinkID);

TOS_Result_T PLT_IOC_execCMD(
    /*ARG_IN*/IOC_LinkID_T LinkID, 
    /*ARG_IN*/const IOC_CmdDesc_pT pCmdDesc,
    /*ARG_IN_OPTIONAL*/IOC_Options_pT);
TOS_Result_T PLT_IOC_waitCMD(
    /*ARG_IN*/IOC_LinkID_T LinkID, 
    /*ARG_OUT*/IOC_CmdDesc_pT pCmdDesc,
    /*ARG_IN_OPTIONAL*/IOC_Options_pT);
TOS_Result_T PLT_IOC_ackCMD(
    /*ARG_IN*/IOC_LinkID_T LinkID, 
    /*ARG_IN*/const IOC_CmdDesc_pT pCmdDesc,
    /*ARG_IN_OPTIONAL*/IOC_Options_pT);

TOS_Result_T PLT_IOC_sendDAT(
    /*ARG_IN*/IOC_LinkID_T LinkID, 
    /*ARG_IN*/IOC_DatDesc_pT,
    /*ARG_IN_OPTIONAL*/IOC_Options_pT);
TOS_Result_T PLT_IOC_recvDAT(
    /*ARG_IN*/IOC_LinkID_T LinkID, 
    /*ARG_OUT*/IOC_DatDesc_pT,
    /*ARG_IN_OPTIONAL*/IOC_Options_pT);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//===> APIs for both ConetMode and ConlesMode
TOS_Result_T PLT_IOC_postEVT(
    /*ARG_IN*/IOC_LinkID_T LinkID, 
    /*ARG_IN*/const IOC_EvtDesc_pT pEvtDesc,
    /*ARG_IN_OPTIONAL*/IOC_Options_pT);
TOS_Result_T PLT_IOC_listenEVT(
    /*ARG_IN*/IOC_LinkID_T LinkID, 
    /*ARG_OUT*/IOC_EvtDesc_pT pEvtDesc,
    /*ARG_IN_OPTIONAL*/IOC_Options_pT);

//TOS_Result_T PLT_IOC_pubEVT(/*ARG_IN*/IOC_LinkID_T LinkID, /*ARG_IN*/const IOC_EvtPubArgs_pT pEvtPubArgs);
TOS_Result_T PLT_IOC_subEVT(
    /*ARG_IN*/IOC_LinkID_T LinkID, 
    /*ARG_IN*/const IOC_EvtSubArgs_pT pEvtSubArgs);
TOS_Result_T PLT_IOC_unsubEVT(
    /*ARG_IN*/ IOC_LinkID_T LinkID,
    /*ARG_IN_OPTIONAL*/ const IOC_EvtUnsubArgs_pT pEvtUnsubArgs);

#ifdef __cplusplus
}
#endif
#endif //__PLATIF_IOC_H__