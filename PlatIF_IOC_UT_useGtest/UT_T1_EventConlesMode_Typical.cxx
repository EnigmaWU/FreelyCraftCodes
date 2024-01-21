#include <sys/_pthread/_pthread_mutex_t.h>

#include "_UT_IOC_Common.h"

// This is IOC_Event in ConlesMode's typical UT from API user's perspective
//===>Case[01]: Ref::ConlesMode's call flow typical example[1] in PlatIF_IOC.h
//-Step1: EvtSuber as ObjX subEvt(EvtID==TEST_KEEPALIVE)
//-Step2: EvtPuber as ObjY postEVT(EvtID==TEST_KEEPALIVE * $_UT_EVTCNT_KEEPALIVE)
//-Step3: EvtSuber's EvtArgs::CbProcEvt_F is called $_UT_EVTCNT_KEEPALIVE times
//-Step4: EvtSuber unsubEvt(EvtID==TEST_KEEPALIVE)
//-Step5: EvtPuber postEVT(EvtID==TEST_KEEPALIVE * $_UT_EVTCNT_KEEPALIVE) again
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

  //===>Step1: EvtSuber as ObjX subEvt(EvtID==TEST_KEEPALIVE)
  _UT_Case01_EvtSuberPriv_T EvtSuberPriv = {.KeepAliveEvtCnt = 0};
  IOC_EvtID_T SubEvtIDs[] = {IOC_EVTID_TEST_KEEPALIVE};
  IOC_EvtSubArgs_T EvtSubArgs = {.CbProcEvt_F = _UT_Case01_EvtSuberCbProcEvt_F,
                                 .pCbPriv = &EvtSuberPriv,
                                 .EvtNum = TOS_calcArrayElmtCnt(SubEvtIDs),
                                 .pEvtIDs = SubEvtIDs};
  TOS_Result_T Result = PLT_IOC_subEVT(LinkID, &EvtSubArgs);
  ASSERT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint

  //===>Step2: EvtPuber as ObjY postEVT(EvtID==TEST_KEEPALIVE*$_UT_EVTCNT_KEEPALIVE)
  for (int i = 0; i < _UT_EVTCNT_KEEPALIVE; i++) {
    IOC_EvtDesc_T EvtDesc = {.EvtID = IOC_EVTID_TEST_KEEPALIVE};
    Result = PLT_IOC_postEVT(LinkID, &EvtDesc, NULL);
    ASSERT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint
  }

  //===>Step3: EvtSuber's EvtArgs::CbProcEvt_F is called $$_UT_EVTCNT_KEEPALIVE times
  ASSERT_EQ(_UT_EVTCNT_KEEPALIVE, EvtSuberPriv.KeepAliveEvtCnt);  //@CheckPoint

  //===>Step4: EvtSuber unsubEvt(EvtID==TEST_KEEPALIVE)
  Result = PLT_IOC_unsubEVT(LinkID, NULL);
  ASSERT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint

  //===>Step5: EvtPuber postEVT(EvtID==TEST_KEEPALIVE*$_UT_EVTCNT_KEEPALIVE) again
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
// Step-1: ObjA as EvtSuber subEvt(EvtID==TEST_KEEPALIVE/MOVE_VERTICAL_ACK/MOVE_HORIZONTAL_ACK/COLLISION_DETECTED_ACK)
// Step-2: ObjC as EvtSuber subEvt(EvtID==TEST_MOVE_VERTICAL/COILLISION_DETECTED) with
//            EvtArgs::CbProcEvt_F(_UT_Case02_ObjC_CbProcEvt_F),
//          as EvtPuber postEVT(EvtID==TEST_MOVE_VERTICAL_ACK/COLLISION_DETECTED_ACK) in each CbProcEvt_F callbacked,
//          and start a thread named "ObjC_Thread" as EvtPuber postEvt(EvtID==TEST_KEEPALIVE) every 1ms*$_UT_EVTCNT_KEEPALIVE
// Step-3: ObjD as EvtSuber subEvt(EvtID==TEST_MOVE_HORIZONTAL/COILLISION_DETECTED) with
//            EvtArgs::CbProcEvt_F(_UT_Case02_ObjD_CbProcEvt_F),
//          as EvtPuber postEVT(EvtID==TEST_MOVE_HORIZONTAL_ACK/COLLISION_DETECTED_ACK) in each CbProcEvt_F callbacked,
//          and start a thread named "ObjD_Thread" as EvtPuber postEvt(EvtID==TEST_KEEPALIVE) every 1ms*$_UT_EVTCNT_KEEPALIVE
// Step-4: ObjB as EvtPuber postEVT(EvtID==TEST_MOVE_VERTICAL) in a thread named "ObjB_Thread1"
//            every 1ms*$_UT_EVTCNT_MOVE_VERTICAL;
//          and start a thread named "ObjB_Thread2" as EvtPuber postEVT(EvtID==TEST_MOVE_HORIZONTAL)
//            every 1ms*$_UT_EVTCNT_MOVE_HORIZONTAL;
//          and start a thread named "ObjB_Thread3" as EvtPuber postEVT(EvtID==TEST_KEEPALIVE)
//            every 1ms*$_UT_EVTCNT_KEEPALIVE
// Step-5: ObjE as EvtPuber postEVT(EvtID==TEST_COLLISION_DETECTED) in a thread named "ObjE_Thread1"
//          every 2ms*$_UT_EVTCNT_COLLISION_DETECTED, and start a thread named "ObjE_Thread2" to
//          postEVT(EvtID==TEST_KEEPALIVE) every 1ms*$_UT_EVTCNT_KEEPALIVE
// Step-6: ObjA check KeepAliveEvtCnt, MoveVerticalAckEvtCnt, MoveHorizontalAckEvtCnt, CollisionDetectedAckEvtCnt
// Step-7: ObjC check MoveVerticalEvtCnt, CollisionDetectedEvtCnt
// Step-8: ObjD check MoveHorizontalEvtCnt, CollisionDetectedEvtCnt

