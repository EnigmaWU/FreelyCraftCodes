#include "_UT_EVT_Common.h"
#include <stdint.h>

/**
 * @brief Typical PubSubEvt Case Lists
 * UT_T1_PubSubEvt_Typical_NN
 *  =[01]: PubEvt/SubEvt, PostEvtSRT/CbProcEvtSRT of TOS_EVTID_TEST_KEEPALIVE
 *  =[02]: PubEvt/SubEvt, PostEvtSRT/CbProcEvtSRT of TOS_EVTID_TEST_MSGDATA
 *  =[03]: PubEvt/SubEvt, PostEvtSRT/CbProcEvtSRT of TOS_EVTID_TEST_ECHO_REQUEST/_ECHO_RESPONSE
 */

#define _UT_OPERATOR_COUNT 12
 typedef struct 
 {
    TOS_EvtOperID_T EvtOperID_ModObjUT_A[_UT_OPERATOR_COUNT/3];
    TOS_EvtOperID_T EvtOperID_ModObjUT_B[_UT_OPERATOR_COUNT/3];
    TOS_EvtOperID_T EvtOperID_ModObjUT_C[_UT_OPERATOR_COUNT/3];
 } _UT_OperatorContext_T, *_UT_OperatorContext_pT;

 typedef struct 
 {
    TOS_EvtOperID_T EvtSuberID;
    unsigned long KeepAliveTotalCnt, KeepAliveNextSeqID;
 } _UT_EvtSuberPrivT01_T, *_UT_EvtSuberPrivT01_pT;

static TOS_Result_T __UT_ProcEvtSRT_Typical_01
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToObjPriv)
{
    _UT_EvtSuberPrivT01_pT pEvtSuberPriv = (_UT_EvtSuberPrivT01_pT)pToObjPriv;
    assert_int_equal(EvtSuberID, pEvtSuberPriv->EvtSuberID);//CheckPoint

    assert_int_equal(pEvtDesc->SeqID, pEvtSuberPriv->KeepAliveNextSeqID);//CheckPoint
    pEvtSuberPriv->KeepAliveNextSeqID++;

    TOS_EvtID_T EvtID = pEvtDesc->EvtID;
    assert_int_equal(EvtID, TOS_EVTID_TEST_KEEPALIVE);//CheckPoint

    return TOS_RESULT_SUCCESS;
}

//TOS_EVTID_TEST_KEEPALIVE: 1xEvtPuber, 1xEvtSuber, 1024xPostEvtSRT
void UT_T1_PubSubEvt_Typical_01_1xEvtPuber_1xEvtSuber_1024xPostEvtSRT(void **state)
{
    TOS_Result_T Result = TOS_RESULT_BUG;
    _UT_OperatorContext_pT pPubSubCtx = (_UT_OperatorContext_pT)(*state);

    TOS_EvtOperID_T EvtPuberID = pPubSubCtx->EvtOperID_ModObjUT_A[0];
    TOS_EvtID_T PubEvtIDs[] = {TOS_EVTID_TEST_KEEPALIVE};
    Result = PLT_EVT_pubEvts(EvtPuberID, PubEvtIDs, TOS_calcArrayElmtCnt(PubEvtIDs));
    assert_int_equal(Result, TOS_RESULT_SUCCESS);


    TOS_EvtOperID_T EvtSuberID = pPubSubCtx->EvtOperID_ModObjUT_B[0];
    TOS_EvtID_T SubEvtIDs[] = {TOS_EVTID_TEST_KEEPALIVE};
    _UT_EvtSuberPrivT01_T EvtSuberPriv = { .EvtSuberID = EvtSuberID, .KeepAliveTotalCnt = 1024, .KeepAliveNextSeqID = 0 };
    TOS_EvtSubArgs_T EvtSubArgs = { .CbProcEvtSRT_F = __UT_ProcEvtSRT_Typical_01, .ToObjPriv = &EvtSuberPriv };
    Result = PLT_EVT_subEvts(EvtSuberID, SubEvtIDs, TOS_calcArrayElmtCnt(SubEvtIDs), &EvtSubArgs);
    assert_int_equal(Result, TOS_RESULT_SUCCESS);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    expect_function_calls(__UT_ProcEvtSRT_Typical_01, EvtSuberPriv.KeepAliveTotalCnt);

    //-----------------------------------------------------------------------------------------------------------------
    PLT_EVT_enableModule();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    for( int EvtCnt=0; EvtCnt<EvtSuberPriv.KeepAliveTotalCnt; EvtCnt++ )
    {
        TOS_EvtDesc_T EvtTestKeepAlive = { .EvtID = TOS_EVTID_TEST_KEEPALIVE, .ToModObjID = TOS_MODOBJID_EVTSUBERS, };
        Result = PLT_EVT_postEvtSRT(EvtPuberID, &EvtTestKeepAlive);
        assert_int_equal(Result, TOS_RESULT_SUCCESS);
    }

    //-----------------------------------------------------------------------------------------------------------------
    PLT_EVT_disableModule();

}

