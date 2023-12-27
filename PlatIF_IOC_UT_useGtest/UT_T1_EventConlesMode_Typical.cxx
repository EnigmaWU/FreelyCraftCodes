#include "_UT_IOC_Common.h"

// This is IOC_Event in ConlesMode's typical UT from API user's perspective
//===>Case[01]: Ref::ConlesMode's call flow typical example[1] in PlatIF_IOC.h
/**
 * Step1: EvtSuber as ObjX subEvt(EvtID==IOC_EVTID_TEST_KEEPALIVE)
 * Step2: EvtPuber as ObjY postEVT(EvtID==IOC_EVTID_TEST_KEEPALIVE*$_UT_EVTCNT_KEEPALIVE)
 * Step3: EvtSuber's EvtArgs::CbProcEvt_F is called $$_UT_EVTCNT_KEEPALIVE times 
 * Step4: EvtSuber unsubEvt(EvtID==IOC_EVTID_TEST_KEEPALIVE) 
 * Step5: EvtPuber postEVT(EvtID==IOC_EVTID_TEST_KEEPALIVE*$_UT_EVTCNT_KEEPALIVE) again 
 * Step6: EvtSuber's EvtArgs::CbProcEvt_F is NOT called
 */
//===>Case[02]: ...

#define _UT_EVTCNT_KEEPALIVE 1024

TEST(EventConlesModeTypical, Case01) {
  IOC_LinkID_T AnonyLinkID = (IOC_LinkID_T)IOC_CONLESMODE_ANONYMOUS_LINK_ID;

  _Case01EvtSuberPriv_T EvtSuberPriv = {.EvtKeepAliveCnt = 0};
  IOC_EvtID_T EvtSubIDs[] = {IOC_EVTID_TEST_KEEPALIVE};
  IOC_EvtSubArgs_T EvtSubArgs = {.NumOfEvtIDs = TOS_calcArrayElmtCnt(EvtSubIDs),
                                 .EvtIDs = EvtSubIDs,
                                 .CbProcEvt_F = __Case01_CbProcEvt_F};

  TOS_Result_T Result = PLT_IOC_subEVT(AnonyLinkID, &EvtSubArgs);
  ASSERT_EQ(TOS_RESULT_SUCCESS, Result);

  for (int i = 0; i < _UT_EVTCNT_KEEPALIVE; i++) {
    IOC_EvtDesc_T EvtDesc = {.EvtID = IOC_EVTID_TEST_KEEPALIVE};
    Result = PLT_IOC_pubEVT(AnonyLinkID, &EvtDesc);
    ASSERT_EQ(TOS_RESULT_SUCCESS, Result);
  }

    ASSERT_EQ(_UT_EVTCNT_KEEPALIVE, EvtSuberPriv.EvtKeepAliveCnt);//@CheckPoint

    Result = PLT_IOC_unsubEVT(AnonyLinkID, &EvtSubArgs);
    ASSERT_EQ(TOS_RESULT_SUCCESS, Result);

    for (int i = 0; i < _UT_EVTCNT_KEEPALIVE; i++) {
        IOC_EvtDesc_T EvtDesc = {.EvtID = IOC_EVTID_TEST_KEEPALIVE};
        Result = PLT_IOC_pubEVT(AnonyLinkID, &EvtDesc);
        ASSERT_EQ(TOS_RESULT_SUCCESS, Result);
    
    }

    ASSERT_EQ(_UT_EVTCNT_KEEPALIVE, EvtSuberPriv.EvtKeepAliveCnt);//@CheckPoint
}