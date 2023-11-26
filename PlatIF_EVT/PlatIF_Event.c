#include "PlatIF_Event.h"
#include <sys/_types/_null.h>

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
            _mRegedOperObjs[idx] = (_TOS_EvtOperObj_pT)calloc(sizeof(_TOS_EvtOperObj_T), 1);
            if( _mRegedOperObjs[idx] == NULL )
            {
                return TOS_RESULT_NOT_ENOUGH_MEMORY;
            }

            _mRegedOperObjs[idx]->EvtOperID = idx;
            _mRegedOperObjs[idx]->EvtOperArgs = *pEvtOperArgs;

            *pEvtOperID = idx;
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
TOS_Result_T PLT_EVT_enableEvtManger(void)
{
    return TOS_RESULT_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PSDB = _mEvtPSDB_PubTable + _mEvtPSDB_SubTable

typedef struct 
{
    TOS_EvtOperID_T EvtPuberID;

    uint32_t NumIDs;
    TOS_EvtID_T EvtIDs[0];
} _TOS_EvtPSDB_Row_T, *_TOS_EvtPSDB_Row_pT;

static uint32_t _mMayPubEvtNumMax = 16;
static _TOS_EvtPSDB_Row_pT* _mEvtPSDB_PubTable = NULL;

static TOS_Result_T __PLT_EVT_doPubEvts
    (TOS_EvtOperID_T EvtPuberID, TOS_EvtID_T EvtIDs[], uint32_t NumIDs)
{
    if( _mEvtPSDB_PubTable == NULL ){ return TOS_RESULT_BUG;}

    for(uint32_t row = 0; row < _mMayPubEvtNumMax; row++)
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

            pRow->EvtPuberID = EvtPuberID;
            pRow->NumIDs = NumIDs;
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

TOS_Result_T __PLT_EVT_doSubEvts
    (TOS_EvtOperID_T EvtSuberID, TOS_EvtID_T EvtIDs[], uint32_t NumIDs)
{
    if( _mEvtPSDB_SubTable == NULL ){ return TOS_RESULT_BUG;}

    for(uint32_t row = 0; row < _mMaySubEvtNumMax; row++)
    {
        if( _mEvtPSDB_SubTable[row] == NULL )
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
                _mEvtPSDB_SubTable[row] = pRow;
            }

            pRow->EvtPuberID = EvtSuberID;
            pRow->NumIDs = NumIDs;
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

    return __PLT_EVT_doSubEvts(EvtSuberID, EvtIDs, NumIDs);
}

TOS_Result_T PLT_EVT_postEvtSRT
    (/*ARG_IN*/TOS_EvtOperID_T EvtPuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc)
{
    return TOS_RESULT_NOT_SUPPORTED;
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
    return Result;
}

#ifdef CONFIG_BUILD_WITH_UNIT_TESTING
void PLT_EVT_deinitEvtManger(void)
{
    return;
}
void PLT_EVT_unsubEvts(/*ARG_IN*/TOS_EvtOperID_T EvtOperID)
{

}
void PLT_EVT_unpubEvts(/*ARG_IN*/TOS_EvtOperID_T EvtOperID)
{

}
void PLT_EVT_disableEvtManger(void)
{
    return;
}
void PLT_EVT_unregOper(/*ARG_IN*/ TOS_EvtOperID_T EvtOperID)
{
    return;
}
#endif//CONFIG_BUILD_WITH_UNIT_TESTING