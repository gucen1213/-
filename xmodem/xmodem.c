#include "xmodem.h"
#include "usart.h"

#define this (*ptThis)

#define XMODEM_1K_HEAD    0x02;
#define XMODEM_128_HEAD   0x01;

#define TIME_OUT_COUNT   50000ul
#define MAX_SEND_COUNT   5
#define XMODEM_1K_SIZE   1024
#define XMODEM_128_SIZE  128

bool xmodem_start(xmodem_t *ptThis)
{
    if(NULL == ptThis){
        return false;
    }
    SET_EVENT(&(this.tStart));
    return true;
}

bool xmodem_stop(xmodem_t *ptThis)
{
    if(NULL == ptThis){
        return false;
    }
    SET_EVENT(&(this.tStop));
    return true;
}

void xmodem_insert_1ms_timer_handler(xmodem_t *ptThis)
{
    if(this.hwTimerCount == 0){
        this.hwTimerCount = 0;
    } else {
        this.hwTimerCount--;
    }
}

//Xmodem初始化
bool xmodem_init(xmodem_t *ptThis,xmodem_check_sum_mode_t tCheckSUM,
uint16_t hwDataSize,xmodem_report_t *fnReport,stream_in_t *fnIn,stream_out_t *fnOut)
{
    if((NULL == ptThis) || (NULL == fnReport) || (NULL == fnIn) || (NULL == fnOut)){
        return false;
    }
    if((XMODEM_1K_SIZE != hwDataSize) && (XMODEM_128_SIZE != hwDataSize)){
        return false;
    }
    if(XMODEM_1K_SIZE == hwDataSize){
        this.chHead = XMODEM_1K_HEAD;
    }else{
        this.chHead = XMODEM_128_HEAD;
    }
    this.bCrcMode = tCheckSUM;       //CRC_mode
    this.hwDataSize = hwDataSize;
    this.fnReport = fnReport;
    this.Pipe.fnIn = fnIn;
    this.Pipe.fnOut = fnOut;
    INIT_EVENT(&this.tStart,false,false);  //自动复位
    INIT_EVENT(&this.tStop,false,false);   //自动复位
    this.hwDataNumCount = 0;
    this.hwDataTimeCount = 0;
    this.ID.hwCompareID = 1;
    this.Check.chVerify[0] = 0;
    this.Check.chVerify[0] = 0;
    this.Check.hwCompareCheck = 0;
    this.State.tReadByteState = START_READ_BYTE;
    this.State.tReadCrcState = START_READ_CRC;
    this.State.tReadDataState = START_READ_DATA;
    this.State.tReadFrameState = START_READ_FRAME;
    this.State.tXmodemState = START_XMODEM_TASK;
    return true;
}

//CRC16 CCIT算法
static uint16_t CRC_16_CCITT(uint16_t hwCRCValue, uint8_t chData)
{ 
    uint8_t chTemp = 8; 

    hwCRCValue = hwCRCValue ^ chData << 8; 

    do 
    { 
        if (hwCRCValue & 0x8000) {
            hwCRCValue = hwCRCValue << 1 ^ 0x1021; 
        } else {
            hwCRCValue = hwCRCValue << 1; 
        }        
    }while (--chTemp); 

    return hwCRCValue; 
} 

//接收一个字节
#define XMODE_READ_A_BYTE_RESET_FSM()      \
    do {\
        this.State.tReadByteState = START_READ_BYTE;\
    } while(0);
