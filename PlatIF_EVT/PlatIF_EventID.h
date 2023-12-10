#define TOS_EVENT_CLASS_MASK        (0x000000FFUL)
#define TOS_EVENT_ALL_IN_CLASS      (0x00FFFFFFUL)

//USE this to define individual event ID used by TOS_EVT_[sub,pub]Evts() and TOS_EVT_postEvtSRT()
#define TOS_MAKE_EVTID(EvtMetaClassID, EvtMetaClassmateID)                  \
    ((uint32_t)((EvtMetaClassID)&TOS_EVENT_CLASS_MASK)                      \
        | (((uint32_t)(EvtMetaClassmateID)&TOS_EVENT_ALL_IN_CLASS)<<8))

//USE this to define event ID group used by TOS_EVT_subEvts() and TOS_EVT_[sub,pub]Evts()
#define TOS_MAKE_EVTID_GROUP(EvtMetaClassID, EvtMetaClassmateIDs)           \
    (((uint32_t)(EvtMetaClassID)&TOS_EVENT_CLASS_MASK)                      \
        | (((uint32_t)(EvtMetaClassmateIDs)&TOS_EVENT_ALL_IN_CLASS)<<8))

#define TOS_GET_EVTID_CLASS(EvtID)      ((EvtID)&TOS_EVENT_CLASS_MASK)
#define TOS_GET_EVTID_CLASSMATE(EvtID)  (((EvtID)>>8)&TOS_EVENT_ALL_IN_CLASS)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//EVTID_TEST = CLASS_TEST + EVENT_TEST_*
#define TOS_EVTID_TEST_KEEPALIVE \
    TOS_MAKE_EVTID(TOS_EVENT_CLASS_TEST, TOS_EVENT_TEST_KEEPALIVE)
#define TOS_EVTID_TEST_MSGDATA \
    TOS_MAKE_EVTID(TOS_EVENT_CLASS_TEST, TOS_EVENT_TEST_MSGDATA)
#define TOS_EVTID_TEST_ECHO_REQUEST \
    TOS_MAKE_EVTID(TOS_EVENT_CLASS_TEST, TOS_EVENT_TEST_ECHO_REQUEST)
#define TOS_EVTID_TEST_ECHO_RESPONSE \
    TOS_MAKE_EVTID(TOS_EVENT_CLASS_TEST, TOS_EVENT_TEST_ECHO_RESPONSE)
#define TOS_EVTID_TEST_CMD_X1 \
    TOS_MAKE_EVTID(TOS_EVENT_CLASS_TEST, TOS_EVENT_TEST_CMD_X1)
#define TOS_EVTID_TEST_CMD_X1_ACK \
    TOS_MAKE_EVTID(TOS_EVENT_CLASS_TEST, TOS_EVENT_TEST_CMD_X1_ACK)
#define TOS_EVTID_TEST_CMD_X2 \
    TOS_MAKE_EVTID(TOS_EVENT_CLASS_TEST, TOS_EVENT_TEST_CMD_X2)
#define TOS_EVTID_TEST_CMD_X2_ACK \
    TOS_MAKE_EVTID(TOS_EVENT_CLASS_TEST, TOS_EVENT_TEST_CMD_X2_ACK)
#define TOS_EVTID_TEST_CMD_X3 \
    TOS_MAKE_EVTID(TOS_EVENT_CLASS_TEST, TOS_EVENT_TEST_CMD_X3)
#define TOS_EVTID_TEST_CMD_X3_ACK \
    TOS_MAKE_EVTID(TOS_EVENT_CLASS_TEST, TOS_EVENT_TEST_CMD_X3_ACK)
#define TOS_EVTID_TEST_CMD_X4 \
    TOS_MAKE_EVTID(TOS_EVENT_CLASS_TEST, TOS_EVENT_TEST_CMD_X4)
#define TOS_EVTID_TEST_CMD_X4_ACK \
    TOS_MAKE_EVTID(TOS_EVENT_CLASS_TEST, TOS_EVENT_TEST_CMD_X4_ACK)


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//EVTID_VEHICLE = CLASS_VEHICLE_MAIN + EVENT_VEHICLE_*
#define TOS_EVTID_VEHICLE_STATE_TRANSFER \
    TOS_MAKE_EVTID(TOS_EVENT_CLASS_VEHICLE, TOS_EVENT_VEHICLE_STATE_TRANSFER)
//TODO:+More...


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//EVTID_CHASSIS = CLASS_VEHICLE_SUB_CHASSIS + EVENT_CHASSIS_*
//TODO:+More...


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//EVTID_PALLET = CLASS_VEHICLE_SUB_PALLET + EVENT_PALLET_*
//TODO:+More...


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//EVTID_SYSTEM = CLASS_SYSTEM + EVENT_SYSTEM_*
#define TOS_EVTID_SYSTEM_TIMER \
    TOS_MAKE_EVTID(TOS_EVENT_CLASS_SYSTEM, TOS_EVENT_SYSTEM_TIMER)
//TODO(@W):+More...
