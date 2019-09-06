/*--------------------------------------------------------------------------*/
/*  Includes                                                                */
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <stdint.h>

#include "portmacro.h"
#include "uITRON.h"
#define __GLOBAL_DEFINITION_USER_DEFINITIONS__
#include "UserDefinitions.h"

/* ��ނȂ�FreeRTOS I/F���g���ꍇ�A�蓮�Œǉ� */
void vTaskStartScheduler(void);

/*--------------------------------------------------------------------------*/
/*  Macro and Typedef Definitions                                           */
/*--------------------------------------------------------------------------*/
/*--- Message ---*/
typedef struct {
    T_MSG       msg;
    ID          id;
    uint32_t    params[1];
} MESSAGE_ENTITY;

#define MESSAGE_SIZE    (64)
static MESSAGE_ENTITY   g_Messages[MESSAGE_SIZE];
static uint16_t         g_MessageIndex = 0;

/* ���b�Z�[�W�擾 */
static MESSAGE_ENTITY* GetUserMessage(void) {
    uint16_t index = g_MessageIndex++;
    index %= MESSAGE_SIZE;
    return &(g_Messages[index]);
}
/* ����͂܂��p�ӂ��Ă��Ȃ� */

/*--- Event ---*/
#define EVENT_ALL       (0x00FFFFFF)    /* FreeRTOS�d�l�����ŁA�ő��24-bit */
#define EVENT_CYCLIC    (0x00000001)
/* Reserved bit1 - bit15 */
#define EVENT_STOP      (0x00010000)
#define EVENT_RESTART   (0x00020000)

/*--- Task ---*/
#define CREATE_TASK(name, act, func) {   \
    T_CTSK ctsk_;   \
    ATR tskatr_ = TA_HLNG;  \
    if (act) tskatr_ |= TA_ACT; \
    ctsk_.tskatr = tskatr_;                 /* �^�X�N���� */ \
    ctsk_.exinf = NULL;                     /* �^�X�N�N�����p�����[�^ */ \
    ctsk_.task = func;                      /* �^�X�N�֐� */ \
    ctsk_.itskpri = TASK_PRI(name);         /* �^�X�N�̗D��x */   \
    ctsk_.stksz = sizeof(TASK_STACK(name)); /* �^�X�N�X�^�b�N�T�C�Y */   \
    ctsk_.stk = TASK_STACK(name);           /* �^�X�N�X�^�b�N�i�ÓI�Ɋm�ۂ��Ă����K�v����j */    \
    cre_tsk(TASK_ID(name), &ctsk_); \
}

/*--------------------------------------------------------------------------*/
/*  Prototypes                                                              */
/*--------------------------------------------------------------------------*/
/* Tasks */
static void SendTask(void*);
static void RecvTask(void*);
static void TimerTask(void*);
static void CommandTask(void*);
/* Handlers */
static void UserCyclicHandler(void*);

/*--------------------------------------------------------------------------*/
/*  Static Functions                                                        */
/*--------------------------------------------------------------------------*/

static ER CreateOsResources(void)
{
    T_CFLG cflg;
    cre_flg(FLAG_ID(SEND), &cflg);

    T_CMBX cmbx;
    cre_mbx(MAILBOX_ID(RECV), &cmbx);

    T_CCYC ccyc;
    ccyc.cychdr = UserCyclicHandler;
    ccyc.cyctim = 1000;
    cre_cyc(CYCLIC_ID(USER), &ccyc);

    return E_OK;
}

static ER CreateOsTasks(void)
{
    CREATE_TASK(SEND, TRUE, SendTask);
    CREATE_TASK(RECV, TRUE, RecvTask);

    CREATE_TASK(TIMER, FALSE, TimerTask);

    CREATE_TASK(COMMAND, TRUE, CommandTask);

    return E_OK;
}

static void PrintUsage()
{
    DEBUG_PRINT("");
    DEBUG_PRINT("uTITRON (based on FreeRTOS) Demo Program");
    DEBUG_PRINT("   1 - 15�̐��l    :   ���l���b�Z�[�W�̑���M");
    DEBUG_PRINT("   stop            :   ��M�^�X�N�̈ꎞ��~");
    DEBUG_PRINT("   start           :   ��M�^�X�N�̍ĊJ");
    DEBUG_PRINT("   cyclic          :   �����n���h���̊J�n/��~�i������Ԃ͒�~�j");
    DEBUG_PRINT("   exit            :   �f���̏I��");
    DEBUG_PRINT("   help            :   ���̃��b�Z�[�W��\��");
    DEBUG_PRINT("");
    DEBUG_PRINT("");
}

/*--------------------------------------------------------------------------*/
/*  Main and Task Functions                                                 */
/*--------------------------------------------------------------------------*/

