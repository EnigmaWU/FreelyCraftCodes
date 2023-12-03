#include "../PlatIF_EVT_UT_Common/_UT_EVT_Common.h"

/**
 * @brief Typical PubSubEvt Case Lists
 * UT_T1_PubSubEvt_Typical_NN
 *  =[01]: 
 *      UT_A: PubEvt/PostEvtSRT of TOS_EVTID_TEST_KEEPALIVE * 1024
 *      UT_B/_C/_D: SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_KEEPALIVE
 *  =[02]: 
 *      UT_A: PubEvt/PostEvtSRT of TOS_EVTID_TEST_KEEPALIVE * 1024 + TOS_EVTID_TEST_MSGDATA * 1024 alternating
 *      UT_B: SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_KEEPALIVE
 *      UT_C: SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_MSGDATA
 *      UT_D: SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_KEEPALIVE/_MSGDATA
 *  TODO:=[03]: PubEvt/SubEvt, PostEvtSRT/CbProcEvtSRT of TOS_EVTID_TEST_ECHO_REQUEST/_ECHO_RESPONSE
 */

#define _UT_OPERATOR_COUNT      4//UT_A/_B/_C/_D

 #define _UT_KEEPALIVE_EVT_CNT 1024
// #define _UT_MSGDATA_EVT_CNT   1024

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 typedef struct 
 {
    TOS_EvtOperID_T EvtSuberID; 
    unsigned long KeepAliveTotalCnt, KeepAliveNextSeqID;

    sem_t *pSemAllProced;
 } _UT_EvtSuberPrivCase01_T, *_UT_EvtSuberPrivCase01_pT;      

static TOS_Result_T __UT_CbProcEvtSRT_Case01
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToEvtSuberPriv)
{
    _UT_EvtSuberPrivCase01_pT pEvtSuberPriv = (_UT_EvtSuberPrivCase01_pT)pToEvtSuberPriv;
    EXPECT_EQ(EvtSuberID, pEvtSuberPriv->EvtSuberID);//CheckPoint

    EXPECT_EQ(pEvtDesc->SeqID, pEvtSuberPriv->KeepAliveNextSeqID);//CheckPoint
    pEvtSuberPriv->KeepAliveNextSeqID++;

    TOS_EvtID_T EvtID = pEvtDesc->EvtID;
    EXPECT_EQ(EvtID, TOS_EVTID_TEST_KEEPALIVE);//CheckPoint

    if( pEvtSuberPriv->KeepAliveNextSeqID == pEvtSuberPriv->KeepAliveTotalCnt )
    {
        sem_post(pEvtSuberPriv->pSemAllProced);
    }
    return TOS_RESULT_SUCCESS;
}