static xmodemRunstate_t xmode_read_byte(xmodem_t *ptThis,uint8_t *pchByte)
{
    uint8_t chChar;
    if((NULL == pchByte) || (NULL == ptThis)){
        this.State.tXmodemRunState = FSM_STATE_ERR;
        return this.State.tXmodemRunState;
    }
    switch(this.State.tReadByteState){
        case START_READ_BYTE:
            this.hwDataTimeCount = 0;
            this.State.tReadByteState = WAIT_CHAR;
//            break;
        case WAIT_CHAR:
            if(this.Pipe.fnIn(&chChar)){
                *pchByte = chChar;
                XMODE_READ_A_BYTE_RESET_FSM();
                this.State.tXmodemRunState = FSM_REC_OK;
                return this.State.tXmodemRunState;
            }else{
                this.State.tReadByteState = DELAY;
            }
            break;
        case DELAY:
            this.hwDataTimeCount++;
            if(this.hwDataTimeCount >= TIME_OUT_COUNT){
                XMODE_READ_A_BYTE_RESET_FSM();
                this.State.tXmodemRunState = FSM_REC_TIME_OUT;
                return this.State.tXmodemRunState;
            }else{
                this.State.tReadByteState = WAIT_CHAR;
            }
            break;
    }
    this.State.tXmodemRunState = FSM_REC_ON_GOING;
    return this.State.tXmodemRunState;
}

//接收数据
#define  XMODE_REC_DATA_RESET_FSM()      \
    do {\
        this.State.tReadDataState = START_READ_DATA;\
    } while(0);
static xmodemRunstate_t xmodem_rec_data(xmodem_t *ptThis,uint16_t *phwCrcVal)
{

    uint8_t chChar;

    if((NULL == ptThis) || (NULL == phwCrcVal)){ 
        return FSM_STATE_ERR;
    }
    
    switch(this.State.tReadDataState){
        case START_READ_DATA:
            this.hwDataNumCount = 0;
//        break;
        case REC_BYTE:
            this.State.tXmodemRunState = xmode_read_byte(ptThis,&chChar);
            if(FSM_REC_OK == this.State.tXmodemRunState){    //read OK
                this.pchBuffer[this.hwDataNumCount] = chChar;
                if(this.bCrcMode){   //CRC
                    *phwCrcVal = CRC_16_CCITT(*phwCrcVal,chChar);
                }else{
                    *phwCrcVal += chChar;
                }
                this.hwDataNumCount++;
                this.State.tReadDataState = CHECK_CPL;
            }else if(FSM_REC_TIME_OUT == this.State.tXmodemRunState){
                XMODE_REC_DATA_RESET_FSM();
                this.State.tXmodemRunState = FSM_REC_FRAME_TIME_OUT;
                return this.State.tXmodemRunState;
            }
            break;
        case CHECK_CPL:
            if(this.hwDataNumCount >= this.hwDataSize){
                this.hwDataNumCount = 0;
                XMODE_REC_DATA_RESET_FSM();
                this.State.tXmodemRunState = FSM_REC_OK;
                return this.State.tXmodemRunState;
            }else{
                this.State.tReadDataState = REC_BYTE;
            }
            break;
    }
    this.State.tXmodemRunState = FSM_REC_FRAME_ON_GOING;
    return this.State.tXmodemRunState;
}

//接收CRC
#define XMODE_REC_CRC_RESET_FSM()      \
    do {\
        this.State.tReadCrcState = START_READ_CRC;\
    } while(0);
