#include "../PlatIF_EVT_UT_Common/_UT_EVT_Common.h"
#include <sys/_pthread/_pthread_t.h>
#include <sys/semaphore.h>
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
 * =[Case04]:
 *      UT_A(as VehicleMainObj a.k.a VMainObj)@Thread_A: 
 *          PubEvt/PostEvtSRT of TOS_EVTID_TEST_MSGDATA * $_UT_MSGDATA_EVT_CNT(asStateTransfer)
 *          SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_CMD_X1_ACK(asCmdAck_fromChassicObj)
 *          SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_CMD_X2_ACK(asCmdAck_fromPalletObj)
 *          SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_CMD_X3_ACK(asCmdAck_fromChassicObj)
 *          SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_CMD_X4_ACK(asCmdAck_fromPalletObj)
 *          SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_KEEPALIVE(asModAlive_fromRCObj_fromChassicObj_fromPalletObj_fromCollisionDetectorObj)
 *      UT_B(as RemogeControlAgentObj a.k.a RCAgentObj)@Thread_B:
 *          PubEvt/PostEvtSRT of TOS_EVTID_TEST_CMD_X1 * $_UT_CMD_X1_EVT_CNT(asCmdReq_toChassicObj)
 *          PubEvt/PostEvtSRT of TOS_EVTID_TEST_CMD_X2 * $_UT_CMD_X2_EVT_CNT(asCmdReq_toPalletObj)
 *          PubEvt/PostEvtSRT of TOS_EVTID_TEST_KEEPALIVE * $_UT_KEEPALIVE_EVT_CNT(asModAlive_toVMainObj)
 *          SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_MSGDATA(asStateTransfer_fromVMainObj)
 *      UT_C(asChassicObj)@Thread_C:
 *          SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_CMD_X1(asCmdReq_fromRCObj)
 *          PubEvt/PostEvtSRT of TOS_EVTID_TEST_CMD_X1_ACK(asCmdAck_toVMainObj)
 *          SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_CMD_X3(asCmdReq_fromCollisionDetectorObj)
 *          PubEvt/PostEvtSRT of TOS_EVTID_TEST_CMD_X3_ACK(asCmdAck_toVMainObj)
 *          PubEvt/PostEvtSRT of TOS_EVTID_TEST_KEEPALIVE * $_UT_KEEPALIVE_EVT_CNT(asModAlive_toVMainObj)
 *          SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_MSGDATA(asStateTransfer_fromVMainObj)
 *      UT_D(asPalletObj)@Thread_D:
 *          SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_CMD_X2(asCmdReq_fromRCObj)
 *          PubEvt/PostEvtSRT of TOS_EVTID_TEST_CMD_X2_ACK(asCmdAck_toVMainObj)
 *          SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_CMD_X4(asCmdReq_fromCollisionDetectorObj)
 *          PubEvt/PostEvtSRT of TOS_EVTID_TEST_CMD_X4_ACK(asCmdAck_toVMainObj)
 *          PubEvt/PostEvtSRT of TOS_EVTID_TEST_KEEPALIVE * $_UT_KEEPALIVE_EVT_CNT(asModAlive_toVMainObj)
 *          SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_MSGDATA(asStateTransfer_fromVMainObj)
 *      UT_E(asCollisionDetectorObj a.k.a CDObj)@Thread_E:
 *          PubEvt/PostEvtSRT of TOS_EVTID_TEST_CMD_X3 * $_UT_CMD_X3_EVT_CNT(asCmdReq_toChassicObj)
 *          PubEvt/PostEvtSRT of TOS_EVTID_TEST_CMD_X4 * $_UT_CMD_X4_EVT_CNT(asCmdReq_toPalletObj)
 *          PubEvt/PostEvtSRT of TOS_EVTID_TEST_KEEPALIVE * $_UT_KEEPALIVE_EVT_CNT(asModAlive_toVMainObj)
 *          SubEvt/CbProcEvtSRT of TOS_EVTID_TEST_MSGDATA(asStateTransfer_fromVMainObj)
 *  TODO(@W)=[Case04]:...
 */

#define _UT_OPERATOR_COUNT      5//UT_A/_B/_C/_D/_E

#define _UT_KEEPALIVE_EVT_CNT       1024
#define _UT_MSGDATA_EVT_CNT         1024
#define _UT_ECHO_REQUEST_EVT_CNT    1024

#define _UT_CMD_X1_EVT_CNT          1024
#define _UT_CMD_X2_EVT_CNT          1024
#define _UT_CMD_X3_EVT_CNT          1024
#define _UT_CMD_X4_EVT_CNT          1024

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


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//VMainObj in ThreadA(RefMore: [Case04]:UT_A(as VehicleMainObj a.k.a VMainObj)@Thread_A)
typedef struct
{
    pthread_t   ThreadID;

    //ModObj=VMainObj|RCAgent|ChassisObj|PalletObj|CDObj
    sem_t       *pSem4ReadyState;  //1: ModObj in ThreadA do some initialization work such Pub/Sub some Evt,
                                 //     then post Sem4ReadyState to notify MAIN that ModObj is in ReadyStage.
    sem_t       *pSem4EnterRunning;//ModObj wait MAIN to post Sem4EnterRunning to notify ModObj to enter RunningStage.
    sem_t       *pSem4LeaveRunning;//ModObj post Sem4LeaveRunning to notify MAIN that ModObj is leaving RunningStage.
    sem_t       *pSem4ExitState;   //ModObj wait MAIN to post Sem4ExitState to notify ModObj to exit.

    union
    {
        struct
        {
            long Rzvd;
        } VMainObj;

        struct
        {
            TOS_EvtOperID_T EvtPuberID;
        } RCAgentPriv;

        struct 
        {
            TOS_EvtOperID_T EvtPuberID;
        } CDObjPriv;
    };
} _UT_CtxThreadABCDE_ModObj_T, *_UT_CtxThreadABCDE_ModObj_pT;

typedef struct 
{
    TOS_EvtOperID_T EvtSuberID; 
    unsigned long KeepAliveTotalCnt, KeepAliveProcedCnt;
    unsigned long CmdAckTotalCnt, CmdAckProcedCnt;

    sem_t *pSemAllProced;
} _UT_EvtSuberPrivCase04_ofA_T, *_UT_EvtSuberPrivCase04_ofA_pT;

static inline TOS_Result_T __UT_CbProcEvtSRT_Case04_ofA
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToEvtSuberPriv)
{
    _UT_EvtSuberPrivCase04_ofA_pT pEvtSuberPriv = (_UT_EvtSuberPrivCase04_ofA_pT)pToEvtSuberPriv;
    EXPECT_EQ(EvtSuberID, pEvtSuberPriv->EvtSuberID);//CheckPoint

    TOS_EvtID_T EvtID = pEvtDesc->EvtID;
    EXPECT_TRUE( (EvtID == TOS_EVTID_TEST_KEEPALIVE) || (EvtID == TOS_EVTID_TEST_CMD_X1_ACK) || (EvtID == TOS_EVTID_TEST_CMD_X2_ACK) || (EvtID == TOS_EVTID_TEST_CMD_X3_ACK) || (EvtID == TOS_EVTID_TEST_CMD_X4_ACK) );//CheckPoint

    if( EvtID == TOS_EVTID_TEST_KEEPALIVE )
    {
        pEvtSuberPriv->KeepAliveProcedCnt++;
    }
    else if( (EvtID == TOS_EVTID_TEST_CMD_X1_ACK) || (EvtID == TOS_EVTID_TEST_CMD_X2_ACK) || (EvtID == TOS_EVTID_TEST_CMD_X3_ACK) || (EvtID == TOS_EVTID_TEST_CMD_X4_ACK) )
    {
        pEvtSuberPriv->CmdAckProcedCnt++;
    }

    if( (pEvtSuberPriv->KeepAliveProcedCnt == pEvtSuberPriv->KeepAliveTotalCnt) 
        && (pEvtSuberPriv->CmdAckProcedCnt == pEvtSuberPriv->CmdAckTotalCnt) )
    {
        sem_post(pEvtSuberPriv->pSemAllProced);
    }
    return TOS_RESULT_SUCCESS;
}