#include <thread>
#define _UT_EVTCNT_MOVE_VERTICAL 1024
#define _UT_EVTCNT_MOVE_HORIZONTAL 1024
#define _UT_EVTCNT_COLLISION_DETECTED 1024

typedef struct {
  unsigned long KeepAliveEvtCnt;
  unsigned long MoveVerticalAckEvtCnt;
  unsigned long MoveHorizontalAckEvtCnt;
  unsigned long CollisionDetectedAckEvtCnt;
} _UT_Case02_CbPrivObjA_T, *_UT_Case02_CbPrivObjA_pT;

static TOS_Result_T _UT_Case02_CbProcEvtObjA_F(IOC_EvtDesc_pT pEvtDesc, void *pCbPriv) {
  _UT_Case02_CbPrivObjA_pT pPrivObjA = (_UT_Case02_CbPrivObjA_pT)pCbPriv;

  switch (pEvtDesc->EvtID) {
    case IOC_EVTID_TEST_KEEPALIVE:
      pPrivObjA->KeepAliveEvtCnt++;
      break;
    case IOC_EVTID_TEST_MOVE_VERTICAL_ACK:
      pPrivObjA->MoveVerticalAckEvtCnt++;
      break;
    case IOC_EVTID_TEST_MOVE_HORIZONTAL_ACK:
      pPrivObjA->MoveHorizontalAckEvtCnt++;
      break;
    case IOC_EVTID_TEST_COLLISION_DETECTED_ACK:
      pPrivObjA->CollisionDetectedAckEvtCnt++;
      break;
    default:
      break;
  }

  return TOS_RESULT_SUCCESS;
}

typedef struct {
  unsigned long MoveVerticalEvtCnt;
  unsigned long CollisionDetectedEvtCnt;
} _UT_Case02_CbPrivObjC_T, *_UT_Case02_CbPrivObjC_pT;

static TOS_Result_T _UT_Case02_CbProcEvtObjC_F(IOC_EvtDesc_pT pEvtDesc, void *pCbPriv) {
  _UT_Case02_CbPrivObjC_pT pEvtSuberPriv = (_UT_Case02_CbPrivObjC_pT)pCbPriv;
  switch (pEvtDesc->EvtID) {
    case IOC_EVTID_TEST_MOVE_VERTICAL: {
      pEvtSuberPriv->MoveVerticalEvtCnt++;

      //===>Step2: ObjC as EvtPuber postEVT(EvtID==TEST_MOVE_VERTICAL_ACK) in each CbProcEvt_F callbacked
      IOC_EvtDesc_T EvtDesc = {.EvtID = IOC_EVTID_TEST_MOVE_VERTICAL_ACK};
      TOS_Result_T Result = PLT_IOC_postEVT(IOC_CONLESMODE_AUTO_LINK_ID, &EvtDesc, NULL);
      EXPECT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint
    } break;

    case IOC_EVTID_TEST_COLLISION_DETECTED: {
      pEvtSuberPriv->CollisionDetectedEvtCnt++;

      //===>Step2: ObjC as EvtPuber postEVT(EvtID==TEST_COLLISION_DETECTED_ACK) in each CbProcEvt_F callbacked
      IOC_EvtDesc_T EvtDesc = {.EvtID = IOC_EVTID_TEST_COLLISION_DETECTED_ACK};
      TOS_Result_T Result = PLT_IOC_postEVT(IOC_CONLESMODE_AUTO_LINK_ID, &EvtDesc, NULL);
      EXPECT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint
    } break;

    default:
      break;
  }
  return TOS_RESULT_SUCCESS;
}

