#include "_UT_IOC_Common.h"

// This is IOC_Event in ConlesMode's typical UT from API user's perspective
//===>Case[01]: Ref::ConlesMode's call flow typical example[1] in PlatIF_IOC.h
//-Step1: EvtSuber as ObjX subEvt(EvtID==IOC_EVTID_TEST_KEEPALIVE)
//-Step2: EvtPuber as ObjY postEVT(EvtID==IOC_EVTID_TEST_KEEPALIVE * $_UT_EVTCNT_KEEPALIVE)
//-Step3: EvtSuber's EvtArgs::CbProcEvt_F is called $_UT_EVTCNT_KEEPALIVE times
//-Step4: EvtSuber unsubEvt(EvtID==IOC_EVTID_TEST_KEEPALIVE)
//-Step5: EvtPuber postEVT(EvtID==IOC_EVTID_TEST_KEEPALIVE * $_UT_EVTCNT_KEEPALIVE) again
//-Step6:EvtSuber's EvtArgs::CbProcEvt_F is NOT called more, stile $_UT_EVTCNT_KEEPALIVE times

//===>Case[02]: ...
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
