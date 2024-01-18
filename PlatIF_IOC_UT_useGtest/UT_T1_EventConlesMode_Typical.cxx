#include "_UT_IOC_Common.h"

// This is IOC_Event in ConlesMode's typical UT from API user's perspective
//===>Case[01]: Ref::ConlesMode's call flow typical example[1] in PlatIF_IOC.h
//-Step1: EvtSuber as ObjX subEvt(EvtID==IOC_EVTID_TEST_KEEPALIVE)
//-Step2: EvtPuber as ObjY postEVT(EvtID==IOC_EVTID_TEST_KEEPALIVE * $_UT_EVTCNT_KEEPALIVE)
//-Step3: EvtSuber's EvtArgs::CbProcEvt_F is called $_UT_EVTCNT_KEEPALIVE times
//-Step4: EvtSuber unsubEvt(EvtID==IOC_EVTID_TEST_KEEPALIVE)
//-Step5: EvtPuber postEVT(EvtID==IOC_EVTID_TEST_KEEPALIVE * $_UT_EVTCNT_KEEPALIVE) again
//-Step6:EvtSuber's EvtArgs::CbProcEvt_F is NOT called more, stile $_UT_EVTCNT_KEEPALIVE times

#define _UT_EVTCNT_KEEPALIVE 1024

typedef struct _UT_Case01_EvtSuberPriv_T {
  unsigned long KeepAliveEvtCnt;
} _UT_Case01_EvtSuberPriv_T, *_UT_Case01_EvtSuberPriv_pT;

static TOS_Result_T _UT_Case01_EvtSuberCbProcEvt_F(IOC_EvtDesc_pT pEvtDesc, void *pCbPriv) {
  _UT_Case01_EvtSuberPriv_pT pEvtSuberPriv = (_UT_Case01_EvtSuberPriv_pT)pCbPriv;
  pEvtSuberPriv->KeepAliveEvtCnt++;
  return TOS_RESULT_SUCCESS;
}

TEST(EventConlesModeTypical, Case01) {
  IOC_LinkID_T LinkID = IOC_CONLESMODE_AUTO_LINK_ID;

  //===>Step1: EvtSuber as ObjX subEvt(EvtID==IOC_EVTID_TEST_KEEPALIVE)
  _UT_Case01_EvtSuberPriv_T EvtSuberPriv = {.KeepAliveEvtCnt = 0};
  IOC_EvtID_T SubEvtIDs[] = {IOC_EVTID_TEST_KEEPALIVE};
  IOC_EvtSubArgs_T EvtSubArgs = {.CbProcEvt_F = _UT_Case01_EvtSuberCbProcEvt_F,
                                 .pCbPriv = &EvtSuberPriv,
                                 .EvtNum = TOS_calcArrayElmtCnt(SubEvtIDs),
                                 .pEvtIDs = SubEvtIDs};
  TOS_Result_T Result = PLT_IOC_subEVT(LinkID, &EvtSubArgs);
  ASSERT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint

  //===>Step2: EvtPuber as ObjY postEVT(EvtID==IOC_EVTID_TEST_KEEPALIVE*$_UT_EVTCNT_KEEPALIVE)
  for (int i = 0; i < _UT_EVTCNT_KEEPALIVE; i++) {
    IOC_EvtDesc_T EvtDesc = {.EvtID = IOC_EVTID_TEST_KEEPALIVE};
    Result = PLT_IOC_postEVT(LinkID, &EvtDesc, NULL);
    ASSERT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint
  }

  //===>Step3: EvtSuber's EvtArgs::CbProcEvt_F is called $$_UT_EVTCNT_KEEPALIVE times
  ASSERT_EQ(_UT_EVTCNT_KEEPALIVE, EvtSuberPriv.KeepAliveEvtCnt);  //@CheckPoint

  //===>Step4: EvtSuber unsubEvt(EvtID==IOC_EVTID_TEST_KEEPALIVE)
  Result = PLT_IOC_unsubEVT(LinkID, NULL);
  ASSERT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint

  //===>Step5: EvtPuber postEVT(EvtID==IOC_EVTID_TEST_KEEPALIVE*$_UT_EVTCNT_KEEPALIVE) again
  for (int i = 0; i < _UT_EVTCNT_KEEPALIVE; i++) {
    IOC_EvtDesc_T EvtDesc = {.EvtID = IOC_EVTID_TEST_KEEPALIVE};
    Result = PLT_IOC_postEVT(LinkID, &EvtDesc, NULL);
    ASSERT_EQ(TOS_RESULT_NO_EVT_SUBER, Result);  //@CheckPoint
  }

  //===>Step6:EvtSuber's EvtArgs::CbProcEvt_F is NOT called
  ASSERT_EQ(_UT_EVTCNT_KEEPALIVE, EvtSuberPriv.KeepAliveEvtCnt);  //@CheckPoint
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//===>Case[02]: Ref::ConlesMode's call flow typical example[2] in PlatIF_IOC.h
// Step-1: ObjA as EvtSuber subEvt(EvtID==IOC_EVTID_TEST_KEEPALIVE/MOVE_VERTICAL_ACK/MOVE_HORIZONTAL_ACK/COLLISION_DETECTED_ACK)
// Step-2: ObjC as EvtSuber subEvt(EvtID==IOC_EVTID_TEST_MOVE_VERTICAL/COILLISION_DETECTED),
//          postEVT(EvtID==IOC_EVTID_TEST_MOVE_VERTICAL_ACK/COLLISION_DETECTED_ACK) in EvtArgs::CbProcEvt_F,
//          and start a thread named "ObjC_Thread" to postEvt(EvtID==IOC_EVTID_TEST_KEEPALIVE) every 1ms*$_UT_EVTCNT_KEEPALIVE
// Step-3: ObjD as EvtSuber subEvt(EvtID==IOC_EVTID_TEST_MOVE_HORIZONTAL/COILLISION_DETECTED),
//          postEVT(EvtID==IOC_EVTID_TEST_MOVE_HORIZONTAL_ACK/COLLISION_DETECTED_ACK) in EvtArgs::CbProcEvt_F,
//          and start a thread named "ObjD_Thread" to postEvt(EvtID==IOC_EVTID_TEST_KEEPALIVE) every 1ms*$_UT_EVTCNT_KEEPALIVE
// Step-4: ObjB as EvtPuber postEVT(EvtID==IOC_EVTID_TEST_MOVE_VERTICAL/IOC_EVTID_TEST_MOVE_HORIZONTAL)
//          in a thread named "ObjB_Thread1" every 1ms*$_UT_EVTCNT_MOVE_VERTICAL/$_UT_EVTCNT_MOVE_HORIZONTAL,
//          and start a thread named "ObjB_Thread2" to postEVT(EvtID==IOC_EVTID_TEST_KEEPALIVE) every 1ms*$_UT_EVTCNT_KEEPALIVE
// Step-5: ObjE as EvtPuber postEVT(EvtID==IOC_EVTID_TEST_COLLISION_DETECTED) in a thread named "ObjE_Thread1"
//        every 1ms*$_UT_EVTCNT_COLLISION_DETECTED, and start a thread named "ObjE_Thread2" to
//        postEVT(EvtID==IOC_EVTID_TEST_KEEPALIVE) every 1ms*$_UT_EVTCNT_KEEPALIVE

#define _UT_EVTCNT_MOVE_VERTICAL 1024
#define _UT_EVTCNT_MOVE_HORIZONTAL 1024
#define _UT_EVTCNT_COLLISION_DETECTED 1024