//UT_T1_PubSubEvt_Typical_NN=[01] defined in Typical PubSubEvt Case Lists
TEST(UT_T1_PubSubEvt_Typical, Case01)
{
    TOS_Result_T Result = TOS_RESULT_BUG;

    //===> EvtPuberA
    TOS_EvtOperID_T EvtPuberA      = TOS_EVTOPERID_INVALID;
    TOS_EvtOperArgs_T EvtPuberArgs = { .ModObjID = TOS_MODOBJID_UT_A };
    Result = PLT_EVT_regOper(&EvtPuberA, &EvtPuberArgs);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    TOS_EvtID_T PubEvtIDs[] = {TOS_EVTID_TEST_KEEPALIVE};
    Result = PLT_EVT_pubEvts(EvtPuberA, PubEvtIDs, TOS_calcArrayElmtCnt(PubEvtIDs));
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    //==> EvtSuberB
    TOS_EvtOperID_T EvtSuberB = TOS_EVTOPERID_INVALID;
    TOS_EvtOperArgs_T EvtSuberArgs = { .ModObjID = TOS_MODOBJID_UT_B };
    Result = PLT_EVT_regOper(&EvtSuberB, &EvtSuberArgs);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    TOS_EvtID_T SubEvtIDs[] = {TOS_EVTID_TEST_KEEPALIVE};
    _UT_EvtSuberPrivCase01_T EvtSuberPrivB = { .EvtSuberID = EvtSuberB, .KeepAliveTotalCnt = _UT_KEEPALIVE_EVT_CNT, .KeepAliveNextSeqID = 0 };
    EvtSuberPrivB.pSemAllProced = sem_open("Sem4EvtProcCmpltB", O_CREAT, 0644, 0);
    ASSERT_NE(EvtSuberPrivB.pSemAllProced, nullptr);
    
    TOS_EvtSubArgs_T EvtSubArgs = { .CbProcEvtSRT_F = __UT_CbProcEvtSRT_Case01, };
    EvtSubArgs.ToObjPriv = &EvtSuberPrivB;

    Result = PLT_EVT_subEvts(EvtSuberB, SubEvtIDs, TOS_calcArrayElmtCnt(SubEvtIDs), &EvtSubArgs);
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    //===> EvtSuberC
    TOS_EvtOperID_T EvtSuberC = TOS_EVTOPERID_INVALID;
    EvtSuberArgs.ModObjID = TOS_MODOBJID_UT_C;
    Result = PLT_EVT_regOper(&EvtSuberC, &EvtSuberArgs);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    _UT_EvtSuberPrivCase01_T EvtSuberPrivC = { .EvtSuberID = EvtSuberC, .KeepAliveTotalCnt = _UT_KEEPALIVE_EVT_CNT, .KeepAliveNextSeqID = 0 };
    EvtSuberPrivC.pSemAllProced = sem_open("Sem4EvtProcCmpltC", O_CREAT, 0644, 0);
    ASSERT_NE(EvtSuberPrivC.pSemAllProced, nullptr);

    EvtSubArgs.ToObjPriv = &EvtSuberPrivC;

    Result = PLT_EVT_subEvts(EvtSuberC, SubEvtIDs, TOS_calcArrayElmtCnt(SubEvtIDs), &EvtSubArgs);
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    //===> EvtSuberD
    TOS_EvtOperID_T EvtSuberD = TOS_EVTOPERID_INVALID;
    EvtSuberArgs.ModObjID = TOS_MODOBJID_UT_D;
    Result = PLT_EVT_regOper(&EvtSuberD, &EvtSuberArgs);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    _UT_EvtSuberPrivCase01_T EvtSuberPrivD = { .EvtSuberID = EvtSuberD, .KeepAliveTotalCnt = _UT_KEEPALIVE_EVT_CNT, .KeepAliveNextSeqID = 0 };
    EvtSuberPrivD.pSemAllProced = sem_open("Sem4EvtProcCmpltD", O_CREAT, 0644, 0);
    ASSERT_NE(EvtSuberPrivD.pSemAllProced, nullptr);

    EvtSubArgs.ToObjPriv = &EvtSuberPrivD;
    
    Result = PLT_EVT_subEvts(EvtSuberD, SubEvtIDs, TOS_calcArrayElmtCnt(SubEvtIDs), &EvtSubArgs);
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    //-----------------------------------------------------------------------------------------------------------------
    Result = PLT_EVT_enableEvtManger();
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    for( int EvtCnt=0; EvtCnt<EvtSuberPrivB.KeepAliveTotalCnt; EvtCnt++ )
    {
        TOS_EVT_defineEvtDesc(MyTestKeepAliveEvt,TOS_EVTID_TEST_KEEPALIVE);

        Result = PLT_EVT_postEvtSRT(EvtPuberA, &MyTestKeepAliveEvt);
        EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

        usleep(1000);
    }

    //Wait for all EvtSuberPriv.KeepAliveTotalCnt of MyTestKeepAliveEvt to be processed
    sem_wait(EvtSuberPrivB.pSemAllProced);
    EXPECT_EQ(EvtSuberPrivB.KeepAliveNextSeqID, _UT_KEEPALIVE_EVT_CNT);

    sem_wait(EvtSuberPrivC.pSemAllProced);
    EXPECT_EQ(EvtSuberPrivC.KeepAliveNextSeqID, _UT_KEEPALIVE_EVT_CNT);

    sem_wait(EvtSuberPrivD.pSemAllProced);
    EXPECT_EQ(EvtSuberPrivD.KeepAliveNextSeqID, _UT_KEEPALIVE_EVT_CNT);

    //-----------------------------------------------------------------------------------------------------------------
    PLT_EVT_disableEvtManger();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    PLT_EVT_unpubEvts(EvtPuberA);
    PLT_EVT_unsubEvts(EvtSuberB);
    PLT_EVT_unsubEvts(EvtSuberC);
    PLT_EVT_unsubEvts(EvtSuberD);

    PLT_EVT_unregOper(EvtPuberA);
    PLT_EVT_unregOper(EvtSuberB);
    PLT_EVT_unregOper(EvtSuberC);
    PLT_EVT_unregOper(EvtSuberD);
}

//UT_T1_PubSubEvt_Typical_NN=[02] defined in Typical PubSubEvt Case Lists
TEST(UT_T1_PubSubEvt_Typical, Case02)
{
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
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);

    return;
}

void UTG_T1_PubSubEvt_Typical_teardownGroupContext(void)
{
    PLT_EVT_deinitEvtManger();
    return;
}


int main(int argc, char *argv[])
{
    testing::InitGoogleTest();

    UTG_T1_PubSubEvt_Typical_setupGroupContext();
    int RetRAT = RUN_ALL_TESTS();
    UTG_T1_PubSubEvt_Typical_teardownGroupContext();

    return RetRAT;
}