static void* __UT_ThreadA_ofVMainObj( void* arg )
{
    TOS_Result_T Result = TOS_RESULT_BUG;
    _UT_CtxThreadABCDE_ModObj_pT pVMainObj = (_UT_CtxThreadABCDE_ModObj_pT)arg;
    EXPECT_NE(pVMainObj, nullptr);

    //-----------------------------------------------------------------------------------------------------------------
    //VMainObj in ThreadA do some initialization work:
    //  1. regEvtOper as EvtPuberA and EvtSuberA
    //  2. pubEvts(TOS_EVTID_TEST_MSGDATA) with EvtPuberA
    //  3. subEvts(TOS_EVTID_TEST_CMD_X[1-4]_ACK),TOS_EVTID_TEST_KEEPALIVE with EvtSuberA

    //===> EvtPuberA
    TOS_EvtOperID_T EvtPuberA      = TOS_EVTOPERID_INVALID;
    TOS_EvtOperArgs_T EvtPuberArgs = { .ModObjID = TOS_MODOBJID_UT_A };
    Result = PLT_EVT_regOper(&EvtPuberA, &EvtPuberArgs);
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    TOS_EvtID_T PubEvtIDs_ofA[] = {TOS_EVTID_TEST_MSGDATA};
    Result = PLT_EVT_pubEvts(EvtPuberA, PubEvtIDs_ofA, TOS_calcArrayElmtCnt(PubEvtIDs_ofA));
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    //===> EvtSuberA
    TOS_EvtOperID_T EvtSuberA = TOS_EVTOPERID_INVALID;
    TOS_EvtOperArgs_T EvtSuberArgs = { .ModObjID = TOS_MODOBJID_UT_A };
    Result = PLT_EVT_regOper(&EvtSuberA, &EvtSuberArgs);
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    _UT_EvtSuberPrivCase04_ofA_T EvtSuberPrivA = { .EvtSuberID = EvtSuberA, .KeepAliveTotalCnt = _UT_KEEPALIVE_EVT_CNT * 4, .KeepAliveProcedCnt = 0, .CmdAckTotalCnt = _UT_CMD_X1_EVT_CNT + _UT_CMD_X2_EVT_CNT + _UT_CMD_X3_EVT_CNT + _UT_CMD_X4_EVT_CNT, .CmdAckProcedCnt = 0 };
    EvtSuberPrivA.pSemAllProced = sem_open("Sem4EvtProcCmpltA", O_CREAT, 0644, 0);
    EXPECT_NE(EvtSuberPrivA.pSemAllProced, nullptr);

    TOS_EvtSubArgs_T EvtSubArgs = { .CbProcEvtSRT_F = __UT_CbProcEvtSRT_Case04_ofA, };
    EvtSubArgs.ToObjPriv = &EvtSuberPrivA;

    TOS_EvtID_T SubEvtIDs_ofA[] = {TOS_EVTID_TEST_CMD_X1_ACK, TOS_EVTID_TEST_CMD_X2_ACK, TOS_EVTID_TEST_CMD_X3_ACK, TOS_EVTID_TEST_CMD_X4_ACK, TOS_EVTID_TEST_KEEPALIVE};
    Result = PLT_EVT_subEvts(EvtSuberA, SubEvtIDs_ofA, TOS_calcArrayElmtCnt(SubEvtIDs_ofA), &EvtSubArgs);
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    
    //-----------------------------------------------------------------------------------------------------------------
    sem_post(pVMainObj->pSem4ReadyState);//Notify MAIN that VMainObj is in ReadyStage.
    sem_wait(pVMainObj->pSem4EnterRunning);//Wait for MAIN to notify VMainObj to enter RunningStage.

    //-----------------------------------------------------------------------------------------------------------------
    for( int EvtCnt=0; EvtCnt<_UT_MSGDATA_EVT_CNT; EvtCnt++ )
    {
        TOS_EVT_defineEvtDesc(MyTestMsgDataEvt,TOS_EVTID_TEST_MSGDATA);

        Result = PLT_EVT_postEvtSRT(EvtPuberA, &MyTestMsgDataEvt);
        EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

        usleep(1000);
    }

    //Wait for all EvtSuberPriv.[KeepAliveTotalCnt+CmdAckTotalCnt] to be processed
    sem_wait(EvtSuberPrivA.pSemAllProced);
    EXPECT_EQ(EvtSuberPrivA.KeepAliveProcedCnt, _UT_KEEPALIVE_EVT_CNT * 4);
    EXPECT_EQ(EvtSuberPrivA.CmdAckProcedCnt, _UT_CMD_X1_EVT_CNT + _UT_CMD_X2_EVT_CNT + _UT_CMD_X3_EVT_CNT + _UT_CMD_X4_EVT_CNT);

    //-----------------------------------------------------------------------------------------------------------------
    sem_post(pVMainObj->pSem4LeaveRunning);//Notify MAIN that VMainObj is leaving RunningStage.
    sem_wait(pVMainObj->pSem4ExitState);//Wait for MAIN to notify VMainObj to exit.

    //-----------------------------------------------------------------------------------------------------------------
    PLT_EVT_unpubEvts(EvtPuberA);
    PLT_EVT_unsubEvts(EvtSuberA);

    PLT_EVT_unregOper(EvtPuberA);
    PLT_EVT_unregOper(EvtSuberA);
    return NULL;
}

//---------------------------------------------------------------------------------------------------------------------
//RCAgentObj in ThreadB(RefMore: [Case04]:UT_B(as RemogeControlAgentObj a.k.a RCAgentObj)@Thread_B
typedef struct 
{
    TOS_EvtOperID_T EvtSuberID; 
    unsigned long KeepAliveTotalCnt;
    unsigned long CmdX1TotalCnt;
    unsigned long CmdX2TotalCnt;
    unsigned long MsgDataProcedCnt;

    sem_t *pSemAllProced;
} _UT_EvtSuberPrivCase04_ofB_T, *_UT_EvtSuberPrivCase04_ofB_pT;