static xmodemRunstate_t xmodem_rec_crc(xmodem_t *ptThis)
{  
    uint8_t chChar;
    if(NULL == ptThis){
        this.State.tXmodemRunState = FSM_STATE_ERR;
        return this.State.tXmodemRunState;
    }
    switch(this.State.tReadCrcState){
        case START_READ_CRC:
            this.State.tReadCrcState = REC_VERIFY1;
//            break;
        case REC_VERIFY1:
            this.State.tXmodemRunState = xmode_read_byte(ptThis,&chChar);
            if(FSM_REC_OK == this.State.tXmodemRunState){
                this.Check.chVerify[0] = chChar;
                if(this.bCrcMode){    //CRC
                    this.State.tReadCrcState = REC_VERIFY2;
                }else{     
                    XMODE_REC_CRC_RESET_FSM();  //SUM
                    this.State.tXmodemRunState = FSM_REC_OK;
                    return this.State.tXmodemRunState;
                }
            }else if(FSM_REC_TIME_OUT == this.State.tXmodemRunState){
                XMODE_REC_CRC_RESET_FSM();
                this.State.tXmodemRunState = FSM_REC_FRAME_TIME_OUT;
                return this.State.tXmodemRunState;
            }
            break;
        case REC_VERIFY2:
            this.State.tXmodemRunState = xmode_read_byte(ptThis,&chChar);
            if(FSM_REC_OK == this.State.tXmodemRunState){
                this.Check.chVerify[1] = chChar;
                XMODE_REC_CRC_RESET_FSM();  //SUM
                this.State.tXmodemRunState = FSM_REC_OK;
                return FSM_REC_OK;                
            }else if(FSM_REC_TIME_OUT == this.State.tXmodemRunState){
                XMODE_REC_CRC_RESET_FSM();
                this.State.tXmodemRunState = FSM_REC_FRAME_TIME_OUT;
                return this.State.tXmodemRunState;
            }            
            break;
    }
    this.State.tXmodemRunState = FSM_REC_FRAME_ON_GOING;
    return this.State.tXmodemRunState;
}

//接收一帧数据
#define XMODEM_REC_FRAME_RESET_FSM()      \
    do {\
        this.State.tReadFrameState = START_READ_FRAME;\
    } while(0);