//TOS_EVTID_TEST_KEEPALIVE: 1xEvtPuber, 1xEvtSuber, 1024xPostEvtSRT
void UT_T1_PubSubEvt_Typical_01_1xEvtPuber_3xEvtSuber_1024xPostEvtSRT(void **state)
{
    //TODO(@W)
}

 typedef struct 
 {
    TOS_EvtOperID_T EvtSuberID;
    uint32_t MsgDataTotalCnt, MsgDataNextSeqID;
 } _UT_EvtSuberPrivT02_T, *_UT_EvtSuberPrivT02_pT;

static TOS_Result_T __UT_ProcEvtSRT_Typical_02
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToObjPriv)
{
    _UT_EvtSuberPrivT02_pT pEvtSuberPriv = (_UT_EvtSuberPrivT02_pT)pToObjPriv;
    assert_int_equal(EvtSuberID, pEvtSuberPriv->EvtSuberID);//CheckPoint

    assert_int_equal(pEvtDesc->SeqID, pEvtSuberPriv->MsgDataNextSeqID);//CheckPoint
    pEvtSuberPriv->MsgDataNextSeqID++;

    TOS_EvtID_T EvtID = pEvtDesc->EvtID;
    assert_int_equal(EvtID, TOS_EVTID_TEST_MSGDATA);//CheckPoint

    for( int Idx=0; Idx<TOS_calcArrayElmtCnt(pEvtDesc->EvtData.U32); Idx++ )
    {
        assert_int_equal(pEvtDesc->EvtData.U32[Idx], pEvtDesc->SeqID + Idx);//CheckPoint
    }

    return TOS_RESULT_SUCCESS;
}

//TOS_EVTID_TEST_MSGDATA: 1xEvtPuber, 1xEvtSuber, 1024xPostEvtSRT(with MsgData)
void UT_T1_PubSubEvt_Typical_02_1xEvtPuber_1xEvtSuber_1024xPostEvtSRT(void **state)
{
    _UT_OperatorContext_pT pPubSubCtx = (_UT_OperatorContext_pT)(*state);

    TOS_EvtOperID_T EvtPuberID = pPubSubCtx->EvtOperID_ModObjUT_A[0];
    TOS_EvtID_T PubEvtIDs[] = {TOS_EVTID_TEST_MSGDATA};
    TOS_Result_T Result = PLT_EVT_pubEvts(EvtPuberID, PubEvtIDs, TOS_calcArrayElmtCnt(PubEvtIDs));
    assert_int_equal(Result, TOS_RESULT_SUCCESS);//CheckPoint

    TOS_EvtOperID_T EvtSuberID = pPubSubCtx->EvtOperID_ModObjUT_B[0];
    TOS_EvtID_T SubEvtIDs[] = {TOS_EVTID_TEST_MSGDATA};
    _UT_EvtSuberPrivT02_T EvtSuberPriv = { .EvtSuberID = EvtSuberID, .MsgDataTotalCnt = 1024, .MsgDataNextSeqID = 0 };
    TOS_EvtSubArgs_T EvtSubArgs = { .CbProcEvtSRT_F = __UT_ProcEvtSRT_Typical_02, .ToObjPriv = &EvtSuberPriv };
    Result = PLT_EVT_subEvts(EvtSuberID, SubEvtIDs, TOS_calcArrayElmtCnt(SubEvtIDs), &EvtSubArgs);
    assert_int_equal(Result, TOS_RESULT_SUCCESS);//CheckPoint

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    expect_function_calls(__UT_ProcEvtSRT_Typical_02, EvtSuberPriv.MsgDataTotalCnt);

    //-----------------------------------------------------------------------------------------------------------------
    PLT_EVT_enableModule();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    for( uint32_t EvtCnt=0; EvtCnt<EvtSuberPriv.MsgDataTotalCnt; EvtCnt++ )
    {
        TOS_EvtDesc_T EvtTestMsgData = { .EvtID = TOS_EVTID_TEST_MSGDATA, .ToModObjID = TOS_MODOBJID_EVTSUBERS, };

        for( int Idx=0; Idx<TOS_calcArrayElmtCnt(EvtTestMsgData.EvtData.U32); Idx++ )
        {
            EvtTestMsgData.EvtData.U32[Idx] = EvtCnt;
        }

        Result = PLT_EVT_postEvtSRT(EvtPuberID, &EvtTestMsgData);
        assert_int_equal(Result, TOS_RESULT_SUCCESS);//CheckPoint
    }

    //-----------------------------------------------------------------------------------------------------------------
    PLT_EVT_disableModule();
}

