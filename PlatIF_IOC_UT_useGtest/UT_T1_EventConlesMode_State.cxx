#include <cstddef>

#include "_UT_IOC_Common.h"

// This is IOC_Event in ConlesMode's State/FSM UT from API user's perspective
// RefMore: FSM definition of ConlesMode_of[EVT] in PlatIF_IOC.h

//===> Case[01]: ObjA as EvtPuber postEvt, ObjB as EvtSuber subEvt,
//      ObjB can't unsubEvt if CbProcEvt_F is callbacking by ObjA's event.
//  Step-1: ObjA as EvtSuber subEvt(TEST_KEEPALIVE), set ObjA's private magic value X
//  Step-2: ObjB as EvtPuber postEvt(TEST_KEEPALIVE)
//            |-> In ObjA's CbProcEvt_F(), sleep(3), set ObjA's private magic value Y
//  Step-3: ObjA unsubEvt(TEST_KEEPALIVE)
//            |-> ObjA will be blocked in unsubEvt() until ObjA's CbProcEvt_F() return,
//              and check ObjA's private magic value is Y now

// RefCode: TEST(EventConlesModeTypical, Case01) in UT_T1_EventConlesMode_Typical.cxx
typedef struct {
  unsigned long MagicValue;
} _UT_Case01_CbPrivObjA_T, *_UT_Case01_CbPrivObjA_pT;

static TOS_Result_T _UT_Case01_CbProcEvtObjA_F(IOC_EvtDesc_pT pEvtDesc, void* pCbPriv) {
  _UT_Case01_CbPrivObjA_pT pCbPrivObjA = (_UT_Case01_CbPrivObjA_pT)pCbPriv;
  pCbPrivObjA->MagicValue = 0x87654321;
  sleep(3);
  return TOS_RESULT_SUCCESS;
}

TEST(ConlesModeState, Case01) {
  IOC_LinkID_T LinkID = IOC_CONLESMODE_AUTO_LINK_ID;

  // Step-1: ObjA as EvtSuber subEvt(TEST_KEEPALIVE), set ObjA's private magic value X
  _UT_Case01_CbPrivObjA_T CbPrivObjA = {0};
  CbPrivObjA.MagicValue = 0x12345678;

  IOC_EvtID_T EvtIDsObjA[] = {IOC_EVTID_TEST_KEEPALIVE};
  IOC_EvtSubArgs_T EvtSubArgsObjA = {
      .CbProcEvt_F = _UT_Case01_CbProcEvtObjA_F,
      .pCbPriv = &CbPrivObjA,
      .EvtNum = TOS_calcArrayElmtCnt(EvtIDsObjA),
      .pEvtIDs = EvtIDsObjA,
  };
  TOS_Result_T Result = PLT_IOC_subEVT(LinkID, &EvtSubArgsObjA);
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint

  // Step-2: ObjB as EvtPuber postEvt(TEST_KEEPALIVE)
  IOC_EvtDesc_T EvtDescObjB = {.EvtID = IOC_EVTID_TEST_KEEPALIVE};
  Result = PLT_IOC_postEVT(LinkID, &EvtDescObjB, NULL);
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint

  // Step-3: ObjA unsubEvt(TEST_KEEPALIVE)
  IOC_EvtUnsubArgs_T EvtUnsubArgsObjA = {.CbProcEvt_F = _UT_Case01_CbProcEvtObjA_F, .pCbPriv = &CbPrivObjA};
  Result = PLT_IOC_unsubEVT(LinkID, &EvtUnsubArgsObjA);
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint

  // Check ObjA's private magic value is Y now
  ASSERT_EQ(CbPrivObjA.MagicValue, 0x87654321);  // CheckPoint
}