static xmodemRunstate_t xmode_rec_frame(xmodem_t *ptThis)
{
    uint8_t chChar;
    
    if(NULL == ptThis){
        this.State.tXmodemRunState = FSM_STATE_ERR;
        return this.State.tXmodemRunState;
    }
    
    switch(this.State.tReadFrameState){
        case START_READ_FRAME:
            this.State.tReadFrameState = REC_HEAD;
//            break;
        case REC_HEAD:
            this.State.tXmodemRunState = xmode_read_byte(ptThis,&chChar);
            if(this.State.tXmodemRunState == FSM_REC_OK){
                if(this.chHead == chChar){
                    this.pchBuffer = this.fnReport(XMODEM_START,this.pchBuffer);
                    this.State.tReadFrameState = REC_BLK;
                }else if(EOT == chChar){        //接收到终止信号
                    this.ID.hwCompareID = 1;
                    this.pchBuffer = this.fnReport(XMODEM_EOT,this.pchBuffer);
                    XMODEM_REC_FRAME_RESET_FSM();
                    this.State.tXmodemRunState = FSM_REC_FRAME_EOT;
                    return this.State.tXmodemRunState;
                }
            }else if(this.State.tXmodemRunState == FSM_REC_TIME_OUT){
                XMODEM_REC_FRAME_RESET_FSM();
                this.State.tXmodemRunState = FSM_REC_FRAME_NO_CHAR;
                return this.State.tXmodemRunState;   //head timeout无响应
            }
            break;
        case REC_BLK:
            this.State.tXmodemRunState = xmode_read_byte(ptThis,&chChar);
            if(FSM_REC_OK == this.State.tXmodemRunState){
                this.ID.chBlk = chChar;
                this.State.tReadFrameState = REC_NBLK;
            }else if(this.State.tXmodemRunState == FSM_REC_TIME_OUT){
                XMODEM_REC_FRAME_RESET_FSM();
                this.State.tXmodemRunState = FSM_REC_FRAME_TIME_OUT;
                return this.State.tXmodemRunState;
            }
            break;
        case REC_NBLK:
            this.State.tXmodemRunState = xmode_read_byte(ptThis,&chChar);
            if(FSM_REC_OK == this.State.tXmodemRunState){
                this.ID.chNBlk = chChar;
                this.State.tReadFrameState = REC_DATA;
            }else if(this.State.tXmodemRunState == FSM_REC_TIME_OUT){
                XMODEM_REC_FRAME_RESET_FSM();
                this.State.tXmodemRunState = FSM_REC_FRAME_TIME_OUT;
                return this.State.tXmodemRunState;
            }
            break;
        case REC_DATA:
            this.State.tXmodemRunState = xmodem_rec_data(ptThis,&this.Check.hwCompareCheck);   
            if(this.State.tXmodemRunState == FSM_REC_OK){
                this.State.tReadFrameState = REC_CRC;
            }else if(this.State.tXmodemRunState == FSM_REC_FRAME_TIME_OUT){
                XMODEM_REC_FRAME_RESET_FSM();
                this.State.tXmodemRunState = FSM_REC_FRAME_TIME_OUT;
                return this.State.tXmodemRunState;                
            }
            break;
        case REC_CRC: 
            this.State.tXmodemRunState = xmodem_rec_crc(ptThis);
            if(this.State.tXmodemRunState == FSM_REC_OK){
                this.State.tReadFrameState = CHECK_REC;
            }else if(this.State.tXmodemRunState == FSM_REC_TIME_OUT){
                XMODEM_REC_FRAME_RESET_FSM();
                this.State.tXmodemRunState = FSM_REC_FRAME_TIME_OUT;
                return this.State.tXmodemRunState;  
            }
            break;
        case CHECK_REC:      //check frame
            if(this.ID.chBlk == this.ID.hwCompareID-1){
                XMODEM_REC_FRAME_RESET_FSM();
                this.State.tXmodemRunState = FSM_REC_FRAME_BLK_REPEAT;
                return this.State.tXmodemRunState;      
            }
            if(this.ID.chBlk != this.ID.hwCompareID){
                XMODEM_REC_FRAME_RESET_FSM();
                this.State.tXmodemRunState = FSM_REC_FRAME_BLK_ERR;
                return this.State.tXmodemRunState;
            }
            if(0xFF != (this.ID.chNBlk + this.ID.chBlk)){
                XMODEM_REC_FRAME_RESET_FSM();
                this.State.tXmodemRunState = FSM_REC_FRAME_NBLK_ERR;
                return this.State.tXmodemRunState;
            }
            if(this.bCrcMode){
                if(this.Check.hwCompareCheck != (((uint16_t)(this.Check.chVerify[0])) << 8) 
                                                + ((uint16_t)(this.Check.chVerify[1]))){
                    XMODEM_REC_FRAME_RESET_FSM();
                    this.State.tXmodemRunState = FSM_REC_FRAME_CHECK_ERR;
                    return this.State.tXmodemRunState;
                }
            }else{
                if(this.Check.hwCompareCheck != this.Check.chVerify[0]){
                    XMODEM_REC_FRAME_RESET_FSM();
                    this.State.tXmodemRunState = FSM_REC_FRAME_CHECK_ERR;
                    return this.State.tXmodemRunState;
                }
            }
            this.Check.hwCompareCheck = 0;
            this.ID.hwCompareID++;
            XMODEM_REC_FRAME_RESET_FSM();
            this.State.tXmodemRunState = FSM_REC_FRAME_OK;
            return this.State.tXmodemRunState;
//            break;
    }
    this.State.tXmodemRunState = FSM_REC_FRAME_ON_GOING;
    return this.State.tXmodemRunState;
}

#define CRC_C                 0x43
#define XMODE_PROCESS_RESET_FSM()      \
    do {\
        this.State.tXmodemState = START_XMODEM_TASK;\
    } while(0);