//TOS_EVTID_TEST_MSGDATA: 1xEvtPuber, 3xEvtSuber, 1024xPostEvtSRT(with MsgData)
void UT_T1_PubSubEvt_Typical_02_1xEvtPuber_3xEvtSuber_1024xPostEvtSRT(void **state)
{
    //TODO(@W)
}

typedef struct 
{
    TOS_EvtOperID_T EvtSuberID;

    TOS_EvtOperID_T EvtPuberB;
    union
    {
        struct {uint32_t EchoRequestTotalCnt, EchoRequestNextSeqID;};//EvtSuberB
        struct {uint32_t EchoResponseTotalCnt, EchoResponseNextSeqID;};//EvtSuberC
    };
} _UT_EvtSuberPrivT03_T, *_UT_EvtSuberPrivT03_pT;

static TOS_Result_T __UT_ProcEvtSRT_Typical_03_B
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToObjPriv)
{
    _UT_EvtSuberPrivT03_pT pEvtSuberPriv = (_UT_EvtSuberPrivT03_pT)pToObjPriv;
    assert_int_equal(EvtSuberID, pEvtSuberPriv->EvtSuberID);//CheckPoint

    assert_int_equal(pEvtDesc->SeqID, pEvtSuberPriv->EchoRequestNextSeqID);//CheckPoint
    pEvtSuberPriv->EchoRequestNextSeqID++;

    TOS_EvtID_T EvtID = pEvtDesc->EvtID;
    assert_int_equal(EvtID, TOS_EVTID_TEST_ECHO_REQUEST);//CheckPoint

    TOS_EvtDesc_T EvtTestEchoResponse = { .EvtID = TOS_EVTID_TEST_ECHO_RESPONSE, .ToModObjID = TOS_MODOBJID_EVTSUBERS, };
    EvtTestEchoResponse.SeqID = pEvtDesc->SeqID;
    TOS_Result_T Result = PLT_EVT_postEvtSRT(pEvtSuberPriv->EvtPuberB, &EvtTestEchoResponse);
    assert_int_equal(Result, TOS_RESULT_SUCCESS);//CheckPoint

    return TOS_RESULT_SUCCESS;
}

static TOS_Result_T __UT_ProcEvtSRT_Typical_03_C
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToObjPriv)
{
    _UT_EvtSuberPrivT03_pT pEvtSuberPriv = (_UT_EvtSuberPrivT03_pT)pToObjPriv;
    assert_int_equal(EvtSuberID, pEvtSuberPriv->EvtSuberID);//CheckPoint

    assert_int_equal(pEvtDesc->SeqID, pEvtSuberPriv->EchoResponseNextSeqID);//CheckPoint
    pEvtSuberPriv->EchoResponseNextSeqID++;

    TOS_EvtID_T EvtID = pEvtDesc->EvtID;
    assert_int_equal(EvtID, TOS_EVTID_TEST_ECHO_RESPONSE);//CheckPoint

    return TOS_RESULT_SUCCESS;
}

