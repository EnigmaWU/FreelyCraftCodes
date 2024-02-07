#include <unistd.h>

#include "_UT_IOC_Common.h"

// This is IOC_Event in ConlesMode's Concurrency UT from API user's perspective
// Concurrency here means multiple objects using same $AutoLinkID to postEvt and subEvt,
//  and their behavior is still work as expected.

//===>Case[01]: ObjA as EvtSuber, ObjB/C as EvtPuber,
//      ObjB postEvt(TEST_BLOCK_SLEEP_1MS * $_UT_EVT_CNT_BLOCK),
//      ObjC postEvt(TEST_NONBLOCK_SLEEP_5MS * $_UT_EVT_CNT_NONBLOCK).
//    Expect:
//      ObjA's EvtCntBlockSleep1MS is $_UT_EVT_CNT_BLOCK
//      ObjA's EvtCntNonBlockSleep5MS>0 && EvtCntPostRstTimeout>0
//          && (EvtCntNonBlockSleep5MS + ObjC's EvtCntPostRstTimeout) == $_UT_EVT_CNT_NONBLOCK
//      AND ObjB/C's postEvt() performance is still FAST(<=100us)
//
//  Step-1:

typedef struct {
  unsigned long EvtCntBlockSleep1MS;
  unsigned long EvtCntNonBlockSleep5MS;
} _UT_Case01_CbPrivObjA_T, *_UT_Case01_CbPrivObjA_pT;

