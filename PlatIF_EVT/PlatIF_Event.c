#include "PlatIF_Event.h"
#include <sys/_types/_null.h>
#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum 
{
    _EVTMGR_STATE_UNINITED = 0,
    _EVTMGR_STATE_READY,
    _EVTMGR_STATE_RUNNING,

} _EvtMangerState_T;
static _EvtMangerState_T _mEvtMangerState = _EVTMGR_STATE_UNINITED;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static uint16_t _mMayRegOperNumMax = 16;

typedef struct 
{
    TOS_EvtOperID_T EvtOperID;

    TOS_EvtOperArgs_T EvtOperArgs;
    
    //TODO(@W): +More...
} _TOS_EvtOperObj_T, *_TOS_EvtOperObj_pT,
  _TOS_EvtSuberObj_T, *_TOS_EvtSuberObj_pT,
    _TOS_EvtPuberObj_T, *_TOS_EvtPuberObj_pT;

static _TOS_EvtOperObj_pT* _mRegedOperObjs = NULL;

static TOS_Result_T __PLT_EVT_isRegedOperID(TOS_EvtOperID_T EvtOperID)
{
    if( _mRegedOperObjs == NULL ){ return TOS_RESULT_BUG;}
    if( EvtOperID >= _mMayRegOperNumMax ){ return TOS_RESULT_BUG;}

    if( _mRegedOperObjs[EvtOperID] 
            && _mRegedOperObjs[EvtOperID]->EvtOperID == EvtOperID )
    {
        return TOS_YES;
    }

    return TOS_NO;
}

static TOS_Result_T __PLT_EVT_doRegOper
    (/*ARG_OUT*/ TOS_EvtOperID_T* pEvtOperID, /*ARG_IN*/const TOS_EvtOperArgs_pT pEvtOperArgs)
{
    if( _mRegedOperObjs == NULL ){ return TOS_RESULT_BUG;}

    for(uint16_t idx = 0; idx < _mMayRegOperNumMax; idx++)
    {
        if( _mRegedOperObjs[idx] == NULL )
        {
            _TOS_EvtOperObj_pT pEvtOperObj 
                = (_TOS_EvtOperObj_pT)calloc(1, sizeof(_TOS_EvtOperObj_T));
            if( pEvtOperObj == NULL )
            {
                return TOS_RESULT_NOT_ENOUGH_MEMORY;
            }

            pEvtOperObj->EvtOperArgs = *pEvtOperArgs;
            *pEvtOperID = pEvtOperObj->EvtOperID = idx;

            _mRegedOperObjs[idx] = pEvtOperObj;
            return TOS_RESULT_SUCCESS;
        }
    }

    return TOS_RESULT_TOO_MANY_REGED;
}