//TOS_EVTID_TEST_ECHO_REQUEST/_ECHO_RESPONSE: 
//  1xA::EvtPuber::1024xPostEvtSRT::ECHO_REQUEST
//      1xB::(EvtSuber::ECHO_REQUEST -> EvtPuber::PostEvtSRT::ECHO_RESPONSE)
//          1xC::EvtSub::ECHO_RESPONSE
void UT_T1_PubSubEvt_Typical_03_1xEvtPuber_1xEvtSuber_1024xPostEvtSRT(void **state)
{
    TOS_Result_T Result = TOS_RESULT_BUG;
    _UT_OperatorContext_pT pPubSubCtx = (_UT_OperatorContext_pT)(*state);

    TOS_EvtOperID_T EvtPuberA = pPubSubCtx->EvtOperID_ModObjUT_A[0];
    TOS_EvtOperID_T EvtSuberB = pPubSubCtx->EvtOperID_ModObjUT_B[0];
    TOS_EvtOperID_T EvtPuberB = pPubSubCtx->EvtOperID_ModObjUT_B[1];
    TOS_EvtOperID_T EvtSuberC = pPubSubCtx->EvtOperID_ModObjUT_C[0];

    TOS_EvtID_T EvtIDsEchoRequest[] = {TOS_EVTID_TEST_ECHO_REQUEST};
    Result = PLT_EVT_pubEvts(EvtPuberA, EvtIDsEchoRequest, TOS_calcArrayElmtCnt(EvtIDsEchoRequest));
    assert_int_equal(Result, TOS_RESULT_SUCCESS);//CheckPoint

    _UT_EvtSuberPrivT03_T EvtSuberPrivB = { .EvtSuberID = EvtSuberB, .EchoRequestTotalCnt = 1024, .EchoRequestNextSeqID = 0, .EvtPuberB = EvtPuberB };
    TOS_EvtSubArgs_T EvtSubArgsB = { .CbProcEvtSRT_F = __UT_ProcEvtSRT_Typical_03_B, .ToObjPriv = &EvtSuberPrivB };
    Result = PLT_EVT_subEvts(EvtSuberB, EvtIDsEchoRequest, TOS_calcArrayElmtCnt(EvtIDsEchoRequest), &EvtSubArgsB);
    assert_int_equal(Result, TOS_RESULT_SUCCESS);//CheckPoint

    TOS_EvtID_T EvtIDs_EchoResponse[] = {TOS_EVTID_TEST_ECHO_RESPONSE};
    Result = PLT_EVT_pubEvts(EvtPuberB, EvtIDs_EchoResponse, TOS_calcArrayElmtCnt(EvtIDs_EchoResponse));
    assert_int_equal(Result, TOS_RESULT_SUCCESS);//CheckPoint

    _UT_EvtSuberPrivT03_T EvtSuberPrivC = { .EvtSuberID = EvtSuberC, .EchoResponseTotalCnt = 1024, .EchoResponseNextSeqID = 0 };
    TOS_EvtSubArgs_T EvtSubArgsC = { .CbProcEvtSRT_F = __UT_ProcEvtSRT_Typical_03_C, .ToObjPriv = &EvtSuberPrivC };
    Result = PLT_EVT_subEvts(EvtSuberC, EvtIDs_EchoResponse, TOS_calcArrayElmtCnt(EvtIDs_EchoResponse), &EvtSubArgsC);
    assert_int_equal(Result, TOS_RESULT_SUCCESS);//CheckPoint

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    expect_function_calls(__UT_ProcEvtSRT_Typical_03_B, EvtSuberPrivB.EchoRequestTotalCnt);
    expect_function_calls(__UT_ProcEvtSRT_Typical_03_C, EvtSuberPrivC.EchoResponseTotalCnt);

    //-----------------------------------------------------------------------------------------------------------------
    PLT_EVT_enableModule();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    for( uint32_t EvtCnt=0; EvtCnt<EvtSuberPrivB.EchoRequestTotalCnt; EvtCnt++ )
    {
        TOS_EvtDesc_T EvtTestEchoRequest = { .EvtID = TOS_EVTID_TEST_ECHO_REQUEST, .ToModObjID = TOS_MODOBJID_EVTSUBERS, };
        EvtTestEchoRequest.SeqID = EvtCnt;
        Result = PLT_EVT_postEvtSRT(EvtPuberA, &EvtTestEchoRequest);
        assert_int_equal(Result, TOS_RESULT_SUCCESS);//CheckPoint
    }

    //-----------------------------------------------------------------------------------------------------------------
    PLT_EVT_disableModule();
    
}

//TOS_EVTID_TEST_ECHO_REQUEST/_ECHO_RESPONSE: ...
void UT_T1_PubSubEvt_Typical_03_1xEvtPuber_3xEvtSuber_1024xPostEvtSRT(void **state)
{
    //TODO(@W)
}

