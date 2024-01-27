#include "_PlatIF_IOC.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//===>BEGIN_OF_NS::IOC_ConlesMode of Event
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  unsigned long MaxSuberNum;
  unsigned long CurSuberNum;

  IOC_EvtSubArgs_pT pSuberArgs;
  pthread_mutex_t Mutex;
} _IOC_ConslesEventContext_T, *_IOC_ConslesEventContext_pT;

static _IOC_ConslesEventContext_T _mConlesEvtCtx = {
    .MaxSuberNum = 32,
    .CurSuberNum = 0,
    .pSuberArgs = NULL,
    .Mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER,
};

static TOS_Result_T __IOC_ConlesMode_subEVT(const IOC_EvtSubArgs_pT pEvtSubArgs) {
  int RetPSX = pthread_mutex_lock(&_mConlesEvtCtx.Mutex);
  if (RetPSX != 0) {
    return TOS_RESULT_NOT_TESTED_BUG;
  }

  if (_mConlesEvtCtx.CurSuberNum >= _mConlesEvtCtx.MaxSuberNum) {
    pthread_mutex_unlock(&_mConlesEvtCtx.Mutex);
    return TOS_RESULT_NOT_ENOUGH_RESOURCE;
  }

  if (_mConlesEvtCtx.pSuberArgs == NULL) {
    _mConlesEvtCtx.pSuberArgs = (IOC_EvtSubArgs_pT)calloc(_mConlesEvtCtx.MaxSuberNum, sizeof(IOC_EvtSubArgs_T));
    if (_mConlesEvtCtx.pSuberArgs == NULL) {
      pthread_mutex_unlock(&_mConlesEvtCtx.Mutex);
      return TOS_RESULT_NOT_TESTED_BUG;
    }
  }

  unsigned long FreeSuberIdx = 0;
  for (; FreeSuberIdx < _mConlesEvtCtx.MaxSuberNum; FreeSuberIdx++) {
    if (_mConlesEvtCtx.pSuberArgs[FreeSuberIdx].CbProcEvt_F == NULL) {
      break;
    }
  }

  if (FreeSuberIdx == _mConlesEvtCtx.MaxSuberNum) {
    pthread_mutex_lock(&_mConlesEvtCtx.Mutex);
    return TOS_RESULT_NOT_TESTED_BUG;
  }

  IOC_EvtSubArgs_pT pSavdSubArgs = &_mConlesEvtCtx.pSuberArgs[FreeSuberIdx];
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

  if (_mConlesEvtCtx.pSuberArgs == NULL) {
    pthread_mutex_unlock(&_mConlesEvtCtx.Mutex);
    return Result;
  }

  if (pEvtUnsubArgs == NULL) {
    for (unsigned long Idx = 0; Idx < _mConlesEvtCtx.CurSuberNum; Idx++) {
      IOC_EvtSubArgs_pT pSavdSubArgs = &_mConlesEvtCtx.pSuberArgs[Idx];
      if (pSavdSubArgs->pEvtIDs != NULL) {
        free(pSavdSubArgs->pEvtIDs);
        pSavdSubArgs->pEvtIDs = NULL;
        pSavdSubArgs->CbProcEvt_F = NULL;
      }
    }

    _mConlesEvtCtx.CurSuberNum = 0;
    Result = TOS_RESULT_SUCCESS;
  } else {
    for (unsigned long Idx = 0; Idx < _mConlesEvtCtx.CurSuberNum; Idx++) {
      IOC_EvtSubArgs_pT pSavdSubArgs = &_mConlesEvtCtx.pSuberArgs[Idx];
      if (pSavdSubArgs->CbProcEvt_F == pEvtUnsubArgs->CbProcEvt_F && pSavdSubArgs->pCbPriv == pEvtUnsubArgs->pCbPriv) {
        free(pSavdSubArgs->pEvtIDs);
        pSavdSubArgs->pEvtIDs = NULL;
        pSavdSubArgs->CbProcEvt_F = NULL;

        _mConlesEvtCtx.CurSuberNum--;
        Result = TOS_RESULT_SUCCESS;
        break;
      }
    }
  }

  pthread_mutex_unlock(&_mConlesEvtCtx.Mutex);
  return TOS_RESULT_SUCCESS;
}