static TOS_Result_T __UT_CbProcEvtSRT_Case04_ofB
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToEvtSuberPriv)
{
    _UT_EvtSuberPrivCase04_ofB_pT pEvtSuberPriv = (_UT_EvtSuberPrivCase04_ofB_pT)pToEvtSuberPriv;
    EXPECT_EQ(EvtSuberID, pEvtSuberPriv->EvtSuberID);//CheckPoint

    TOS_EvtID_T EvtID = pEvtDesc->EvtID;
    EXPECT_TRUE( EvtID == TOS_EVTID_TEST_MSGDATA );//CheckPoint

    if( EvtID == TOS_EVTID_TEST_MSGDATA )
    {
        pEvtSuberPriv->MsgDataProcedCnt++;
    }

    if( pEvtSuberPriv->MsgDataProcedCnt == _UT_MSGDATA_EVT_CNT )
    {
        sem_post(pEvtSuberPriv->pSemAllProced);
    }
    return TOS_RESULT_SUCCESS;
}

static void* __UT_ThreadB_ofRCAgentObj_postEvtCmdX1( void* arg )
{
    TOS_Result_T Result = TOS_RESULT_BUG;
    _UT_CtxThreadABCDE_ModObj_pT pRCAgentObj = (_UT_CtxThreadABCDE_ModObj_pT)arg;
    EXPECT_NE(pRCAgentObj, nullptr);

    TOS_EvtOperID_T EvtPuberID = pRCAgentObj->RCAgentPriv.EvtPuberID;
    EXPECT_NE(EvtPuberID, TOS_EVTOPERID_INVALID);//CheckPoint

    for( int EvtCnt=0; EvtCnt<_UT_CMD_X1_EVT_CNT; EvtCnt++ )
    {
        TOS_EVT_defineEvtDesc(MyTestCmdX1Evt,TOS_EVTID_TEST_CMD_X1);

        Result = PLT_EVT_postEvtSRT(EvtPuberID, &MyTestCmdX1Evt);
        EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

        usleep(1000);
    }
    return NULL;
}

static void* __UT_ThreadB_ofRCAgentObj_postEvtCmdX2( void* arg )
{
    TOS_Result_T Result = TOS_RESULT_BUG;
    _UT_CtxThreadABCDE_ModObj_pT pRCAgentObj = (_UT_CtxThreadABCDE_ModObj_pT)arg;
    EXPECT_NE(pRCAgentObj, nullptr);

    TOS_EvtOperID_T EvtPuberID = pRCAgentObj->RCAgentPriv.EvtPuberID;
    EXPECT_NE(EvtPuberID, TOS_EVTOPERID_INVALID);//CheckPoint

    for( int EvtCnt=0; EvtCnt<_UT_CMD_X2_EVT_CNT; EvtCnt++ )
    {
        TOS_EVT_defineEvtDesc(MyTestCmdX2Evt,TOS_EVTID_TEST_CMD_X2);

        Result = PLT_EVT_postEvtSRT(EvtPuberID, &MyTestCmdX2Evt);
        EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

        usleep(1000);
    }
    return NULL;
}

static void* __UT_ThreadB_ofRCAgentObj( void* arg )
{
    TOS_Result_T Result = TOS_RESULT_BUG;
    _UT_CtxThreadABCDE_ModObj_pT pRCAgentObj = (_UT_CtxThreadABCDE_ModObj_pT)arg;
    EXPECT_NE(pRCAgentObj, nullptr);

    //-----------------------------------------------------------------------------------------------------------------
    //RCAgentObj in ThreadB do some initialization work:
    //  1. regEvtOper as EvtOperB
    //  2. pubEvts(TOS_EVTID_TEST_CMD_X[1,2]) with EvtOperB as EvtPuber
    //  3. pubEvts(TOS_EVTID_TEST_KEEPALIVE) with EvtOperB as EvtPuber
    //  4. subEvts(TOS_EVTID_TEST_MSGDATA) with EvtOperB as EvtSuber

    //===> EvtOperB
    TOS_EvtOperID_T EvtOperB      = TOS_EVTOPERID_INVALID;
    TOS_EvtOperArgs_T EvtOperArgs = { .ModObjID = TOS_MODOBJID_UT_B };
    Result = PLT_EVT_regOper(&EvtOperB, &EvtOperArgs);
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    TOS_EvtOperID_T EvtPuberID = EvtOperB;
    TOS_EvtOperID_T EvtSuberID = EvtOperB;

    pRCAgentObj->RCAgentPriv.EvtPuberID = EvtOperB;

    TOS_EvtID_T PubEvtIDs_ofB[] = {TOS_EVTID_TEST_CMD_X1, TOS_EVTID_TEST_CMD_X2, TOS_EVTID_TEST_KEEPALIVE};
    Result = PLT_EVT_pubEvts(EvtPuberID, PubEvtIDs_ofB, TOS_calcArrayElmtCnt(PubEvtIDs_ofB));
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    TOS_EvtID_T SubEvtIDs_ofB[] = {TOS_EVTID_TEST_MSGDATA};

    _UT_EvtSuberPrivCase04_ofB_T EvtSuberPrivB = { .EvtSuberID = EvtSuberID, .KeepAliveTotalCnt = _UT_KEEPALIVE_EVT_CNT, .CmdX1TotalCnt = _UT_CMD_X1_EVT_CNT, .CmdX2TotalCnt = _UT_CMD_X2_EVT_CNT, .MsgDataProcedCnt = 0 };
    EvtSuberPrivB.pSemAllProced = sem_open("Sem4EvtProcCmpltB", O_CREAT, 0644, 0);
    EXPECT_NE(EvtSuberPrivB.pSemAllProced, nullptr);

    TOS_EvtSubArgs_T EvtSubArgs = { .CbProcEvtSRT_F = __UT_CbProcEvtSRT_Case04_ofB, };
    EvtSubArgs.ToObjPriv = &EvtSuberPrivB;

    Result = PLT_EVT_subEvts(EvtSuberID, SubEvtIDs_ofB, TOS_calcArrayElmtCnt(SubEvtIDs_ofB), &EvtSubArgs);
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    //-----------------------------------------------------------------------------------------------------------------
    sem_post(pRCAgentObj->pSem4ReadyState);//Notify MAIN that RCAgentObj is in ReadyStage.
    sem_wait(pRCAgentObj->pSem4EnterRunning);//Wait for MAIN to notify RCAgentObj to enter RunningStage.

    //-----------------------------------------------------------------------------------------------------------------
    //Startup ThreadB_ofRCAgentObj_CmdX1
    pthread_t ThreadID_postEvtCmdX1;
    int RetPSX = pthread_create(&ThreadID_postEvtCmdX1, NULL, __UT_ThreadB_ofRCAgentObj_postEvtCmdX1, pRCAgentObj);
    EXPECT_EQ(RetPSX, 0);//CheckPoint

    //Startup ThreadB_ofRCAgentObj_CmdX2
    pthread_t ThreadID_postEvtCmdX2;
    RetPSX = pthread_create(&ThreadID_postEvtCmdX2, NULL, __UT_ThreadB_ofRCAgentObj_postEvtCmdX2, pRCAgentObj);
    EXPECT_EQ(RetPSX, 0);//CheckPoint
    
    //-----------------------------------------------------------------------------------------------------------------
    for( int EvtCnt=0; EvtCnt<_UT_KEEPALIVE_EVT_CNT; EvtCnt++ )
    {
        TOS_EVT_defineEvtDesc(MyTestKeepAliveEvt,TOS_EVTID_TEST_KEEPALIVE);

        Result = PLT_EVT_postEvtSRT(EvtPuberID, &MyTestKeepAliveEvt);
        EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

        usleep(1000);
    }

    //Wait for EvtSuberPrivB.MsgDataProcedCnt==_UT_MSGDATA_EVT_CNT
    sem_wait(EvtSuberPrivB.pSemAllProced);
    EXPECT_EQ(EvtSuberPrivB.MsgDataProcedCnt, _UT_MSGDATA_EVT_CNT);

    //Wait for ThreadB_ofRCAgentObj_CmdX1/X2 to exit
    pthread_join(ThreadID_postEvtCmdX1, NULL);
    pthread_join(ThreadID_postEvtCmdX2, NULL);

    //-----------------------------------------------------------------------------------------------------------------
    sem_wait(pRCAgentObj->pSem4LeaveRunning);//Wait for MAIN to notify RCAgentObj to leave RunningStage.

    //-----------------------------------------------------------------------------------------------------------------
    PLT_EVT_unpubEvts(EvtOperB);
    PLT_EVT_unsubEvts(EvtOperB);

    PLT_EVT_unregOper(EvtOperB);
    return NULL;
}

