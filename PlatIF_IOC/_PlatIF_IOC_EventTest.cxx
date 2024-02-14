#include <pthread.h>
#include <sys/_pthread/_pthread_cond_t.h>
#include <sys/_pthread/_pthread_key_t.h>
#include <sys/_pthread/_pthread_mutex_t.h>
#include <sys/_pthread/_pthread_rwlock_t.h>
#include <sys/_types/_null.h>

#include "_PlatIF_IOC.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//===>BEGIN_OF_NS::IOC_ConlesMode of Event
#ifdef __cplusplus
extern "C" {
#endif

// Every ConlesMode_subEVT need a _IOC_ConlesEventSuber_T instance space.
typedef struct {
  pthread_mutex_t Mutex;  // Mutex for each ConlesEventSuber
  pthread_cond_t Cond;    // Cond for each ConlesEventSuber

  bool ShouldExitASyncThread;  //

  IOC_EvtSubArgs_T Args;  // saved from subEVT(pEvtSubArgs)

  pthread_t ASyncThreadID;  // If ASyncMode, each EvtSuber will start a ASyncThread to process the event

  ULONG_T EvtPostCnt;  // How many postEVT(pEvtDesc) save into EvtDescQueue
  ULONG_T EvtProcCnt;  // How many CbProcEvt_F(pEvtDesc) read from EvtDescQueue

#define _IOC_CONLES_EVTQUEUE_MAX 1
  // IF EvtPostCnt-EvtProcCnt == 0, it means EvtDescQueue is empty,
  //   ASyncThread will be blocked in pthread_cond_wait(&Cond, &Mutex).
  // WHEN postEVT(pEvtDesc) is called AND EvtPostCnt-EvtProcCnt < _IOC_CONLES_EVTQUEUE_MAX,
  //   it means EvtDescQueue is not full,
  //   save to EvtDescQueue, EvtPostCnt++, waked up ASyncThread by pthread_cond_signal(&Cond).
  // THEN ASyncThread will copy EvtDescQueue[EvtProcCnt%_IOC_CONLES_EVTQUEUE_MAX] to $ToProcEvtDesc,
  //   and EvtProcCnt++, then call CbProcEvt_F($ToProcEvtDesc, pCbPriv).
  IOC_EvtDesc_T EvtDescQueue[_IOC_CONLES_EVTQUEUE_MAX];

#define _IOC_CONLES_EVTCACHED_MAX 4  // RefUT: EventConlesModeTypical.Case02
  ULONG_T EvtPostCachedCnt;          // How many CbProcEvt_F->postEVT->postEvt_ASyncEventCached saved in EvtDescCachedQueue
  ULONG_T EvtProcCachedCnt;
  IOC_EvtDesc_T EvtDescCachedQueue[_IOC_CONLES_EVTCACHED_MAX];
} _IOC_ConlesEventSuber_T, *_IOC_ConlesEventSuber_pT;

typedef struct {
  pthread_mutex_t Mutex;  // Global Mutex for ConlesEventContext

  ULONG_T MaxSuberNum;
  ULONG_T CurSuberNum;

  _IOC_ConlesEventSuber_pT pEvtSubers;  // MAX=MaxSuberNum, (0,CurSuberNum-1) is valid/used/subed

  bool IsSyncMode;  // set by setLinkParams

#define _IOC_CONLES_ASYNC_EVENT_THREAD_FLAG_KEY 0x20240211UL
  pthread_key_t ASyncEvtThreadFlagKey;
  pthread_key_t ASyncEvtThreadEvtSuberKey;
} _IOC_ConlesEventContext_T, *_IOC_ConlesEventContext_pT;

static _IOC_ConlesEventContext_T _mConlesEvtCtx = {
    .Mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER,
    .MaxSuberNum = 32,
    .CurSuberNum = 0,
    .pEvtSubers = NULL,
    .IsSyncMode = false,
};

static void __IOC_ConlesEvent_stopASyncThread(_IOC_ConlesEventSuber_pT pEvtSuber) {
  pthread_mutex_lock(&pEvtSuber->Mutex);
  pEvtSuber->ShouldExitASyncThread = true;
  pthread_mutex_unlock(&pEvtSuber->Mutex);

  pthread_cond_signal(&pEvtSuber->Cond);
  pthread_join(pEvtSuber->ASyncThreadID, NULL);
  pEvtSuber->ASyncThreadID = 0;
}

static TOS_Result_T __IOC_ConlesMode_subEVT(const IOC_EvtSubArgs_pT pEvtSubArgs) {
  int RetPSX = pthread_mutex_lock(&_mConlesEvtCtx.Mutex);
  if (RetPSX != 0) {
    return TOS_RESULT_NOT_TESTED_BUG;
  }

  if (_mConlesEvtCtx.CurSuberNum >= _mConlesEvtCtx.MaxSuberNum) {
    pthread_mutex_unlock(&_mConlesEvtCtx.Mutex);
    // TOS_abortNotTested(); -> @TEST(UT_ConlesModeEventINFILE, Case01_subEVT)
    return TOS_RESULT_NOT_ENOUGH_RESOURCE;
  }

  if (_mConlesEvtCtx.pEvtSubers == NULL) {
    _mConlesEvtCtx.pEvtSubers = (_IOC_ConlesEventSuber_pT)calloc(_mConlesEvtCtx.MaxSuberNum, sizeof(_IOC_ConlesEventSuber_T));
    if (_mConlesEvtCtx.pEvtSubers == NULL) {
      TOS_abortNotTested();
    }

    int RetPSX = pthread_key_create(&_mConlesEvtCtx.ASyncEvtThreadFlagKey, NULL);
    if (RetPSX == (-1)) {
      TOS_abortNotTested();
    }

    RetPSX = pthread_key_create(&_mConlesEvtCtx.ASyncEvtThreadEvtSuberKey, NULL);
    if (RetPSX == (-1)) {
      TOS_abortNotTested();
    }
  }

  ULONG_T FreeSuberIdx = 0;
  for (; FreeSuberIdx < _mConlesEvtCtx.MaxSuberNum; FreeSuberIdx++) {
    if (_mConlesEvtCtx.pEvtSubers[FreeSuberIdx].Args.CbProcEvt_F == NULL) {
      break;
    }
  }

  if (FreeSuberIdx == _mConlesEvtCtx.MaxSuberNum) {
    pthread_mutex_lock(&_mConlesEvtCtx.Mutex);
    return TOS_RESULT_NOT_TESTED_BUG;
  }

  _IOC_ConlesEventSuber_pT pEvtSuber = &_mConlesEvtCtx.pEvtSubers[FreeSuberIdx];
  memset(pEvtSuber, 0, sizeof(_IOC_ConlesEventSuber_T));
  pthread_mutex_init(&pEvtSuber->Mutex, NULL);
  pthread_cond_init(&pEvtSuber->Cond, NULL);

  IOC_EvtSubArgs_pT pSavdSubArgs = &pEvtSuber->Args;
  size_t SavdEvtIDSiz = sizeof(IOC_EvtID_T) * pEvtSubArgs->EvtNum;
  memcpy(pSavdSubArgs, pEvtSubArgs, sizeof(IOC_EvtSubArgs_T));

  pSavdSubArgs->pEvtIDs = (IOC_EvtID_T*)malloc(SavdEvtIDSiz);
  if (pSavdSubArgs->pEvtIDs == NULL) {
    pthread_mutex_unlock(&_mConlesEvtCtx.Mutex);
    return TOS_RESULT_NOT_TESTED_BUG;
  } else {
    memcpy(pSavdSubArgs->pEvtIDs, pEvtSubArgs->pEvtIDs, SavdEvtIDSiz);
  }

  _mConlesEvtCtx.CurSuberNum++;
  pthread_mutex_unlock(&_mConlesEvtCtx.Mutex);

  return TOS_RESULT_SUCCESS;
}

static TOS_Result_T __IOC_ConlesMode_unsubEVT(const IOC_EvtUnsubArgs_pT pEvtUnsubArgs) {
  int RetPSX = pthread_mutex_lock(&_mConlesEvtCtx.Mutex);
  if (RetPSX != 0) {
    return TOS_RESULT_BUG;
  }

  TOS_Result_T Result = TOS_RESULT_NOT_FOUND;

  if (_mConlesEvtCtx.pEvtSubers == NULL) {
    pthread_mutex_unlock(&_mConlesEvtCtx.Mutex);
    return Result;
  }

  if (pEvtUnsubArgs == NULL) {  // unsubEVT ALL
    for (ULONG_T Idx = 0; Idx < _mConlesEvtCtx.MaxSuberNum; Idx++) {
      _IOC_ConlesEventSuber_pT pEvtSuber = &_mConlesEvtCtx.pEvtSubers[Idx];
      if (pEvtSuber->ASyncThreadID) {
        __IOC_ConlesEvent_stopASyncThread(pEvtSuber);
      }

      IOC_EvtSubArgs_pT pSavdSubArgs = &pEvtSuber->Args;
      if (pSavdSubArgs->pEvtIDs != NULL) {
        free(pSavdSubArgs->pEvtIDs);
        pSavdSubArgs->pEvtIDs = NULL;
        pSavdSubArgs->CbProcEvt_F = NULL;
        pSavdSubArgs->pCbPriv = NULL;
      }
    }

    _mConlesEvtCtx.CurSuberNum = 0;
    Result = TOS_RESULT_SUCCESS;
  } else {  // unsubEVT by CbProcEvt_F and pCbPriv
    for (ULONG_T Idx = 0; Idx < _mConlesEvtCtx.MaxSuberNum; Idx++) {
      _IOC_ConlesEventSuber_pT pEvtSuber = &_mConlesEvtCtx.pEvtSubers[Idx];
      IOC_EvtSubArgs_pT pSavdSubArgs = &pEvtSuber->Args;
      if (pSavdSubArgs->CbProcEvt_F == pEvtUnsubArgs->CbProcEvt_F && pSavdSubArgs->pCbPriv == pEvtUnsubArgs->pCbPriv) {
        if (pEvtSuber->ASyncThreadID) {
          __IOC_ConlesEvent_stopASyncThread(pEvtSuber);
        }

        free(pSavdSubArgs->pEvtIDs);
        pSavdSubArgs->pEvtIDs = NULL;
        pSavdSubArgs->CbProcEvt_F = NULL;
        pSavdSubArgs->pCbPriv = NULL;

        _mConlesEvtCtx.CurSuberNum--;  // FIXME: MOVE UnSubIdx::CurSuberNum
        Result = TOS_RESULT_SUCCESS;
        break;
      }
    }
  }

  pthread_mutex_unlock(&_mConlesEvtCtx.Mutex);
  return TOS_RESULT_SUCCESS;
}

#ifdef __APPLE__
static int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abs_timeout) {
  int result;
  struct timespec ts;
  struct timespec rem;

  while ((result = pthread_mutex_trylock(mutex)) == EBUSY) {
    clock_gettime(CLOCK_REALTIME, &ts);
    if (ts.tv_sec > abs_timeout->tv_sec || (ts.tv_sec == abs_timeout->tv_sec && ts.tv_nsec >= abs_timeout->tv_nsec)) {
      return ETIMEDOUT;
    }

    ts.tv_sec = 0;
    ts.tv_nsec = 1000;  // 1 us
    nanosleep(&ts, &rem);
  }

  return result;
}
#endif

