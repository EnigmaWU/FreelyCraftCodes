//THIS FILE IS PlatIF_IOC's PUBLIC HEADER FILE.
#include "../ThinkOS/TOS_Base4ALL.h"

/**
 * @brief IOC = Inter-Object Communication
 *  This module is used to communicate between Objects.
 *  Object here is a abstract concept, it is a struct/class instance in a System/Process/Thread.
 *  Object will be instanced as ObjX/ObjY/ObjZ, who will call IOC's API to communicate with other in Message(a.k.a MSG),
 *      such as ObjX send a MSG to ObjY, who processed MSG and send a new MSG to ObjZ.
 *
 *:->Communicate has Connect or Connectless Mode(a.k.a ConMode ConlesMode).
 *      ConMode: 
 *          [1] ObjX MUST call PLT_IOC_onlineService to online a service and identfied as $SrvID.
 *          [2] ObjY MUST call PLT_IOC_connectService to establish a connection to that service and identified as $LinkID,
 *          [3.1] ObjX call PLT_IOC_execCMD with $LinkID to ask ObjY execute commands and get result, or ObjY call PLT_IOC_execCMD.
 *          [3.2] ObjX call PLT_IOC_postEVT with $LinkID to notify ObjY something happened, or ObjY call PLT_IOC_postEVT.
 *          [3.2] ObjX call PLT_IOC_sendDAT with $LinkID to send data to ObjY, or ObjY call PLT_IOC_sendDAT.
 *      ConlesMode: ObjX call PLT_IOC_postEVT with pre-defined $AnonyLinkID to notify all ObjYZs,
 *          who call PLT_IOC_waitEVT or PLT_IOC_subEVT, without PLT_IOC_onlineService and PLT_IOC_connectService.
 *
 *      In ConMode service has dynamic or static online mode:
 *          [D] Dynamic: ObjX call PLT_IOC_onlineService in its context to online a service and identfied as $SrvID,
 *          [S] Static: ObjX use PLT_IOC_defineService in its source to define and identfied by $SrvURL. 
 *
 *:->MSG is a Command(a.k.a CMD) or Event(a.k.a EVT) or Data(a.k.a DAT).
 *      CMD is SYNC and DGRAM defined by IOC identified by CmdID;
 *      EVT is ASYNC and DGRAM defined by IOC identified by EvtID; 
 *      DAT is ASNYC and STREAM defined by IOC knowns only by object pair;
 *
 *:->Call flow examples of ConMode:
 *  [1]: ObjX as service, accept connection from ObjY, and ObjX ask ObjY to execute commands.
 *      ObjX: PLT_IOC_onlineService($SrvURL) -> $SrvID_atObjX
 *      ObjY: PLT_IOC_connectService($SrvURL) -> $LinkID_atObjY
 *        |~> ObjX: PLT_IOC_acceptService($SrvID_atObjX) -> $LinkID_atObjX
 *      ObjX: PLT_IOC_execCMD($LinkID_atObjX, $CmdID) -> ...
 *        |~> ObjY: PLT_IOC_recvCMD($LinkID_atObjY) -> $CmdID ... $CmdResult -> PLT_IOC_ackCMD($LinkID_atObjY)
 *          |~> ObjX: <resume>PLT_IOC_execCMD() -> $CmdResult
 *      ObjX: PLT_IOC_closeLink($LinkID_atObjX)
 *      ObjY: PLT_IOC_closeLink($LinkID_atObjY)
 *      ObjX: PLT_IOC_offlineService($SrvID_atObjX)
 *  [2]
 *      TODO(@W)
 *
 *:->Call flow examples of ConlesMode:
 *  [1]: ObjX subscribe EVT, ObjY post EVT, ObjX process EVT.
 *      ObjX: PLT_IOC_subEVT($AnonyLinkID, $EvtID, $OnEvtProc_F)
 *      ObjY: PLT_IOC_postEVT($AnonyLinkID, $EvtID)
 *        |~> ObjX: $OnEvtProc_F($EvtID)
 *      ObjX: PLT_IOC_unsubEVT($AnonyLinkID)
 *  [2]
 *      TODO(@W)
 */

 /**
  * @brief FSM of IOC Module from Manager's view
  *     TODO(@W)
  */

/**
 * @brief FSM of Service in IOC from User such as ObjXYZ's view
 *  TODO(@W)
 */

