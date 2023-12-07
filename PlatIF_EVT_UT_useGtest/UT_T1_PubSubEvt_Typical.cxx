#include "../PlatIF_EVT_UT_Common/_UT_EVT_Common.h"
#include <stdint.h>

/**
 * @brief Typical PubSubEvt Case Lists
 * UT_T1_PubSubEvt_Typical_NN
 *  =[Case01]: 
 *      UT_A: PubEvt/PostEvtSRT of TOS_EVTID_TEST_KEEPALIVE * 1024
 *      UT_B/_C/_D: SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_KEEPALIVE
 *  =[Case02]: 
 *      UT_A: PubEvt/PostEvtSRT of TOS_EVTID_TEST_KEEPALIVE * 1024 + TOS_EVTID_TEST_MSGDATA * 1024 alternating
 *      UT_B: SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_KEEPALIVE
 *      UT_C: SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_MSGDATA
 *      UT_D: SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_KEEPALIVE/_MSGDATA
 *  =[Case03]:
 *      UT_A: PubEvt/PostEvtSRT of TOS_EVTID_TEST_ECHO_REQUEST * 1024
 *          SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_ECHO_RESPONSE
 *      UT_B: SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_ECHO_REQUEST
 *          PubEvt/PostEvtSRT of TOS_EVTID_TEST_ECHO_RESPONSE
 *      UT_C/_D: SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_ECHO_RESPONSE
 *  TODO(@W)=[Case04]:...
 */

#define _UT_OPERATOR_COUNT      4//UT_A/_B/_C/_D

#define _UT_KEEPALIVE_EVT_CNT       1024
#define _UT_MSGDATA_EVT_CNT         1024
#define _UT_ECHO_REQUEST_EVT_CNT    1024

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

typedef struct 
{
    TOS_EvtOperID_T EvtSuberID; 
    unsigned long KeepAliveTotalCnt, KeepAliveProcedCnt;

    sem_t *pSemAllProced;
} _UT_EvtSuberPrivCase02_ofB_T, *_UT_EvtSuberPrivCase02_ofB_pT;

static TOS_Result_T __UT_CbProcEvtSRT_Case02_ofB
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToEvtSuberPriv)
{
    _UT_EvtSuberPrivCase02_ofB_pT pEvtSuberPriv = (_UT_EvtSuberPrivCase02_ofB_pT)pToEvtSuberPriv;
    EXPECT_EQ(EvtSuberID, pEvtSuberPriv->EvtSuberID);//CheckPoint

    pEvtSuberPriv->KeepAliveProcedCnt++;

    TOS_EvtID_T EvtID = pEvtDesc->EvtID;
    EXPECT_EQ(EvtID, TOS_EVTID_TEST_KEEPALIVE);//CheckPoint

    if( pEvtSuberPriv->KeepAliveProcedCnt == pEvtSuberPriv->KeepAliveTotalCnt )
    {
        sem_post(pEvtSuberPriv->pSemAllProced);
    }
    return TOS_RESULT_SUCCESS;
}

typedef struct 
{
    TOS_EvtOperID_T EvtSuberID; 
    unsigned long MsgDataTotalCnt, MsgDataProcedCnt;

    uint32_t    MsgDataNextValue;//0,1,2,...,MsgDataTotalCnt-1

    sem_t *pSemAllProced;
} _UT_EvtSuberPrivCase02_ofC_T, *_UT_EvtSuberPrivCase02_ofC_pT;

static TOS_Result_T __UT_CbProcEvtSRT_Case02_ofC
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToEvtSuberPriv)
{
    _UT_EvtSuberPrivCase02_ofC_pT pEvtSuberPriv = (_UT_EvtSuberPrivCase02_ofC_pT)pToEvtSuberPriv;
    EXPECT_EQ(EvtSuberID, pEvtSuberPriv->EvtSuberID);//CheckPoint

    TOS_EvtID_T EvtID = pEvtDesc->EvtID;
    EXPECT_EQ(EvtID, TOS_EVTID_TEST_MSGDATA);//CheckPoint

    EXPECT_EQ(pEvtDesc->EvtData.U32[0], pEvtSuberPriv->MsgDataNextValue);//CheckPoint
    pEvtSuberPriv->MsgDataNextValue++;

    pEvtSuberPriv->MsgDataProcedCnt++;

    if( pEvtSuberPriv->MsgDataProcedCnt == pEvtSuberPriv->MsgDataTotalCnt )
    {
        sem_post(pEvtSuberPriv->pSemAllProced);
    }
    return TOS_RESULT_SUCCESS;
}