TOS_Result_T PLT_EVT_regOper(/*ARG_OUT*/ TOS_EvtOperID_T* pEvtOperID, /*ARG_IN*/const TOS_EvtOperArgs_pT pEvtOperArgs)
{
    if(_mEvtMangerState != _EVTMGR_STATE_READY)
    {
        return TOS_RESULT_BAD_STATE;
    }
    else
    {
        return __PLT_EVT_doRegOper(pEvtOperID, pEvtOperArgs);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "pthread.h"
typedef struct 
{
    pthread_t TID;
    pthread_mutex_t Mutex;
    pthread_cond_t  Cond;

    uint32_t SeqID;

    uint16_t CurEvtNum;//Current queued EvtDesc number in EvtQueue, a.k.a how many events to processing.
    uint16_t CurEvtHead;//Current queued EvtDesc head index in EvtQueue, a.k.a EvtProcer's next to be processed EvtDesc.
    uint16_t CurEvtTail;//Current queued EvtDesc tail index in EvtQueue, a.k.a EvtPuber's next postEvt's EvtDesc saved index.

    int32_t EvtDescQueueNum;//Number of EvtDesc in EvtQueue, a.k.a EvtQueue's length or depth
    TOS_EvtDesc_T EvtDescQueue[0];
} _TOS_EvtQueue_T, *_TOS_EvtQueue_pT;

static uint16_t _mMayEvtQueueNumMax = 1;//FIX==1 for now, how many EvtQueueProcer may create&run
static _TOS_EvtQueue_pT* _mEvtQueueProcer = NULL;//Each EvtQueueProcer is a thread known as EvtProcer

static uint16_t _mMayEvtDescQueueNumMax = 32;//Each EvtQueue's EvtDescQueue's length or depth

static void ___PLT_EVT_EvtProcer_callEachSuberCbProcEvt(TOS_EvtDesc_pT pEvtDesc);

static void* __PLT_EVT_EvtProcer_Thread(void* pArg)
{
    _TOS_EvtQueue_pT pEvtQueue = (_TOS_EvtQueue_pT)pArg;
    if( pEvtQueue == NULL ){ return NULL;}

    do 
    {
        pthread_mutex_lock(&pEvtQueue->Mutex);
        while( pEvtQueue->CurEvtNum == 0 )
        {
            pthread_cond_wait(&pEvtQueue->Cond, &pEvtQueue->Mutex);
        }

        TOS_EvtDesc_pT pEvtDesc = &pEvtQueue->EvtDescQueue[pEvtQueue->CurEvtHead];
        pEvtQueue->CurEvtHead = (pEvtQueue->CurEvtHead + 1) % pEvtQueue->EvtDescQueueNum;
        pEvtQueue->CurEvtNum--;

        pthread_mutex_unlock(&pEvtQueue->Mutex);

        ___PLT_EVT_EvtProcer_callEachSuberCbProcEvt(pEvtDesc);
    }while ( 0x20231126 );
}

TOS_Result_T __PLT_EVT_doEnableEvtManger_ofStartupEvtProcer(void)
{
    if( _mEvtQueueProcer == NULL ){ return TOS_RESULT_BUG;}

    for(uint16_t idx = 0; idx < _mMayEvtQueueNumMax; idx++)
    {
        if( _mEvtQueueProcer[idx] == NULL )
        {
            _TOS_EvtQueue_pT pEvtQueue 
                = (_TOS_EvtQueue_pT)calloc
                    (sizeof(_TOS_EvtQueue_T) + sizeof(TOS_EvtDesc_T) * _mMayEvtDescQueueNumMax, 1);
            if( pEvtQueue == NULL )
            {
                return TOS_RESULT_NOT_ENOUGH_MEMORY;
            }
            else 
            {
                _mEvtQueueProcer[idx] = pEvtQueue;
            }

            pEvtQueue->EvtDescQueueNum = _mMayEvtDescQueueNumMax;
            pthread_mutex_init(&pEvtQueue->Mutex, NULL);
            pthread_cond_init(&pEvtQueue->Cond, NULL);
            pthread_create(&pEvtQueue->TID, NULL, __PLT_EVT_EvtProcer_Thread, (void*)pEvtQueue);

            return TOS_RESULT_SUCCESS;
        }
    }

    return TOS_RESULT_TOO_MANY_REGED;
}

//Step-1: Check EvtManger is StateReady
//Step-2: Startup EvtProcer Thread<s>
TOS_Result_T PLT_EVT_enableEvtManger(void)
{
    if(_mEvtMangerState != _EVTMGR_STATE_READY)
    {
        return TOS_RESULT_BAD_STATE;
    }
    
    TOS_Result_T Result = __PLT_EVT_doEnableEvtManger_ofStartupEvtProcer();
    if( TOS_RESULT_SUCCESS == Result )
    {
        _mEvtMangerState = _EVTMGR_STATE_RUNNING;
    }

    return Result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PSDB = _mEvtPSDB_PubTable + _mEvtPSDB_SubTable

typedef enum 
{
    _TOS_RowType_EvtPuber = 1,
    _TOS_RowType_EvtSuber = 2,
} _TOS_EvtPBDB_RowType_T;

typedef struct 
{
    _TOS_EvtPBDB_RowType_T Type;

    union 
    {
        struct 
        {
            TOS_EvtOperID_T EvtPuberID;
        };

        struct
        {
            TOS_EvtOperID_T   EvtSuberID;
            TOS_EvtSubArgs_pT pEvtSubArgs;
        };
    };

    uint32_t NumIDs;
    TOS_EvtID_T EvtIDs[0];
} _TOS_EvtPSDB_Row_T, *_TOS_EvtPSDB_Row_pT;

static uint32_t _mMayPubEvtNumMax = 16;
static _TOS_EvtPSDB_Row_pT* _mEvtPSDB_PubTable = NULL;


static TOS_Result_T __PLT_EVT_doPubEvts
    (TOS_EvtOperID_T EvtPuberID, TOS_EvtID_T EvtIDs[], uint32_t NumIDs)
{
    if( _mEvtPSDB_PubTable == NULL ){ return TOS_RESULT_BUG;}

    for(uint32_t row = 0; row <= _mMayPubEvtNumMax; row++)
    {
        if( _mEvtPSDB_PubTable[row] == NULL )
        {
            _TOS_EvtPSDB_Row_pT pRow 
                = (_TOS_EvtPSDB_Row_pT)calloc
                    (sizeof(_TOS_EvtPSDB_Row_T) + sizeof(TOS_EvtID_T) * NumIDs, 1);
            if( pRow == NULL )
            {
                return TOS_RESULT_NOT_ENOUGH_MEMORY;
            }
            else 
            {
                _mEvtPSDB_PubTable[row] = pRow;
            }

            pRow->Type       = _TOS_RowType_EvtPuber;
            pRow->NumIDs     = NumIDs;
            pRow->EvtPuberID = EvtPuberID;
            
            for(uint32_t num = 0; num < NumIDs; num++)
            {
                pRow->EvtIDs[num] = EvtIDs[num];
            }

            return TOS_RESULT_SUCCESS;
        }
    }

    return TOS_RESULT_TOO_MANY_PUBED_EVENTS;
}

/**
 * Step-1: Check if the EvtPuberID is valid by regOper as EvtPuber
 * Step-2: Add EvtIDs to PubSub's EvtIDs DB, a.k.a PSDB.
 */
TOS_Result_T PLT_EVT_pubEvts
    (/*ARG_IN*/TOS_EvtOperID_T EvtPuberID, /*ARG_IN*/TOS_EvtID_T EvtIDs[], /*ARG_IN*/uint32_t NumIDs)
{
    if( TOS_YES != __PLT_EVT_isRegedOperID(EvtPuberID) )
    {
        return TOS_RESULT_NOT_REGED;
    }

    return __PLT_EVT_doPubEvts(EvtPuberID, EvtIDs, NumIDs);
}

static uint32_t _mMaySubEvtNumMax = 16;
static _TOS_EvtPSDB_Row_pT* _mEvtPSDB_SubTable = NULL;

static void ___PLT_EVT_EvtProcer_callEachSuberCbProcEvt(TOS_EvtDesc_pT pEvtDesc)
{
    if( _mEvtPSDB_SubTable == NULL ){ return;}

    for(uint32_t row = 0; row < _mMaySubEvtNumMax; row++)//Which EvtSuber in PSDB from PLT_EVT_subEvt()
    {
        if( _mEvtPSDB_SubTable[row] == NULL ){ continue;}

        _TOS_EvtPSDB_Row_pT pEvtSuber = _mEvtPSDB_SubTable[row];
        for(uint32_t num = 0; num <= pEvtSuber->NumIDs; num++)//Which EvtID of current EvtSuber
        {
            if( (TOS_GET_EVTID_CLASS(pEvtSuber->EvtIDs[num]) == TOS_GET_EVTID_CLASS(pEvtDesc->EvtID))
                && (TOS_GET_EVTID_CLASSMATE(pEvtSuber->EvtIDs[num]) & TOS_GET_EVTID_CLASSMATE(pEvtDesc->EvtID)) )
            {
                pEvtSuber->pEvtSubArgs->CbProcEvtSRT_F(pEvtSuber->EvtSuberID, pEvtDesc, pEvtSuber->pEvtSubArgs->ToObjPriv);
            }
        }
    }
}

TOS_Result_T __PLT_EVT_doSubEvts
    (TOS_EvtOperID_T EvtSuberID, TOS_EvtID_T EvtIDs[], uint32_t NumIDs, const TOS_EvtSubArgs_pT pEvtSubArgs)
{
    if( _mEvtPSDB_SubTable == NULL ){ return TOS_RESULT_BUG;}

    for(uint32_t row = 0; row < _mMaySubEvtNumMax; row++)
    {
        if( _mEvtPSDB_SubTable[row] == NULL )
        {
            _TOS_EvtPSDB_Row_pT pEvtSuber 
                = (_TOS_EvtPSDB_Row_pT)calloc
                    (sizeof(_TOS_EvtPSDB_Row_T) + sizeof(TOS_EvtID_T) * NumIDs, 1);
            if( pEvtSuber == NULL )
            {
                return TOS_RESULT_NOT_ENOUGH_MEMORY;
            }

            pEvtSuber->Type        = _TOS_RowType_EvtSuber;
            pEvtSuber->EvtPuberID  = EvtSuberID;
            pEvtSuber->pEvtSubArgs = malloc(sizeof(TOS_EvtSubArgs_T));
            if( pEvtSuber->pEvtSubArgs == NULL )
            {
                free(pEvtSuber);
                return TOS_RESULT_NOT_ENOUGH_MEMORY;
            }
            else 
            {
                *pEvtSuber->pEvtSubArgs = *pEvtSubArgs;
            }

            pEvtSuber->NumIDs = NumIDs;
            for(uint32_t num = 0; num < NumIDs; num++)
            {
                pEvtSuber->EvtIDs[num] = EvtIDs[num];
            }

            _mEvtPSDB_SubTable[row] = pEvtSuber;
            return TOS_RESULT_SUCCESS;
        }
    }

    return TOS_RESULT_TOO_MANY_PUBED_EVENTS;
}

/**
 * Step-1: Check if the EvtSuberID is valid by regOper as EvtSuber
 * Step-2: Add EvtIDs to PubSub's EvtIDs DB, a.k.a PSDB.
 */
TOS_Result_T PLT_EVT_subEvts
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/TOS_EvtID_T EvtIDs[], /*ARG_IN*/uint32_t NumIDs, 
     /*ARG_IN*/const TOS_EvtSubArgs_pT pEvtSubArgs)
{
    if( TOS_YES != __PLT_EVT_isRegedOperID(EvtSuberID) )
    {
        return TOS_RESULT_NOT_REGED;
    }

    return __PLT_EVT_doSubEvts(EvtSuberID, EvtIDs, NumIDs, pEvtSubArgs);
}

TOS_Result_T __PLT_EVT_doPostEvtSRT
    (/*ARG_IN*/TOS_EvtOperID_T EvtPuberID, /*ARG_IN*/TOS_EvtDesc_pT pEvtDesc)
{
    _TOS_EvtQueue_pT pEvtQueue = _mEvtQueueProcer[0];//FIX==0 for now, only one EvtQueueProcer

    pthread_mutex_lock(&pEvtQueue->Mutex);
    if( pEvtQueue->CurEvtNum == pEvtQueue->EvtDescQueueNum )
    {
        pthread_mutex_unlock(&pEvtQueue->Mutex);
        return TOS_RESULT_TOO_MANY_PUBED_EVENTS;
    }

    pEvtDesc->EvtPuberID = EvtPuberID;
    pEvtDesc->SeqID = pEvtQueue->SeqID++;

    pEvtQueue->EvtDescQueue[pEvtQueue->CurEvtTail] = *pEvtDesc;
    pEvtQueue->CurEvtTail = (pEvtQueue->CurEvtTail + 1) % pEvtQueue->EvtDescQueueNum;
    pEvtQueue->CurEvtNum++;
    pthread_mutex_unlock(&pEvtQueue->Mutex);

    pthread_cond_signal(&pEvtQueue->Cond);

    return TOS_RESULT_SUCCESS;
}

TOS_Result_T PLT_EVT_postEvtSRT
    (/*ARG_IN*/TOS_EvtOperID_T EvtPuberID, /*ARG_IN*/TOS_EvtDesc_pT pEvtDesc)
{
    if( TOS_YES != __PLT_EVT_isRegedOperID(EvtPuberID) )
    {
        return TOS_RESULT_NOT_REGED;
    }

    return __PLT_EVT_doPostEvtSRT(EvtPuberID, pEvtDesc);
}

#ifdef CONFIG_BUILD_FEATURE_HARD_REAL_TIME
TOS_Result_T PLT_EVT_postEvtHRT
    (/*ARG_IN*/TOS_EvtOperID_T, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc);
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TOS_Result_T PLT_EVT_initEvtManger(/*ARG_IN*/const TOS_EvtModuleArgs_pT pEvtModArgs)
{
    TOS_Result_T Result = TOS_RESULT_BUG;

    if( _mEvtMangerState == _EVTMGR_STATE_UNINITED )
    {
        if( pEvtModArgs )
        {
            //---------------------------------------------------------------------------------------------------------
            // Init _mMayRegOperNumMax
            if( pEvtModArgs->Params.MayRegOperNumMax > 0 )
            {
                _mMayRegOperNumMax = pEvtModArgs->Params.MayRegOperNumMax;
            }

            _mRegedOperObjs = (_TOS_EvtOperObj_pT*)calloc(sizeof(_TOS_EvtOperObj_pT), _mMayRegOperNumMax);
            if( _mRegedOperObjs == NULL )
            {
                Result = TOS_RESULT_NOT_ENOUGH_MEMORY;
                goto _RetErrResult;
            }

            //---------------------------------------------------------------------------------------------------------
            // Init _mMayPubEvtNumMax
            if( pEvtModArgs->Params.MayPubEvtNumMax > 0 )
            {
                _mMayPubEvtNumMax = pEvtModArgs->Params.MayPubEvtNumMax;
            }

            _mEvtPSDB_PubTable = (_TOS_EvtPSDB_Row_pT*)calloc(sizeof(_TOS_EvtPSDB_Row_pT), _mMayPubEvtNumMax);
            if( _mEvtPSDB_PubTable == NULL )
            {
                Result = TOS_RESULT_NOT_ENOUGH_MEMORY;
                goto _RetErrResult;
            }

            //---------------------------------------------------------------------------------------------------------
            // Init _mMaySubEvtNumMax
            if( pEvtModArgs->Params.MaySubEvtNumMax > 0 )
            {
                _mMaySubEvtNumMax = pEvtModArgs->Params.MaySubEvtNumMax;
            }

            _mEvtPSDB_SubTable = (_TOS_EvtPSDB_Row_pT*)calloc(sizeof(_TOS_EvtPSDB_Row_pT), _mMaySubEvtNumMax);
            if( _mEvtPSDB_SubTable == NULL )
            {
                Result = TOS_RESULT_NOT_ENOUGH_MEMORY;
                goto _RetErrResult;
            }

            //---------------------------------------------------------------------------------------------------------
            // Init _mMayEvtQueueNumMax
            //if( pEvtModArgs->Params.MayEvtQueueNumMax > 0 )
            //{
            //    _mMayEvtQueueNumMax = pEvtModArgs->Params.MayEvtQueueNumMax;
            //}

            _mEvtQueueProcer = (_TOS_EvtQueue_pT*)calloc(sizeof(_TOS_EvtQueue_pT), _mMayEvtQueueNumMax);
            if( _mEvtQueueProcer == NULL )
            {
                Result = TOS_RESULT_NOT_ENOUGH_MEMORY;
                goto _RetErrResult;
            }

            //---------------------------------------------------------------------------------------------------------
            //TODO(@W): pEvtModArgs->Params.xyz
        }

        _mEvtMangerState = _EVTMGR_STATE_READY;
        return TOS_RESULT_SUCCESS;
    }
    else
    {
        return TOS_RESULT_BUG;
    }

_RetErrResult:
    if(_mRegedOperObjs)
    {
        free(_mRegedOperObjs);
        _mRegedOperObjs = NULL;
    }

    if(_mEvtPSDB_PubTable)
    {
        free(_mEvtPSDB_PubTable);
        _mEvtPSDB_PubTable = NULL;
    }

    if(_mEvtPSDB_SubTable)
    {
        free(_mEvtPSDB_SubTable);
        _mEvtPSDB_SubTable = NULL;
    }

    if(_mEvtQueueProcer)
    {
        free(_mEvtQueueProcer);
        _mEvtQueueProcer = NULL;
    }
    return Result;
}

#ifdef CONFIG_BUILD_WITH_UNIT_TESTING
void PLT_EVT_deinitEvtManger(void)
{
    return;
}

void PLT_EVT_unsubEvts(/*ARG_IN*/TOS_EvtOperID_T EvtOperID)
{
    if( _mEvtPSDB_SubTable == NULL ){ return;}

    for(uint32_t row = 0; row < _mMaySubEvtNumMax; row++)
    {
        if( _mEvtPSDB_SubTable[row] 
            && _mEvtPSDB_SubTable[row]->Type == _TOS_RowType_EvtSuber
            && _mEvtPSDB_SubTable[row]->EvtSuberID == EvtOperID )
        {
            if( _mEvtPSDB_SubTable[row]->pEvtSubArgs )
            {
                free(_mEvtPSDB_SubTable[row]->pEvtSubArgs);
                _mEvtPSDB_SubTable[row]->pEvtSubArgs = NULL;
            }

            free(_mEvtPSDB_SubTable[row]);
            _mEvtPSDB_SubTable[row] = NULL;
            return;
        }
    }
    
}

void PLT_EVT_unpubEvts(/*ARG_IN*/TOS_EvtOperID_T EvtOperID)
{
    if( _mEvtPSDB_PubTable == NULL ){ return;}

    for(uint32_t row = 0; row < _mMayPubEvtNumMax; row++)
    {
        if( _mEvtPSDB_PubTable[row] 
            && _mEvtPSDB_PubTable[row]->Type == _TOS_RowType_EvtPuber
            && _mEvtPSDB_PubTable[row]->EvtPuberID == EvtOperID )
        {
            free(_mEvtPSDB_PubTable[row]);
            _mEvtPSDB_PubTable[row] = NULL;
            return;
        }
    }
}

static void __PLT_EVT_doDisableEvtManger_ofStopEvtProcer(void)
{
    if( _mEvtQueueProcer == NULL ){ return;}

    for(uint16_t idx = 0; idx < _mMayEvtQueueNumMax; idx++)
    {
        if( _mEvtQueueProcer[idx] )
        {
            pthread_cancel(_mEvtQueueProcer[idx]->TID);
            pthread_join(_mEvtQueueProcer[idx]->TID, NULL);

            pthread_mutex_destroy(&_mEvtQueueProcer[idx]->Mutex);
            pthread_cond_destroy(&_mEvtQueueProcer[idx]->Cond);

            free(_mEvtQueueProcer[idx]);
            _mEvtQueueProcer[idx] = NULL;
        }
    }

    return;

}

void PLT_EVT_disableEvtManger(void)
{
    if( _mEvtMangerState == _EVTMGR_STATE_RUNNING )
    {
        __PLT_EVT_doDisableEvtManger_ofStopEvtProcer();
        _mEvtMangerState = _EVTMGR_STATE_READY;
    }

    return;
}


void PLT_EVT_unregOper(/*ARG_IN*/ TOS_EvtOperID_T EvtOperID)
{
    if( _mRegedOperObjs == NULL ){ return;}

    for(uint16_t idx = 0; idx < _mMayRegOperNumMax; idx++)
    {
        if( _mRegedOperObjs[idx] 
            && _mRegedOperObjs[idx]->EvtOperID == EvtOperID )
        {
            free(_mRegedOperObjs[idx]);
            _mRegedOperObjs[idx] = NULL;
            return;
        }
    }

    return;
}
#endif//CONFIG_BUILD_WITH_UNIT_TESTING