#include "../ThinkOS/TOS_Base4ALL.h"

#ifndef __PLATIF_IOC_EVTID_H__
#define __PLATIF_IOC_EVTID_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  IOC_EVT_CLASS_TEST = 1 << 0ULL,
  // TODO(@W): add more event class here
} IOC_EvtClass_T;

typedef enum {
  IOC_EVT_NAME_TEST_KEEPALIVE = 1 << 0ULL,
  // TODO(@W): add more event name here
} IOC_EvtNameTest_T;

#define IOC_defineEvtID(EvtClass, EvtName) ((IOC_EvtID_T)((EvtClass) | ((EvtName) << 16)))

#define IOC_EVTID_TEST_KEEPALIVE IOC_defineEvtID(IOC_EVT_CLASS_TEST, IOC_EVT_NAME_TEST_KEEPALIVE)
// TODO(@W):

#ifdef __cplusplus
}
#endif
#endif /* __PLATIF_IOC_EVTID_H__ */