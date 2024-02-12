#include "_UT_IOC_Common.h"

// This is IOC_Event in ConlesMode's Concurrency UT from API user's perspective
// Concurrency here means multiple objects using same $AutoLinkID to postEvt and subEvt,
//  and their behavior is still work as expected.

//===>Case[01]: ObjA as EvtSuber, ObjB/C as EvtPuber,
//      ObjB postEvt(TEST_MAYBLOCK_SLEEP_1MS * $_UT_EVT_CNT_MAYBLOCK),
//      ObjC postEvt(TEST_NONBLOCK_SLEEP_5MS * $_UT_EVT_CNT_NONBLOCK).
//    Expect:
//      ObjA's EvtCntBlockSleep1MS is $_UT_EVT_CNT_MAYBLOCK
//      ObjA's EvtCntNonBlockSleep5MS>0 && ObjC's EvtCntPostRstTimeout>0
//          && (EvtCntNonBlockSleep5MS + ObjC's EvtCntPostRstTimeout) == $_UT_EVT_CNT_NONBLOCK
//      AND ObjB's postEvt() PerfFAST(FROM<=100us,TO<=5ms(+~3ms)) caused by MAYBLOCK of TEST_NONBLOCK_SLEEP_5MS,
//          ObjC's postEvt() PerfFAST(<=100us,TO<=100us) caused by NONBLOCK.
//
//  Step-1:

typedef struct {
  ULONG_T EvtCntBlockSleep1MS;
  ULONG_T EvtCntNonBlockSleep5MS;
} _UT_Case01_CbPrivObjA_T, *_UT_Case01_CbPrivObjA_pT;

static TOS_Result_T _UT_Case01_CbProcEvtObjA_F(IOC_EvtDesc_pT pEvtDesc, void* pCbPriv) {
  _UT_Case01_CbPrivObjA_pT pCbPrivObjA = (_UT_Case01_CbPrivObjA_pT)pCbPriv;

  if (pEvtDesc->EvtID == IOC_EVTID_TEST_MAYBLOCK_SLEEP_1MS) {
    pCbPrivObjA->EvtCntBlockSleep1MS++;
    usleep(1000);
  } else if (pEvtDesc->EvtID == IOC_EVTID_TEST_NONBLOCK_SLEEP_5MS) {
    pCbPrivObjA->EvtCntNonBlockSleep5MS++;
    usleep(5000);
  } else {
    EXPECT_TRUE(false) << "Unknown EvtID: " << pEvtDesc->EvtID;
  }

  return TOS_RESULT_SUCCESS;
}

#define _UT_EVT_CNT_MAYBLOCK 1000
#define _UT_EVT_CNT_NONBLOCK 1000000

static void* _UT_Case01_ThreadObjB(void* pArg) {
  IOC_EvtDesc_T EvtDescObjB = {.EvtID = IOC_EVTID_TEST_MAYBLOCK_SLEEP_1MS};

  for (int i = 0; i < _UT_EVT_CNT_MAYBLOCK; i++) {
    struct timeval BeforePostTime, AfterPostTime;

    gettimeofday(&BeforePostTime, NULL);
    TOS_Result_T Result = PLT_IOC_postEVT_inConlesMode(&EvtDescObjB, NULL);
    gettimeofday(&AfterPostTime, NULL);

    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint

    ULONG_T PostTimeUS = (AfterPostTime.tv_sec - BeforePostTime.tv_sec) * 1000000 + AfterPostTime.tv_usec - BeforePostTime.tv_usec;
    EXPECT_LE(PostTimeUS, 8000);  // CheckPoint@Performance_FAST(<=100us)@MAYBLOCK(<=5ms + ~3ms)

    usleep(30);
  }
  return NULL;
}

typedef struct {
  ULONG_T EvtCntPostRstTimeout;
} _UT_Case01_CbPrivObjC_T, *_UT_Case01_CbPrivObjC_pT;