typedef struct 
{
    TOS_EvtOperID_T EvtSuberID; 
    unsigned long KeepAliveTotalCnt, KeepAliveProcedCnt;
    unsigned long MsgDataTotalCnt, MsgDataProcedCnt;

    uint32_t    MsgDataNextValue;//0,1,2,...,MsgDataTotalCnt-1

    sem_t *pSemAllProced;
} _UT_EvtSuberPrivCase02_ofD_T, *_UT_EvtSuberPrivCase02_ofD_pT;

static TOS_Result_T __UT_CbProcEvtSRT_Case02_ofD
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToEvtSuberPriv)
{
    _UT_EvtSuberPrivCase02_ofD_pT pEvtSuberPriv = (_UT_EvtSuberPrivCase02_ofD_pT)pToEvtSuberPriv;
    EXPECT_EQ(EvtSuberID, pEvtSuberPriv->EvtSuberID);//CheckPoint

    TOS_EvtID_T EvtID = pEvtDesc->EvtID;
    EXPECT_TRUE( (EvtID == TOS_EVTID_TEST_KEEPALIVE) || (EvtID == TOS_EVTID_TEST_MSGDATA) );//CheckPoint

    if( EvtID == TOS_EVTID_TEST_KEEPALIVE )
    {
        pEvtSuberPriv->KeepAliveProcedCnt++;
    }
    else if( EvtID == TOS_EVTID_TEST_MSGDATA )
    {
        EXPECT_EQ(pEvtDesc->EvtData.U32[0], pEvtSuberPriv->MsgDataNextValue);//CheckPoint
        pEvtSuberPriv->MsgDataNextValue++;

        pEvtSuberPriv->MsgDataProcedCnt++;
    }

    if( (pEvtSuberPriv->KeepAliveProcedCnt == pEvtSuberPriv->KeepAliveTotalCnt) 
        && (pEvtSuberPriv->MsgDataProcedCnt == pEvtSuberPriv->MsgDataTotalCnt) )
    {
        sem_post(pEvtSuberPriv->pSemAllProced);
    }
    return TOS_RESULT_SUCCESS;
}

