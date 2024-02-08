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
  ULONG_T MagicValue;
} _UT_Case01_CbPrivObjA_T, *_UT_Case01_CbPrivObjA_pT;

static TOS_Result_T _UT_Case01_CbProcEvtObjA_F(IOC_EvtDesc_pT pEvtDesc, void* pCbPriv) {
  _UT_Case01_CbPrivObjA_pT pCbPrivObjA = (_UT_Case01_CbPrivObjA_pT)pCbPriv;
  sleep(3);
  pCbPrivObjA->MagicValue = 0x87654321;
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

//===> Case[02]: Same as Case[01], but ObjB postEvt in a thread context
//  Step-1: Same with Case[01]-Step-1
//  Step-2: Start Thread_ObjB to postEvt(TEST_KEEPALIVE)
//  Step-3: ObjA wait its CbProcEvt_F is callbacking as its private magic value is Y now
//  Step-4: ObjA unsubEvt(TEST_KEEPALIVE)
//  Step-5: Check ObjA's private magic value is Z now

typedef enum {
  _UT_Case02_MagicValue_X = 0x12345678,
  _UT_Case02_MagicValue_Y = 0x87654321,
  _UT_Case02_MagicValue_Z = 0x13572468,
} _UT_Case02_MagicValue_T;

typedef struct {
  _UT_Case02_MagicValue_T MagicValue;
} _UT_Case02_CbPrivObjA_T, *_UT_Case02_CbPrivObjA_pT;

static TOS_Result_T _UT_Case02_CbProcEvtObjA_F(IOC_EvtDesc_pT pEvtDesc, void* pCbPriv) {
  _UT_Case02_CbPrivObjA_pT pCbPrivObjA = (_UT_Case02_CbPrivObjA_pT)pCbPriv;
  pCbPrivObjA->MagicValue = _UT_Case02_MagicValue_Y;
  sleep(3);
  pCbPrivObjA->MagicValue = _UT_Case02_MagicValue_Z;
  return TOS_RESULT_SUCCESS;
}

static void* _UT_Case02_ThreadObjB(void* pArg) {
  IOC_LinkID_T LinkID = IOC_CONLESMODE_AUTO_LINK_ID;
  IOC_EvtDesc_T EvtDescObjB = {.EvtID = IOC_EVTID_TEST_KEEPALIVE};

  while (1) {
    PLT_IOC_postEVT(LinkID, &EvtDescObjB, NULL);
    usleep(1);
  }

  return NULL;
}

TEST(ConlesModeState, Case02) {
  IOC_LinkID_T LinkID = IOC_CONLESMODE_AUTO_LINK_ID;

  // Step-1: ObjA as EvtSuber subEvt(TEST_KEEPALIVE), set ObjA's private magic value X
  _UT_Case02_CbPrivObjA_T CbPrivObjA = {};
  CbPrivObjA.MagicValue = _UT_Case02_MagicValue_X;

  IOC_EvtID_T EvtIDsObjA[] = {IOC_EVTID_TEST_KEEPALIVE};
  IOC_EvtSubArgs_T EvtSubArgsObjA = {
      .CbProcEvt_F = _UT_Case02_CbProcEvtObjA_F,
      .pCbPriv = &CbPrivObjA,
      .EvtNum = TOS_calcArrayElmtCnt(EvtIDsObjA),
      .pEvtIDs = EvtIDsObjA,
  };
  TOS_Result_T Result = PLT_IOC_subEVT(LinkID, &EvtSubArgsObjA);
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint

  // Step-2: Start Thread_ObjB to postEvt(TEST_KEEPALIVE)
  pthread_t Thread_ObjB;
  int PSXRet = pthread_create(&Thread_ObjB, NULL, _UT_Case02_ThreadObjB, NULL);
  ASSERT_EQ(PSXRet, 0);  // CheckPoint

  // Step-3: ObjA wait its CbProcEvt_F is callbacking
  while (CbPrivObjA.MagicValue != _UT_Case02_MagicValue_Y) {
    usleep(1);
  }

  // Step-4: ObjA unsubEvt(TEST_KEEPALIVE)
  IOC_EvtUnsubArgs_T EvtUnsubArgsObjA = {.CbProcEvt_F = _UT_Case02_CbProcEvtObjA_F, .pCbPriv = &CbPrivObjA};
  Result = PLT_IOC_unsubEVT(LinkID, &EvtUnsubArgsObjA);
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint

  // Step-5: Check ObjA's private magic value is Y now
  ASSERT_EQ(CbPrivObjA.MagicValue, _UT_Case02_MagicValue_Z);  // CheckPoint

  // TEARDOWN: Cancel&Wait Thread_ObjB exit
  pthread_cancel(Thread_ObjB);
  pthread_join(Thread_ObjB, NULL);
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

#define _UT_KEEPALIVE_EVT_CNT 200000

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

TEST(ConlesModeState, Case03) {
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
