#include "_UT_IOC_Common.h"

// This is IOC_Event in ConlesMode's State/FSM UT from API user's perspective
// RefMore: FSM definition of FSM_ofConlesEVT in PlatIF_IOC.h

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @[Name]: verifyUnsubEvtInLinkReadyState_byCbProcEvtSleep3sToForceInLinkBusyState
 * @[Purpose]: verify FSM_ofConlesEVT's {TSF-1}[STATE:LinkStateReady] -> <ACT:unsubEvt> -> [STATE:LinkStateReady].
 * @[Steps]:
 *   1. ObjA as EvtSuber subEVT(TEST_KEEPALIVE), set ObjA's private magic value '1'
 *   2. ObjB as EvtPuber postEVT(TEST_KEEPALIVE)
 *         |-> In ObjA's CbProcEvt_F(), sleep(3), set ObjA's private magic value '3'
 *   3. ObjA unsubEvt(TEST_KEEPALIVE)
 *         |-> ObjA will be blocked in unsubEvt() until ObjA's CbProcEvt_F() return,
 * @[Expect]: ObjA unsubEVT SUCCESS, and ObjA's private magic value is '3' after unsubEVT.
 * @[Notes]:
 *    RefCode: TEST(UT_ConlesEventTypical, Case01) in UT_ConlesEvent_Typical.cxx
 */
typedef struct {
  ULONG_T MagicValue;
  pthread_mutex_t Mutex;
  pthread_cond_t Cond;
} _UT_Case01_CbPrivObjA_T, *_UT_Case01_CbPrivObjA_pT;

static TOS_Result_T _UT_Case01_CbProcEvtObjA_F(IOC_EvtDesc_pT pEvtDesc, void* pCbPriv) {
  _UT_Case01_CbPrivObjA_pT pCbPrivObjA = (_UT_Case01_CbPrivObjA_pT)pCbPriv;

  pCbPrivObjA->MagicValue = 2;
  pthread_cond_signal(&pCbPrivObjA->Cond);

  sleep(3);
  pCbPrivObjA->MagicValue = 3;
  return TOS_RESULT_SUCCESS;
}

TEST(UT_ConlesEventState, Case01_verifyUnsubEvtInLinkReadyState_byCbProcEvtSleep3sToForceInLinkBusyState) {
  // Step-1: ObjA as EvtSuber subEvt(TEST_KEEPALIVE), set ObjA's private magic value 1
  _UT_Case01_CbPrivObjA_T CbPrivObjA = {.MagicValue = 1};
  pthread_mutex_init(&CbPrivObjA.Mutex, NULL);
  pthread_cond_init(&CbPrivObjA.Cond, NULL);

  IOC_EvtID_T EvtIDsObjA[] = {IOC_EVTID_TEST_KEEPALIVE};
  IOC_EvtSubArgs_T EvtSubArgsObjA = {
      .CbProcEvt_F = _UT_Case01_CbProcEvtObjA_F,
      .pCbPriv = &CbPrivObjA,
      .EvtNum = TOS_calcArrayElmtCnt(EvtIDsObjA),
      .pEvtIDs = EvtIDsObjA,
  };
  TOS_Result_T Result = PLT_IOC_subEVT_inConlesMode(&EvtSubArgsObjA);
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint

  // Step-2: ObjB as EvtPuber postEvt(TEST_KEEPALIVE)
  IOC_EvtDesc_T EvtDescObjB = {.EvtID = IOC_EVTID_TEST_KEEPALIVE};
  Result = PLT_IOC_postEVT_inConlesMode(&EvtDescObjB, NULL);
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint

  pthread_mutex_lock(&CbPrivObjA.Mutex);
  pthread_cond_wait(&CbPrivObjA.Cond, &CbPrivObjA.Mutex);  // Wait ObjA's CbProcEvt_F() enter

  // Step-3: ObjA unsubEvt(TEST_KEEPALIVE)
  IOC_EvtUnsubArgs_T EvtUnsubArgsObjA = {.CbProcEvt_F = _UT_Case01_CbProcEvtObjA_F, .pCbPriv = &CbPrivObjA};
  Result = PLT_IOC_unsubEVT_inConlesMode(&EvtUnsubArgsObjA);

  //===VERIFY===
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // KeyCheckPoint
  ASSERT_EQ(CbPrivObjA.MagicValue, 3);    // KeyCheckPoint

  //===CLEANUP===
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @[Name]: verifyUnsubEvtInLinkStateReady_byDynamicSubUnsubEvtMultiTimes
 * @[Purpose]: same with Case[01], plus EvtSuber will dynamiclly Sub/UnSubEvt multiply times .
 * @[Steps]:
 *   1. Start Thread_ObjA to dynamiclly Sub/UnSubEvt(TEST_KEEPALIVE) $_UT_CASE02_DYNAMIC_SUBUNSUB_CNT times
 *       |-> Each Sub/UnSubEvt will success, and usleep(100)
 *       |-> In ObjA's CbProcEvt_F(), usleep(100), save $ProcKeepAliveEvtCnt in CbPrivObjA
 *   2. Start Thread_ObjB to postEvt(TEST_KEEPALIVE * $_UT_CASE02_KEEPALIVE_EVT_CNT) every 10us
 *       |-> save $PostEvtRetSuccessCnt and $PostEvtRetNoEvtSuberCnt in PrivObjB
 *   3. Wait Thread_ObjA/ObjB exit
 * @[Expect]: all Sub/UnSubEvt return RESULT_SUCCESS && $ProcKeepAliveEvtCnt == $PostEvtRetSuccessCnt
 *              && $PostEvtRetSuccessCnt + $PostEvtRetNoEvtSuberCnt == $_UT_CASE02_KEEPALIVE_EVT_CNT
 * @[Notes]:
 *  ImplRefCode: UT_ConlesEventState::Case01_verifyUnsubEvtInLinkReadyState_...
 */
#define _UT_CASE02_DYNAMIC_SUBUNSUB_CNT 10000
#define _UT_CASE02_KEEPALIVE_EVT_CNT 100000

typedef struct {
  ULONG_T ProcKeepAliveEvtCnt;
} _UT_Case02_CbPrivObjA_T, *_UT_Case02_CbPrivObjA_pT;

static TOS_Result_T _UT_Case02_CbProcEvtObjA_F(IOC_EvtDesc_pT pEvtDesc, void* pCbPriv) {
  _UT_Case02_CbPrivObjA_pT pCbPrivObjA = (_UT_Case02_CbPrivObjA_pT)pCbPriv;

  pCbPrivObjA->ProcKeepAliveEvtCnt++;
  usleep(100);
  return TOS_RESULT_SUCCESS;
}

typedef struct {
  ULONG_T PostEvtRetSuccessCnt;
  ULONG_T PostEvtRetNoEvtSuberCnt;
} _UT_Case02_CbPrivObjB_T, *_UT_Case02_CbPrivObjB_pT;

static void* _UT_Case02_ThreadObjA(void* pArg) {
  IOC_EvtID_T EvtIDsObjA[] = {IOC_EVTID_TEST_KEEPALIVE};
  _UT_Case02_CbPrivObjA_pT pCbPrivObjA = (_UT_Case02_CbPrivObjA_pT)pArg;

  for (int i = 0; i < _UT_CASE02_DYNAMIC_SUBUNSUB_CNT; i++) {
    IOC_EvtSubArgs_T EvtSubArgsObjA = {
        .CbProcEvt_F = _UT_Case02_CbProcEvtObjA_F,
        .pCbPriv = pCbPrivObjA,
        .EvtNum = TOS_calcArrayElmtCnt(EvtIDsObjA),
        .pEvtIDs = EvtIDsObjA,
    };
    TOS_Result_T Result = PLT_IOC_subEVT_inConlesMode(&EvtSubArgsObjA);
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS) << "ObjA subEVT() return " << Result;  // KeyCheckPoint

    usleep(100);

    IOC_EvtUnsubArgs_T EvtUnsubArgsObjA = {.CbProcEvt_F = _UT_Case02_CbProcEvtObjA_F, .pCbPriv = pCbPrivObjA};
    Result = PLT_IOC_unsubEVT_inConlesMode(&EvtUnsubArgsObjA);
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS) << "ObjA unsubEVT() return " << Result;  // KeyCheckPoint
  }

  return NULL;
}

static void* _UT_Case02_ThreadObjB(void* pArg) {
  IOC_EvtDesc_T EvtDescObjB = {.EvtID = IOC_EVTID_TEST_KEEPALIVE};
  _UT_Case02_CbPrivObjB_pT pCbPrivObjB = (_UT_Case02_CbPrivObjB_pT)pArg;

  for (int i = 0; i < _UT_CASE02_KEEPALIVE_EVT_CNT; i++) {
    TOS_Result_T Result = PLT_IOC_postEVT_inConlesMode(&EvtDescObjB, NULL);
    if (Result == TOS_RESULT_SUCCESS) {
      pCbPrivObjB->PostEvtRetSuccessCnt++;
    } else if (Result == TOS_RESULT_NO_EVT_SUBER) {
      pCbPrivObjB->PostEvtRetNoEvtSuberCnt++;
    } else {
      EXPECT_TRUE(false) << "ObjB postEVT() return " << Result;  // @CheckPoint
    }

    usleep(10);
  }

  return NULL;
}