static TOS_Result_T _UT_Case01_CbProcEvtObjA_F(IOC_EvtDesc_pT pEvtDesc, void* pCbPriv) {
  _UT_Case01_CbPrivObjA_pT pCbPrivObjA = (_UT_Case01_CbPrivObjA_pT)pCbPriv;

  if (pEvtDesc->EvtID == IOC_EVTID_TEST_BLOCK_SLEEP_1MS) {
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

#define _UT_EVT_CNT_BLOCK 1000
#define _UT_EVT_CNT_NONBLOCK 1000000

static void* _UT_Case01_ThreadObjB(void* pArg) {
  IOC_LinkID_T LinkID = IOC_CONLESMODE_AUTO_LINK_ID;
  IOC_EvtDesc_T EvtDescObjB = {.EvtID = IOC_EVTID_TEST_BLOCK_SLEEP_1MS};

  for (int i = 0; i < _UT_EVT_CNT_BLOCK; i++) {
    struct timeval BeforePostTime, AfterPostTime;

    gettimeofday(&BeforePostTime, NULL);
    TOS_Result_T Result = PLT_IOC_postEVT(LinkID, &EvtDescObjB, NULL);
    gettimeofday(&AfterPostTime, NULL);

    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint

    unsigned long PostTimeUS =
        (AfterPostTime.tv_sec - BeforePostTime.tv_sec) * 1000000 + AfterPostTime.tv_usec - BeforePostTime.tv_usec;
    EXPECT_LE(PostTimeUS, 8000);  // CheckPoint@Performance_FAST(<=100us)@MAYBLOCK(<=5ms + ~3ms)

    usleep(30);
  }
  return NULL;
}

typedef struct {
  unsigned long EvtCntPostRstTimeout;
} _UT_Case01_CbPrivObjC_T, *_UT_Case01_CbPrivObjC_pT;

static void* _UT_Case01_ThreadObjC(void* pArg) {
  _UT_Case01_CbPrivObjC_pT pCbPrivObjC = (_UT_Case01_CbPrivObjC_pT)pArg;
  IOC_LinkID_T LinkID = IOC_CONLESMODE_AUTO_LINK_ID;
  IOC_EvtDesc_T EvtDescObjC = {.EvtID = IOC_EVTID_TEST_NONBLOCK_SLEEP_5MS};

  for (int i = 0; i < _UT_EVT_CNT_NONBLOCK; i++) {
    IOC_Options_T OptTimeout = {.IDs = IOC_OPTID_TIMEOUT, .Payload{.TimeoutUS = 0 /*nonblock*/}};
    struct timeval BeforePostTime, AfterPostTime;

    gettimeofday(&BeforePostTime, NULL);
    TOS_Result_T Result = PLT_IOC_postEVT(LinkID, &EvtDescObjC, &OptTimeout);
    gettimeofday(&AfterPostTime, NULL);

    if (Result == TOS_RESULT_TIMEOUT) {
      pCbPrivObjC->EvtCntPostRstTimeout++;
    } else {
      EXPECT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint
    }

    unsigned long PostTimeUS =
        (AfterPostTime.tv_sec - BeforePostTime.tv_sec) * 1000000 + AfterPostTime.tv_usec - BeforePostTime.tv_usec;
    EXPECT_LE(PostTimeUS, 100);  // CheckPoint@Performance_FAST(<=100us)@NONBLOCK

    usleep(1);
  }
  return NULL;
}

TEST(UT_ConlesModeEventConcurrency, Case01) {
  IOC_LinkID_T LinkID = IOC_CONLESMODE_AUTO_LINK_ID;

  // Step-1: ObjA as EvtSuber subEvt(TEST_BLOCK_SLEEP_5MS, TEST_NONBLOCK_SLEEP_1MS)
  _UT_Case01_CbPrivObjA_T CbPrivObjA = {.EvtCntBlockSleep1MS = 0, .EvtCntNonBlockSleep5MS = 0};
  IOC_EvtID_T EvtIDsObjA[] = {IOC_EVTID_TEST_BLOCK_SLEEP_1MS, IOC_EVTID_TEST_NONBLOCK_SLEEP_5MS};
  IOC_EvtSubArgs_T EvtSubArgsObjA = {
      .CbProcEvt_F = _UT_Case01_CbProcEvtObjA_F,
      .pCbPriv = &CbPrivObjA,
      .EvtNum = TOS_calcArrayElmtCnt(EvtIDsObjA),
      .pEvtIDs = EvtIDsObjA,
  };

  TOS_Result_T Result = PLT_IOC_subEVT(LinkID, &EvtSubArgsObjA);
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint

  // Step-2: ObjB/C as EvtPuber postEvt(TEST_BLOCK_SLEEP_5MS, TEST_NONBLOCK_SLEEP_1MS) in each's thread context
  pthread_t ThreadID[2];  // ObjB/C
  pthread_create(&ThreadID[0], NULL, _UT_Case01_ThreadObjB, NULL);

  _UT_Case01_CbPrivObjC_T CbPrivObjC = {.EvtCntPostRstTimeout = 0};
  pthread_create(&ThreadID[1], NULL, _UT_Case01_ThreadObjC, &CbPrivObjC);

  // Step-3: Wait for all ObjB/C's thread to finish
  pthread_join(ThreadID[0], NULL);
  pthread_join(ThreadID[1], NULL);

  // Step-4: ObjA unsubEvt(TEST_BLOCK_SLEEP_5MS, TEST_NONBLOCK_SLEEP_1MS) when all CbProcEvt_F() is done
  IOC_EvtUnsubArgs_T EvtUnsubArgsObjA = {
      .CbProcEvt_F = _UT_Case01_CbProcEvtObjA_F,
      .pCbPriv = &CbPrivObjA,
  };
  Result = PLT_IOC_unsubEVT(LinkID, &EvtUnsubArgsObjA);
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // Check

  // Step-5: Check ObjA's EvtCntBlockSleep1MS is $_UT_EVT_CNT_BLOCK
  ASSERT_EQ(CbPrivObjA.EvtCntBlockSleep1MS, _UT_EVT_CNT_BLOCK);  // CheckPoint

  // Step-6: Check ObjA's EvtCntNonBlockSleep5MS>0 && EvtCntPostRstTimeout>0
  //          && (EvtCntNonBlockSleep5MS + ObjC's EvtCntPostRstTimeout) == $_UT_EVT_CNT_NONBLOCK
  ASSERT_GT(CbPrivObjA.EvtCntNonBlockSleep5MS, 0);                                                       // CheckPoint
  ASSERT_GT(CbPrivObjC.EvtCntPostRstTimeout, 0);                                                         // CheckPoint
  ASSERT_EQ(CbPrivObjA.EvtCntNonBlockSleep5MS + CbPrivObjC.EvtCntPostRstTimeout, _UT_EVT_CNT_NONBLOCK);  // CheckPoint
}