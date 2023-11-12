typedef enum 
{
    TOS_EVENT_CLASS_TEST                    = (1<<0UL),//RefMore: TOS_EvtTestID_T
    TOS_EVENT_CLASS_SYSTEM                  = (1<<1UL),//RefMore: TOS_EvtSystemID_T
    TOS_EVENT_CLASS_VEHICLE_MAIN            = (1<<2UL),//RefMore: TOS_EvtVehicleMainID_T
    TOS_EVENT_CLASS_VEHICLE_SUB_CHASSIS     = (1<<3UL),//RefMore: TOS_EvtVehicleSubChassisID_T
    TOS_EVENT_CLASS_VEHICLE_SUB_PALLET      = (1<<4UL),//RefMore: TOS_EvtVehicleSubPalletID_T
    
} TOS_EvtClassID_T;

typedef enum 
{
    TOS_EVENT_TEST_KEEPALIVE                = (1<<0UL),//NoData
    TOS_EVENT_TEST_MSGDATA                  = (1<<1UL),//Ues EvtData to carry message data

    TOS_EVENT_TEST_ECHO_REQUEST             = (1<<2UL),//Hello from EvtPuber
    TOS_EVENT_TEST_ECHO_RESPONSE            = (1<<3UL),//HelloAck from EvtSuber in CbProcEvt_F

} TOS_EvtTestID_T;

typedef enum
{
    TOS_EVENT_SYSTEM_KEEPALIVE              = (1<<0UL),//NoData
    TOS_EVENT_SYSTEM_TIMER                  = (1<<1UL),//Ues EvtData to carry timer data
} TOS_EvtSystemID_T;

/**
 * @brief Vehicle State values and transfer events
 *  <EVT::PowerOn> -> [STATE::StartupSelfTesting]
 *                  |-> <EVT::NxSelfTestPassed>      -> [STATE::RemoteReady]
 *                  |-> <EVT::1*xSelfTestFailed>     -> [STATE::ManualMode]
 *                  |-> <EVT::1*xSelfTestExcepted>   -> [STATE::ExceptedNeedReboot]
 * [STATE::RemoteReady]
 *      |-> <EVT::1xRemoteCtrlStartCmdAck>           -> [STATE::RemoteControlling]
 *      |-> <EVT::SwitchManualAck>                   -> [STATE::ManualMode]
 *      |-> <EVT::TimeoutExcepted>                   -> [STATE::ExceptedNeedReboot]
 * [STATE::RemoteControlling]
 *      |-> <EVT::(M-1)xRemoteCtrlStartCmdAck>       -> [STATE::RemoteControlling]
 *      |-> <EVT::(M-1)xRemoteCtrlStopCmdAck>        -> [STATE::RemoteControlling]
 *      |-> <EVT::MxRemoteCtrlContinueCmdAck>        -> [STATE::RemoteControlling]
 *      |-> <EVT::MxRemoteCtrlStopCmdAck>            -> [STATE::RemoteReady]
 *      |-> <EVT::EmergencyFaultEvtAck>              -> [STATE::LockedNeedResolv]
 *      |-> <EVT::TimeoutExcepted>                   -> [STATE::ExceptedNeedReboot]
 * [STATE::LockedNeedResolv]
 *      |-> <EVT::EmergencyLockResolved>             -> [STATE::RemoteReady]
 *      |-> <EVT::FaultLockResolved>                 -> [STATE::ManualMode]
 *      |-> <EVT::TimeoutExcepted>                   -> [STATE::ExceptedNeedReboot]
 * [STATE::ManualMode]
 *      |-> <EVT::SwitchRemoteAck>                   -> [STATE::RemoteReady]
 *      |-> <EVT::ManualCtrlCmdAck>                  -> [STATE::ManualMode]
 *      |-> <EVT::TimeoutExcepted>                   -> [STATE::ExceptedNeedReboot]
 */