static void* _UT_Case01_ThreadObjC(void* pArg) {
  _UT_Case01_CbPrivObjC_pT pCbPrivObjC = (_UT_Case01_CbPrivObjC_pT)pArg;
  IOC_EvtDesc_T EvtDescObjC = {.EvtID = IOC_EVTID_TEST_NONBLOCK_SLEEP_5MS};

  for (int i = 0; i < _UT_EVT_CNT_NONBLOCK; i++) {
    IOC_Options_T OptTimeout = {.IDs = IOC_OPTID_TIMEOUT, .Payload{.TimeoutUS = 0 /*nonblock*/}};
    struct timeval BeforePostTime, AfterPostTime;

    gettimeofday(&BeforePostTime, NULL);
    TOS_Result_T Result = PLT_IOC_postEVT_inConlesMode(&EvtDescObjC, &OptTimeout);
    gettimeofday(&AfterPostTime, NULL);

    if (Result == TOS_RESULT_TIMEOUT) {
      pCbPrivObjC->EvtCntPostRstTimeout++;
    } else {
      EXPECT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint
    }

    ULONG_T PostTimeUS = (AfterPostTime.tv_sec - BeforePostTime.tv_sec) * 1000000 + AfterPostTime.tv_usec - BeforePostTime.tv_usec;
    EXPECT_LE(PostTimeUS, 100);  // CheckPoint@Performance_FAST(<=100us)@NONBLOCK

    usleep(1);
  }
  return NULL;
}

TEST(UT_ConlesEventConcurrency, Case01) {
  //===SETUP===
  // Step-1: ObjA as EvtSuber subEvt(TEST_BLOCK_SLEEP_5MS, TEST_NONBLOCK_SLEEP_1MS)
  _UT_Case01_CbPrivObjA_T CbPrivObjA = {.EvtCntBlockSleep1MS = 0, .EvtCntNonBlockSleep5MS = 0};
  IOC_EvtID_T EvtIDsObjA[] = {IOC_EVTID_TEST_MAYBLOCK_SLEEP_1MS, IOC_EVTID_TEST_NONBLOCK_SLEEP_5MS};
  IOC_EvtSubArgs_T EvtSubArgsObjA = {
      .CbProcEvt_F = _UT_Case01_CbProcEvtObjA_F,
      .pCbPriv = &CbPrivObjA,
      .EvtNum = TOS_calcArrayElmtCnt(EvtIDsObjA),
      .pEvtIDs = EvtIDsObjA,
  };

  TOS_Result_T Result = PLT_IOC_subEVT_inConlesMode(&EvtSubArgsObjA);
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint

  //===EXECUTE===
  // Step-2: ObjB/C as EvtPuber postEvt(TEST_BLOCK_SLEEP_5MS, TEST_NONBLOCK_SLEEP_1MS) in each's thread context
  pthread_t ThreadID[2];  // ObjB/C
  pthread_create(&ThreadID[0], NULL, _UT_Case01_ThreadObjB, NULL);

  _UT_Case01_CbPrivObjC_T CbPrivObjC = {.EvtCntPostRstTimeout = 0};
  pthread_create(&ThreadID[1], NULL, _UT_Case01_ThreadObjC, &CbPrivObjC);

  // Step-3: Wait for all ObjB/C's thread to finish
  pthread_join(ThreadID[0], NULL);
  pthread_join(ThreadID[1], NULL);

  usleep(10000);  // A hard-coded wait sleep to wait for all CbProcEvt_F() is done

  //===VERIFY===
  // Step-4: Check ObjA's EvtCntBlockSleep1MS is $_UT_EVT_CNT_MAYBLOCK
  ASSERT_EQ(CbPrivObjA.EvtCntBlockSleep1MS, _UT_EVT_CNT_MAYBLOCK);  // CheckPoint

  // Step-5: Check ObjA's EvtCntNonBlockSleep5MS>0 && EvtCntPostRstTimeout>0
  //          && (EvtCntNonBlockSleep5MS + ObjC's EvtCntPostRstTimeout) == $_UT_EVT_CNT_NONBLOCK
  ASSERT_GT(CbPrivObjA.EvtCntNonBlockSleep5MS, 0);                                                       // CheckPoint
  ASSERT_GT(CbPrivObjC.EvtCntPostRstTimeout, 0);                                                         // CheckPoint
  ASSERT_EQ(CbPrivObjA.EvtCntNonBlockSleep5MS + CbPrivObjC.EvtCntPostRstTimeout, _UT_EVT_CNT_NONBLOCK);  // CheckPoint

  //===CLEANUP===
  // Step-6: ObjA unsubEvt(TEST_BLOCK_SLEEP_5MS, TEST_NONBLOCK_SLEEP_1MS) when all CbProcEvt_F() is done
  IOC_EvtUnsubArgs_T EvtUnsubArgsObjA = {
      .CbProcEvt_F = _UT_Case01_CbProcEvtObjA_F,
      .pCbPriv = &CbPrivObjA,
  };
  Result = PLT_IOC_unsubEVT_inConlesMode(&EvtUnsubArgsObjA);
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // Check
}