typedef struct {
  unsigned long MoveHorizontalEvtCnt;
  unsigned long CollisionDetectedEvtCnt;
} _UT_Case02_CbPrivObjD_T, *_UT_Case02_CbPrivObjD_pT;

static TOS_Result_T _UT_Case02_CbProcEvtObjD_F(IOC_EvtDesc_pT pEvtDesc, void *pCbPriv) {
  _UT_Case02_CbPrivObjD_pT pEvtSuberPriv = (_UT_Case02_CbPrivObjD_pT)pCbPriv;
  switch (pEvtDesc->EvtID) {
    case IOC_EVTID_TEST_MOVE_HORIZONTAL: {
      pEvtSuberPriv->MoveHorizontalEvtCnt++;

      //===>Step3: ObjD as EvtPuber postEVT(EvtID==TEST_MOVE_HORIZONTAL_ACK) in each CbProcEvt_F callbacked
      IOC_EvtDesc_T EvtDesc = {.EvtID = IOC_EVTID_TEST_MOVE_HORIZONTAL_ACK};
      TOS_Result_T Result = PLT_IOC_postEVT(IOC_CONLESMODE_AUTO_LINK_ID, &EvtDesc, NULL);
      EXPECT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint
    } break;

    case IOC_EVTID_TEST_COLLISION_DETECTED: {
      pEvtSuberPriv->CollisionDetectedEvtCnt++;

      //===>Step3: ObjD as EvtPuber postEVT(EvtID==TEST_COLLISION_DETECTED_ACK) in each CbProcEvt_F callbacked
      IOC_EvtDesc_T EvtDesc = {.EvtID = IOC_EVTID_TEST_COLLISION_DETECTED_ACK};
      TOS_Result_T Result = PLT_IOC_postEVT(IOC_CONLESMODE_AUTO_LINK_ID, &EvtDesc, NULL);
      EXPECT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint
    } break;

    default:
      break;
  }
  return TOS_RESULT_SUCCESS;
}