int UT_T1_PubSubEvt_Typical_setupUnitContext(void **state)
{
    static _UT_OperatorContext_T _mPubSubCtx = { };

    TOS_EvtOperArgs_T EvtPuberArgs = { };

    for( uint32_t OperCnt=0; OperCnt<TOS_calcArrayElmtCnt(_mPubSubCtx.EvtOperID_ModObjUT_A); OperCnt++ )
    {
        EvtPuberArgs.ModObjID = TOS_MODOBJID_UT_A;
        TOS_Result_T Result = PLT_EVT_registerOperator(&_mPubSubCtx.EvtOperID_ModObjUT_A[OperCnt], &EvtPuberArgs);
        assert_int_equal(Result, TOS_RESULT_SUCCESS);
    }

    for( uint32_t OperCnt=0; OperCnt<TOS_calcArrayElmtCnt(_mPubSubCtx.EvtOperID_ModObjUT_B); OperCnt++ )
    {
        EvtPuberArgs.ModObjID = TOS_MODOBJID_UT_B;
        TOS_Result_T Result = PLT_EVT_registerOperator(&_mPubSubCtx.EvtOperID_ModObjUT_B[OperCnt], &EvtPuberArgs);
        assert_int_equal(Result, TOS_RESULT_SUCCESS);
    }

    for( uint32_t OperCnt=0; OperCnt<TOS_calcArrayElmtCnt(_mPubSubCtx.EvtOperID_ModObjUT_C); OperCnt++ )
    {
        EvtPuberArgs.ModObjID = TOS_MODOBJID_UT_C;
        TOS_Result_T Result = PLT_EVT_registerOperator(&_mPubSubCtx.EvtOperID_ModObjUT_C[OperCnt], &EvtPuberArgs);
        assert_int_equal(Result, TOS_RESULT_SUCCESS);
    }
    
    *state = &_mPubSubCtx;
    return 0;
}

int UT_T1_PubSubEvt_Typical_teardownUnitContext(void **state)
{
    _UT_OperatorContext_pT pPubSubCtx = (_UT_OperatorContext_pT)(*state);

    for( uint32_t OperCnt=0; OperCnt<TOS_calcArrayElmtCnt(pPubSubCtx->EvtOperID_ModObjUT_A); OperCnt++ )
    {
        PLT_EVT_unregisterOperator(pPubSubCtx->EvtOperID_ModObjUT_A[OperCnt]);
    }
    
    for( uint32_t OperCnt=0; OperCnt<TOS_calcArrayElmtCnt(pPubSubCtx->EvtOperID_ModObjUT_B); OperCnt++ )
    {
        PLT_EVT_unregisterOperator(pPubSubCtx->EvtOperID_ModObjUT_B[OperCnt]);
    }

    for( uint32_t OperCnt=0; OperCnt<TOS_calcArrayElmtCnt(pPubSubCtx->EvtOperID_ModObjUT_C); OperCnt++ )
    {
        PLT_EVT_unregisterOperator(pPubSubCtx->EvtOperID_ModObjUT_C[OperCnt]);
    }

    return 0;
}

int UTG_T1_PubSubEvt_Typical_setupGroupContext(void **state)
{
    TOS_EvtModuleArgs_T EvtModArgs = 
    { 
        .Params = 
        { 
            .EvtQueueDepth = 16, 
            .OperatorCount = _UT_OPERATOR_COUNT,
        },
    };

    TOS_Result_T Result = PLT_EVT_initModule(&EvtModArgs);
    assert_int_equal(Result, TOS_RESULT_SUCCESS);

    return 0;
}

int UTG_T1_PubSubEvt_Typical_teardownGroupContext(void **state)
{
    PLT_EVT_deinitModule();

    return 0;
}

int main(int argc, char *argv[])
{
    struct CMUnitTest UTG_T1_PubSubEvt_Typical[] = {
        cmocka_unit_test_setup_teardown(
                UT_T1_PubSubEvt_Typical_01_1xEvtPuber_1xEvtSuber_1024xPostEvtSRT, 
                UT_T1_PubSubEvt_Typical_setupUnitContext, 
                UT_T1_PubSubEvt_Typical_teardownUnitContext),
        cmocka_unit_test_setup_teardown(
                UT_T1_PubSubEvt_Typical_02_1xEvtPuber_1xEvtSuber_1024xPostEvtSRT, 
                UT_T1_PubSubEvt_Typical_setupUnitContext, 
                UT_T1_PubSubEvt_Typical_teardownUnitContext),
    };

    return cmocka_run_group_tests(
        UTG_T1_PubSubEvt_Typical, 
        UTG_T1_PubSubEvt_Typical_setupGroupContext, 
        UTG_T1_PubSubEvt_Typical_teardownGroupContext);
}