//---------------------------------------------------------------------------------------------------------------------
//ChassicObj in ThreadC(RefMore: [Case04]:UT_C(as ChassicObj)@Thread_C
typedef struct 
{
    TOS_EvtOperID_T EvtSuberID; 

    unsigned long CmdX1TotalCnt;
    unsigned long CmdX1ProcedCnt;

    unsigned long CmdX3TotalCnt;
    unsigned long CmdX3ProcedCnt;

    unsigned long MsgDataTotalCnt;
    unsigned long MsgDataProcedCnt;

    sem_t *pSemAllProced;
} _UT_EvtSuberPrivCase04_ofC_T, *_UT_EvtSuberPrivCase04_ofC_pT;

static TOS_Result_T __UT_CbProcEvtSRT_Case04_ofC
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToEvtSuberPriv)
{
    _UT_EvtSuberPrivCase04_ofC_pT pEvtSuberPriv = (_UT_EvtSuberPrivCase04_ofC_pT)pToEvtSuberPriv;
    EXPECT_EQ(EvtSuberID, pEvtSuberPriv->EvtSuberID);//CheckPoint

    TOS_EvtID_T EvtID = pEvtDesc->EvtID;
    EXPECT_TRUE( (EvtID == TOS_EVTID_TEST_CMD_X1) || (EvtID == TOS_EVTID_TEST_CMD_X3) || (EvtID == TOS_EVTID_TEST_MSGDATA) );//CheckPoint

    if( EvtID == TOS_EVTID_TEST_CMD_X1 )
    {
        pEvtSuberPriv->CmdX1ProcedCnt++;

        TOS_EVT_defineEvtDesc(MyTestCmdX1AckEvt,TOS_EVTID_TEST_CMD_X1_ACK);
        TOS_Result_T Result = PLT_EVT_postEvtSRT(pEvtSuberPriv->EvtSuberID, &MyTestCmdX1AckEvt);
        EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint
    }
    else if( EvtID == TOS_EVTID_TEST_CMD_X3 )
    {
        pEvtSuberPriv->CmdX3ProcedCnt++;

        TOS_EVT_defineEvtDesc(MyTestCmdX3AckEvt,TOS_EVTID_TEST_CMD_X3_ACK);
        TOS_Result_T Result = PLT_EVT_postEvtSRT(pEvtSuberPriv->EvtSuberID, &MyTestCmdX3AckEvt);
        EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint
    }
    else if( EvtID == TOS_EVTID_TEST_MSGDATA )
    {
        pEvtSuberPriv->MsgDataProcedCnt++;
    }

    if( (pEvtSuberPriv->CmdX1ProcedCnt == pEvtSuberPriv->CmdX1TotalCnt) 
        && (pEvtSuberPriv->CmdX3ProcedCnt == pEvtSuberPriv->CmdX3TotalCnt)
        && (pEvtSuberPriv->MsgDataProcedCnt == pEvtSuberPriv->MsgDataTotalCnt) )
    {
        sem_post(pEvtSuberPriv->pSemAllProced);
    }
    return TOS_RESULT_SUCCESS;
}

static void* __UT_ThreadC_ofChassicObj( void* arg )
{
    TOS_Result_T Result = TOS_RESULT_BUG;
    _UT_CtxThreadABCDE_ModObj_pT pChassicObj = (_UT_CtxThreadABCDE_ModObj_pT)arg;
    EXPECT_NE(pChassicObj, nullptr);

    //-----------------------------------------------------------------------------------------------------------------
    //ChassicObj in ThreadC do some initialization work:
    //  1. regEvtOper as EvtOperC, alias as EvtPuberC and EvtSuberC
    //  2. subEvts(TOS_EVTID_TEST_CMD_X1/_X3) with EvtSuberC
    //  3. pubEvts(TOS_EVTID_TEST_CMD_X1_ACK/_X3_ACK) with EvtPuberC 
    //      and postEvt in CbProcEvtSRT_F of TOS_EVTID_TEST_CMD_X1/_X3
    //  4. pubEvts(TOS_EVTID_TEST_KEEPALIVE) with EvtPuberC
    //  5. subEvts(TOS_EVTID_TEST_MSGDATA) with EvtSuberC

    //===> EvtOperC
    TOS_EvtOperID_T EvtOperC      = TOS_EVTOPERID_INVALID;
    TOS_EvtOperArgs_T EvtOperArgs = { .ModObjID = TOS_MODOBJID_UT_C };
    Result = PLT_EVT_regOper(&EvtOperC, &EvtOperArgs);
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    TOS_EvtOperID_T EvtPuberID = EvtOperC;
    TOS_EvtOperID_T EvtSuberID = EvtOperC;

    //-----------------------------------------------------------------------------------------------------------------
    //===> EvtPuberC
    TOS_EvtID_T PubEvtIDs_ofC[] = {TOS_EVTID_TEST_CMD_X1_ACK, TOS_EVTID_TEST_CMD_X3_ACK, TOS_EVTID_TEST_KEEPALIVE};
    Result = PLT_EVT_pubEvts(EvtPuberID, PubEvtIDs_ofC, TOS_calcArrayElmtCnt(PubEvtIDs_ofC));
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    //===> EvtSuberC
    _UT_EvtSuberPrivCase04_ofC_T EvtSuberPrivC = { .EvtSuberID = EvtSuberID, .CmdX1TotalCnt = _UT_CMD_X1_EVT_CNT, .CmdX1ProcedCnt = 0, .CmdX3TotalCnt = _UT_CMD_X3_EVT_CNT, .CmdX3ProcedCnt = 0, .MsgDataTotalCnt = _UT_MSGDATA_EVT_CNT, .MsgDataProcedCnt = 0 };
    EvtSuberPrivC.pSemAllProced = sem_open("Sem4EvtProcCmpltC", O_CREAT, 0644, 0);
    EXPECT_NE(EvtSuberPrivC.pSemAllProced, nullptr);
    
    TOS_EvtSubArgs_T EvtSubArgs = { .CbProcEvtSRT_F = __UT_CbProcEvtSRT_Case04_ofC, };
    EvtSubArgs.ToObjPriv = &EvtSuberPrivC;

    TOS_EvtID_T SubEvtIDs_ofC[] = {TOS_EVTID_TEST_CMD_X1, TOS_EVTID_TEST_CMD_X3, TOS_EVTID_TEST_MSGDATA};
    Result = PLT_EVT_subEvts(EvtSuberID, SubEvtIDs_ofC, TOS_calcArrayElmtCnt(SubEvtIDs_ofC), &EvtSubArgs);
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    //-----------------------------------------------------------------------------------------------------------------
    sem_post(pChassicObj->pSem4ReadyState);//Notify MAIN that ChassicObj is in ReadyStage.
    sem_wait(pChassicObj->pSem4EnterRunning);//Wait for MAIN to notify ChassicObj to enter RunningStage.

    //-----------------------------------------------------------------------------------------------------------------
    for( int EvtCnt=0; EvtCnt<_UT_KEEPALIVE_EVT_CNT; EvtCnt++ )
    {
        TOS_EVT_defineEvtDesc(MyTestKeepAliveEvt,TOS_EVTID_TEST_KEEPALIVE);

        Result = PLT_EVT_postEvtSRT(EvtPuberID, &MyTestKeepAliveEvt);
        EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

        usleep(1000);
    }

    //Wait for EvtSuberPrivC.MsgDataProcedCnt==_UT_MSGDATA_EVT_CNT && CmdX1ProcedCnt==_UT_CMD_X1_EVT_CNT && CmdX3ProcedCnt==_UT_CMD_X3_EVT_CNT
    sem_wait(EvtSuberPrivC.pSemAllProced);
    EXPECT_EQ(EvtSuberPrivC.MsgDataProcedCnt, _UT_MSGDATA_EVT_CNT);
    EXPECT_EQ(EvtSuberPrivC.CmdX1ProcedCnt, _UT_CMD_X1_EVT_CNT);
    EXPECT_EQ(EvtSuberPrivC.CmdX3ProcedCnt, _UT_CMD_X3_EVT_CNT);

    //-----------------------------------------------------------------------------------------------------------------
    sem_post(pChassicObj->pSem4LeaveRunning);//Notify MAIN that ChassicObj is leaving RunningStage.
    sem_wait(pChassicObj->pSem4ExitState);//Wait for MAIN to notify ChassicObj to exit.

    //-----------------------------------------------------------------------------------------------------------------
    PLT_EVT_unpubEvts(EvtPuberID);
    PLT_EVT_unsubEvts(EvtSuberID);

    PLT_EVT_unregOper(EvtOperC);
    return NULL;
}