typedef enum 
{
    TOS_VState_StartupSelfTesting     = 1,
    TOS_VState_ExceptedNeedReboot,
    TOS_VState_RemoteReady,
    TOS_VState_ManualMode,
    TOS_VState_RemoteControlling,
    TOS_VState_LockedNeedResolv,
} TOS_VehicleStateValue_T; //TODO: Put this enum in PlatIF_State.h

typedef enum 
{
    //EvtData.U8[0]==FromState, EvtData.U8[1]==ToState
    //RefMore: TOS_StateVehicleID_T
    TOS_EVENT_VEHICLE_STATE_TRANSFER        = (1<<0UL),//Ues EvtData to carry vehicle state transfer value 
                                                       // which defined in TOS_VehicleStateValue_T
    //IF initModule's result is TOS_RESULT_ASYNC_SELF_TESTING
    //  following events will be posted by MFuncObj<XYZ> to VMainObj
    //  and will be processed by VMainObj to post TOS_EVENT_VEHICLE_STATE_TRANSFER
    TOS_EVENT_VEHICLE_SELF_TEST_PASS            = (1<<1UL),//EvtData.U8[0]==MFuncObjID
    TOS_EVENT_VEHICLE_SELF_TEST_FAILED          = (1<<2UL),//EvtData.U8[0]==MFuncObjID, EvtData.U8[1]==SelfTestFailedReason
    TOS_EVENT_VEHICLE_SELF_TEST_EXCEPTED        = (1<<3UL),//EvtData.U8[0]==MFuncObjID, EvtData.U8[1]==SelfTestExceptedReason

    TOS_EVENT_VEHICLE_EMERGENCY_STOP            = (1<<4UL),//EvtData.U8[0]==MFuncObjID, EvtData.U8[1]==EmergencyStopReason
    TOS_EVENT_VEHICLE_EMERGENCY_STOP_ACK        = (1<<5UL),

    TOS_EVENT_VEHICLE_FAULT_STOP                = (1<<7UL),//EvtData.U8[0]==MFuncObjID, EvtData.U8[1]==FaultStopReason
    TOS_EVENT_VEHICLE_FAULT_STOP_ACK            = (1<<8UL),

#if 1
    TOS_EVENT_VEHICLE_EMERGENCY_LOCK_RESOLVED   = (1<<9UL),//EvtData.U8[0]==MFuncObjID
    TOS_EVENT_VEHICLE_FAULT_LOCK_RESOLVED       = (1<<10UL),//EvtData.U8[0]==MFuncObjID
#else
    TOS_EVENT_VEHICLE_RESOLVE_EMERGENCY_LOCK    = (1<<9UL),
    TOS_EVENT_VEHICLE_RESOLVE_EMERGENCY_LOCK_ACK= (1<<10UL),

    TOS_EVENT_VEHICLE_RESOLVE_FAULT_LOCK        = (1<<11UL),
    TOS_EVENT_VEHICLE_RESOLVE_FAULT_LOCK_ACK    = (1<<12UL),
#endif

    TOS_EVENT_VEHICLE_SWITCH_MANUAL             = (1<<13UL),
    TOS_EVENT_VEHICLE_SWITCH_MANUAL_ACK         = (1<<14UL),

    TOS_EVENT_VEHICLE_SWITCH_REMOTE             = (1<<15UL),
    TOS_EVENT_VEHICLE_SWITCH_REMOTE_ACK         = (1<<16UL),

    TOS_EVENT_VEHICLE_MC_MOVE_CMD               = (1<<17UL),//PostEvt from MFuncObj<OperatePanel> and ProcEvt by MFuncObj<Chassis>
    TOS_EVENT_VEHICLE_MC_TURN_CMD               = (1<<18UL),
    TOS_EVENT_VEHICLE_MC_UPWARD_CMD             = (1<<19UL),
    TOS_EVENT_VEHICLE_MC_DOWNWARD_CMD           = (1<<20UL),
    TOS_EVENT_VEHICLE_MC_ROTATE_CMD             = (1<<21UL),
    
} TOS_EvtVehicleMainID_T;//RefEvtClass: TOS_EVENT_CLASS_VEHICLE_MAIN