//UT_T1_PubSubEvt_Typical_NN=[02] defined in Typical PubSubEvt Case Lists
TEST(UT_T1_PubSubEvt_Typical, Case02)
{
    TOS_Result_T Result = TOS_RESULT_BUG;

    //===> EvtPuberA
    TOS_EvtOperID_T EvtPuberA      = TOS_EVTOPERID_INVALID;
    TOS_EvtOperArgs_T EvtPuberArgs = { .ModObjID = TOS_MODOBJID_UT_A };
    Result = PLT_EVT_regOper(&EvtPuberA, &EvtPuberArgs);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    TOS_EvtID_T PubEvtIDs[] = {TOS_EVTID_TEST_KEEPALIVE, TOS_EVTID_TEST_MSGDATA};
    Result = PLT_EVT_pubEvts(EvtPuberA, PubEvtIDs, TOS_calcArrayElmtCnt(PubEvtIDs));
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    //==> EvtSuberB
    TOS_EvtOperID_T EvtSuberB = TOS_EVTOPERID_INVALID;
    TOS_EvtOperArgs_T EvtSuberArgs = { .ModObjID = TOS_MODOBJID_UT_B };
    Result = PLT_EVT_regOper(&EvtSuberB, &EvtSuberArgs);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    _UT_EvtSuberPrivCase02_ofB_T EvtSuberPrivB = { .EvtSuberID = EvtSuberB, .KeepAliveTotalCnt = _UT_KEEPALIVE_EVT_CNT, .KeepAliveProcedCnt = 0 };
    EvtSuberPrivB.pSemAllProced = sem_open("Sem4EvtProcCmpltB", O_CREAT, 0644, 0);
    ASSERT_NE(EvtSuberPrivB.pSemAllProced, nullptr);
    
    TOS_EvtID_T SubEvtIDs_ofB[] = {TOS_EVTID_TEST_KEEPALIVE};
    TOS_EvtSubArgs_T EvtSubArgs = { .CbProcEvtSRT_F = __UT_CbProcEvtSRT_Case02_ofB, };
    EvtSubArgs.ToObjPriv = &EvtSuberPrivB;

    Result = PLT_EVT_subEvts(EvtSuberB, SubEvtIDs_ofB, TOS_calcArrayElmtCnt(SubEvtIDs_ofB), &EvtSubArgs);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    //===> EvtSuberC
    TOS_EvtOperID_T EvtSuberC = TOS_EVTOPERID_INVALID;
    EvtSuberArgs.ModObjID = TOS_MODOBJID_UT_C;
    Result = PLT_EVT_regOper(&EvtSuberC, &EvtSuberArgs);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    _UT_EvtSuberPrivCase02_ofC_T EvtSuberPrivC = { .EvtSuberID = EvtSuberC, .MsgDataTotalCnt = _UT_MSGDATA_EVT_CNT, .MsgDataProcedCnt = 0, .MsgDataNextValue = 0 };
    EvtSuberPrivC.pSemAllProced = sem_open("Sem4EvtProcCmpltC", O_CREAT, 0644, 0);
    ASSERT_NE(EvtSuberPrivC.pSemAllProced, nullptr);

    TOS_EvtID_T SubEvtIDs_ofC[] = {TOS_EVTID_TEST_MSGDATA};
    EvtSubArgs.CbProcEvtSRT_F = __UT_CbProcEvtSRT_Case02_ofC;
    EvtSubArgs.ToObjPriv = &EvtSuberPrivC;

    Result = PLT_EVT_subEvts(EvtSuberC, SubEvtIDs_ofC, TOS_calcArrayElmtCnt(SubEvtIDs_ofC), &EvtSubArgs);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    //===> EvtSuberD
    TOS_EvtOperID_T EvtSuberD = TOS_EVTOPERID_INVALID;
    EvtSuberArgs.ModObjID = TOS_MODOBJID_UT_D;
    Result = PLT_EVT_regOper(&EvtSuberD, &EvtSuberArgs);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    _UT_EvtSuberPrivCase02_ofD_T EvtSuberPrivD = { .EvtSuberID = EvtSuberD, .KeepAliveTotalCnt = _UT_KEEPALIVE_EVT_CNT, .KeepAliveProcedCnt = 0, .MsgDataTotalCnt = _UT_MSGDATA_EVT_CNT, .MsgDataProcedCnt = 0, .MsgDataNextValue = 0 };
    EvtSuberPrivD.pSemAllProced = sem_open("Sem4EvtProcCmpltD", O_CREAT, 0644, 0);
    ASSERT_NE(EvtSuberPrivD.pSemAllProced, nullptr);

    TOS_EvtID_T SubEvtIDs_ofD[] = {TOS_EVTID_TEST_KEEPALIVE, TOS_EVTID_TEST_MSGDATA};
    EvtSubArgs.CbProcEvtSRT_F = __UT_CbProcEvtSRT_Case02_ofD;
    EvtSubArgs.ToObjPriv = &EvtSuberPrivD;

    Result = PLT_EVT_subEvts(EvtSuberD, SubEvtIDs_ofD, TOS_calcArrayElmtCnt(SubEvtIDs_ofD), &EvtSubArgs);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    //-----------------------------------------------------------------------------------------------------------------
    Result = PLT_EVT_enableEvtManger();
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    for( int EvtCnt=0; EvtCnt<(_UT_KEEPALIVE_EVT_CNT + _UT_MSGDATA_EVT_CNT); EvtCnt++ )
    {
        //EvtCnt%2==0: TOS_EVTID_TEST_KEEPALIVE, EvtCnt%2==1: TOS_EVTID_TEST_MSGDATA
        if( EvtCnt % 2 )
        {
            TOS_EVT_defineEvtDesc(MyTestMsgDataEvt,TOS_EVTID_TEST_MSGDATA);
            MyTestMsgDataEvt.EvtData.U32[0] = EvtCnt/2;

            Result = PLT_EVT_postEvtSRT(EvtPuberA, &MyTestMsgDataEvt);
            EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint
        }
        else
        {
            TOS_EVT_defineEvtDesc(MyTestKeepAliveEvt,TOS_EVTID_TEST_KEEPALIVE);

            Result = PLT_EVT_postEvtSRT(EvtPuberA, &MyTestKeepAliveEvt);
            EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint
        }

        usleep(1000);
    }

    //Wait for all EvtSuberPriv.KeepAliveTotalCnt of MyTestKeepAliveEvt to be processed
    sem_wait(EvtSuberPrivB.pSemAllProced);
    EXPECT_EQ(EvtSuberPrivB.KeepAliveProcedCnt, _UT_KEEPALIVE_EVT_CNT);

    sem_wait(EvtSuberPrivC.pSemAllProced);
    EXPECT_EQ(EvtSuberPrivC.MsgDataProcedCnt, _UT_MSGDATA_EVT_CNT);

    sem_wait(EvtSuberPrivD.pSemAllProced);
    EXPECT_EQ(EvtSuberPrivD.KeepAliveProcedCnt, _UT_KEEPALIVE_EVT_CNT);
    EXPECT_EQ(EvtSuberPrivD.MsgDataProcedCnt, _UT_MSGDATA_EVT_CNT);

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct 
{
    TOS_EvtOperID_T EvtSuberID; 
    unsigned long EchoResponseTotalCnt, EchoResponseProcedCnt;

    sem_t *pSemAllProced;
} _UT_EvtSuberPrivCase03_ofA_T, *_UT_EvtSuberPrivCase03_ofA_pT;

static TOS_Result_T __UT_CbProcEvtSRT_Case03_ofA
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToEvtSuberPriv)
{
    _UT_EvtSuberPrivCase03_ofA_pT pEvtSuberPriv = (_UT_EvtSuberPrivCase03_ofA_pT)pToEvtSuberPriv;
    EXPECT_EQ(EvtSuberID, pEvtSuberPriv->EvtSuberID);//CheckPoint

    pEvtSuberPriv->EchoResponseProcedCnt++;

    TOS_EvtID_T EvtID = pEvtDesc->EvtID;
    EXPECT_EQ(EvtID, TOS_EVTID_TEST_ECHO_RESPONSE);//CheckPoint

    if( pEvtSuberPriv->EchoResponseProcedCnt == pEvtSuberPriv->EchoResponseTotalCnt )
    {
        sem_post(pEvtSuberPriv->pSemAllProced);
    }
    return TOS_RESULT_SUCCESS;
}

