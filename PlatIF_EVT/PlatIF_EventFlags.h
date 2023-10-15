//DEFAULT EVENT FLAGS:
//  SoftRealtime: as fast as possible
//      RefMore: TOS_EVTFLAG_HARD_REALTIME
//  ASYNC: postEvent -> enqueueEvent -> EvtMgr's EvtProcessor -> Suber's OpProcessEvent_F
//      RefMore: TOS_EVTFLAG_SYNC

#define TOS_EVTFLAG_HARD_REALTIME    (0UL<<0)
#define TOS_EVTFLAG_SYNC             (0UL<<1)//postEvent -> Suber's OpProcessEvent_F
