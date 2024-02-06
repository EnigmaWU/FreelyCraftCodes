#include "_UT_IOC_Common.h"

// This is IOC_Event in ConlesMode's Performance UT from API user's perspective
// Performance means SPEED vs BOTTLENECK, which means as fast as possible until reach the bottleneck.

//===>Case[01]: ObjA as EvtSuber, ObjB/C/D/E/F as EvtPuber, use TEST_KEEPALIVE to simulate multi functional
//  objects here is ObjB/C/D/E, tell single watchdog timer object here is ObjA, that their are still alive.
//  Here performance means how FAST ObjA can process all the TEST_KEEPALIVE from ObjB/C/D/E.
//  And we define the FAST as <=100us, and define it as IOC_Event in ConlesMode's Specification in 5P->1S per 10us.
//  [Step-1]: ObjA as EvtSuber subEvt(TEST_KEEPALIVE), set ObjA's private KeepAliveEvtCnt to 0
//            |-> In ObjA's CbProcEvt_F(), KeepAliveEvtCnt++
//  [Step-2]: ObjB/C/D/E/F as EvtPuber postEvt(TEST_KEEPALIVE) in each's thread context
//            |-> In each's thread context, postEvt(TEST_KEEPALIVE) and check the post cost time <=100us
//  [Step-3]: Wait for all ObjB/C/D/E/F's thread to finish
//  [Step-4]: Check ObjA's KeepAliveEvtCnt is total of ObjB/C/D/E/F's postEvt(TEST_KEEPALIVE)

typedef struct {
  unsigned long KeepAliveEvtCnt;
} _UT_Case01_CbPrivObjA_T, *_UT_Case01_CbPrivObjA_pT;

static TOS_Result_T _UT_Case01_CbProcEvtObjA_F(IOC_EvtDesc_pT pEvtDesc, void* pCbPriv) {
  _UT_Case01_CbPrivObjA_pT pCbPrivObjA = (_UT_Case01_CbPrivObjA_pT)pCbPriv;
  pCbPrivObjA->KeepAliveEvtCnt++;
  return TOS_RESULT_SUCCESS;
}

#define _UT_Case01_KEEPALIVE_EVT_NUM 100000

static void* _UT_Case01_ThreadObjX(void* pArg) {
  IOC_LinkID_T LinkID = IOC_CONLESMODE_AUTO_LINK_ID;
  IOC_EvtDesc_T EvtDescObjX = {.EvtID = IOC_EVTID_TEST_KEEPALIVE};
  for (int i = 0; i < _UT_Case01_KEEPALIVE_EVT_NUM; i++) {
    struct timeval BeforePostTime, AfterPostTime;

    gettimeofday(&BeforePostTime, NULL);
    TOS_Result_T Result = PLT_IOC_postEVT(LinkID, &EvtDescObjX, NULL);
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint
    gettimeofday(&AfterPostTime, NULL);

    unsigned long PostTimeUS =
        (AfterPostTime.tv_sec - BeforePostTime.tv_sec) * 1000000 + (AfterPostTime.tv_usec - BeforePostTime.tv_usec);
    EXPECT_LE(PostTimeUS, 100);  // CheckPoint@Performance_FAST(<=100us)

    usleep(10);
  }
  return NULL;
}

TEST(UT_ConlesModeEventPerf, Case01) {
  IOC_LinkID_T LinkID = IOC_CONLESMODE_AUTO_LINK_ID;

  // Step-1: ObjA as EvtSuber subEvt(TEST_KEEPALIVE)
  _UT_Case01_CbPrivObjA_T CbPrivObjA = {.KeepAliveEvtCnt = 0};
  IOC_EvtID_T EvtIDsObjA[] = {IOC_EVTID_TEST_KEEPALIVE};
  IOC_EvtSubArgs_T EvtSubArgsObjA = {
      .CbProcEvt_F = _UT_Case01_CbProcEvtObjA_F,
      .pCbPriv = &CbPrivObjA,
      .EvtNum = TOS_calcArrayElmtCnt(EvtIDsObjA),
      .pEvtIDs = EvtIDsObjA,
  };

  TOS_Result_T Result = PLT_IOC_subEVT(LinkID, &EvtSubArgsObjA);
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint

  // Step-2: ObjB/C/D/E/F as EvtPuber postEvt(TEST_KEEPALIVE) in each's thread context
  pthread_t ThreadID[5];  // ObjX=ObjB/C/D/E/F
  for (int i = 0; i < TOS_calcArrayElmtCnt(ThreadID); i++) {
    pthread_create(&ThreadID[i], NULL, _UT_Case01_ThreadObjX, NULL);
  }

  // Step-3: Wait for all ObjB/C/D/E/F's thread to finish
  for (int i = 0; i < TOS_calcArrayElmtCnt(ThreadID); i++) {
    pthread_join(ThreadID[i], NULL);
  }

  // Step-4: Check ObjA's KeepAliveEvtCnt is total of ObjB/C/D/E/F's postEvt(TEST_KEEPALIVE)
  ASSERT_EQ(CbPrivObjA.KeepAliveEvtCnt, _UT_Case01_KEEPALIVE_EVT_NUM * 5);  // CheckPoint
}

//===>Case[02]: