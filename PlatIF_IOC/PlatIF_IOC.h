//THIS FILE IS PlatIF_IOC's PUBLIC HEADER FILE.
#include <stdint.h>

#include "../ThinkOS/TOS_Base4ALL.h"

/**
 * @brief IOC = Inter-Object Communication
 *  This module is used to communicate between Objects.
 *  Object here is a abstract concept, it is a struct/class instance in a System/Process/Thread.
 *  Object will be instanced as ObjX/ObjY/ObjZ, who will call IOC's API to communicate with other in Message(a.k.a MSG),
 *      such as ObjX send a MSG to ObjY, who processed MSG and acked to ObjX, then send a new MSG to ObjZ.
 *
 *  ===================================================================================================================
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
 *  ===================================================================================================================
 *:->MSG is a Command(a.k.a CMD) or an Event(a.k.a EVT) or a piece of Data(a.k.a DAT).
 *  [CMD] is SYNC and DGRAM defined by IOC identified by CmdID;
 *  -------------------------------------------------------------------------------------------------------------------
 *  [EVT] is ASYNC and DGRAM defined by IOC identified by EvtID;
 *    Its default property is ASYNC+MAYBLOCK+NODROP, and may be changed by setLinkParams or IOC_Options_T.
 *    [ASYNC]：means ObjX in its current context postEVT to LinkID,
 *      then ObjY's CbProcEvt_F will be callbacked in IOC's context, not in ObjX's context.
 *      Here IOC's context is designed&implemented by IOC, may be a standalone thread or a thread pool.
 *      USE setLinkParams to change Link's each postEvt to SYNC,
 *      USE IOC_Options_T to change Link's current postEvt to SYNC,
 *          which means ObjY's CbProcEvt_F callbacked in ObjX's context.
 *    [MAYBLOCK]: means ObjX's postEVT may be blocked if not enough resource to postEVT,
 *      such as no free space to pending buffer the pEvtDesc.
 *      USE setLinkParams to change Link's each postEvt to NONBLOCK,
 *      USE IOC_Options_T to change Link's current postEvt to NONBLOCK,
 *          by set enable timeout checking and with timeout value '0',
 *          which means ObjX's postEVT will return TOS_RESULT_TIMEOUT if not enough resource to postEVT.
 *    [NODROP]: means after ObjX's postEVT success, if IOC's internal MAY drop this EVT,
 *      such as IOC's internal subsystem or submodule is busy or not enough resource to process this EVT.
 *      Here assume IOC is a complex system, such as ObjX vs ObjY is inter-process or inter-machine communication.
 *      USE setLinkParams to change Link's each postEvt to MAYDROP,
 *      USE IOC_Options_T to change Link's current postEvt to MAYDROP,
 *          which means ObjX's postEVT success, but sometimes ObjY never get this EVT.
 *  -------------------------------------------------------------------------------------------------------------------
 *  [DAT] is ASNYC and STREAM defined by IOC knowns only by object pair;
 *
 *  ===================================================================================================================
 *:->Link+MSG has Single or Hybrid Mode(a.k.a S-Mode vs H-Mode).
 *      S-Mode: means a $LinkID ONLY used by ONE of execCMD or postEVT or sendDAT.
 *          This S-Mode is enabled by default, equals to CONFIG_BUILD_ENABLE_IOC_LINK_SINGLE_MODE.
 *      H-Mode: means a $LinkID MAY used by ANY execCMD or postEVT or sendDAT.
 *          This H-Mode is enabled by CONFIG_BUILD_ENABLE_IOC_LINK_HYBRID_MODE.
 *
 *:->Notes:
 *      ...
 *
 *  ===================================================================================================================
 *:->Call flow typical examples of ConetMode:
 *  [1]: ObjX as service, accept connection from ObjY, and ObjX ask ObjY to execute commands.
 *      ObjX: PLT_IOC_onlineService($SrvArgs::SrvURL) -> $SrvID_atObjX
 *      ObjY: PLT_IOC_connectService($LinkArgs::SrvURL) -> $LinkID_atObjY
 *        |~> ObjX: PLT_IOC_acceptLink($SrvID_atObjX) -> $LinkID_atObjX
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
 *  ===================================================================================================================
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
 *     <ACT:initModule> -> [<T>STATE:ModuleStateIniting]
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
 * TODO(@W): @ConetMode_of[CMD,EVT,DAT]:
 *
 * --------------------------------------------------------------------------------------------------------------------
 * @ConlesMode_of[EVT]:
 *      <ACT:_initCRuntimeSuccess>  -> [STATE:LinkStateReady]
 *
 *      [STATE:LinkStateReady]  ->  <ACT:subEvt/unsubEvt>   -> [STATE:LinkStateReady]
 *
 *      [STATE:LinkStateReady]  ->  <ACT:postEvt>
 *          |-> <EVT:enterCbProcEvt_F>  ->  [STATE:LinkStateBusy]
 *      [STATE:LinkStateBusy]
 *          |-> <EVT:leaveCbProcEvt_F>  ->  [STATE:LinkStateReady]
 */