static void __IOC_ConlesMode_setASyncEventThreadSpecificKey(_IOC_ConlesEventSuber_pT pEvtSuber) {
  pthread_setspecific(_mConlesEvtCtx.ASyncEvtThreadFlagKey, (void *)_IOC_CONLES_ASYNC_EVENT_THREAD_FLAG_KEY);
  pthread_setspecific(_mConlesEvtCtx.ASyncEvtThreadEvtSuberKey, (void *)pEvtSuber);
}

static bool __IOC_ConlesMode_isAsyncEventThreadContext(void) {
  ULONG_T KeyVal = (ULONG_T)pthread_getspecific(_mConlesEvtCtx.ASyncEvtThreadFlagKey);
  if (KeyVal == _IOC_CONLES_ASYNC_EVENT_THREAD_FLAG_KEY) {
    return true;
  } else {
    return false;
  }
}

static TOS_Result_T __IOC_ConlesMode_postEVT(const IOC_EvtDesc_pT pEvtDesc, const IOC_Options_pT pOptions);

static void *__IOC_ConlesMode_ASyncEventThread(void *pArg) {
  _IOC_ConlesEventSuber_pT pEvtSuber = (_IOC_ConlesEventSuber_pT)pArg;

  __IOC_ConlesMode_setASyncEventThreadSpecificKey(pEvtSuber);

  // Follow comments in _IOC_ConlesEventSuber_T
  while (1) {
    pthread_mutex_lock(&pEvtSuber->Mutex);
    if (pEvtSuber->ShouldExitASyncThread && (pEvtSuber->EvtPostCnt == pEvtSuber->EvtProcCnt)) {
      pthread_mutex_unlock(&pEvtSuber->Mutex);
      break;
    }

    while (pEvtSuber->EvtPostCnt == pEvtSuber->EvtProcCnt) {
      pthread_cond_wait(&pEvtSuber->Cond, &pEvtSuber->Mutex);

      if (pEvtSuber->ShouldExitASyncThread && (pEvtSuber->EvtPostCnt == pEvtSuber->EvtProcCnt)) {
        break;
      }
    }

    if (pEvtSuber->ShouldExitASyncThread && (pEvtSuber->EvtPostCnt == pEvtSuber->EvtProcCnt)) {
      pthread_mutex_unlock(&pEvtSuber->Mutex);
      break;
    }

    IOC_EvtDesc_pT pEvtDesc = &pEvtSuber->EvtDescQueue[pEvtSuber->EvtProcCnt % _IOC_CONLES_EVTQUEUE_MAX];
    IOC_EvtDesc_T ToProcEvtDesc = *pEvtDesc;

    pEvtSuber->EvtProcCnt++;
    pthread_mutex_unlock(&pEvtSuber->Mutex);

    pEvtSuber->Args.CbProcEvt_F(&ToProcEvtDesc, pEvtSuber->Args.pCbPriv);

    //-----------------------------------------------------------------------------------------------------------------
    do {
#if 1
      pthread_mutex_lock(&pEvtSuber->Mutex);
      bool IsEvtDescQueueEmpty = (pEvtSuber->EvtPostCnt == pEvtSuber->EvtProcCnt) ? true : false;
      pthread_mutex_unlock(&pEvtSuber->Mutex);

      if (IsEvtDescQueueEmpty && (pEvtSuber->EvtPostCachedCnt - pEvtSuber->EvtProcCachedCnt > 0)) {
        IOC_EvtDesc_pT pToProcCachedEvtDesc =
            &pEvtSuber->EvtDescCachedQueue[pEvtSuber->EvtProcCachedCnt % _IOC_CONLES_EVTCACHED_MAX];

        IOC_Options_T OptNonBlock = {.IDs = IOC_OPTID_TIMEOUT, .Payload = {.TimeoutUS = 0}};
        TOS_Result_T Result = __IOC_ConlesMode_postEVT(pToProcCachedEvtDesc, &OptNonBlock);
        if (TOS_RESULT_SUCCESS == Result) {
          pEvtSuber->EvtProcCachedCnt++;
        } else {
          if (pToProcCachedEvtDesc->MsgDesc.Flags & IOC_MSGFLAG_MAYDROP) {
            pEvtSuber->EvtProcCachedCnt++;
            TOS_abortNotTested();
          }
        }
      } else {
        break;
      }
#else
      if (pEvtSuber->EvtPostCachedCnt - pEvtSuber->EvtProcCachedCnt > 0) {
        IOC_EvtDesc_pT pToProcCachedEvtDesc =
            &pEvtSuber->EvtDescCachedQueue[pEvtSuber->EvtProcCachedCnt % _IOC_CONLES_EVTCACHED_MAX];

        IOC_Options_T OptNonBlock = {.IDs = IOC_OPTID_TIMEOUT, .Payload = {.TimeoutUS = 0}};
        TOS_Result_T Result = __IOC_ConlesMode_postEVT(pToProcCachedEvtDesc, &OptNonBlock);
        if (TOS_RESULT_SUCCESS == Result) {
          pEvtSuber->EvtProcCachedCnt++;
          TOS_abortNotTested();
        } else {
          usleep(1000);
          // TOS_abortNotTested(); -> @EventConlesModeTypical.Case02
        }
      } else {
        break;
      }
#endif
    } while (0x20240211);
  }

  return NULL;
}

