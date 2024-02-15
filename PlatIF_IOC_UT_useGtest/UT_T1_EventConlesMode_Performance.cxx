#include "_UT_IOC_Common.h"

// This is IOC_Event in ConlesMode's Performance UT from API user's perspective
// Performance means SPEED/CAPACITY beyond BOTTLENECK, which defined as FAST in our context.
//    So here as FAST as possible typically means bottleneck is caused by hardware or physical limitation,
//      such as CPU frequency, memory bandwidth, network latency, etc.
//    And here FAST will be limited by EVT's SYNC/ASYNC, MAYBLOCK/NONBLOCK properties.
// SYNC will be a little FAST than ASYNC, because of no context switch when inter-thread communication,
//  but exectally same FAST with ASYNC when inter-process/machine communication.
// MAYBLOCK will be a little SLOW than NONBLOCK, because EvtPuber is too FAST than EvtSuber to process the event,
//  such as EvtSuber's CbProcEvt_F() need more time to process event than EvtPuber's event occurent rate.

//===>Case[01]: ObjA as EvtSuber, ObjB/C/D/E/F as EvtPuber, use TEST_KEEPALIVE to simulate multi functional
//  objects here is ObjB/C/D/E, tell single watchdog timer object here is ObjA, that their are still alive.
//  Here performance means how FAST ObjA can process all the TEST_KEEPALIVE from ObjB/C/D/E,
//    so postEVT in SYNC, CbProcEvt_F only take counter++ will be the best performance here.
//  So we define the FAST as <=10us, and define it as $SPEC_PerfFAST_SyncEvt in 5Puber->1Suber per 100us.
//  [Step-1]: ObjA as EvtSuber subEvt(TEST_KEEPALIVE), set ObjA's private KeepAliveEvtCnt to 0
//            |-> In ObjA's CbProcEvt_F(), KeepAliveEvtCnt++
//  [Step-2]: ObjB/C/D/E/F as EvtPuber postEvt(TEST_KEEPALIVE) in each's thread context
//            |-> In each's thread context, postEvt(TEST_KEEPALIVE) and check the post cost time <=100us
//  [Step-3]: Wait for all ObjB/C/D/E/F's thread to finish
//  [Step-4]: Check ObjA's KeepAliveEvtCnt is total of ObjB/C/D/E/F's postEvt(TEST_KEEPALIVE)

#define SPEC_PerfFAST_SyncEvt 10

typedef struct {
  ULONG_T KeepAliveEvtCnt;
} _UT_Case01_CbPrivObjA_T, *_UT_Case01_CbPrivObjA_pT;

static TOS_Result_T _UT_Case01_CbProcEvtObjA_F(IOC_EvtDesc_pT pEvtDesc, void* pCbPriv) {
  _UT_Case01_CbPrivObjA_pT pCbPrivObjA = (_UT_Case01_CbPrivObjA_pT)pCbPriv;
  pCbPrivObjA->KeepAliveEvtCnt++;
  return TOS_RESULT_SUCCESS;
}

#define _UT_Case01_KEEPALIVE_EVT_NUM 100000

static void* _UT_Case01_ThreadObjX(void* pArg) {
  IOC_EvtDesc_T EvtDescObjX = {.EvtID = IOC_EVTID_TEST_KEEPALIVE};
  for (int i = 0; i < _UT_Case01_KEEPALIVE_EVT_NUM; i++) {
    IOC_Options_T OptSync = {.IDs = IOC_OPTID_SYNC_MODE};
    struct timeval BeforePostTime, AfterPostTime;

    //===BEHAVIOR===
    gettimeofday(&BeforePostTime, NULL);
    TOS_Result_T Result = PLT_IOC_postEVT_inConlesMode(&EvtDescObjX, &OptSync);
    gettimeofday(&AfterPostTime, NULL);

    ULONG_T PostTimeUS = (AfterPostTime.tv_sec - BeforePostTime.tv_sec) * 1000000 + AfterPostTime.tv_usec - BeforePostTime.tv_usec;

    //===VERIFY===
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);         // CheckPoint
    EXPECT_LE(PostTimeUS, SPEC_PerfFAST_SyncEvt);  // CheckPoint@PerfFAST_SyncEvt(<=10us)

    usleep(100);
  }
  return NULL;
}

TEST(UT_ConlesModeEventPerf, Case01) {
  //===SETUP===
  // Step-1: ObjA as EvtSuber subEvt(TEST_KEEPALIVE)
  _UT_Case01_CbPrivObjA_T CbPrivObjA = {.KeepAliveEvtCnt = 0};
  IOC_EvtID_T EvtIDsObjA[] = {IOC_EVTID_TEST_KEEPALIVE};
  IOC_EvtSubArgs_T EvtSubArgsObjA = {
      .CbProcEvt_F = _UT_Case01_CbProcEvtObjA_F,
      .pCbPriv = &CbPrivObjA,
      .EvtNum = TOS_calcArrayElmtCnt(EvtIDsObjA),
      .pEvtIDs = EvtIDsObjA,
  };

  TOS_Result_T Result = PLT_IOC_subEVT_inConlesMode(&EvtSubArgsObjA);
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint

  //===BEHAVIOR===
  // Step-2: ObjB/C/D/E/F as EvtPuber postEvt(TEST_KEEPALIVE) in each's thread context
  pthread_t ThreadID[5];  // ObjX=ObjB/C/D/E/F
  for (int i = 0; i < TOS_calcArrayElmtCnt(ThreadID); i++) {
    pthread_create(&ThreadID[i], NULL, _UT_Case01_ThreadObjX, NULL);
  }

  // Step-3: Wait for all ObjB/C/D/E/F's thread to finish
  for (int i = 0; i < TOS_calcArrayElmtCnt(ThreadID); i++) {
    pthread_join(ThreadID[i], NULL);
  }

  //===VERIFY===
  // Step-4: Check ObjA's KeepAliveEvtCnt is total of ObjB/C/D/E/F's postEvt(TEST_KEEPALIVE)
  ASSERT_EQ(CbPrivObjA.KeepAliveEvtCnt, _UT_Case01_KEEPALIVE_EVT_NUM * 5);  // CheckPoint

  //===TEARDOWN===
  // Step-5: ObjA unsubEvt(TEST_KEEPALIVE)
  IOC_EvtUnsubArgs_T EvtUnsubArgsObjA = {.CbProcEvt_F = _UT_Case01_CbProcEvtObjA_F, .pCbPriv = &CbPrivObjA};
  Result = PLT_IOC_unsubEVT_inConlesMode(&EvtUnsubArgsObjA);
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint
}

//===>Case[02]: