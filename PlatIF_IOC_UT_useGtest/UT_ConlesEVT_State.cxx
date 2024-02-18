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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @[Name]: verifyCbProcEvtInLinkStateBusy_byPostEvtSimultaneouslyInMultiThread
 * @[Purpose]: verify FSM_ofConlesEVT's FSM-2/3, which means each Link's CbProcEvt_F will be called one-by-one,
 *      even multipy EvtPubers postEvt() simultaneously in multi-thread.
 *    Such as: ObjA as EvtSuber call subEvt() with CbProcEvt_F, ObjB/C/... as EvtPuber call postEvt() simultaneously,
 *      then ObjA's CbProcEvt_F will be callbacked one-by-one according to LinkStateBusy.
 *
 * @[Steps]: ${how to do}
 *   1. <SETUP>Same with Case[01]-Step-1, in CbProcEvt_F():
 *      |-> a. tryLock() first, save $TryLockFailCnt in CbPrivObjA if failed and return BUG
 *      |-> b. if TEST_KEEPALIVE and save $ProcKeepAliveEvtCnt in CbPrivObjA,
 *      |-> c. usleep(10) then unlock() return
 *   2. <BEHAVIOR>Start Thread_ObjB/C/($_UT_CASE03_EVTPUBER_CNT) to postEvt(TEST_KEEPALIVE*$_UT_CASE03_KEEPALIVE_EVT_CNT) every 10us
 *   3. <BEHAVIOR>Wait Thread_ObjB/C... completion and hard-wait a while(100ms)
 *   4. <VERIFY>Check ObjA's private counter values
 *   5. <CLEANUP>ObjA unsubEvt(TEST_KEEPALIVE)
 * @[Expect]: ObjA's $ProcKeepAliveEvtCnt == $_UT_CASE03_KEEPALIVE_EVT_CNT*$_UT_CASE03_EVTPUBER_CNT, and $TryLockFailCnt == 0
 * @[Notes]:
 *    ImplRefCode: UT_ConlesEventState::Case02_verifyUnsubEvtInLinkStateReady_...
 */
#define _UT_CASE03_EVTPUBER_CNT 10
#define _UT_CASE03_KEEPALIVE_EVT_CNT 10000

typedef struct {
  ULONG_T TryLockFailCnt;
  ULONG_T ProcKeepAliveEvtCnt;
  pthread_mutex_t Mutex;
} _UT_Case03_CbPrivObjA_T, *_UT_Case03_CbPrivObjA_pT;

static TOS_Result_T _UT_Case03_CbProcEvtObjA_F(IOC_EvtDesc_pT pEvtDesc, void* pCbPriv) {
  _UT_Case03_CbPrivObjA_pT pCbPrivObjA = (_UT_Case03_CbPrivObjA_pT)pCbPriv;

  if (pthread_mutex_trylock(&pCbPrivObjA->Mutex) != 0) {
    pCbPrivObjA->TryLockFailCnt++;
    return TOS_RESULT_BUG;
  }

  if (pEvtDesc->EvtID == IOC_EVTID_TEST_KEEPALIVE) {
    pCbPrivObjA->ProcKeepAliveEvtCnt++;
  }

  usleep(10);
  pthread_mutex_unlock(&pCbPrivObjA->Mutex);
  return TOS_RESULT_SUCCESS;
}

static void* _UT_Case03_ThreadObjB(void* pArg) {
  IOC_EvtDesc_T EvtDescObjB = {.EvtID = IOC_EVTID_TEST_KEEPALIVE};
  _UT_Case03_CbPrivObjA_pT pCbPrivObjA = (_UT_Case03_CbPrivObjA_pT)pArg;

  for (int i = 0; i < _UT_CASE03_KEEPALIVE_EVT_CNT; i++) {
    TOS_Result_T Result = PLT_IOC_postEVT_inConlesMode(&EvtDescObjB, NULL);
    if (Result != TOS_RESULT_SUCCESS) {
      EXPECT_TRUE(false) << "ObjB postEVT() return " << Result;  // @CheckPoint
    }

    usleep(10);
  }

  return NULL;
}

TEST(UT_ConlesEventState, Case03_verifyCbProcEvtInLinkStateBusy_byPostEvtSimultaneouslyInMultiThread) {
  // Step-1: <SETUP>Same with Case[01]-Step-1
  _UT_Case03_CbPrivObjA_T CbPrivObjA = {.TryLockFailCnt = 0, .ProcKeepAliveEvtCnt = 0};
  pthread_mutex_init(&CbPrivObjA.Mutex, NULL);

  IOC_EvtID_T EvtIDsObjA[] = {IOC_EVTID_TEST_KEEPALIVE};
  IOC_EvtSubArgs_T EvtSubArgsObjA = {
      .CbProcEvt_F = _UT_Case03_CbProcEvtObjA_F,
      .pCbPriv = &CbPrivObjA,
      .EvtNum = TOS_calcArrayElmtCnt(EvtIDsObjA),
      .pEvtIDs = EvtIDsObjA,
  };
  TOS_Result_T Result = PLT_IOC_subEVT_inConlesMode(&EvtSubArgsObjA);
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint

  // Step-2: <BEHAVIOR>Start Thread_ObjB/C/($_UT_CASE03_EVTPUBER_CNT) to postEvt(TEST_KEEPALIVE*$_UT_CASE03_KEEPALIVE_EVT_CNT) every
  // 10us
  pthread_t Threads_ObjB[_UT_CASE03_EVTPUBER_CNT];
  for (int i = 0; i < _UT_CASE03_EVTPUBER_CNT; i++) {
    int PSXRet = pthread_create(&Threads_ObjB[i], NULL, _UT_Case03_ThreadObjB, &CbPrivObjA);
    ASSERT_EQ(PSXRet, 0);  // CheckPoint
  }

  // Step-3: <BEHAVIOR>Wait Thread_ObjB/C... completion and hard-wait a while(100ms)
  for (int i = 0; i < _UT_CASE03_EVTPUBER_CNT; i++) {
    pthread_join(Threads_ObjB[i], NULL);
  }
  usleep(100 * 1000);

  // Step-4: <VERIFY>Check ObjA's private counter values
  ASSERT_EQ(CbPrivObjA.TryLockFailCnt, 0);                                                            // KeyCheckPoint
  ASSERT_EQ(CbPrivObjA.ProcKeepAliveEvtCnt, _UT_CASE03_KEEPALIVE_EVT_CNT * _UT_CASE03_EVTPUBER_CNT);  // KeyCheckPoint

  // Step-5: <CLEANUP>ObjA unsubEvt(TEST_KEEPALIVE)
  IOC_EvtUnsubArgs_T EvtUnsubArgsObjA = {.CbProcEvt_F = _UT_Case03_CbProcEvtObjA_F, .pCbPriv = &CbPrivObjA};
  Result = PLT_IOC_unsubEVT_inConlesMode(&EvtUnsubArgsObjA);
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint
}