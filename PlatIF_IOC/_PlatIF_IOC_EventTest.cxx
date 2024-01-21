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
    return TOS_RESULT_BUG;
  }

  if (_mConlesEvtCtx.CurSuberNum >= _mConlesEvtCtx.MaxSuberNum) {
    pthread_mutex_unlock(&_mConlesEvtCtx.Mutex);
    return TOS_RESULT_NOT_ENOUGH_RESOURCE;
  }

  if (_mConlesEvtCtx.pSuberArgs == NULL) {
    _mConlesEvtCtx.pSuberArgs = (IOC_EvtSubArgs_pT)calloc(_mConlesEvtCtx.MaxSuberNum, sizeof(IOC_EvtSubArgs_T));
    if (_mConlesEvtCtx.pSuberArgs == NULL) {
      pthread_mutex_unlock(&_mConlesEvtCtx.Mutex);
      return TOS_RESULT_NOT_ENOUGH_RESOURCE;
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
    return TOS_RESULT_TOO_MANY_SUBED_EVENTS;
  }

  IOC_EvtSubArgs_pT pSavdSubArgs = &_mConlesEvtCtx.pSuberArgs[FreeSuberIdx];
  size_t SavdEvtIDSiz = sizeof(IOC_EvtID_T) * pEvtSubArgs->EvtNum;
  memcpy(pSavdSubArgs, pEvtSubArgs, sizeof(IOC_EvtSubArgs_T));

  pSavdSubArgs->pEvtIDs = (IOC_EvtID_T*)malloc(SavdEvtIDSiz);
  if (pSavdSubArgs->pEvtIDs == NULL) {
    pthread_mutex_unlock(&_mConlesEvtCtx.Mutex);
    return TOS_RESULT_NOT_ENOUGH_RESOURCE;
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

  if (_mConlesEvtCtx.pSuberArgs == NULL) {
    pthread_mutex_unlock(&_mConlesEvtCtx.Mutex);
    return TOS_RESULT_NOT_FOUND;
  }

  if (pEvtUnsubArgs == NULL) {
    for (unsigned long Idx = 0; Idx < _mConlesEvtCtx.CurSuberNum; Idx++) {
      IOC_EvtSubArgs_pT pSavdSubArgs = &_mConlesEvtCtx.pSuberArgs[Idx];
      if (pSavdSubArgs->pEvtIDs != NULL) {
        free(pSavdSubArgs->pEvtIDs);
        pSavdSubArgs->CbProcEvt_F = NULL;
      }
    }
  } else {
    for (unsigned long Idx = 0; Idx < _mConlesEvtCtx.CurSuberNum; Idx++) {
      IOC_EvtSubArgs_pT pSavdSubArgs = &_mConlesEvtCtx.pSuberArgs[Idx];
      if (pSavdSubArgs->CbProcEvt_F == pEvtUnsubArgs->CbProcEvt_F && pSavdSubArgs->pCbPriv == pEvtUnsubArgs->pCbPriv) {
        free(pSavdSubArgs->pEvtIDs);
        pSavdSubArgs->CbProcEvt_F = NULL;
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
TEST(UT_INTNAL_ConlesMode, subEVT) {
  // Write UT for __IOC_ConlesMode_subEVT
}
TEST(UT_INTNAL_ConlesMode, unsubEVT) {}
TEST(UT_INTNAL_ConlesMode, postEVT) {}
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
