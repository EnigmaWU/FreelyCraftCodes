//DEFAULT EVENT FLAGS means EvtDesc.EvtFlags=0UL:
//  TOS_EVTFLAG_SOFT_REAL_TIME: as fast as possible/Priority
//      RefMore: TOS_EVTFLAG_HARD_REALTIME
//  TOS_EVTFLAG_ASYNC: postEvent -> enqueueEvent -> EvtMgr's EvtProcessor -> Suber's OpProcessEvent_F
//      RefMore: TOS_EVTFLAG_SYNC
//  TOS_EVTFLAG_NONBLOCK: from post to enqueue, to process, all non-blocking
//      RefMore: TOS_EVTFLAG_MAY_SLEEP
//  TOS_EVTFLAG_NO_DROP: from post to enqueue, to process, all no-drop
//      RefMore: TOS_EVTFLAG_MAY_DROP

#define TOS_EVTFLAG_HARD_REAL_TIME   (0UL<<0)//RefDft: TOS_EVTFLAG_SOFT_REAL_TIME
#define TOS_EVTFLAG_SYNC             (0UL<<1)//RefDft: TOS_EVTFLAG_ASYNC
#define TOS_EVTFLAG_MAY_SLEEP        (0UL<<2)//RefDft: TOS_EVTFLAG_NONBLOCK
#define TOS_EVTFLAG_MAY_DROP         (0UL<<3)//RefDft: TOS_EVTFLAG_NO_DROP