static TOS_Result_T __IOC_ConlesMode_postEVT(const IOC_EvtDesc_pT pEvtDesc, const IOC_Options_pT pOptions) {
  bool IsCbProcEvt_Found = false;

  int RetPSX = pthread_mutex_lock(&_mConlesEvtCtx.Mutex);
  if (RetPSX != 0) {
    return TOS_RESULT_BUG;
  }
  if (_mConlesEvtCtx.pSuberArgs == NULL) {
    pthread_mutex_unlock(&_mConlesEvtCtx.Mutex);
    return TOS_RESULT_NOT_FOUND;
  }

  for (unsigned long Idx = 0; Idx < _mConlesEvtCtx.CurSuberNum; Idx++) {
    IOC_EvtSubArgs_pT pSavdSubArgs = &_mConlesEvtCtx.pSuberArgs[Idx];
    if (pSavdSubArgs->CbProcEvt_F != NULL && pSavdSubArgs->pEvtIDs != NULL) {
      for (unsigned long Idx2 = 0; Idx2 < pSavdSubArgs->EvtNum; Idx2++) {
        if (pSavdSubArgs->pEvtIDs[Idx2] == pEvtDesc->EvtID) {
          pSavdSubArgs->CbProcEvt_F(pEvtDesc, pSavdSubArgs->pCbPriv);
          IsCbProcEvt_Found = true;
          break;
        }
      }
    }
  }
  pthread_mutex_unlock(&_mConlesEvtCtx.Mutex);

  if (!IsCbProcEvt_Found) {
    return TOS_RESULT_NO_EVT_SUBER;
  } else {
    return TOS_RESULT_SUCCESS;
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
TEST(EventConlesModeInternal, Case01_subEVT) {
  //===>SETUP
  _IOC_ConslesEventContext_T BackupCtx = _mConlesEvtCtx;

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
  for (unsigned long Idx = 0; Idx < _mConlesEvtCtx.MaxSuberNum; Idx++) {
    IOC_EvtSubArgs_pT pSavdSubArgs = &_mConlesEvtCtx.pSuberArgs[Idx];
    if (pSavdSubArgs && pSavdSubArgs->pEvtIDs != NULL) {
      free(pSavdSubArgs->pEvtIDs);
      pSavdSubArgs->pEvtIDs = NULL;
      pSavdSubArgs->CbProcEvt_F = NULL;
    }
  }

  free(_mConlesEvtCtx.pSuberArgs);
  _mConlesEvtCtx = BackupCtx;
}

//--->CASE[02]: MaxSuberNum is 1, CurSuberNum is 0, Repeat Nx(ObjA as EvtSuber do [subEvt/unsubEvt]) will SUCCESS
TEST(EventConlesModeInternal, Case02_subEVT) {
  //===>SETUP
  _IOC_ConslesEventContext_T BackupCtx = _mConlesEvtCtx;

  //===>EXECUTE
  IOC_EvtID_T EvtIDsKeepAlive[] = {IOC_EVTID_TEST_KEEPALIVE};

  _mConlesEvtCtx.MaxSuberNum = 1;
  _mConlesEvtCtx.CurSuberNum = 0;

  unsigned long RptNumMax = 1024;

  for (unsigned long RptNum = 0; RptNum < RptNumMax; RptNum++) {
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
  for (unsigned long Idx = 0; Idx < _mConlesEvtCtx.MaxSuberNum; Idx++) {
    IOC_EvtSubArgs_pT pSavdSubArgs = &_mConlesEvtCtx.pSuberArgs[Idx];
    if (pSavdSubArgs && pSavdSubArgs->pEvtIDs != NULL) {
      free(pSavdSubArgs->pEvtIDs);
      pSavdSubArgs->pEvtIDs = NULL;
      pSavdSubArgs->CbProcEvt_F = NULL;
    }
  }
  _mConlesEvtCtx = BackupCtx;
}

// TEST(EventConlesModeInternal, unsubEVT) {}
// TEST(EventConlesModeInternal, postEVT) {}
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