static TOS_Result_T __IOC_ConlesMode_postEVT_ASyncThreadCached(const IOC_EvtDesc_pT pEvtDesc) {
  _IOC_ConlesEventSuber_pT pEvtSuber = (_IOC_ConlesEventSuber_pT)pthread_getspecific(_mConlesEvtCtx.ASyncEvtThreadEvtSuberKey);
  if (!pEvtSuber) {
    TOS_abortNotTested();
  }

  //-------------------------------------------------------------------------------------------------------------------
  TOS_Result_T Result = TOS_RESULT_BUG;

  if ((pEvtSuber->EvtPostCachedCnt - pEvtSuber->EvtProcCachedCnt) < _IOC_CONLES_EVTCACHED_MAX) {
    pEvtSuber->EvtDescCachedQueue[pEvtSuber->EvtPostCachedCnt % _IOC_CONLES_EVTCACHED_MAX] = *pEvtDesc;
    pEvtSuber->EvtPostCachedCnt++;
    Result = TOS_RESULT_SUCCESS;
  } else {
    // TOS_abortNotTested(); -> @ventConlesModeTypical.Case02
    Result = TOS_RESULT_NOT_ENOUGH_RESOURCE;
  }

  return Result;
}

static TOS_Result_T __IOC_ConlesMode_postEVT(const IOC_EvtDesc_pT pEvtDesc, const IOC_Options_pT pOptions) {
  bool IsCbProcEvt_Found = false;
  bool IsNonBlock = false;
  bool IsTimeout = false;
  bool IsSyncMode = false;
  int RetPSX = -1;
  uint32_t TimeoutUS = 0;

  //-------------------------------------------------------------------------------------------------------------------
  if (pOptions && (pOptions->IDs & IOC_OPTID_SYNC_MODE)) {
    // TOS_abortNotTested(); // @TEST(UT_ConlesModeEventPerf, Case01)
    IsSyncMode = true;
  }

  if (pOptions && (pOptions->IDs & IOC_OPTID_TIMEOUT)) {
    TimeoutUS = pOptions->Payload.TimeoutUS;
    if (TimeoutUS == 0) {
      IsNonBlock = true;
    } else {
      IsTimeout = true;
    }
  }

  //-------------------------------------------------------------------------------------------------------------------
  if (IsNonBlock) {
    RetPSX = pthread_mutex_trylock(&_mConlesEvtCtx.Mutex);
    if (RetPSX == EBUSY) {
      return TOS_RESULT_TIMEOUT;
    }
  } else if (IsTimeout) {
    struct timespec AbsTime;
    clock_gettime(CLOCK_REALTIME, &AbsTime);
    AbsTime.tv_nsec += TimeoutUS * 1000;
    AbsTime.tv_sec += AbsTime.tv_nsec / 1000000000;
    AbsTime.tv_nsec %= 1000000000;

    RetPSX = pthread_mutex_timedlock(&_mConlesEvtCtx.Mutex, &AbsTime);
    if (RetPSX == ETIMEDOUT) {
      return TOS_RESULT_TIMEOUT;
    }
  } else {
    if (__IOC_ConlesMode_isAsyncEventThreadContext()) {
      RetPSX = pthread_mutex_trylock(&_mConlesEvtCtx.Mutex);
      if (RetPSX != 0) {
        // TOS_abortNotTested(); -> @EventConlesModeTypical.Case02
        return __IOC_ConlesMode_postEVT_ASyncThreadCached(pEvtDesc);
        ;
      }
    } else {
      RetPSX = pthread_mutex_lock(&_mConlesEvtCtx.Mutex);
    }
  }

  if (RetPSX != 0) {
    return TOS_RESULT_BUG;
  }

  if (_mConlesEvtCtx.pEvtSubers == NULL) {
    pthread_mutex_unlock(&_mConlesEvtCtx.Mutex);
    return TOS_RESULT_NOT_FOUND;
  }

  //-------------------------------------------------------------------------------------------------------------------
  TOS_Result_T Result = TOS_RESULT_BUG;

  for (ULONG_T SuberIdx = 0; SuberIdx < _mConlesEvtCtx.MaxSuberNum; SuberIdx++) {
    _IOC_ConlesEventSuber_pT pEvtSuber = _mConlesEvtCtx.pEvtSubers + SuberIdx;
    IOC_EvtSubArgs_pT pSavdSubArgs = &pEvtSuber->Args;
    if (pSavdSubArgs->CbProcEvt_F == NULL || pSavdSubArgs->pEvtIDs == NULL) {
      continue;
    }

    for (ULONG_T EvtIdx = 0; EvtIdx < pSavdSubArgs->EvtNum; EvtIdx++) {
      if (pSavdSubArgs->pEvtIDs[EvtIdx] != pEvtDesc->EvtID) {
        continue;
      }

      //-------------------------------------------------------------------------------------------------------------------
      if (IsSyncMode || _mConlesEvtCtx.IsSyncMode) {
        Result = pSavdSubArgs->CbProcEvt_F(pEvtDesc, pSavdSubArgs->pCbPriv);
      } else /*ASyncMode*/ {
        do {
          pthread_mutex_lock(&pEvtSuber->Mutex);
          if (!pEvtSuber->ASyncThreadID) {
            int PSXRet = pthread_create(&pEvtSuber->ASyncThreadID, NULL, __IOC_ConlesMode_ASyncEventThread, (void *)pEvtSuber);
            if (PSXRet != 0) {
              pthread_mutex_unlock(&pEvtSuber->Mutex);
              TOS_abortNotTested();
              return TOS_RESULT_BUG;
            }
          }

          if (pEvtSuber->EvtPostCnt - pEvtSuber->EvtProcCnt < _IOC_CONLES_EVTQUEUE_MAX) {
            pEvtSuber->EvtDescQueue[pEvtSuber->EvtPostCnt % _IOC_CONLES_EVTQUEUE_MAX] = *pEvtDesc;
            pEvtSuber->EvtPostCnt++;
            // pthread_cond_signal(&pEvtSuber->Cond);
            Result = TOS_RESULT_SUCCESS;
          } else {
            if (IsNonBlock) {
              // TOS_abortNotTested(); // @UT_ConlesModeEventConcurrency.Case01
              Result = TOS_RESULT_TIMEOUT;
            } else {
              if (IsTimeout) {
                TOS_abortNotTested();
                Result = TOS_RESULT_TIMEOUT;
              } else {
                pthread_mutex_unlock(&pEvtSuber->Mutex);
                pthread_cond_signal(&pEvtSuber->Cond);
                usleep(1000);  // 1ms
                // TOS_abortNotTested(); // @UT_ConlesModeEventConcurrency.Case01
                continue;
              }
            }
          }

          pthread_mutex_unlock(&pEvtSuber->Mutex);

          if (Result == TOS_RESULT_SUCCESS) {
            pthread_cond_signal(&pEvtSuber->Cond);
          }

          break;
        } while (0x20240211);
      }

      IsCbProcEvt_Found = true;
      break;
    }
  }
  pthread_mutex_unlock(&_mConlesEvtCtx.Mutex);

  if (!IsCbProcEvt_Found) {
    return TOS_RESULT_NO_EVT_SUBER;
  } else {
    return Result;
  }
}
#ifdef __cplusplus
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//===> BEGIN of internal UT for ConlesMod of Event in CXX
#ifdef CONFIG_BUILD_INFILE_UNIT_TESTING_USE_UTFWK_GTEST
#include <gtest/gtest.h>
//--->CASE[01]: MaxSuberNum is 1, CurSuberNum is 0, SuberArgA will SUCCESS, SuberArgB will NOT_ENOUGH_RESOURCE
TEST(UT_ConlesModeEventINFILE, Case01_subEVT) {
  //===>SETUP
  _IOC_ConlesEventContext_T BackupCtx = _mConlesEvtCtx;

  //===>EXECUTE
  _mConlesEvtCtx.MaxSuberNum = 1;
  _mConlesEvtCtx.CurSuberNum = 0;

  IOC_EvtID_T EvtIDsKeepAlive[] = {IOC_EVTID_TEST_KEEPALIVE};
#define CbProcEvtA_F 0x01UL
#define CbPrivA 0x02UL
  IOC_EvtSubArgs_T SuberArgA = {
      .CbProcEvt_F = (IOC_CbProcEvt_F)CbProcEvtA_F,
      .pCbPriv = (void*)CbPrivA,
      .EvtNum = 1,
      .pEvtIDs = EvtIDsKeepAlive,
  };

  TOS_Result_T Result = __IOC_ConlesMode_subEVT(&SuberArgA);
  ASSERT_EQ(Result, TOS_RESULT_SUCCESS);  // CheckPoint

  IOC_EvtID_T EvtIDsB[] = {IOC_EVTID_TEST_KEEPALIVE};
#define CbProcEvtB_F 0x03UL
#define CbPrivB 0x04UL
  IOC_EvtSubArgs_T SuberArgB = {
      .CbProcEvt_F = (IOC_CbProcEvt_F)CbProcEvtB_F,
      .pCbPriv = (void*)CbPrivB,
      .EvtNum = 1,
      .pEvtIDs = EvtIDsB,
  };

  Result = __IOC_ConlesMode_subEVT(&SuberArgB);
  ASSERT_EQ(Result, TOS_RESULT_NOT_ENOUGH_RESOURCE);  // CheckPoint

  //===>TEARDOWN
  for (ULONG_T Idx = 0; Idx < _mConlesEvtCtx.MaxSuberNum; Idx++) {
    IOC_EvtSubArgs_pT pSavdSubArgs = &_mConlesEvtCtx.pEvtSubers[Idx].Args;
    if (pSavdSubArgs && pSavdSubArgs->pEvtIDs != NULL) {
      free(pSavdSubArgs->pEvtIDs);
      pSavdSubArgs->pEvtIDs = NULL;
      pSavdSubArgs->CbProcEvt_F = NULL;
    }
  }

  free(_mConlesEvtCtx.pEvtSubers);
  _mConlesEvtCtx = BackupCtx;
}

