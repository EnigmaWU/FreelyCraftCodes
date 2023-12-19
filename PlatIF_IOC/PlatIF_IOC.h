//THIS FILE IS PlatIF_IOC's PUBLIC HEADER FILE.
#include "../ThinkOS/TOS_Base4ALL.h"

/**
 * @brief IOC = Inter-Object Communication
 *  This module is used to communicate between objects.
 *  Object is a abstract concept here, it a struct/class instance in System/Process/Thread.
 *  Object call IOC's API to Communicate with other objects in Message.
 *
 *  Communicate use Connect or Connectless mode(a.k.a ConMode ConlesMode).
 *      ConMode: ObjectX MUST call PLT_IOC_connectService to establish a connection named $LinkID to a service,
 *          which is call PLT_IOC_onlineSerivce by ObjectY, before call PLT_IOC_execCMD/_postEVT/_sendDAT to communicating.
 *      ConlesMode: ObjectX call PLT_IOC_execCMD/_postEVT with pre-defined $AnonyLinkID to communicating.
 *
 *  Message is a Command(a.k.a CMD) or Event(a.k.a EVT) or Data(a.k.a DAT).
 *      CMD is SYNC and DGRAM defined by IOC identified by CmdID;
 *      EVT is ASYNC and DGRAM defined by IOC identified by EvtID; 
 *      DAT is ASNYC and STREAM defined by IOC knowns only by object pair;
 */

#ifndef __PLATIF_IOC_H__
#define __PLATIF_IOC_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef TOS_BaseObjectID_T      IOC_ServiceID_T;
typedef TOS_BaseObjectID_T      IOC_LinkID_T;


#ifdef __cplusplus
}
#endif
#endif //__PLATIF_IOC_H__