//===>Case[02]: ObjA/B/C/... as EvtSuber, ObjZ as EvtPuber to postEVT(TEST_MAYBLOCK_SLEEP_1MS)
//  ObjA/...(=$_UT_CASE02_EVT_SUBER_CNT) call subEVT_inConlesMode(TEST_MAYBLOCK_SLEEP_1MS)
//  ObjZ call postEVT_inConlesMode(TEST_MAYBLOCK_SLEEP_1MS * $_UT_CASE02_MAYBLOCK_EVT_CNT)
//  Expect:
//    Each ObjA/...'s CbProcEvt_F() is callbacked $_UT_CASE02_MAYBLOCK_EVT_CNT times
//    AND ObjZ's postEVT_inConlesMode() PerfFAST(FROM<=100us,TO<=1ms * $_UT_CASE02_EVT_SUBER_CNT)

typedef struct {
  ULONG_T EvtCntMaySleep1MS;
} _UT_Case02_CbPrivObjX_T, *_UT_Case02_CbPrivObjX_pT;

static TOS_Result_T _UT_Case02_CbProcEvtObjX_F(IOC_EvtDesc_pT pEvtDesc, void* pCbPriv) {
  _UT_Case02_CbPrivObjX_pT pCbPrivObjX = (_UT_Case02_CbPrivObjX_pT)pCbPriv;

  if (pEvtDesc->EvtID == IOC_EVTID_TEST_MAYBLOCK_SLEEP_1MS) {
    pCbPrivObjX->EvtCntMaySleep1MS++;
    usleep(1000);
  } else {
    EXPECT_TRUE(false) << "Unknown EvtID: " << pEvtDesc->EvtID;
  }

  return TOS_RESULT_SUCCESS;
}

TEST(UT_ConlesEventConcurrency, Case02) {
#define _UT_CASE02_EVT_SUBER_CNT 10
#define _UT_CASE02_MAYBLOCK_EVT_CNT 1000

  //===SETUP===
  // Step-1: ObjA/... as EvtSuber subEvt(TEST_MAYBLOCK_SLEEP_1MS)
  _UT_Case02_CbPrivObjX_T CbPrivObjX[_UT_CASE02_EVT_SUBER_CNT] = {};

  for (int i = 0; i < _UT_CASE02_EVT_SUBER_CNT; i++) {
    IOC_EvtID_T EvtIDsObjX[] = {IOC_EVTID_TEST_MAYBLOCK_SLEEP_1MS};
    IOC_EvtSubArgs_T EvtSubArgsObjX = {
        .CbProcEvt_F = _UT_Case02_CbProcEvtObjX_F,
        .pCbPriv = &CbPrivObjX[i],
        .EvtNum = TOS_calcArrayElmtCnt(EvtIDsObjX),
        .pEvtIDs = EvtIDsObjX,
    };

    TOS_Result_T Result = PLT_IOC_subEVT_inConlesMode(&EvtSubArgsObjX);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint
  }

  //===EXECUTE===
  // Step-2: ObjZ as EvtPuber postEvt(TEST_MAYBLOCK_SLEEP_1MS * $_UT_CASE02_MAYBLOCK_EVT_CNT)
  for (int i = 0; i < _UT_CASE02_MAYBLOCK_EVT_CNT; i++) {
    struct timeval BeforePostTime, AfterPostTime;
    IOC_EvtDesc_T EvtDescObjZ = {.EvtID = IOC_EVTID_TEST_MAYBLOCK_SLEEP_1MS};

    gettimeofday(&BeforePostTime, NULL);
    TOS_Result_T Result = PLT_IOC_postEVT_inConlesMode(&EvtDescObjZ, NULL);
    gettimeofday(&AfterPostTime, NULL);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint

    ULONG_T PostTimeUS = (AfterPostTime.tv_sec - BeforePostTime.tv_sec) * 1000000 + AfterPostTime.tv_usec - BeforePostTime.tv_usec;
    ULONG_T PostBlockTimeUS = (1000 + 500) * _UT_CASE02_EVT_SUBER_CNT;  // 500=ASyncPerfFAST(<=500us) vs 100=SyncPerfFAST(<=100us)
                                                                        // 1000=MayBlock@CbProcEvt_Sleep1MS
    EXPECT_LE(PostTimeUS, PostBlockTimeUS);                             // CheckPoint
  }

  // A hard-coded wait sleep to wait for all CbProcEvt_F() is done
  sleep(1);

  //===VERIFY===
  // Step-3: Each ObjA/...'s CbProcEvt_F() is callbacked $_UT_CASE02_MAYBLOCK_EVT_CNT times
  for (int i = 0; i < _UT_CASE02_EVT_SUBER_CNT; i++) {
    ASSERT_EQ(CbPrivObjX[i].EvtCntMaySleep1MS, _UT_CASE02_MAYBLOCK_EVT_CNT);  // CheckPoint
  }

  //===CLEANUP===
  // Step-4: ObjA/... unsubEvt(TEST_MAYBLOCK_SLEEP_1MS) when all CbProcEvt_F() is done
  for (int i = 0; i < _UT_CASE02_EVT_SUBER_CNT; i++) {
    IOC_EvtUnsubArgs_T EvtUnsubArgsObjX = {
        .CbProcEvt_F = _UT_Case02_CbProcEvtObjX_F,
        .pCbPriv = &CbPrivObjX[i],
    };
    TOS_Result_T Result = PLT_IOC_unsubEVT_inConlesMode(&EvtUnsubArgsObjX);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint
  }
}