//---------------------------------------------------------------------------------------------------------------------
//PalletObj in ThreadD(RefMore: [Case04]:UT_D(as PalletObj)@Thread_D
typedef struct 
{
    TOS_EvtOperID_T EvtSuberID; 

    unsigned long CmdX2TotalCnt;
    unsigned long CmdX2ProcedCnt;

    unsigned long CmdX4TotalCnt;
    unsigned long CmdX4ProcedCnt;

    unsigned long MsgDataTotalCnt;
    unsigned long MsgDataProcedCnt;

    sem_t *pSemAllProced;
} _UT_EvtSuberPrivCase04_ofD_T, *_UT_EvtSuberPrivCase04_ofD_pT;

static TOS_Result_T __UT_CbProcEvtSRT_Case04_ofD
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToEvtSuberPriv)
{
    _UT_EvtSuberPrivCase04_ofD_pT pEvtSuberPriv = (_UT_EvtSuberPrivCase04_ofD_pT)pToEvtSuberPriv;
    EXPECT_EQ(EvtSuberID, pEvtSuberPriv->EvtSuberID);//CheckPoint

    TOS_EvtID_T EvtID = pEvtDesc->EvtID;
    EXPECT_TRUE( (EvtID == TOS_EVTID_TEST_CMD_X2) || (EvtID == TOS_EVTID_TEST_CMD_X4) || (EvtID == TOS_EVTID_TEST_MSGDATA) );//CheckPoint

    if( EvtID == TOS_EVTID_TEST_CMD_X2 )
    {
        pEvtSuberPriv->CmdX2ProcedCnt++;

        TOS_EVT_defineEvtDesc(MyTestCmdX2AckEvt,TOS_EVTID_TEST_CMD_X2_ACK);
        TOS_Result_T Result = PLT_EVT_postEvtSRT(pEvtSuberPriv->EvtSuberID, &MyTestCmdX2AckEvt);
        EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint
    }
    else if( EvtID == TOS_EVTID_TEST_CMD_X4 )
    {
        pEvtSuberPriv->CmdX4ProcedCnt++;

        TOS_EVT_defineEvtDesc(MyTestCmdX4AckEvt,TOS_EVTID_TEST_CMD_X4_ACK);
        TOS_Result_T Result = PLT_EVT_postEvtSRT(pEvtSuberPriv->EvtSuberID, &MyTestCmdX4AckEvt);
        EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint
    }
    else if( EvtID == TOS_EVTID_TEST_MSGDATA )
    {
        pEvtSuberPriv->MsgDataProcedCnt++;
    }

    if( (pEvtSuberPriv->CmdX2ProcedCnt == pEvtSuberPriv->CmdX2TotalCnt) 
        && (pEvtSuberPriv->CmdX4ProcedCnt == pEvtSuberPriv->CmdX4TotalCnt)
        && (pEvtSuberPriv->MsgDataProcedCnt == pEvtSuberPriv->MsgDataTotalCnt) )
    {   
        sem_post(pEvtSuberPriv->pSemAllProced);
    }
    return TOS_RESULT_SUCCESS;
}