//--->CASE[02]: MaxSuberNum is 1, CurSuberNum is 0, Repeat Nx(ObjA as EvtSuber do [subEvt/unsubEvt]) will SUCCESS
TEST(UT_ConlesModeEventINFILE, Case02_subEVT) {
  //===>SETUP
  _IOC_ConlesEventContext_T BackupCtx = _mConlesEvtCtx;

  //===>EXECUTE
  IOC_EvtID_T EvtIDsKeepAlive[] = {IOC_EVTID_TEST_KEEPALIVE};

  _mConlesEvtCtx.MaxSuberNum = 1;
  _mConlesEvtCtx.CurSuberNum = 0;

  ULONG_T RptNumMax = 1024;

  for (ULONG_T RptNum = 0; RptNum < RptNumMax; RptNum++) {
    IOC_EvtSubArgs_T SubArgsA = {
        .CbProcEvt_F = (IOC_CbProcEvt_F)0x20240126,
        .pCbPriv = (void*)0x20240127,
        .EvtNum = 1,
        .pEvtIDs = EvtIDsKeepAlive,
    };
    TOS_Result_T Result = __IOC_ConlesMode_subEVT(&SubArgsA);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS) << "RptNum: " << RptNum;  // CheckPoint

    IOC_EvtUnsubArgs_T UnsubArgsA = {
        .CbProcEvt_F = (IOC_CbProcEvt_F)0x20240126,
        .pCbPriv = (void*)0x20240127,
    };
    Result = __IOC_ConlesMode_unsubEVT(&UnsubArgsA);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS) << "RptNum: " << RptNum;  // CheckPoint
  }

  //===>TEARDOWN
  for (ULONG_T Idx = 0; Idx < _mConlesEvtCtx.MaxSuberNum; Idx++) {
    IOC_EvtSubArgs_pT pSavdSubArgs = &_mConlesEvtCtx.pEvtSubers[Idx].Args;
    if (pSavdSubArgs && pSavdSubArgs->pEvtIDs != NULL) {
      free(pSavdSubArgs->pEvtIDs);
      pSavdSubArgs->pEvtIDs = NULL;
      pSavdSubArgs->CbProcEvt_F = NULL;
    }
  }
  _mConlesEvtCtx = BackupCtx;
}

