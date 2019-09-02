#ifndef _XMODEM_H_
#define _XMODEM_H_

#   include "system.h"
#   include "oopc.h"
#   include "event.h"

#define SOH                             0x01
#define STX                             0x02
#define EOT                             0x04
#define ACK                             0x06
#define NAK                             0x15
#define CAN                             0x18

typedef enum{
    XMODEM_START,
    XMODEM_EOT,
    XMODEM_CAN,
    TIMER_OUT_1S,
    TIMER_OUT_10S,
    CHECK_ERR,
    INVALID_SN,
    PACKAGE_OK,
}xmodem_event_t;

typedef enum{
    FSM_STATE_ERR               = -1,
    FSM_REC_OK                  =  0,
    FSM_REC_TIME_OUT            =  1,
    FSM_REC_ON_GOING            =  2,
    FSM_REC_FRAME_OK            =  3,
    FSM_REC_FRAME_ON_GOING      =  4,
    FSM_REC_FRAME_TIME_OUT      =  5,
    FSM_REC_FRAME_EOT           =  6,
    FSM_REC_FRAME_HEAD_ERR      =  7,
    FSM_REC_FRAME_BLK_ERR       =  8,
    FSM_REC_FRAME_NBLK_ERR      =  9,
    FSM_REC_FRAME_BLK_REPEAT    =  10,   //÷ÿ∏¥÷°
    FSM_REC_FRAME_CHECK_ERR     =  11,
    FSM_REC_FRAME_ERR           =  12,
    FSM_REC_FRAME_CAN           =  13,
    FSM_REC_FRAME_NO_CHAR       =  14,
}xmodemRunstate_t;

typedef uint8_t* xmodem_report_t(xmodem_event_t tEvent,uint8_t *ptBuffer);
typedef bool stream_in_t(uint8_t* pchChar);
typedef bool stream_out_t(uint8_t chChar);

typedef enum {
    XMODEM_CHECK_SUM = 0,
    XMODEM_ChECK_CRC,
}xmodem_check_sum_mode_t;

typedef enum {
    START_READ_BYTE,
    WAIT_CHAR,
    DELAY,
}ReadByteState_t;

typedef enum {
    START_READ_DATA,
    REC_BYTE,
    CHECK_CPL,
}ReadDataState_t;

typedef enum {
    START_READ_CRC,
    REC_VERIFY1,
    REC_VERIFY2,
}ReadCrcState_t;   

typedef enum {
    START_READ_FRAME,
    REC_HEAD,
    REC_BLK,
    REC_NBLK,
    REC_DATA,
    REC_CRC,
    CHECK_REC,
}ReadFrameState_t;  
      
typedef enum {
    START_XMODEM_TASK,
    WAIT_START,
    SEND_BYTE,
    REC_PACKET,
    CHECK_COUNT,
}XmodemState_t;

DEF_STRUCT(xmodem_t)
    uint16_t             hwDataSize;
    event_t              tStart;
    event_t              tStop;
    uint8_t              chSendCount;
    xmodem_check_sum_mode_t  bCrcMode;
    xmodem_report_t*     fnReport;
    struct {
        stream_in_t*     fnIn;
        stream_out_t*    fnOut;
    } Pipe;
    uint32_t             hwTimerCount;   
    uint8_t              chSendByte;
    struct{
        uint8_t          chVerify[2];   
        uint16_t         hwCompareCheck;             
    } Check;
    struct{
        uint8_t          chBlk;  
        uint8_t          chNBlk;
        uint8_t          hwCompareID;   
    } ID;
    uint8_t              *pchBuffer;
    uint8_t              chHead;
    uint16_t             hwDataTimeCount;
    uint16_t             hwDataNumCount;
    struct{
        ReadByteState_t  tReadByteState;
        ReadDataState_t  tReadDataState;
        ReadCrcState_t   tReadCrcState;
        ReadFrameState_t tReadFrameState;
        XmodemState_t    tXmodemState;
        xmodemRunstate_t tXmodemRunState;
    } State;
END_DEF_STRUCT(xmodem_t)
 
extern fsm_rt_t xmodem_task(xmodem_t *ptXmodem);
extern bool     xmodem_start(xmodem_t *ptThis);
extern bool     xmodem_stop(xmodem_t *ptThis);
extern bool     xmodem_init(xmodem_t *ptThis,
                            xmodem_check_sum_mode_t tCheckSUM,
                            uint16_t hwDataSize,
                            xmodem_report_t *fnReport,
                            stream_in_t *fnIn,
                            stream_out_t *fnOut);
extern void     xmodem_insert_1ms_timer_handler(xmodem_t *ptThis);

#endif
