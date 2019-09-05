/*--------------------------------------------------------------------------*/
/*  Preprocessors                                                           */
/*--------------------------------------------------------------------------*/
#undef EXTERN
#ifdef __GLOBAL_DEFINITION_USER_DEFINITIONS__
#define EXTERN
#else
#define EXTERN extern
#endif

/*--------------------------------------------------------------------------*/
/*  Tasks                                                                   */
/*--------------------------------------------------------------------------*/
/* Task ID */
enum {
    TASK_ID(SEND),
    TASK_ID(RECV),
    TASK_ID(TIMER),
    TASK_ID(COMMAND),

/* �����܂Ń��[�U�[��` */
/* �C���f�b�N�X�Ƃ��Ă��g�p����̂ŁA0����̘A���l�Ƃ��Ē�`���邱�� */
/* �܂��A�ȉ��͏����Ȃ����� */
    TASK_ID(MAX),
};

/* Task Priority */
#define TASK_PRI_HIGH   (7)     // See configMAX_PRIORITIES
#define TASK_PRI_MIDDLE (5)
#define TASK_PRI_LOW    (3)
enum {
    TASK_PRI(SEND)      = TASK_PRI_MIDDLE,
    TASK_PRI(RECV)      = TASK_PRI_MIDDLE,
    TASK_PRI(TIMER)     = TASK_PRI_HIGH,
    TASK_PRI(COMMAND)   = TASK_PRI_LOW,
};

/* Task Stack */
EXTERN STACK_TYPE   TASK_STACK(SEND)[1024];
EXTERN STACK_TYPE   TASK_STACK(RECV)[1024];
EXTERN STACK_TYPE   TASK_STACK(TIMER)[512];
EXTERN STACK_TYPE   TASK_STACK(COMMAND)[1024];

/*--------------------------------------------------------------------------*/
/*  Resources                                                               */
/*--------------------------------------------------------------------------*/
/*--- Event Flag ---*/
/* Flag ID */
enum {
    FLAG_ID(SEND),

/* Task ID�Ɠ��l */
    FLAG_ID(MAX),
};

/*--- Mail Box ---*/
/* Mail Box ID */
enum {
    MAILBOX_ID(RECV),

    /* Task ID�Ɠ��l */
    MAILBOX_ID(MAX),
};

/*--- Cyclic Handler ---*/
/* Cyclic ID */
enum {
    CYCLIC_ID(USER),

    /* Task ID�Ɠ��l */
    CYCLIC_ID(MAX),
};

/*--------------------------------------------------------------------------*/
/*  For Debug                                                               */
/*--------------------------------------------------------------------------*/
#define DEBUG_PRINT(fmt, ...)    printf(fmt "\r\n", __VA_ARGS__)
