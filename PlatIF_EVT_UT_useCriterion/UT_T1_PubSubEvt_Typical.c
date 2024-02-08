#include "_UT_EVT_Common.h"

/**
 * @brief Typical PubSubEvt Case Lists
 * UT_T1_PubSubEvt_Typical_NN
 *  =[01]: PubEvt/SubEvt, PostEvtSRT/CbProcEvtSRT of TOS_EVTID_TEST_KEEPALIVE
 *  =[02]: PubEvt/SubEvt, PostEvtSRT/CbProcEvtSRT of TOS_EVTID_TEST_MSGDATA
 *  =[03]: PubEvt/SubEvt, PostEvtSRT/CbProcEvtSRT of TOS_EVTID_TEST_ECHO_REQUEST/_ECHO_RESPONSE
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define _UT_OPERATOR_COUNT 12
 typedef struct 
 {
    TOS_EvtOperID_T EvtOperID_ModObjUT_A[_UT_OPERATOR_COUNT/3];
    TOS_EvtOperID_T EvtOperID_ModObjUT_B[_UT_OPERATOR_COUNT/3];
    TOS_EvtOperID_T EvtOperID_ModObjUT_C[_UT_OPERATOR_COUNT/3];
 } _UT_OperatorContext_T, *_UT_OperatorContext_pT;

static _UT_OperatorContext_T _mPubSubCtx = { };
void _UT_PubSubEvt_Typical_CASE_setupUnitContext(void)
{
    TOS_EvtOperArgs_T EvtPuberArgs = { };

    for( uint32_t OperCnt=0; OperCnt<TOS_calcArrayElmtCnt(_mPubSubCtx.EvtOperID_ModObjUT_A); OperCnt++ )
    {
        EvtPuberArgs.ModObjID = TOS_MODOBJID_UT_A;
        TOS_Result_T Result = PLT_EVT_regOper(&_mPubSubCtx.EvtOperID_ModObjUT_A[OperCnt], &EvtPuberArgs);
        cr_assert_eq(Result, TOS_RESULT_SUCCESS);
    }

    for( uint32_t OperCnt=0; OperCnt<TOS_calcArrayElmtCnt(_mPubSubCtx.EvtOperID_ModObjUT_B); OperCnt++ )
    {
        EvtPuberArgs.ModObjID = TOS_MODOBJID_UT_B;
        TOS_Result_T Result = PLT_EVT_regOper(&_mPubSubCtx.EvtOperID_ModObjUT_B[OperCnt], &EvtPuberArgs);
        cr_assert_eq(Result, TOS_RESULT_SUCCESS);
    }

    for( uint32_t OperCnt=0; OperCnt<TOS_calcArrayElmtCnt(_mPubSubCtx.EvtOperID_ModObjUT_C); OperCnt++ )
    {
        EvtPuberArgs.ModObjID = TOS_MODOBJID_UT_C;
        TOS_Result_T Result = PLT_EVT_regOper(&_mPubSubCtx.EvtOperID_ModObjUT_C[OperCnt], &EvtPuberArgs);
        cr_assert_eq(Result, TOS_RESULT_SUCCESS);
    }
    
    return ;
}

void _UT_PubSubEvt_Typical_CASE_teardownUnitContext(void)
{
    _UT_OperatorContext_pT pPubSubCtx = &_mPubSubCtx;

    for( uint32_t OperCnt=0; OperCnt<TOS_calcArrayElmtCnt(pPubSubCtx->EvtOperID_ModObjUT_A); OperCnt++ )
    {
        PLT_EVT_unregOper(pPubSubCtx->EvtOperID_ModObjUT_A[OperCnt]);
    }
    
    for( uint32_t OperCnt=0; OperCnt<TOS_calcArrayElmtCnt(pPubSubCtx->EvtOperID_ModObjUT_B); OperCnt++ )
    {
        PLT_EVT_unregOper(pPubSubCtx->EvtOperID_ModObjUT_B[OperCnt]);
    }

    for( uint32_t OperCnt=0; OperCnt<TOS_calcArrayElmtCnt(pPubSubCtx->EvtOperID_ModObjUT_C); OperCnt++ )
    {
        PLT_EVT_unregOper(pPubSubCtx->EvtOperID_ModObjUT_C[OperCnt]);
    }

    return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 typedef struct 
 {
    TOS_EvtOperID_T EvtSuberID;
    ULONG_T KeepAliveTotalCnt, KeepAliveNextSeqID;

    sem_t *pSemAllProced;
 } _UT_EvtSuberPrivT01_T, *_UT_EvtSuberPrivT01_pT;      

static TOS_Result_T __UT_ProcEvtSRT_Typical_01
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToObjPriv)
{
    _UT_EvtSuberPrivT01_pT pEvtSuberPriv = (_UT_EvtSuberPrivT01_pT)pToObjPriv;
    cr_assert_eq(EvtSuberID, pEvtSuberPriv->EvtSuberID);//CheckPoint

    cr_assert_eq(pEvtDesc->SeqID, pEvtSuberPriv->KeepAliveNextSeqID);//CheckPoint
    pEvtSuberPriv->KeepAliveNextSeqID++;

    TOS_EvtID_T EvtID = pEvtDesc->EvtID;
    cr_assert_eq(EvtID, TOS_EVTID_TEST_KEEPALIVE);//CheckPoint

    if( pEvtSuberPriv->KeepAliveNextSeqID == pEvtSuberPriv->KeepAliveTotalCnt )
    {
        sem_post(pEvtSuberPriv->pSemAllProced);
    }
    return TOS_RESULT_SUCCESS;
}

//TOS_EVTID_TEST_KEEPALIVE: 1xEvtPuber, 1xEvtSuber, 1024xPostEvtSRT
Test(UT_T1_PubSubEvt_Typical, CASE_01_1xEvtPuber_1xEvtSuber_PostEvtSRT,
    .init = _UT_PubSubEvt_Typical_CASE_setupUnitContext, 
    .fini = _UT_PubSubEvt_Typical_CASE_teardownUnitContext)
{
    TOS_Result_T Result = TOS_RESULT_BUG;
    _UT_OperatorContext_pT pPubSubCtx = &_mPubSubCtx;

    TOS_EvtOperID_T EvtPuber = pPubSubCtx->EvtOperID_ModObjUT_A[0];
    TOS_EvtID_T PubEvtIDs[] = {TOS_EVTID_TEST_KEEPALIVE};
    Result = PLT_EVT_pubEvts(EvtPuber, PubEvtIDs, TOS_calcArrayElmtCnt(PubEvtIDs));
    cr_assert_eq(Result, TOS_RESULT_SUCCESS);//CheckPoint

    TOS_EvtOperID_T EvtSuber = pPubSubCtx->EvtOperID_ModObjUT_B[0];
    TOS_EvtID_T SubEvtIDs[] = {TOS_EVTID_TEST_KEEPALIVE};
    _UT_EvtSuberPrivT01_T EvtSuberPriv = { .EvtSuberID = EvtSuber, .KeepAliveTotalCnt = 1024, .KeepAliveNextSeqID = 0 };
    EvtSuberPriv.pSemAllProced = sem_open("Sem4EvtProcCompletion", O_CREAT, 0644, 0);
    cr_assert_neq(EvtSuberPriv.pSemAllProced, NULL);
    
    TOS_EvtSubArgs_T EvtSubArgs = { .CbProcEvtSRT_F = __UT_ProcEvtSRT_Typical_01, .ToObjPriv = &EvtSuberPriv };
    Result = PLT_EVT_subEvts(EvtSuber, SubEvtIDs, TOS_calcArrayElmtCnt(SubEvtIDs), &EvtSubArgs);
    cr_assert_eq(Result, TOS_RESULT_SUCCESS);//CheckPoint

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //expect_function_calls(__UT_ProcEvtSRT_Typical_01, EvtSuberPriv.KeepAliveTotalCnt);//CheckPoint

    //-----------------------------------------------------------------------------------------------------------------
    Result = PLT_EVT_enableEvtManger();
    cr_assert_eq(Result, TOS_RESULT_SUCCESS);//CheckPoint

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    for( int EvtCnt=0; EvtCnt<EvtSuberPriv.KeepAliveTotalCnt; EvtCnt++ )
    {
        TOS_EVT_defineEvtDesc(MyTestKeepAliveEvt,TOS_EVTID_TEST_KEEPALIVE);

        Result = PLT_EVT_postEvtSRT(EvtPuber, &MyTestKeepAliveEvt);
        cr_assert_eq(Result, TOS_RESULT_SUCCESS);//CheckPoint

        usleep(1000);
    }

    //Wait for all EvtSuberPriv.KeepAliveTotalCnt of MyTestKeepAliveEvt to be processed
    sem_wait(EvtSuberPriv.pSemAllProced);
    cr_assert_eq(EvtSuberPriv.KeepAliveNextSeqID, EvtSuberPriv.KeepAliveTotalCnt);

    //-----------------------------------------------------------------------------------------------------------------
    PLT_EVT_disableEvtManger();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    PLT_EVT_unpubEvts(EvtPuber);
    PLT_EVT_unsubEvts(EvtSuber);
}


void UTG_T1_PubSubEvt_Typical_setupGroupContext(void)
{
    TOS_EvtModuleArgs_T EvtModArgs = 
    { 
        .Params = 
        { 
            .MayRegOperNumMax = _UT_OPERATOR_COUNT,
            .MayPubEvtNumMax  = TOS_EVTPARAM_DEFAULT,
        },
    };

    TOS_Result_T Result = PLT_EVT_initEvtManger(&EvtModArgs);
    cr_assert_eq(Result, TOS_RESULT_SUCCESS);

    return;
}

void UTG_T1_PubSubEvt_Typical_teardownGroupContext(void)
{
    PLT_EVT_deinitEvtManger();
    return;
}

TestSuite(UT_T1_PubSubEvt_Typical, 
    .init = UTG_T1_PubSubEvt_Typical_setupGroupContext, 
    .fini = UTG_T1_PubSubEvt_Typical_teardownGroupContext);