// implement TEST(ConlesModeTypical, Case02) based on the above description and use TEST(ConlesModeTypical, Case01) as sample
TEST(EventConlesModeTypical, Case02) {
  IOC_LinkID_T LinkID = IOC_CONLESMODE_AUTO_LINK_ID;

  //===>Step1: ObjA as EvtSuber subEvt(EvtID==TEST_KEEPALIVE/MOVE_VERTICAL_ACK/MOVE_HORIZONTAL_ACK/COLLISION_DETECTED_ACK)
  _UT_Case02_CbPrivObjA_T CbPrivObjA = {
      .KeepAliveEvtCnt = 0, .MoveVerticalAckEvtCnt = 0, .MoveHorizontalAckEvtCnt = 0, .CollisionDetectedAckEvtCnt = 0};
  IOC_EvtID_T SubEvtIDsObjA[] = {IOC_EVTID_TEST_KEEPALIVE, IOC_EVTID_TEST_MOVE_VERTICAL_ACK, IOC_EVTID_TEST_MOVE_HORIZONTAL_ACK,
                                 IOC_EVTID_TEST_COLLISION_DETECTED_ACK};
  IOC_EvtSubArgs_T EvtSubArgsObjA = {.CbProcEvt_F = _UT_Case02_CbProcEvtObjA_F,
                                     .pCbPriv = &CbPrivObjA,
                                     .EvtNum = TOS_calcArrayElmtCnt(SubEvtIDsObjA),
                                     .pEvtIDs = SubEvtIDsObjA};
  TOS_Result_T Result = PLT_IOC_subEVT(LinkID, &EvtSubArgsObjA);
  ASSERT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint

  //===>Step2.1: ObjC as EvtSuber subEvt(EvtID==TEST_MOVE_VERTICAL/COILLISION_DETECTED) with
  //            EvtArgs::CbProcEvt_F(_UT_Case02_CbProcEvtObjC_F),
  //          as EvtPuber postEVT(EvtID==TEST_MOVE_VERTICAL_ACK/COLLISION_DETECTED_ACK) in each CbProcEvt_F callbacked,
  _UT_Case02_CbPrivObjC_T CbPrivObjC = {.MoveVerticalEvtCnt = 0, .CollisionDetectedEvtCnt = 0};
  IOC_EvtID_T SubEvtIDsObjC[] = {IOC_EVTID_TEST_MOVE_VERTICAL, IOC_EVTID_TEST_COLLISION_DETECTED};
  IOC_EvtSubArgs_T EvtSubArgsObjC = {.CbProcEvt_F = _UT_Case02_CbProcEvtObjC_F,
                                     .pCbPriv = &CbPrivObjC,
                                     .EvtNum = TOS_calcArrayElmtCnt(SubEvtIDsObjC),
                                     .pEvtIDs = SubEvtIDsObjC};
  Result = PLT_IOC_subEVT(LinkID, &EvtSubArgsObjC);
  ASSERT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint

  //===>Step2.1: ObjC start a thread named "ObjC_Thread" as EvtPuber postEvt(EvtID==TEST_KEEPALIVE) every 1ms*$_UT_EVTCNT_KEEPALIVE
  std::thread ObjC_Thread([]() {
    for (int i = 0; i < _UT_EVTCNT_KEEPALIVE; i++) {
      IOC_EvtDesc_T EvtDesc = {.EvtID = IOC_EVTID_TEST_KEEPALIVE};
      TOS_Result_T Result = PLT_IOC_postEVT(IOC_CONLESMODE_AUTO_LINK_ID, &EvtDesc, NULL);
      EXPECT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  });

  //===>Step3.1: ObjD as EvtSuber subEvt(EvtID==TEST_MOVE_HORIZONTAL/COILLISION_DETECTED) with
  //            EvtArgs::CbProcEvt_F(_UT_Case02_CbProcEvtObjD_F),
  //          as EvtPuber postEVT(EvtID==TEST_MOVE_HORIZONTAL_ACK/COLLISION_DETECTED_ACK) in each CbProcEvt_F callbacked,
  _UT_Case02_CbPrivObjD_T CbPrivObjD = {.MoveHorizontalEvtCnt = 0, .CollisionDetectedEvtCnt = 0};
  IOC_EvtID_T SubEvtIDsObjD[] = {IOC_EVTID_TEST_MOVE_HORIZONTAL, IOC_EVTID_TEST_COLLISION_DETECTED};
  IOC_EvtSubArgs_T EvtSubArgsObjD = {.CbProcEvt_F = _UT_Case02_CbProcEvtObjD_F,
                                     .pCbPriv = &CbPrivObjD,
                                     .EvtNum = TOS_calcArrayElmtCnt(SubEvtIDsObjD),
                                     .pEvtIDs = SubEvtIDsObjD};
  Result = PLT_IOC_subEVT(LinkID, &EvtSubArgsObjD);
  ASSERT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint

  //===>Step3.1: ObjD start a thread named "ObjD_Thread" as EvtPuber postEvt(EvtID==TEST_KEEPALIVE) every 1ms*$_UT_EVTCNT_KEEPALIVE
  std::thread ObjD_Thread([]() {
    for (int i = 0; i < _UT_EVTCNT_KEEPALIVE; i++) {
      IOC_EvtDesc_T EvtDesc = {.EvtID = IOC_EVTID_TEST_KEEPALIVE};
      TOS_Result_T Result = PLT_IOC_postEVT(IOC_CONLESMODE_AUTO_LINK_ID, &EvtDesc, NULL);
      EXPECT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  });

  //===>Step4.1: ObjB as EvtPuber postEVT(EvtID==TEST_MOVE_VERTICAL) in a thread named "ObjB_Thread1", every
  // 1ms*$_UT_EVTCNT_MOVE_VERTICAL;
  std::thread ObjB_Thread1([]() {
    for (int i = 0; i < _UT_EVTCNT_MOVE_VERTICAL; i++) {
      IOC_EvtDesc_T EvtDesc = {.EvtID = IOC_EVTID_TEST_MOVE_VERTICAL};
      TOS_Result_T Result = PLT_IOC_postEVT(IOC_CONLESMODE_AUTO_LINK_ID, &EvtDesc, NULL);
      EXPECT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  });

  //===>Step4.2: ObjB start a thread named "ObjB_Thread2" as EvtPuber postEVT(EvtID==TEST_MOVE_HORIZONTAL) every
  // 1ms*$_UT_EVTCNT_MOVE_HORIZONTAL;
  std::thread ObjB_Thread2([]() {
    for (int i = 0; i < _UT_EVTCNT_MOVE_HORIZONTAL; i++) {
      IOC_EvtDesc_T EvtDesc = {.EvtID = IOC_EVTID_TEST_MOVE_HORIZONTAL};
      TOS_Result_T Result = PLT_IOC_postEVT(IOC_CONLESMODE_AUTO_LINK_ID, &EvtDesc, NULL);
      EXPECT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  });

  //===>Step4.3: ObjB start a thread named "ObjB_Thread3" as EvtPuber postEVT(EvtID==TEST_KEEPALIVE) every 1ms*$_UT_EVTCNT_KEEPALIVE
  std::thread ObjB_Thread3([]() {
    for (int i = 0; i < _UT_EVTCNT_KEEPALIVE; i++) {
      IOC_EvtDesc_T EvtDesc = {.EvtID = IOC_EVTID_TEST_KEEPALIVE};
      TOS_Result_T Result = PLT_IOC_postEVT(IOC_CONLESMODE_AUTO_LINK_ID, &EvtDesc, NULL);
      EXPECT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  });

  //===>Step5.1: ObjE as EvtPuber postEVT(EvtID==TEST_COLLISION_DETECTED) in a thread named "ObjE_Thread1"
  //          every 2ms*$_UT_EVTCNT_COLLISION_DETECTED,
  std::thread ObjE_Thread1([]() {
    for (int i = 0; i < _UT_EVTCNT_COLLISION_DETECTED; i++) {
      IOC_EvtDesc_T EvtDesc = {.EvtID = IOC_EVTID_TEST_COLLISION_DETECTED};
      TOS_Result_T Result = PLT_IOC_postEVT(IOC_CONLESMODE_AUTO_LINK_ID, &EvtDesc, NULL);
      EXPECT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
  });

  //===>Step5.2: ObjE start a thread named "ObjE_Thread2" to postEVT(EvtID==TEST_KEEPALIVE) every 1ms*$_UT_EVTCNT_KEEPALIVE
  std::thread ObjE_Thread2([]() {
    for (int i = 0; i < _UT_EVTCNT_KEEPALIVE; i++) {
      IOC_EvtDesc_T EvtDesc = {.EvtID = IOC_EVTID_TEST_KEEPALIVE};
      TOS_Result_T Result = PLT_IOC_postEVT(IOC_CONLESMODE_AUTO_LINK_ID, &EvtDesc, NULL);
      EXPECT_EQ(TOS_RESULT_SUCCESS, Result);  //@CheckPoint
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  });

  // Wait for ObjC_Thread, ObjD_Thread, ObjB_Thread1, ObjB_Thread2, ObjB_Thread3, ObjE_Thread1, ObjE_Thread2
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  ObjC_Thread.join();
  ObjD_Thread.join();
  ObjB_Thread1.join();
  ObjB_Thread2.join();
  ObjB_Thread3.join();
  ObjE_Thread1.join();
  ObjE_Thread2.join();

  //===>Step6: ObjA check KeepAliveEvtCnt, MoveVerticalAckEvtCnt, MoveHorizontalAckEvtCnt, CollisionDetectedAckEvtCnt
  ASSERT_EQ(_UT_EVTCNT_KEEPALIVE * 4, CbPrivObjA.KeepAliveEvtCnt);                      //@CheckPoint
  ASSERT_EQ(_UT_EVTCNT_MOVE_VERTICAL, CbPrivObjA.MoveVerticalAckEvtCnt);                //@CheckPoint
  ASSERT_EQ(_UT_EVTCNT_MOVE_HORIZONTAL, CbPrivObjA.MoveHorizontalAckEvtCnt);            //@CheckPoint
  ASSERT_EQ(_UT_EVTCNT_COLLISION_DETECTED * 2, CbPrivObjA.CollisionDetectedAckEvtCnt);  //@CheckPoint

  //===>Step7: ObjC check MoveVerticalEvtCnt, CollisionDetectedEvtCnt
  ASSERT_EQ(_UT_EVTCNT_MOVE_VERTICAL, CbPrivObjC.MoveVerticalEvtCnt);            //@CheckPoint
  ASSERT_EQ(_UT_EVTCNT_COLLISION_DETECTED, CbPrivObjC.CollisionDetectedEvtCnt);  //@CheckPoint

  //===>Step8: ObjD check MoveHorizontalEvtCnt, CollisionDetectedEvtCnt
  ASSERT_EQ(_UT_EVTCNT_MOVE_HORIZONTAL, CbPrivObjD.MoveHorizontalEvtCnt);        //@CheckPoint
  ASSERT_EQ(_UT_EVTCNT_COLLISION_DETECTED, CbPrivObjD.CollisionDetectedEvtCnt);  //@CheckPoint
}