typedef enum 
{
    TOS_EVENT_CHASSIS_RC_START_MOVE_CMD         = (1<<0UL),//PostEvt from MFuncObj<RemoteAgent> and ProcEvt by MFuncObj<Chassis>
    TOS_EVENT_CHASSIS_RC_START_MOVE_CMD_ACK     = (1<<1UL),//PostEvt from MFuncObj<Chassis> and ProcEvt by VMainObj to PostEvt of StateTransfer

    TOS_EVENT_CHASSIS_RC_CONTINUE_MOVE_CMD      = (1<<2UL),//Ref: CHASSIS_RC_START_MOVE_CMD/_ACK
    TOS_EVENT_CHASSIS_RC_CONTINUE_MOVE_CMD_ACK  = (1<<3UL),

    TOS_EVENT_CHASSIS_RC_STOP_MOVE_CMD          = (1<<4UL),//Ref: CHASSIS_RC_START_MOVE_CMD/_ACK
    TOS_EVENT_CHASSIS_RC_STOP_MOVE_CMD_ACK      = (1<<5UL),

    TOS_EVENT_CHASSIS_RC_TRUN_CMD               = (1<<6UL),//Ref: CHASSIS_RC_START_MOVE_CMD/_ACK
    TOS_EVENT_CHASSIS_RC_TRUN_CMD_ACK           = (1<<7UL),

    TOS_EVENT_CHASSIS_NORMAL_STOPPED            = (1<<8UL),//Ref: CHASSIS_RC_STOP_MOVE_CMD/_ACK
    TOS_EVENT_CHASSIS_EMERGENCY_STOPPED         = (1<<9UL),//Ref: TOS_EVENT_VEHICLE_EMERGENCY_STOP/_ACK
    TOS_EVENT_CHASSIS_FAULT_STOPPED             = (1<<10UL),//Ref: TOS_EVENT_VEHICLE_FAULT_STOP/_ACK


} TOS_EvtVehicleSubChassisID_T;//RefEvtClass: TOS_EVENT_CLASS_VEHICLE_SUB_CHASSIS

typedef enum 
{
    TOS_EVENT_PALLET_RC_UPWARD_CMD              = (1<<0UL),//PostEvt from MFuncObj<RemoteAgent> and ProcEvt by MFuncObj<Pallet>
    TOS_EVENT_PALLET_RC_UPWARD_CMD_ACK          = (1<<1UL),//PostEvt from MFuncObj<Pallet> and ProcEvt by VMainObj to PostEvt of StateTransfer

    TOS_EVENT_PALLET_RC_DOWNWARD_CMD            = (1<<2UL),//Ref: PALLET_RC_UPWARD_CMD/_ACK
    TOS_EVENT_PALLET_RC_DOWNWARD_CMD_ACK        = (1<<3UL),

    TOS_EVENT_PALLET_RC_ROTATE_CMD              = (1<<4UL),//Ref: PALLET_RC_UPWARD_CMD/_ACK
    TOS_EVENT_PALLET_RC_ROTATE_CMD_ACK          = (1<<5UL),

    TOS_EVENT_PALLET_NORMAL_STOPPED             = (1<<6UL),//Ref: PALLET_RC_ROTATE_CMD/_ACK
    TOS_EVENT_PALLET_EMERGENCY_STOPPED          = (1<<7UL),//Ref: TOS_EVENT_VEHICLE_EMERGENCY_STOP/_ACK
    TOS_EVENT_PALLET_FAULT_STOPPED              = (1<<8UL),//Ref: TOS_EVENT_VEHICLE_FAULT_STOP/_ACK
    
} TOS_EvtVehicleSubPalletID_T;//RefEvtClass: TOS_EVENT_CLASS_VEHICLE_SUB_PALLET