fsm_rt_t xmodem_task(xmodem_t *ptThis)
{
    if(NULL == ptThis){
        return fsm_rt_err;
    }
	
    switch(this.State.tXmodemState){
        case START_XMODEM_TASK:
            this.State.tXmodemRunState = FSM_REC_FRAME_ON_GOING;
            this.State.tXmodemState = WAIT_START;
            break;

        case WAIT_START:
            if(WAIT_EVENT(&(this.tStart))){   //等待执行
                this.ID.hwCompareID = 1;
                this.State.tXmodemState = SEND_BYTE;
                if(this.bCrcMode){
                    this.chSendByte = CRC_C;
                }else{
                    this.chSendByte = NAK;
                }
            }            
            break;

        case SEND_BYTE:
            if(WAIT_EVENT(&(this.tStop))){    //等待停止
                this.ID.hwCompareID = 1;
                this.State.tXmodemRunState = FSM_REC_FRAME_CAN;
                this.chSendByte = CAN;
            }
            if(this.Pipe.fnOut(this.chSendByte)){
                if(this.State.tXmodemRunState == FSM_REC_FRAME_CAN){
                    XMODE_PROCESS_RESET_FSM();
                    return fsm_rt_cpl;                    
                }else if(this.State.tXmodemRunState == FSM_REC_FRAME_EOT){
                    XMODE_PROCESS_RESET_FSM();
                    return fsm_rt_cpl;
                }else{
                    this.State.tXmodemState = REC_PACKET;
                }                
            }
            break;
        
        case REC_PACKET:
            this.State.tXmodemRunState = xmode_rec_frame(ptThis);
            switch(this.State.tXmodemRunState){
                case FSM_REC_FRAME_OK:
                    this.pchBuffer = this.fnReport(PACKAGE_OK,this.pchBuffer);
                    this.chSendByte = ACK;
                    this.State.tXmodemState = SEND_BYTE;
                    break;
                case FSM_REC_FRAME_EOT:
                    this.pchBuffer = this.fnReport(XMODEM_EOT,this.pchBuffer);
                    this.chSendByte = ACK;
                    this.State.tXmodemState = SEND_BYTE; 
                    break;
                case FSM_REC_FRAME_TIME_OUT: 
                    this.pchBuffer = this.fnReport(TIMER_OUT_1S,this.pchBuffer);
                    this.chSendByte = NAK; 
                    this.State.tXmodemState = SEND_BYTE;
                    break;
                case FSM_REC_FRAME_BLK_ERR:      //无条件终止
                    this.pchBuffer = this.fnReport(XMODEM_CAN,this.pchBuffer);
                    this.chSendByte = CAN;    
                    this.State.tXmodemState = SEND_BYTE;                    
                    break;
                case FSM_REC_FRAME_NBLK_ERR:
                    this.pchBuffer = this.fnReport(INVALID_SN,this.pchBuffer);
                    this.chSendByte = NAK;          //发送重传
                    this.State.tXmodemState = SEND_BYTE;                    
                    break;
                case FSM_REC_FRAME_BLK_REPEAT:   //重复包
                    this.chSendByte = ACK;
                    this.State.tXmodemState = SEND_BYTE;
                    break;
                case FSM_REC_FRAME_CHECK_ERR:
                    this.pchBuffer = this.fnReport(CHECK_ERR,this.pchBuffer);
                    this.chSendByte = NAK;
                    this.State.tXmodemState = SEND_BYTE;               
                    break;
                case FSM_REC_FRAME_CAN:
                    this.chSendByte = CAN;      //发送CAN
                    this.State.tXmodemState = SEND_BYTE; 
                    break;
                case FSM_REC_FRAME_NO_CHAR:
                    this.pchBuffer = this.fnReport(TIMER_OUT_10S,this.pchBuffer);
                    if(this.bCrcMode){
                        this.chSendByte = CRC_C;
                    }else{  
                        this.chSendByte = NAK;
                    }
                    this.State.tXmodemState = WAIT_START;
                    break;
            }
            break;
            
        case CHECK_COUNT: 
            this.chSendCount++;
            if(this.chSendCount >= MAX_SEND_COUNT){
                this.pchBuffer = this.fnReport(TIMER_OUT_10S,this.pchBuffer);
                XMODE_PROCESS_RESET_FSM();
                return fsm_rt_err;                  
            }else{
                this.State.tXmodemState = SEND_BYTE;
            }
            break;               
    }
    return fsm_rt_on_going;
}