//===>Case[03]: ObjA/B/C/... as EvtSuber, ObjZ as EvtPuber to postEVT(TEST_NONBLOCK_SLEEP_5MS)
// ObjA/...(=$_UT_CASE03_EVT_SUBER_CNT) call subEVT_inConlesMode(TEST_NONBLOCK_SLEEP_5MS)
// ObjZ call postEVT_inConlesMode(TEST_NONBLOCK_SLEEP_5MS * $_UT_CASE03_NONBLOCK_EVT_CNT)
// Expect:
//   Each ObjA/...'s CbProcEvt_F() is callbacked $ProcedEvtCntSleep5MS<=$_UT_CASE03_NONBLOCK_EVT_CNT times
//     AND (EACH(ObjA/...'s $ProcedEvtCntSleep5MS>0) AND + $PostEvtGetTimeoutRstCnt) == $_UT_CASE03_NONBLOCK_EVT_CNT
//   AND ObjZ's postEVT_inConlesMode() meets SPEC_PerfFAST_ASyncNonBlock(<=10us)

typedef struct {
  ULONG_T ProcedEvtCntSleep5MS;
} _UT_Case03_CbPrivObjX_T, *_UT_Case03_CbPrivObjX_pT;

static TOS_Result_T _UT_Case03_CbProcEvtObjX_F(IOC_EvtDesc_pT pEvtDesc, void* pCbPriv) {
  _UT_Case03_CbPrivObjX_pT pCbPrivObjX = (_UT_Case03_CbPrivObjX_pT)pCbPriv;

  if (pEvtDesc->EvtID == IOC_EVTID_TEST_NONBLOCK_SLEEP_5MS) {
    pCbPrivObjX->ProcedEvtCntSleep5MS++;
    usleep(5000);
  } else {
    EXPECT_TRUE(false) << "Unknown EvtID: " << pEvtDesc->EvtID;
  }

  return TOS_RESULT_SUCCESS;
}