TEST(UT_ConlesEventState, Case02_verifyUnsubEvtInLinkStateReady_byDynamicSubUnsubEvtMultiTimes) {
  // Step-1: Start Thread_ObjA to dynamiclly Sub/UnSubEvt(TEST_KEEPALIVE) $_UT_CASE02_DYNAMIC_SUBUNSUB_CNT times
  pthread_t Thread_ObjA;
  _UT_Case02_CbPrivObjA_T CbPrivObjA = {.ProcKeepAliveEvtCnt = 0};
  int PSXRet = pthread_create(&Thread_ObjA, NULL, _UT_Case02_ThreadObjA, &CbPrivObjA);
  ASSERT_EQ(PSXRet, 0);  // CheckPoint

  // Step-2: Start Thread_ObjB to postEvt(TEST_KEEPALIVE * $_UT_CASE02_KEEPALIVE_EVT_CNT) every 1ms
  pthread_t Thread_ObjB;
  _UT_Case02_CbPrivObjB_T CbPrivObjB = {.PostEvtRetSuccessCnt = 0, .PostEvtRetNoEvtSuberCnt = 0};
  PSXRet = pthread_create(&Thread_ObjB, NULL, _UT_Case02_ThreadObjB, &CbPrivObjB);
  ASSERT_EQ(PSXRet, 0);  // CheckPoint

  // Step-3: Wait Thread_ObjA/ObjB exit
  pthread_join(Thread_ObjA, NULL);
  pthread_join(Thread_ObjB, NULL);

  //===VERIFY===
  ASSERT_GT(CbPrivObjA.ProcKeepAliveEvtCnt, 0);                                                                   // KeyCheckPoint
  ASSERT_EQ(CbPrivObjA.ProcKeepAliveEvtCnt, CbPrivObjB.PostEvtRetSuccessCnt);                                     // KeyCheckPoint
  ASSERT_GT(CbPrivObjB.PostEvtRetNoEvtSuberCnt, 0);                                                               // KeyCheckPoint
  ASSERT_EQ(CbPrivObjB.PostEvtRetSuccessCnt + CbPrivObjB.PostEvtRetNoEvtSuberCnt, _UT_CASE02_KEEPALIVE_EVT_CNT);  // KeyCheckPoint

  //===CLEANUP===
}

//===> Case[03]: Same as Case[01/02], but ObjB/C/D/E postEvt in a thread,
//      while ObjA CbProcEvt_F() one-by-one according to LinkStateBusy.
//  Step-1: Same with Case[01]-Step-1
//  Step-2: Start Thread_ObjB/C/D/E to postEvt(TEST_KEEPALIVE*$_UT_KEEPALIVE_EVT_CNT) every 1ms
//  Stet-3: ObjA sync its CbProcEvt_F() one-by-one in main thread, and wait/usleep 2ms in CbProcEvt_F
//  Step-4: ObjA unsubEvt(TEST_KEEPALIVE) when all CbProcEvt_F() is done
//  Step-5: Check ObjA's private TotalEvtCnt is $_UT_KEEPALIVE_EVT_CNT*4, and TryLockFailCnt is 0

typedef struct {
  ULONG_T TotalEvtCnt;
  ULONG_T TryLockFailCnt;

  pthread_mutex_t Mutex;

  sem_t* pSemCbProcEvtEnterSync;
  sem_t* pSemCbProcEvtContinueSync;
} _UT_Case03_CbPrivObjA_T, *_UT_Case03_CbPrivObjA_pT;

static TOS_Result_T _UT_Case03_CbProcEvtObjA_F(IOC_EvtDesc_pT pEvtDesc, void* pCbPriv) {
  _UT_Case03_CbPrivObjA_pT pCbPrivObjA = (_UT_Case03_CbPrivObjA_pT)pCbPriv;

  int PSXRet = pthread_mutex_trylock(&pCbPrivObjA->Mutex);
  if (PSXRet != 0) {
    pCbPrivObjA->TryLockFailCnt++;
    return TOS_RESULT_BAD_STATE;
  }

  pCbPrivObjA->TotalEvtCnt++;
  usleep(2);

  sem_post(pCbPrivObjA->pSemCbProcEvtEnterSync);
  sem_wait(pCbPrivObjA->pSemCbProcEvtContinueSync);

  pthread_mutex_unlock(&pCbPrivObjA->Mutex);
  return TOS_RESULT_SUCCESS;
}

#define _UT_KEEPALIVE_EVT_CNT 1000

static void* _UT_Case03_ThreadObjX(void* pArg) {
  IOC_LinkID_T LinkID = IOC_CONLESMODE_AUTO_LINK_ID;
  IOC_EvtDesc_T EvtDescObjX = {.EvtID = IOC_EVTID_TEST_KEEPALIVE};
  ULONG_T TotalEvtCnt = 0;

  while (1) {
    PLT_IOC_postEVT(LinkID, &EvtDescObjX, NULL);
    // usleep(1);

    TotalEvtCnt++;
    if (TotalEvtCnt >= _UT_KEEPALIVE_EVT_CNT) {
      break;
    }
  }

  return NULL;
}