typedef enum 
{
    RZVD2,
} IOC_LinkStateValue_T;

enum IOC_PreDefinedConst {
  IOC_CONLESMODE_AUTO_LINK_ID = 0x20231227UL,  // a.k.a $AutoLinkID
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

typedef enum {
  IOC_MSGFLAG_MAYDROP = 1 << 0,  // set this flag to allow drop this message if no enough resource.
} IOC_MsgFlags_T;

typedef struct
{
    unsigned long SeqID;
    IOC_MsgFlags_T Flags;
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
    IOC_CbProcEvt_F CbProcEvt_F;
    void *pCbPriv;
} IOC_EvtUnsubArgs_T, *IOC_EvtUnsubArgs_pT;

typedef enum {
  IOC_OPTID_TIMEOUT = 1 << 0,  // set this IDs and Payload.TimeoutUS>=0, to set timeout for execCMD,waitCMD,sendDAT,recvDAT,...
  // TODO(@W): +More...
} IOC_OptionsID_T;

typedef struct 
{
    IOC_OptionsID_T IDs;

    union {
      uint64_t RZVD[8];  // reserve for MAX payload size.
      uint32_t TimeoutUS;
    } Payload;

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
TOS_Result_T PLT_IOC_acceptLink(
    /*ARG_IN*/ IOC_ServiceID_T SrvID,
    /*ARG_OUT*/ IOC_LinkID_T *pLinkID,
    /*ARG_IN_OPTIONAL*/ IOC_Options_pT pOptions);
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
/**
 * @brief ObjX call PLT_IOC_postEVT to post EVT to LinkID, the EVT is described by pEvtDesc.
 *     Any ObjY who listenEVT this LinkID will get every EVT copy into its pEvtDesc and process the EVT in ObjY's own context,
 *  OR Any ObjY who subEVT this LinkID, and matched $EvtIDs will be callbacked via $CbProcEvt_F with pEvtDesc argment.
 *
 * @param LinkID: LinkID to post EVT used by EvtPuber, and to subscribe EVT used by EvtSuber.
 *          In ConetMode, LinkID is returned by PLT_IOC_connectService or PLT_IOC_acceptLink.
 *          In ConlesMode, LinkID is pre-defined IOC_CONLESMODE_AUTO_LINK_ID.
 * @param pEvtDesc: EVT to post, RefMore: IOC_EvtDesc_T.
 *      IF postEvt SUCCESS, pEvtDesc is copied away by IOC, and ObjX can reuse it.
 * @param pOptions: Optional options, RefMore: IOC_Options_T.
 *      IF NULL, default options will be used, which means EVT's default properity, or effective properity set by setLinkParam.
 *      IF NOT NULL, NONBLOCK, MAYDROP, TIMEOUT, ... can be set to change EVT's properity by each postEVT.
 * @return TOS_RESULT_SUCCESS: post EVT success.
 *         TOS_RESULT_TIMEOUT: post EVT timeout, only valid when pOptions->IDs has IOC_OPTID_TIMEOUT.
 */
TOS_Result_T PLT_IOC_postEVT(
    /*ARG_IN*/IOC_LinkID_T LinkID, 
    /*ARG_IN*/const IOC_EvtDesc_pT pEvtDesc,
    /*ARG_IN_OPTIONAL*/IOC_Options_pT);
TOS_Result_T PLT_IOC_listenEVT(
    /*ARG_IN*/IOC_LinkID_T LinkID, 
    /*ARG_OUT*/IOC_EvtDesc_pT pEvtDesc,
    /*ARG_IN_OPTIONAL*/IOC_Options_pT);

//TOS_Result_T PLT_IOC_pubEVT(/*ARG_IN*/IOC_LinkID_T LinkID, /*ARG_IN*/const IOC_EvtPubArgs_pT pEvtPubArgs);

/**
 * @brief ObjX call PLT_IOC_subEvt to subscribe EVT, when post EVT to LinkID, ObjX's $CbProcEvt_F will be callbacked.
 * @param LinkID: LinkID to subscribe EVT used by EvtSuber, and to post EVT used by EvtPuber.
 *          In ConetMode, LinkID is returned by PLT_IOC_connectService or PLT_IOC_acceptLink.
 *          In ConlesMode, LinkID is pre-defined IOC_CONLESMODE_AUTO_LINK_ID.
 * @param pEvtSubArgs: EvtSuber's $CbProcEvt_F and $pCbPriv, and $EvtIDs to subscribe.
 *
 * @return TOS_RESULT_SUCCESS: subscribe EVT success.
 *      TOS_RESULT_INVALID_LINKID/_ARGS: invalid LinkID or pEvtSubArgs.
 *      TOS_RESULT_NOT_ENOUGH_RESOURCE: no enough resource to subscribe EVT, too many EvtSubers or EvtIDs.
 *      TOS_RESULT_ALREADY_SUBSCRIBED: already subscribed EVT.
 *
 * @note n/a
 */
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