TEST(UT_ConlesEventConcurrency, Case03) {
#define _UT_CASE03_EVT_SUBER_CNT 10  // ObjA/...
#define _UT_CASE03_NONBLOCK_EVT_CNT 1000

  //===SETUP===
  // Step-1: ObjA/... as EvtSuber subEvt(TEST_NONBLOCK_SLEEP_5MS)
  _UT_Case03_CbPrivObjX_T CbPrivObjX[_UT_CASE03_EVT_SUBER_CNT] = {};

  for (int i = 0; i < _UT_CASE03_EVT_SUBER_CNT; i++) {
    IOC_EvtID_T EvtIDsObjX[] = {IOC_EVTID_TEST_NONBLOCK_SLEEP_5MS};
    IOC_EvtSubArgs_T EvtSubArgsObjX = {
        .CbProcEvt_F = _UT_Case03_CbProcEvtObjX_F,
        .pCbPriv = &CbPrivObjX[i],
        .EvtNum = TOS_calcArrayElmtCnt(EvtIDsObjX),
        .pEvtIDs = EvtIDsObjX,
    };

    TOS_Result_T Result = PLT_IOC_subEVT_inConlesMode(&EvtSubArgsObjX);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint
  }

  //===EXECUTE===
  ULONG_T PostEvtGetTimeoutRstCnt = 0;
  // Step-2: ObjZ as EvtPuber postEvt(TEST_NONBLOCK_SLEEP_5MS * $_UT_CASE03_NONBLOCK_EVT_CNT)
  for (int i = 0; i < _UT_CASE03_NONBLOCK_EVT_CNT; i++) {
    struct timeval BeforePostTime, AfterPostTime;
    IOC_EvtDesc_T EvtDescObjZ = {.EvtID = IOC_EVTID_TEST_NONBLOCK_SLEEP_5MS};
    IOC_Options_T OptTimeout = {.IDs = IOC_OPTID_TIMEOUT, .Payload{.TimeoutUS = 0 /*nonblock*/}};

    gettimeofday(&BeforePostTime, NULL);
    TOS_Result_T Result = PLT_IOC_postEVT_inConlesMode(&EvtDescObjZ, &OptTimeout);
    gettimeofday(&AfterPostTime, NULL);
    if (Result == TOS_RESULT_TIMEOUT) {
      PostEvtGetTimeoutRstCnt++;
    } else {
      ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint
    }

    ULONG_T PostTimeUS = (AfterPostTime.tv_sec - BeforePostTime.tv_sec) * 1000000 + AfterPostTime.tv_usec - BeforePostTime.tv_usec;
    EXPECT_LE(PostTimeUS, 10);  // CheckPoint@Performance_FAST(<=10us)@ASyncNonBlock

    usleep(1000);
  }

  // A hard-coded wait sleep to wait for all CbProcEvt_F() is done
  sleep(1);

  //===VERIFY===
  // Step-3: Each ObjA/...'s CbProcEvt_F() is callbacked $ProcedEvtCntSleep5MS<=$_UT_CASE03_NONBLOCK_EVT_CNT times
  for (int i = 0; i < _UT_CASE03_EVT_SUBER_CNT; i++) {
    ASSERT_GT(CbPrivObjX[i].ProcedEvtCntSleep5MS, 0) << "Failed for i = " << i;                            // CheckPoint
    ASSERT_LT(CbPrivObjX[i].ProcedEvtCntSleep5MS, _UT_CASE03_NONBLOCK_EVT_CNT) << "Failed for i = " << i;  // CheckPoint
    ASSERT_EQ(CbPrivObjX[i].ProcedEvtCntSleep5MS + PostEvtGetTimeoutRstCnt, _UT_CASE03_NONBLOCK_EVT_CNT)
        << "Failed for i = " << i;  // CheckPoint
  }

  //===CLEANUP===
  // Step-4: ObjA/... unsubEvt(TEST_NONBLOCK_SLEEP_5MS) when all CbProcEvt_F() is done
  for (int i = 0; i < _UT_CASE03_EVT_SUBER_CNT; i++) {
    IOC_EvtUnsubArgs_T EvtUnsubArgsObjX = {
        .CbProcEvt_F = _UT_Case03_CbProcEvtObjX_F,
        .pCbPriv = &CbPrivObjX[i],
    };
    TOS_Result_T Result = PLT_IOC_unsubEVT_inConlesMode(&EvtUnsubArgsObjX);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS) << "Failed for i = " << i;  // CheckPoint
  }
}

//===>Case[04]: ObjA as EvtSuber, ObjB/C/D/... as EvtPuber to postEVT(TEST_MAYBLOCK_SLEEP_1MS)
// TODO(@W)
//===>Case[05]: ObjA as EvtSuber, ObjB/C/D/... as EvtPuber to postEVT(TEST_NONBLOCK_SLEEP_5MS)
// TODO(@W)