typedef struct 
{
    TOS_EvtOperID_T EvtSuberID; 
    unsigned long EchoRequestTotalCnt, EchoRequestProcedCnt;

    sem_t *pSemAllProced;
} _UT_EvtSuberPrivCase03_ofB_T, *_UT_EvtSuberPrivCase03_ofB_pT;

static TOS_Result_T __UT_CbProcEvtSRT_Case03_ofB
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToEvtSuberPriv)
{
    _UT_EvtSuberPrivCase03_ofB_pT pEvtSuberPriv = (_UT_EvtSuberPrivCase03_ofB_pT)pToEvtSuberPriv;
    EXPECT_EQ(EvtSuberID, pEvtSuberPriv->EvtSuberID);//CheckPoint

    pEvtSuberPriv->EchoRequestProcedCnt++;

    TOS_EvtID_T EvtID = pEvtDesc->EvtID;
    EXPECT_EQ(EvtID, TOS_EVTID_TEST_ECHO_REQUEST);//CheckPoint

    //Post EchoResponse
    TOS_EVT_defineEvtDesc(MyTestEchoResponseEvt,TOS_EVTID_TEST_ECHO_RESPONSE);
    TOS_Result_T Result = PLT_EVT_postEvtSRT(EvtSuberID, &MyTestEchoResponseEvt);
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    if( pEvtSuberPriv->EchoRequestProcedCnt == pEvtSuberPriv->EchoRequestTotalCnt )
    {
        sem_post(pEvtSuberPriv->pSemAllProced);
    }
    return TOS_RESULT_SUCCESS;
}

typedef struct 
{
    TOS_EvtOperID_T EvtSuberID; 
    unsigned long EchoResponseTotalCnt, EchoResponseProcedCnt;

    sem_t *pSemAllProced;
} _UT_EvtSuberPrivCase03_ofCD_T, *_UT_EvtSuberPrivCase03_ofCD_pT;

static TOS_Result_T __UT_CbProcEvtSRT_Case03_ofCD
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToEvtSuberPriv)
{
    _UT_EvtSuberPrivCase03_ofCD_pT pEvtSuberPriv = (_UT_EvtSuberPrivCase03_ofCD_pT)pToEvtSuberPriv;
    EXPECT_EQ(EvtSuberID, pEvtSuberPriv->EvtSuberID);//CheckPoint

    pEvtSuberPriv->EchoResponseProcedCnt++;

    TOS_EvtID_T EvtID = pEvtDesc->EvtID;
    EXPECT_EQ(EvtID, TOS_EVTID_TEST_ECHO_RESPONSE);//CheckPoint

    if( pEvtSuberPriv->EchoResponseProcedCnt == pEvtSuberPriv->EchoResponseTotalCnt )
    {
        sem_post(pEvtSuberPriv->pSemAllProced);
    }
    return TOS_RESULT_SUCCESS;
}