// TEST(UT_ConlesModeEventINFILE, unsubEVT) {}
// TEST(UT_ConlesModeEventINFILE, postEVT) {}
#endif /* CONFIG_BUILD_INFILE_UNIT_TESTING_USE_UTFWK_GTEST */
//===> END of internal UT for ConlesMod of Event in CXX

//===>END_OF_NS::IOC_ConlesMode of Event
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//===>BEGIN_OF_NS::IOC_ConetMode
// TODO(@W): IMPLE IOC_ConetMode of Event
//===>END_OF_NS::IOC_Conet
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//===> BEGIN of IMPL PlatIF_IOC in C
#ifdef __cplusplus
extern "C" {
#endif

TOS_Result_T PLT_IOC_subEVT(
    /*ARG_IN*/ IOC_LinkID_T LinkID,
    /*ARG_IN*/ const IOC_EvtSubArgs_pT pEvtSubArgs) {
  if (LinkID == IOC_CONLESMODE_AUTO_LINK_ID) {
    return __IOC_ConlesMode_subEVT(pEvtSubArgs);
  } else {
    return TOS_RESULT_NOT_IMPLEMENTED;
  }
}

TOS_Result_T PLT_IOC_unsubEVT(
    /*ARG_IN*/ IOC_LinkID_T LinkID,
    /*ARG_IN*/ const IOC_EvtUnsubArgs_pT pEvtUnsubArgs) {
  if (LinkID == IOC_CONLESMODE_AUTO_LINK_ID) {
    return __IOC_ConlesMode_unsubEVT(pEvtUnsubArgs);
  } else {
    return TOS_RESULT_NOT_IMPLEMENTED;
  }
}

TOS_Result_T PLT_IOC_postEVT(
    /*ARG_IN*/ IOC_LinkID_T LinkID,
    /*ARG_IN*/ const IOC_EvtDesc_pT pEvtDesc,
    /*ARG_IN_OPTIONAL*/ IOC_Options_pT pOptions) {
  if (LinkID == IOC_CONLESMODE_AUTO_LINK_ID) {
    return __IOC_ConlesMode_postEVT(pEvtDesc, pOptions);
  } else {
    return TOS_RESULT_NOT_IMPLEMENTED;
  }
}

#ifdef __cplusplus
}
#endif
//===> END of IMPL PlatIF_IOC in C
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//===> BEGIN of IOC_Event's internal UT in CXX
#ifdef CONFIG_BUILD_INFILE_UNIT_TESTING_USE_UTFWK_GTEST
// TEST(IOC_Event, ConlesMode_subEVT) { }
#endif /* CONFIG_BUILD_INFILE_UNIT_TESTING_USE_UTFWK_GTEST */
