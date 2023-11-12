#define TOS_MAKE_EVTID(EvtMetaClassID, EvtMetaClassmateID) \
    (((uint32_t)(EvtMetaClassID)&0xFF) | (((uint32_t)(EvtMetaClassmateID)&0x00FFFFFF)<<8))

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
//TODO:+More...