//UT_T1_PubSubEvt_Typical_NN=[Case03] defined in Typical PubSubEvt Case Lists
TEST(UT_T1_PubSubEvt_Typical, Case03)
{
    TOS_Result_T Result = TOS_RESULT_BUG;

    //===> EvtOperA = EvtPubA + EvtSubB
    TOS_EvtOperID_T EvtOperA      = TOS_EVTOPERID_INVALID;
    TOS_EvtOperArgs_T EvtOperArgs = { .ModObjID = TOS_MODOBJID_UT_A };
    Result = PLT_EVT_regOper(&EvtOperA, &EvtOperArgs);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    TOS_EvtID_T PubEvtIDs_ofA[] = {TOS_EVTID_TEST_ECHO_REQUEST};
    Result = PLT_EVT_pubEvts(EvtOperA, PubEvtIDs_ofA, TOS_calcArrayElmtCnt(PubEvtIDs_ofA));
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    TOS_EvtID_T SubEvtIDs_ofA[] = {TOS_EVTID_TEST_ECHO_RESPONSE};
    _UT_EvtSuberPrivCase03_ofA_T EvtSuberPrivA = { .EvtSuberID = EvtOperA, .EchoResponseTotalCnt = _UT_ECHO_REQUEST_EVT_CNT, .EchoResponseProcedCnt = 0 };
    EvtSuberPrivA.pSemAllProced = sem_open("Sem4EvtProcCmpltA", O_CREAT, 0644, 0);
    ASSERT_NE(EvtSuberPrivA.pSemAllProced, nullptr);

    TOS_EvtSubArgs_T EvtSubArgs = { .CbProcEvtSRT_F = __UT_CbProcEvtSRT_Case03_ofA, };
    EvtSubArgs.ToObjPriv = &EvtSuberPrivA;

    Result = PLT_EVT_subEvts(EvtOperA, SubEvtIDs_ofA, TOS_calcArrayElmtCnt(SubEvtIDs_ofA), &EvtSubArgs);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    //===> EvtOperB = EvtPubB + EvtSubA
    TOS_EvtOperID_T EvtOperB = TOS_EVTOPERID_INVALID;
    EvtOperArgs.ModObjID = TOS_MODOBJID_UT_B;
    Result = PLT_EVT_regOper(&EvtOperB, &EvtOperArgs);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    TOS_EvtID_T PubEvtIDs_ofB[] = {TOS_EVTID_TEST_ECHO_RESPONSE};
    Result = PLT_EVT_pubEvts(EvtOperB, PubEvtIDs_ofB, TOS_calcArrayElmtCnt(PubEvtIDs_ofB));
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    TOS_EvtID_T SubEvtIDs_ofB[] = {TOS_EVTID_TEST_ECHO_REQUEST};
    _UT_EvtSuberPrivCase03_ofB_T EvtSuberPrivB = { .EvtSuberID = EvtOperB, .EchoRequestTotalCnt = _UT_ECHO_REQUEST_EVT_CNT, .EchoRequestProcedCnt = 0 };
    EvtSuberPrivB.pSemAllProced = sem_open("Sem4EvtProcCmpltB", O_CREAT, 0644, 0);
    ASSERT_NE(EvtSuberPrivB.pSemAllProced, nullptr);

    EvtSubArgs.ToObjPriv = &EvtSuberPrivB;
    EvtSubArgs.CbProcEvtSRT_F = __UT_CbProcEvtSRT_Case03_ofB;

    Result = PLT_EVT_subEvts(EvtOperB, SubEvtIDs_ofB, TOS_calcArrayElmtCnt(SubEvtIDs_ofB), &EvtSubArgs);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    //===> EvtSuberC/D
    TOS_EvtOperID_T EvtSuberC = TOS_EVTOPERID_INVALID;
    EvtOperArgs.ModObjID = TOS_MODOBJID_UT_C;
    Result = PLT_EVT_regOper(&EvtSuberC, &EvtOperArgs);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    TOS_EvtOperID_T EvtSuberD = TOS_EVTOPERID_INVALID;
    EvtOperArgs.ModObjID = TOS_MODOBJID_UT_D;
    Result = PLT_EVT_regOper(&EvtSuberD, &EvtOperArgs);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    TOS_EvtID_T SubEvtIDs_ofCD[] = {TOS_EVTID_TEST_ECHO_RESPONSE};
    _UT_EvtSuberPrivCase03_ofCD_T EvtSuberPrivC = { .EvtSuberID = EvtSuberC, .EchoResponseTotalCnt = _UT_ECHO_REQUEST_EVT_CNT, .EchoResponseProcedCnt = 0 };
    EvtSuberPrivC.pSemAllProced = sem_open("Sem4EvtProcCmpltC", O_CREAT, 0644, 0);
    ASSERT_NE(EvtSuberPrivC.pSemAllProced, nullptr);

    _UT_EvtSuberPrivCase03_ofCD_T EvtSuberPrivD = { .EvtSuberID = EvtSuberD, .EchoResponseTotalCnt = _UT_ECHO_REQUEST_EVT_CNT, .EchoResponseProcedCnt = 0 };
    EvtSuberPrivD.pSemAllProced = sem_open("Sem4EvtProcCmpltD", O_CREAT, 0644, 0);
    ASSERT_NE(EvtSuberPrivD.pSemAllProced, nullptr);

    EvtSubArgs.ToObjPriv = &EvtSuberPrivC;
    EvtSubArgs.CbProcEvtSRT_F = __UT_CbProcEvtSRT_Case03_ofCD;

    Result = PLT_EVT_subEvts(EvtSuberC, SubEvtIDs_ofCD, TOS_calcArrayElmtCnt(SubEvtIDs_ofCD), &EvtSubArgs);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    EvtSubArgs.ToObjPriv = &EvtSuberPrivD;
    EvtSubArgs.CbProcEvtSRT_F = __UT_CbProcEvtSRT_Case03_ofCD;

    Result = PLT_EVT_subEvts(EvtSuberD, SubEvtIDs_ofCD, TOS_calcArrayElmtCnt(SubEvtIDs_ofCD), &EvtSubArgs);
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    //-----------------------------------------------------------------------------------------------------------------
    Result = PLT_EVT_enableEvtManger();
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    for( int EvtCnt=0; EvtCnt<_UT_ECHO_REQUEST_EVT_CNT; EvtCnt++ )
    {
        TOS_EVT_defineEvtDesc(MyTestEchoRequestEvt,TOS_EVTID_TEST_ECHO_REQUEST);

        Result = PLT_EVT_postEvtSRT(EvtOperA, &MyTestEchoRequestEvt);
        EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

        usleep(1000);
    }

    //Wait for all EvtSuberPriv.EchoResponseTotalCnt of MyTestEchoResponseEvt to be processed
    sem_wait(EvtSuberPrivA.pSemAllProced);
    EXPECT_EQ(EvtSuberPrivA.EchoResponseProcedCnt, _UT_ECHO_REQUEST_EVT_CNT);

    sem_wait(EvtSuberPrivB.pSemAllProced);
    EXPECT_EQ(EvtSuberPrivB.EchoRequestProcedCnt, _UT_ECHO_REQUEST_EVT_CNT);

    sem_wait(EvtSuberPrivC.pSemAllProced);
    EXPECT_EQ(EvtSuberPrivC.EchoResponseProcedCnt, _UT_ECHO_REQUEST_EVT_CNT);

    sem_wait(EvtSuberPrivD.pSemAllProced);
    EXPECT_EQ(EvtSuberPrivD.EchoResponseProcedCnt, _UT_ECHO_REQUEST_EVT_CNT);

    //-----------------------------------------------------------------------------------------------------------------
    PLT_EVT_disableEvtManger();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    PLT_EVT_unpubEvts(EvtOperA);
    PLT_EVT_unsubEvts(EvtOperA);

    PLT_EVT_unpubEvts(EvtOperB);
    PLT_EVT_unsubEvts(EvtOperB);

    PLT_EVT_unsubEvts(EvtSuberC);
    PLT_EVT_unsubEvts(EvtSuberD);

    PLT_EVT_unregOper(EvtOperA);
    PLT_EVT_unregOper(EvtOperB);
    PLT_EVT_unregOper(EvtSuberC);
    PLT_EVT_unregOper(EvtSuberD);
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