TEST(UT_ConlesEventState, Case03) {
  IOC_LinkID_T LinkID = IOC_CONLESMODE_AUTO_LINK_ID;

  // Step-1: ObjA as EvtSuber subEvt(TEST_KEEPALIVE), set ObjA's private magic value X
  _UT_Case03_CbPrivObjA_T CbPrivObjA = {};
  CbPrivObjA.TotalEvtCnt = 0;
  CbPrivObjA.TryLockFailCnt = 0;
  pthread_mutex_init(&CbPrivObjA.Mutex, NULL);
  CbPrivObjA.pSemCbProcEvtContinueSync = sem_open("/UT_Case03_CbProcEvtContinueSync", O_CREAT, 0644, 0);
  ASSERT_NE(nullptr, CbPrivObjA.pSemCbProcEvtContinueSync);
  CbPrivObjA.pSemCbProcEvtEnterSync = sem_open("/UT_Case03_CbProcEvtEnterSync", O_CREAT, 0644, 0);
  ASSERT_NE(nullptr, CbPrivObjA.pSemCbProcEvtEnterSync);

  IOC_EvtID_T EvtIDsObjA[] = {IOC_EVTID_TEST_KEEPALIVE};
  IOC_EvtSubArgs_T EvtSubArgsObjA = {
      .CbProcEvt_F = _UT_Case03_CbProcEvtObjA_F,
      .pCbPriv = &CbPrivObjA,
      .EvtNum = TOS_calcArrayElmtCnt(EvtIDsObjA),
      .pEvtIDs = EvtIDsObjA,
  };
  TOS_Result_T Result = PLT_IOC_subEVT(LinkID, &EvtSubArgsObjA);
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint

  // Step-2: Start Thread_ObjB/C/D/E to postEvt(TEST_KEEPALIVE*$_UT_KEEPALIVE_EVT_CNT) every 1ms
  pthread_t Thread_ObjB, Thread_ObjC, Thread_ObjD, Thread_ObjE;
  int PSXRet = pthread_create(&Thread_ObjB, NULL, _UT_Case03_ThreadObjX, NULL);
  ASSERT_EQ(PSXRet, 0);  // CheckPoint
  PSXRet = pthread_create(&Thread_ObjC, NULL, _UT_Case03_ThreadObjX, NULL);
  ASSERT_EQ(PSXRet, 0);  // CheckPoint
  PSXRet = pthread_create(&Thread_ObjD, NULL, _UT_Case03_ThreadObjX, NULL);
  ASSERT_EQ(PSXRet, 0);  // CheckPoint
  PSXRet = pthread_create(&Thread_ObjE, NULL, _UT_Case03_ThreadObjX, NULL);
  ASSERT_EQ(PSXRet, 0);  // CheckPoint

  // Stet-3: ObjA sync its CbProcEvt_F() one-by-one in main thread, and wait/usleep 2ms in CbProcEvt_F
  for (int i = 0; i < _UT_KEEPALIVE_EVT_CNT * 4; i++) {
    sem_wait(CbPrivObjA.pSemCbProcEvtEnterSync);
    sem_post(CbPrivObjA.pSemCbProcEvtContinueSync);
  }

  // Step-4: ObjA unsubEvt(TEST_KEEPALIVE) when all CbProcEvt_F() is done
  IOC_EvtUnsubArgs_T EvtUnsubArgsObjA = {.CbProcEvt_F = _UT_Case03_CbProcEvtObjA_F, .pCbPriv = &CbPrivObjA};
  Result = PLT_IOC_unsubEVT(LinkID, &EvtUnsubArgsObjA);
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint

  // Step-5: Check ObjA's private TotalEvtCnt is $_UT_KEEPALIVE_EVT_CNT*4, and TryLockFailCnt is 0
  ASSERT_EQ(CbPrivObjA.TotalEvtCnt, _UT_KEEPALIVE_EVT_CNT * 4);  // CheckPoint
  ASSERT_EQ(CbPrivObjA.TryLockFailCnt, 0);                       // CheckPoint

  // TEARDOWN: Wait Thread_ObjB/C/D/E exit
  pthread_join(Thread_ObjB, NULL);
  pthread_join(Thread_ObjC, NULL);
  pthread_join(Thread_ObjD, NULL);
  pthread_join(Thread_ObjE, NULL);

  pthread_mutex_destroy(&CbPrivObjA.Mutex);
  sem_close(CbPrivObjA.pSemCbProcEvtEnterSync);
  sem_close(CbPrivObjA.pSemCbProcEvtContinueSync);
}