/**
 * @brief FSM of Link in IOC from User such as ObjXYZ's view
 *  TODO(@W)
 */

//#include "PlatIF_IOC_Types.h"
//#include "PlatIF_IOC_Interfaces.h"

#ifndef __PLATIF_IOC_H__
#define __PLATIF_IOC_H__
#ifdef __cplusplus
extern "C" {
#endif

//===> Types

typedef struct 
{
    //TOS_BaseModuleArgs_T BaseArgs;
    long RZVD;
} IOC_ModuleArgs_T, *IOC_ModuleArgs_pT;

typedef TOS_BaseObjectID_T      IOC_ServiceID_T;
typedef TOS_BaseObjectID_T      IOC_LinkID_T;
#define AnonyLinkID             ((IOC_LinkID_T)0x20231221UL)

typedef TOS_BaseID64_T          IOC_CmdID_T;
typedef TOS_BaseID64_T          IOC_EvtID_T;

typedef struct 
{
    const char *pSrvURL;
} IOC_SrvArgs_T, *IOC_SrvArgs_pT;

typedef struct
{
    long RZVD;
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
} IOC_EvtPubArgs_T, *IOC_EvtPubArgs_pT;

typedef struct 
{
    long RZVD;
} IOC_EvtSubArgs_T, *IOC_EvtSubArgs_pT;

//===> APIs for ConMode only 
TOS_Result_T PLT_IOC_initModule(/*ARG_OUT*/TOS_ModuleObjectID_T *pModObjID, /*ARG_IN*/const IOC_ModuleArgs_pT pModArgs);
TOS_Result_T PLT_IOC_deinitModule(/*ARG_IN*/TOS_ModuleObjectID_T ModObjID);

TOS_Result_T PLT_IOC_onlineService(/*ARG_OUT*/IOC_ServiceID_T *pSrvID, /*ARG_IN*/const IOC_SrvArgs_pT pSrvArgs);
TOS_Result_T PLT_IOC_acceptService(/*ARG_IN*/IOC_ServiceID_T SrvID, /*ARG_OUT*/IOC_LinkID_T *pLinkID);
TOS_Result_T PLT_IOC_offlineService(/*ARG_IN*/IOC_ServiceID_T SrvID);

TOS_Result_T PLT_IOC_connectService(/*ARG_OUT*/IOC_LinkID_T *pLinkID, /*ARG_IN*/const IOC_SrvArgs_pT pSrvArgs);
TOS_Result_T PLT_IOC_disconnectService(/*ARG_IN*/IOC_LinkID_T LinkID);

TOS_Result_T PLT_IOC_execCMD(/*ARG_IN*/IOC_LinkID_T LinkID, /*ARG_IN*/const IOC_CmdDesc_pT pCmdDesc);
TOS_Result_T PLT_IOC_recvCMD(/*ARG_IN*/IOC_LinkID_T LinkID, /*ARG_OUT*/IOC_CmdDesc_pT pCmdDesc);
TOS_Result_T PLT_IOC_ackCMD(/*ARG_IN*/IOC_LinkID_T LinkID, /*ARG_IN*/const IOC_CmdDesc_pT pCmdDesc);

//===> APIs for both ConMode and ConlesMode
TOS_Result_T PLT_IOC_postEVT(/*ARG_IN*/IOC_LinkID_T LinkID, /*ARG_IN*/const IOC_EvtDesc_pT pEvtDesc);
TOS_Result_T PLT_IOC_waitEVT(/*ARG_IN*/IOC_LinkID_T LinkID, /*ARG_OUT*/IOC_EvtDesc_pT pEvtDesc);

//TOS_Result_T PLT_IOC_pubEVT(/*ARG_IN*/IOC_LinkID_T LinkID, /*ARG_IN*/const IOC_EvtPubArgs_pT pEvtPubArgs);
TOS_Result_T PLT_IOC_subEVT(/*ARG_IN*/IOC_LinkID_T LinkID, /*ARG_IN*/const IOC_EvtSubArgs_pT pEvtSubArgs);
TOS_Result_T PLT_IOC_unsubEVT(/*ARG_IN*/IOC_LinkID_T LinkID);

#ifdef __cplusplus
}
#endif
#endif //__PLATIF_IOC_H__