static void* __UT_ThreadD_ofPalletObj( void* arg )
{
    TOS_Result_T Result = TOS_RESULT_BUG;
    _UT_CtxThreadABCDE_ModObj_pT pPalletObj = (_UT_CtxThreadABCDE_ModObj_pT)arg;
    EXPECT_NE(pPalletObj, nullptr);

    //-----------------------------------------------------------------------------------------------------------------
    //PalletObj in ThreadD do some initialization work:
    //  1. regEvtOper as EvtOperD, alias as EvtPuberD and EvtSuberD
    //  2. subEvts(TOS_EVTID_TEST_CMD_X2/_X4) with EvtSuberD
    //  3. pubEvts(TOS_EVTID_TEST_CMD_X2_ACK/_X4_ACK) with EvtPuberD
    //      and postEvt in CbProcEvtSRT_F of TOS_EVTID_TEST_CMD_X2/_X4
    //  4. pubEvts(TOS_EVTID_TEST_KEEPALIVE) with EvtPuberD
    //  5. subEvts(TOS_EVTID_TEST_MSGDATA) with EvtSuberD

    //===> EvtOperD
    TOS_EvtOperID_T EvtOperD      = TOS_EVTOPERID_INVALID;
    TOS_EvtOperArgs_T EvtOperArgs = { .ModObjID = TOS_MODOBJID_UT_D };
    Result = PLT_EVT_regOper(&EvtOperD, &EvtOperArgs);
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    TOS_EvtOperID_T EvtPuberID = EvtOperD;
    TOS_EvtOperID_T EvtSuberID = EvtOperD;

    //-----------------------------------------------------------------------------------------------------------------
    //===> EvtPuberD
    TOS_EvtID_T PubEvtIDs_ofD[] = {TOS_EVTID_TEST_CMD_X2_ACK, TOS_EVTID_TEST_CMD_X4_ACK, TOS_EVTID_TEST_KEEPALIVE};
    Result = PLT_EVT_pubEvts(EvtPuberID, PubEvtIDs_ofD, TOS_calcArrayElmtCnt(PubEvtIDs_ofD));
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    //===> EvtSuberD
    _UT_EvtSuberPrivCase04_ofD_T EvtSuberPrivD = { .EvtSuberID = EvtSuberID, .CmdX2TotalCnt = _UT_CMD_X2_EVT_CNT, .CmdX2ProcedCnt = 0, .CmdX4TotalCnt = _UT_CMD_X4_EVT_CNT, .CmdX4ProcedCnt = 0, .MsgDataTotalCnt = _UT_MSGDATA_EVT_CNT, .MsgDataProcedCnt = 0 };
    EvtSuberPrivD.pSemAllProced = sem_open("Sem4EvtProcCmpltD", O_CREAT, 0644, 0);
    EXPECT_NE(EvtSuberPrivD.pSemAllProced, nullptr);

    TOS_EvtSubArgs_T EvtSubArgs = { .CbProcEvtSRT_F = __UT_CbProcEvtSRT_Case04_ofD, };
    EvtSubArgs.ToObjPriv = &EvtSuberPrivD;

    TOS_EvtID_T SubEvtIDs_ofD[] = {TOS_EVTID_TEST_CMD_X2, TOS_EVTID_TEST_CMD_X4, TOS_EVTID_TEST_MSGDATA};
    Result = PLT_EVT_subEvts(EvtSuberID, SubEvtIDs_ofD, TOS_calcArrayElmtCnt(SubEvtIDs_ofD), &EvtSubArgs);
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    //-----------------------------------------------------------------------------------------------------------------
    sem_post(pPalletObj->pSem4ReadyState);//Notify MAIN that PalletObj is in ReadyStage.
    sem_wait(pPalletObj->pSem4EnterRunning);//Wait for MAIN to notify PalletObj to enter RunningStage.

    //-----------------------------------------------------------------------------------------------------------------
    for( int EvtCnt=0; EvtCnt<_UT_KEEPALIVE_EVT_CNT; EvtCnt++ )
    {
        TOS_EVT_defineEvtDesc(MyTestKeepAliveEvt,TOS_EVTID_TEST_KEEPALIVE);

        Result = PLT_EVT_postEvtSRT(EvtPuberID, &MyTestKeepAliveEvt);
        EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

        usleep(1000);
    }

    //Wait for EvtSuberPrivD.MsgDataProcedCnt==_UT_MSGDATA_EVT_CNT && CmdX2ProcedCnt==_UT_CMD_X2_EVT_CNT && CmdX4ProcedCnt==_UT_CMD_X4_EVT_CNT
    sem_wait(EvtSuberPrivD.pSemAllProced);
    EXPECT_EQ(EvtSuberPrivD.MsgDataProcedCnt, _UT_MSGDATA_EVT_CNT);
    EXPECT_EQ(EvtSuberPrivD.CmdX2ProcedCnt, _UT_CMD_X2_EVT_CNT);
    EXPECT_EQ(EvtSuberPrivD.CmdX4ProcedCnt, _UT_CMD_X4_EVT_CNT);

    //-----------------------------------------------------------------------------------------------------------------
    sem_post(pPalletObj->pSem4LeaveRunning);//Notify MAIN that PalletObj is leaving RunningStage.
    sem_wait(pPalletObj->pSem4ExitState);//Wait for MAIN to notify PalletObj to exit.

    //-----------------------------------------------------------------------------------------------------------------
    PLT_EVT_unpubEvts(EvtPuberID);
    PLT_EVT_unsubEvts(EvtSuberID);

    PLT_EVT_unregOper(EvtOperD);
    return NULL;
}

//---------------------------------------------------------------------------------------------------------------------
//CDObj in ThreadE(RefMore: [Case04]:UT_E(as CollisionDetectorObj)@Thread_E
static void* __UT_ThreadE_ofCDObj_postEvtCmdX3( void* arg )
{
    TOS_Result_T Result = TOS_RESULT_BUG;
    _UT_CtxThreadABCDE_ModObj_pT pCDObj = (_UT_CtxThreadABCDE_ModObj_pT)arg;
    EXPECT_NE(pCDObj, nullptr);

    TOS_EvtOperID_T EvtPuberID = pCDObj->CDObjPriv.EvtPuberID;
    EXPECT_NE(EvtPuberID, TOS_EVTOPERID_INVALID);//CheckPoint

    for( int EvtCnt=0; EvtCnt<_UT_CMD_X3_EVT_CNT; EvtCnt++ )
    {
        TOS_EVT_defineEvtDesc(MyTestCmdX3Evt,TOS_EVTID_TEST_CMD_X3);

        Result = PLT_EVT_postEvtSRT(EvtPuberID, &MyTestCmdX3Evt);
        EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

        usleep(1000);
    }
    return NULL;
}

static void* __UT_ThreadE_ofCDObj_postEvtCmdX4( void* arg )
{
    TOS_Result_T Result = TOS_RESULT_BUG;
    _UT_CtxThreadABCDE_ModObj_pT pCDObj = (_UT_CtxThreadABCDE_ModObj_pT)arg;
    EXPECT_NE(pCDObj, nullptr);

    TOS_EvtOperID_T EvtPuberID = pCDObj->CDObjPriv.EvtPuberID;
    EXPECT_NE(EvtPuberID, TOS_EVTOPERID_INVALID);//CheckPoint

    for( int EvtCnt=0; EvtCnt<_UT_CMD_X4_EVT_CNT; EvtCnt++ )
    {
        TOS_EVT_defineEvtDesc(MyTestCmdX4Evt,TOS_EVTID_TEST_CMD_X4);

        Result = PLT_EVT_postEvtSRT(EvtPuberID, &MyTestCmdX4Evt);
        EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

        usleep(1000);
    }
    return NULL;
}

typedef struct 
{
    TOS_EvtOperID_T EvtSuberID; 
    unsigned long MsgDataTotalCnt;
    unsigned long MsgDataProcedCnt;

    sem_t *pSemAllProced;
} _UT_EvtSuberPrivCase04_ofE_T, *_UT_EvtSuberPrivCase04_ofE_pT;

static TOS_Result_T __UT_CbProcEvtSRT_Case04_ofE
    (/*ARG_IN*/TOS_EvtOperID_T EvtSuberID, /*ARG_IN*/const TOS_EvtDesc_pT pEvtDesc, /*ARG_IN*/void* pToEvtSuberPriv)
{
    _UT_EvtSuberPrivCase04_ofE_pT pEvtSuberPriv = (_UT_EvtSuberPrivCase04_ofE_pT)pToEvtSuberPriv;
    EXPECT_EQ(EvtSuberID, pEvtSuberPriv->EvtSuberID);//CheckPoint

    TOS_EvtID_T EvtID = pEvtDesc->EvtID;
    EXPECT_TRUE( EvtID == TOS_EVTID_TEST_MSGDATA );//CheckPoint

    if( EvtID == TOS_EVTID_TEST_MSGDATA )
    {
        pEvtSuberPriv->MsgDataProcedCnt++;
    }

    if( pEvtSuberPriv->MsgDataProcedCnt == pEvtSuberPriv->MsgDataTotalCnt )
    {
        sem_post(pEvtSuberPriv->pSemAllProced);
    }
    return TOS_RESULT_SUCCESS;
}

