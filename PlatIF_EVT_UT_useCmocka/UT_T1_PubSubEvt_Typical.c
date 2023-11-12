#include "_UT_EVT_Common.h"

/**
 * @brief Typical PubSubEvt Case Lists
 * UT_T1_PubSubEvt_Typical_NN
 *  =[01]: PubEvt/SubEvt, PostEvtSRT/CbProcEvtSRT of TOS_EVTID_TEST_KEEPALIVE
 *  =[02]: PubEvt/SubEvt, PostEvtSRT/CbProcEvtSRT of TOS_EVTID_TEST_MSGDATA
 *  =[03]: PubEvt/SubEvt, PostEvtSRT/CbProcEvtSRT of TOS_EVTID_TEST_ECHO_REQUEST/_ECHO_RESPONSE
 */

 typedef struct 
 {
    TOS_EvtOperID_T EvtPuberID;
    TOS_EvtOperID_T EvtSuberID;
 } _UT_OperatorContext_T, *_UT_OperatorContext_pT;

 typedef struct 
 {
    TOS_EvtOperID_T EvtSuberID;
    unsigned long KeepAliveTotalCnt, KeepAliveNextSeqID;
 } _UT_EvtSuberPriv_T, *_UT_EvtSuberPriv_pT;

 //EvtSuber's CbProcEvtSRT_F in TOS_EvtSubArgs_T used by all EvbSuber in UT
static TOS_Result_T __EvtSuberCbProcEvtSRT
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToObjPriv)
{
    _UT_EvtSuberPriv_pT pEvtSuberPriv = (_UT_EvtSuberPriv_pT)pToObjPriv;
    assert_int_equal(EvtSuberID, pEvtSuberPriv->EvtSuberID);//CheckPoint

    assert_int_equal(pEvtDesc->SeqID, pEvtSuberPriv->KeepAliveNextSeqID);//CheckPoint
    pEvtSuberPriv->KeepAliveNextSeqID++;

    TOS_EvtID_T EvtID = pEvtDesc->EvtID;
    const TOS_EvtID_T ExpectEvtIDSets[] = {TOS_EVTID_TEST_KEEPALIVE, TOS_EVTID_TEST_MSGDATA, TOS_EVTID_TEST_ECHO_REQUEST, };
    assert_in_set(EvtID, (const unsigned long*)&ExpectEvtIDSets[0], sizeof(ExpectEvtIDSets)/sizeof(ExpectEvtIDSets[0]));//CheckPoint


    return TOS_RESULT_SUCCESS;
}

void UT_T1_PubSubEvt_Typical_01(void **state)
{
    _UT_OperatorContext_pT pPubSubCtx = (_UT_OperatorContext_pT)(*state);

    TOS_EvtID_T PubEvtIDs[] = {TOS_EVTID_TEST_KEEPALIVE};
    TOS_Result_T Result = PLT_EVT_pubEvts(pPubSubCtx->EvtPuberID, PubEvtIDs, sizeof(PubEvtIDs)/sizeof(PubEvtIDs[0]));
    assert_int_equal(Result, TOS_RESULT_SUCCESS);

    TOS_EvtID_T SubEvtIDs[] = {TOS_EVTID_TEST_KEEPALIVE};
    _UT_EvtSuberPriv_T EvtSuberPriv = { .EvtSuberID = pPubSubCtx->EvtSuberID, .KeepAliveTotalCnt = 1024 };
    TOS_EvtSubArgs_T EvtSubArgs = { .CbProcEvtSRT_F = __EvtSuberCbProcEvtSRT, .ToObjPriv = &EvtSuberPriv };
    Result = PLT_EVT_subEvts(pPubSubCtx->EvtSuberID, SubEvtIDs, sizeof(SubEvtIDs)/sizeof(SubEvtIDs[0]), &EvtSubArgs);
    assert_int_equal(Result, TOS_RESULT_SUCCESS);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    expect_function_calls(__EvtSuberCbProcEvtSRT, EvtSuberPriv.KeepAliveTotalCnt);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    for( int EvtCnt=0; EvtCnt<EvtSuberPriv.KeepAliveTotalCnt; EvtCnt++ )
    {
        TOS_EvtDesc_T EvtTestKeepalive = { .EvtID = TOS_EVTID_TEST_KEEPALIVE, .ToModObjID = TOS_MODOBJID_EVTSUBERS, };
        Result = PLT_EVT_postEvtSRT(pPubSubCtx->EvtPuberID, &EvtTestKeepalive);
        assert_int_equal(Result, TOS_RESULT_SUCCESS);
    }

}

int UTG_T1_PubSubEvt_Typical_setupContext(void **state)
{
    static _UT_OperatorContext_T _mPubSubCtx = { };

    TOS_EvtOperArgs_T EvtPuberArgs = { .ModObjID = TOS_MODOBJID_UT_A, };
    TOS_Result_T Result = PLT_EVT_registerOperator(&_mPubSubCtx.EvtPuberID, &EvtPuberArgs);
    assert_int_equal(Result, TOS_RESULT_SUCCESS);

    TOS_EvtOperArgs_T EvtSuberArgs = { .ModObjID = TOS_MODOBJID_UT_B, };
    Result = PLT_EVT_registerOperator(&_mPubSubCtx.EvtSuberID, &EvtSuberArgs);
    assert_int_equal(Result, TOS_RESULT_SUCCESS);
    
    
    *state = &_mPubSubCtx;
    return 0;
}

int UTG_T1_PubSubEvt_Typical_teardownContext(void **state)
{
    _UT_OperatorContext_pT pPubSubCtx = (_UT_OperatorContext_pT)(*state);

    PLT_EVT_unregisterOperator(pPubSubCtx->EvtPuberID);
    PLT_EVT_unregisterOperator(pPubSubCtx->EvtSuberID);

    return 0;
}

int main(int argc, char *argv[])
{
    struct CMUnitTest UTG_T1_PubSubEvt_Typical[] = {
        cmocka_unit_test(UT_T1_PubSubEvt_Typical_01),
    };

    return cmocka_run_group_tests(
        UTG_T1_PubSubEvt_Typical, 
        UTG_T1_PubSubEvt_Typical_setupContext, 
        UTG_T1_PubSubEvt_Typical_teardownContext);
}