int main(void)
{
    CreateOsResources();
    CreateOsTasks();

    /* �X�P�W���[�����O�J�n */
    /* �Ή�����uITRON�T�[�r�X�R�[���͂Ȃ��E�E�E */
    vTaskStartScheduler();

    /* �����ɂ͗��Ȃ��͂� */

    return 0;
}

static void SendTask(void* params)
{
    uint32_t count = 0;

    /* Just to remove compiler warning. */
    (void)params;

    /* �^�C�}�^�X�N�͒x���N�� */
    /* sta_tsk()���g�����������߂����ɂ������Ă��� */
    sta_tsk(TASK_ID(TIMER), NULL);

    while (1) {
        FLGPTN flgptn;
        wai_flg(FLAG_ID(SEND), EVENT_ALL, TWF_ORW, &flgptn);
        DEBUG_PRINT("[%s]: RECV EVENT (%08X)", __func__, flgptn);

        /* Suspend Task */
        if (flgptn & EVENT_STOP) {
            sus_tsk(TASK_ID(RECV));
            continue;
        }
        /* Resume Task */
        if (flgptn & EVENT_RESTART) {
            rsm_tsk(TASK_ID(RECV));
            continue;
        }

        /* Cyclic Event */
        if (flgptn & EVENT_CYCLIC) {
            static int32_t expired = 0;
            DEBUG_PRINT("[%s]: Cyclic Timer Expired (%d)", __func__, ++expired);
            continue;
        }

        MESSAGE_ENTITY* ent;
        ent = GetUserMessage();
        /* Numeric Event */
        /* ���b�Z�[�WID�̓C�x���g���i���l�ɖ߂��āj�ݒ� */
        /* �p�����[�^�ɃJ�E���^��n�� */
        int32_t num = 0;
        for (num = 0; !((flgptn >> num) & 1) ; num++) ;
        ent->id = (ID)num;
        ent->params[0] = ++count;

        snd_mbx(MAILBOX_ID(RECV), (T_MSG *)ent);
    }
}

static void RecvTask(void* params)
{
    /* Just to remove compiler warning. */
    (void)params;

    while (1) {
        T_MSG *msg;
        rcv_mbx(MAILBOX_ID(RECV), &msg);
        MESSAGE_ENTITY* ent = (MESSAGE_ENTITY*)(msg);
        DEBUG_PRINT("[%s]: RECV MESSAGE (%d,%d)", __func__, ent->id, ent->params[0]);
    }
}

static void TimerTask(void *params)
{
    /* Just to remove compiler warning. */
    (void)params;

    while (1) {
        RELTIM tim = 3000;
        DEBUG_PRINT("[%s]: DELAY %dms", __func__, tim);
        dly_tsk(tim);
    }
}

static void CommandTask(void* params)
{
    char str[256];

    /* Just to remove compiler warning. */
    (void)params;

    while (1) {

        /* gets_s()�ŉ��s���͂܂Ńu���b�N���Ă��܂����A�����RTOS�̂���@�ɔ����Ă���B */
        /* �i�{���̓C�x���g�҂��Ȃ�A���b�Z�[�W�҂��Ȃ�̃T�[�r�X�R�[�����ĂԂׂ��ł��낤�j */
        gets_s(str, sizeof(str));

        /* Command Parser */
        int32_t num = atol(str);
        /* Numeric Event (1 To 15) */
        if ((0 < num) && (num < 16)) {
            set_flg(FLAG_ID(SEND), (1 << num));
        }
#define EQUALS_(x) (0 == strcmp(str, #x))
        /* Suspend */
        else if (EQUALS_(stop)) {
            set_flg(FLAG_ID(SEND), EVENT_STOP);
        }
        /* Resume */
        else if (EQUALS_(start)) {
            set_flg(FLAG_ID(SEND), EVENT_RESTART);
        }
        /* Cyclic */
        else if (EQUALS_(cyclic)) {
            T_RCYC rcyc;
            ref_cyc(CYCLIC_ID(USER), &rcyc);
            if (!(rcyc.cycstat))    sta_cyc(CYCLIC_ID(USER));
            else                    stp_cyc(CYCLIC_ID(USER));
        }
        /* Print Usage */
        else if (EQUALS_(help)) {
            sus_tsk(TASK_ID(TIMER));
            PrintUsage();
            rsm_tsk(TASK_ID(TIMER));
        }
        /* Exit */
        else if (EQUALS_(exit)) {
            /* �����I�� */
            exit(1);
        }
#undef EQUALS_
    }
}

/*--------------------------------------------------------------------------*/
/*  Handlers                                                                */
/*--------------------------------------------------------------------------*/

static void UserCyclicHandler(void* params)
{
    /* Just to remove compiler warning. */
    (void)params;

    iset_flg(FLAG_ID(SEND), EVENT_CYCLIC);
}