static void* __UT_ThreadE_ofCDObj( void* arg )
{
    TOS_Result_T Result = TOS_RESULT_BUG;
    _UT_CtxThreadABCDE_ModObj_pT pCDObj = (_UT_CtxThreadABCDE_ModObj_pT)arg;
    EXPECT_NE(pCDObj, nullptr);

    //-----------------------------------------------------------------------------------------------------------------
    //CDObj in ThreadE do some initialization work:
    //  1. regEvtOper as EvtOperE, alias as EvtPuberE and EvtSuberE
    //  2. pubEvt(TOS_EVTID_TEST_KEEPALIVE) with EvtPuberE
    //  3. pubEvt(TOS_EVTID_CMD_X3/_X4) with EvtPuberE
    //      and postEvt in ThreadE_ofCDObj_postEvtCmdX3/_X4
    //  4. subEvts(TOS_EVTID_TEST_MSGDATA) with EvtSuberE

    //===> EvtOperE
    TOS_EvtOperID_T EvtOperE      = TOS_EVTOPERID_INVALID;
    TOS_EvtOperArgs_T EvtOperArgs = { .ModObjID = TOS_MODOBJID_UT_E };
    Result = PLT_EVT_regOper(&EvtOperE, &EvtOperArgs);
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    TOS_EvtOperID_T EvtPuberID   = EvtOperE;
    pCDObj->CDObjPriv.EvtPuberID = EvtOperE;
    TOS_EvtOperID_T EvtSuberID   = EvtOperE;

    //-----------------------------------------------------------------------------------------------------------------
    //===> EvtPuberE
    TOS_EvtID_T PubEvtIDs_ofE[] = {TOS_EVTID_TEST_KEEPALIVE, TOS_EVTID_TEST_CMD_X3, TOS_EVTID_TEST_CMD_X4};
    Result = PLT_EVT_pubEvts(EvtPuberID, PubEvtIDs_ofE, TOS_calcArrayElmtCnt(PubEvtIDs_ofE));
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    //-----------------------------------------------------------------------------------------------------------------
    //===> EvtSuberE
    _UT_EvtSuberPrivCase04_ofE_T EvtSuberPrivE = { .EvtSuberID = EvtSuberID, .MsgDataTotalCnt = _UT_MSGDATA_EVT_CNT, .MsgDataProcedCnt = 0 };
    EvtSuberPrivE.pSemAllProced = sem_open("Sem4EvtProcCmpltE", O_CREAT, 0644, 0);
    EXPECT_NE(EvtSuberPrivE.pSemAllProced, nullptr);

    TOS_EvtSubArgs_T EvtSubArgs = { .CbProcEvtSRT_F = __UT_CbProcEvtSRT_Case04_ofE, };
    EvtSubArgs.ToObjPriv = &EvtSuberPrivE;

    TOS_EvtID_T SubEvtIDs_ofE[] = {TOS_EVTID_TEST_MSGDATA};
    Result = PLT_EVT_subEvts(EvtSuberID, SubEvtIDs_ofE, TOS_calcArrayElmtCnt(SubEvtIDs_ofE), &EvtSubArgs);
    EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    //-----------------------------------------------------------------------------------------------------------------
    sem_post(pCDObj->pSem4ReadyState);//Notify MAIN that CDObj is in ReadyStage.
    sem_wait(pCDObj->pSem4EnterRunning);//Wait for MAIN to notify CDObj to enter RunningStage.

    //-----------------------------------------------------------------------------------------------------------------
    //Startup ThreadE_ofCDObj_postEvtCmdX3
    pthread_t ThreadID_postEvtCmdX3;
    int RetPSX = pthread_create(&ThreadID_postEvtCmdX3, NULL, __UT_ThreadE_ofCDObj_postEvtCmdX3, pCDObj);
    EXPECT_EQ(RetPSX, 0);//CheckPoint

    //Startup ThreadE_ofCDObj_postEvtCmdX4
    pthread_t ThreadID_postEvtCmdX4;
    RetPSX = pthread_create(&ThreadID_postEvtCmdX4, NULL, __UT_ThreadE_ofCDObj_postEvtCmdX4, pCDObj);
    EXPECT_EQ(RetPSX, 0);//CheckPoint

    //-----------------------------------------------------------------------------------------------------------------
    for( int EvtCnt=0; EvtCnt<_UT_MSGDATA_EVT_CNT; EvtCnt++ )
    {
        TOS_EVT_defineEvtDesc(MyTestKeepAliveEvt,TOS_EVTID_TEST_KEEPALIVE);

        Result = PLT_EVT_postEvtSRT(EvtPuberID, &MyTestKeepAliveEvt);
        EXPECT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

        usleep(1000);
    }

    //Wait for EvtSuberPrivE.MsgDataProcedCnt==_UT_MSGDATA_EVT_CNT
    sem_wait(EvtSuberPrivE.pSemAllProced);
    EXPECT_EQ(EvtSuberPrivE.MsgDataProcedCnt, _UT_MSGDATA_EVT_CNT);

    //Wait for ThreadE_ofCDObj_postEvtCmdX3/X4 to exit
    pthread_join(ThreadID_postEvtCmdX3, NULL);
    pthread_join(ThreadID_postEvtCmdX4, NULL);

    //-----------------------------------------------------------------------------------------------------------------
    sem_post(pCDObj->pSem4LeaveRunning);//Notify MAIN that CDObj is leaving RunningStage.
    sem_wait(pCDObj->pSem4ExitState);//Wait for MAIN to notify CDObj to exit.

    //-----------------------------------------------------------------------------------------------------------------
    PLT_EVT_unpubEvts(EvtPuberID);
    PLT_EVT_unsubEvts(EvtSuberID);

    PLT_EVT_unregOper(EvtOperE);
    return NULL;
}

//UT_T1_PubSubEvt_Typical_NN=[04] defined in Typical PubSubEvt Case Lists
TEST(UT_T1_PubSubEvt_Typical, Case04)
{
    _UT_CtxThreadABCDE_ModObj_T VMainObj = {};
    _UT_CtxThreadABCDE_ModObj_T RCAgentObj = {};
    _UT_CtxThreadABCDE_ModObj_T ChassicObj = {};
    _UT_CtxThreadABCDE_ModObj_T PalletObj = {};
    _UT_CtxThreadABCDE_ModObj_T CDObj = {};

    VMainObj.pSem4ReadyState = sem_open("ThreadA_Sem4ReadyState", O_CREAT, 0644, 0);
    ASSERT_NE(VMainObj.pSem4ReadyState, nullptr);
    VMainObj.pSem4EnterRunning = sem_open("ThreadA_Sem4EnterRunning", O_CREAT, 0644, 0);
    ASSERT_NE(VMainObj.pSem4EnterRunning, nullptr);
    VMainObj.pSem4LeaveRunning = sem_open("ThreadA_Sem4LeaveRunning", O_CREAT, 0644, 0);
    ASSERT_NE(VMainObj.pSem4LeaveRunning, nullptr);
    VMainObj.pSem4ExitState = sem_open("ThreadA_Sem4ExitState", O_CREAT, 0644, 0);
    ASSERT_NE(VMainObj.pSem4ExitState, nullptr);

    RCAgentObj.pSem4ReadyState = sem_open("ThreadB_Sem4ReadyState", O_CREAT, 0644, 0);
    ASSERT_NE(RCAgentObj.pSem4ReadyState, nullptr);
    RCAgentObj.pSem4EnterRunning = sem_open("ThreadB_Sem4EnterRunning", O_CREAT, 0644, 0);
    ASSERT_NE(RCAgentObj.pSem4EnterRunning, nullptr);
    RCAgentObj.pSem4LeaveRunning = sem_open("ThreadB_Sem4LeaveRunning", O_CREAT, 0644, 0);
    ASSERT_NE(RCAgentObj.pSem4LeaveRunning, nullptr);
    RCAgentObj.pSem4ExitState = sem_open("ThreadB_Sem4ExitState", O_CREAT, 0644, 0);
    ASSERT_NE(RCAgentObj.pSem4ExitState, nullptr);

    ChassicObj.pSem4ReadyState = sem_open("ThreadC_Sem4ReadyState", O_CREAT, 0644, 0);
    ASSERT_NE(ChassicObj.pSem4ReadyState, nullptr);
    ChassicObj.pSem4EnterRunning = sem_open("ThreadC_Sem4EnterRunning", O_CREAT, 0644, 0);
    ASSERT_NE(ChassicObj.pSem4EnterRunning, nullptr);
    ChassicObj.pSem4LeaveRunning = sem_open("ThreadC_Sem4LeaveRunning", O_CREAT, 0644, 0);
    ASSERT_NE(ChassicObj.pSem4LeaveRunning, nullptr);
    ChassicObj.pSem4ExitState = sem_open("ThreadC_Sem4ExitState", O_CREAT, 0644, 0);
    ASSERT_NE(ChassicObj.pSem4ExitState, nullptr);

    PalletObj.pSem4ReadyState = sem_open("ThreadD_Sem4ReadyState", O_CREAT, 0644, 0);
    ASSERT_NE(PalletObj.pSem4ReadyState, nullptr);
    PalletObj.pSem4EnterRunning = sem_open("ThreadD_Sem4EnterRunning", O_CREAT, 0644, 0);
    ASSERT_NE(PalletObj.pSem4EnterRunning, nullptr);
    PalletObj.pSem4LeaveRunning = sem_open("ThreadD_Sem4LeaveRunning", O_CREAT, 0644, 0);
    ASSERT_NE(PalletObj.pSem4LeaveRunning, nullptr);
    PalletObj.pSem4ExitState = sem_open("ThreadD_Sem4ExitState", O_CREAT, 0644, 0);
    ASSERT_NE(PalletObj.pSem4ExitState, nullptr);

    CDObj.pSem4ReadyState = sem_open("ThreadE_Sem4ReadyState", O_CREAT, 0644, 0);
    ASSERT_NE(CDObj.pSem4ReadyState, nullptr);
    CDObj.pSem4EnterRunning = sem_open("ThreadE_Sem4EnterRunning", O_CREAT, 0644, 0);
    ASSERT_NE(CDObj.pSem4EnterRunning, nullptr);
    CDObj.pSem4LeaveRunning = sem_open("ThreadE_Sem4LeaveRunning", O_CREAT, 0644, 0);
    ASSERT_NE(CDObj.pSem4LeaveRunning, nullptr);
    CDObj.pSem4ExitState = sem_open("ThreadE_Sem4ExitState", O_CREAT, 0644, 0);
    ASSERT_NE(CDObj.pSem4ExitState, nullptr);
    
    //-----------------------------------------------------------------------------------------------------------------
    //MAIN startup ThreadA/B/C/D/E
    int RetPSX = pthread_create(&VMainObj.ThreadID, NULL, __UT_ThreadA_ofVMainObj, &VMainObj);
    ASSERT_EQ(RetPSX, 0);

    RetPSX = pthread_create(&RCAgentObj.ThreadID, NULL, __UT_ThreadB_ofRCAgentObj, &RCAgentObj);
    ASSERT_EQ(RetPSX, 0);

    RetPSX = pthread_create(&ChassicObj.ThreadID, NULL, __UT_ThreadC_ofChassicObj, &ChassicObj);
    ASSERT_EQ(RetPSX, 0);

    RetPSX = pthread_create(&PalletObj.ThreadID, NULL, __UT_ThreadD_ofPalletObj, &PalletObj);
    ASSERT_EQ(RetPSX, 0);

    RetPSX = pthread_create(&CDObj.ThreadID, NULL, __UT_ThreadE_ofCDObj, &CDObj);
    ASSERT_EQ(RetPSX, 0);


    //-----------------------------------------------------------------------------------------------------------------
    //MAIN wait for ThreadA/B/C/D/E to be ReadyState
    sem_wait(VMainObj.pSem4ReadyState);
    sem_wait(RCAgentObj.pSem4ReadyState);
    sem_wait(ChassicObj.pSem4ReadyState);
    sem_wait(PalletObj.pSem4ReadyState);
    sem_wait(CDObj.pSem4ReadyState);

    //-----------------------------------------------------------------------------------------------------------------
    //enableEvtManger and MAIN notify ThreadA/B/C/D/E to enter RunningState
    TOS_Result_T Result = PLT_EVT_enableEvtManger();
    ASSERT_EQ(Result, TOS_RESULT_SUCCESS);//CheckPoint

    sem_post(VMainObj.pSem4EnterRunning);
    sem_post(RCAgentObj.pSem4EnterRunning);
    sem_post(ChassicObj.pSem4EnterRunning);
    sem_post(PalletObj.pSem4EnterRunning);
    sem_post(CDObj.pSem4EnterRunning);

    //-----------------------------------------------------------------------------------------------------------------
    //MAIN wait for ThreadA/B/C/D/E to be LeaveRunningState and disableEvtManger
    sem_wait(VMainObj.pSem4LeaveRunning);
    sem_wait(RCAgentObj.pSem4LeaveRunning);
    sem_wait(ChassicObj.pSem4LeaveRunning);
    sem_wait(PalletObj.pSem4LeaveRunning);
    sem_wait(CDObj.pSem4LeaveRunning);

    PLT_EVT_disableEvtManger();

    //-----------------------------------------------------------------------------------------------------------------
    //MAIN notify ThreadA/B/C/D/E to exit
    sem_post(VMainObj.pSem4ExitState);
    sem_post(RCAgentObj.pSem4ExitState);
    sem_post(ChassicObj.pSem4ExitState);
    sem_post(PalletObj.pSem4ExitState);
    sem_post(CDObj.pSem4ExitState);

    //-----------------------------------------------------------------------------------------------------------------
    //MAIN wait for ThreadA/B/C/D/E to be exited
    RetPSX = pthread_join(VMainObj.ThreadID, NULL);
    ASSERT_EQ(RetPSX, 0);

    RetPSX = pthread_join(RCAgentObj.ThreadID, NULL);
    ASSERT_EQ(RetPSX, 0);

    RetPSX = pthread_join(ChassicObj.ThreadID, NULL);
    ASSERT_EQ(RetPSX, 0);

    RetPSX = pthread_join(PalletObj.ThreadID, NULL);
    ASSERT_EQ(RetPSX, 0);

    RetPSX = pthread_join(CDObj.ThreadID, NULL);
    ASSERT_EQ(RetPSX, 0);

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