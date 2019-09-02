#include "stm32f10x.h"
#include "system.h"
#include "stdlib.h"
#include ".\critical\critical.h"
#include ".\event\event.h"
#include ".\mail\mail.h"
#include "queue.h"
#include ".\check_string\check_string.h"
#include ".\msg_map\msg_map.h"
#include ".\print_string\print_string.h"
#include "stream_in_out.h"
#include ".\check_list\check_list.h"
#include "xmodem.h"

static byte_queue_t   s_tFIFOin;
static byte_queue_t   s_tFIFOout;

static xmodem_t s_tXmodem;

static event_t s_tF1Event;
static event_t s_tUPEvent;
static event_t s_tDOWNEvent;

static event_t s_tAppleEvent;
static event_t s_tOrangeEvent;
static event_t s_tWorldEvent;

static uint8_t s_chInBuffer[200] = {0};   //input buffer
static uint8_t s_chOutBuffer[16] = {0};  //output buffer

static pipe_io_t s_tPipe;

typedef bool stream_in_t(uint8_t *pchByte);
typedef bool stream_out_t(uint8_t chByte);


bool get_fifoin_byte(uint8_t *pchByte)
{
    if(NULL == pchByte){
        return false;
    }
    return PEEK_BYTE_QUEUE(&s_tFIFOin,pchByte);
}

bool pop_fifoout_byte(uint8_t chByte)
{
    return ENQUEUE_BYTE(&s_tFIFOout,chByte);
}

static bool interface_init(pipe_io_t *ptPipe)
{
    if(NULL == ptPipe){
        return false;
    }
    ptPipe->fnByteIn = &get_fifoin_byte;
    ptPipe->fnByteOut = &pop_fifoout_byte;
    return true;
}

#define SERIAL_IN_TASK_RESET_FSM()      \
    do {\
        s_tState = START;\
    } while(0);
fsm_rt_t serial_in_task(void)
{
    static enum{
        START = 0,
        INPUT,
    }s_tState = START;    
    uint8_t chChar = 0;    
    switch(s_tState){
        case START:
            s_tState = INPUT;
//             break;        
        case INPUT:          
            if(serial_in(&chChar)) {      
                ENQUEUE_BYTE(&s_tFIFOin, chChar);
                SERIAL_IN_TASK_RESET_FSM();
                return fsm_rt_cpl;               
            }
    }   
    return fsm_rt_on_going;
}

#define SERIAL_OUT_TASK_RESET_FSM()      \
    do {\
        s_tState = START;\
    } while(0);
fsm_rt_t serial_out_task(void)
{
    static enum{
        START = 0,
        WAIT_BYTE,
        OUTPUT,
    }s_tState = START;    
    static uint8_t s_chChar = 0;
    switch(s_tState) {
        case START:
            s_tState = WAIT_BYTE;
//             break;       
        case WAIT_BYTE:          
            if(DEQUEUE_BYTE(&s_tFIFOout, &s_chChar)){                 
                s_tState = OUTPUT; 
            } 
            break;           
        case OUTPUT:
            if(serial_out(s_chChar)) {            
                SERIAL_OUT_TASK_RESET_FSM();
                return fsm_rt_cpl;               
            }        
    }    
    return fsm_rt_on_going;
}

bool stream_in(uint8_t *pchByte)
{
    if(NULL == pchByte){
        return false;
    }
    return PEEK_BYTE_QUEUE(&s_tFIFOin,pchByte);
}

bool stream_out(uint8_t chByte)
{
    return ENQUEUE_BYTE(&s_tFIFOout,chByte);
}

simple_fsm(print_Apple,
    def_params(
        print_str_t tApple;
    )
)

implement_fsm(print_Apple)
    def_states(INIT,PRINT)
    body(
        state( INIT,
            if(printf_string_init(&this.tApple,"apple\r\n")){
                update_state_to(PRINT);
            }
            fsm_on_going();
        )
            
        state( PRINT,
            if(fsm_rt_cpl == printf_string(&this.tApple)){
                fsm_cpl();
            }
            fsm_on_going();
        )
    )
    
static fsm(print_Apple) s_fsmPrintApple;
            
#define PRINT_ORANGE_RESET_FSM()  \
    do{                    \
        s_tState = START;  \
    }while(0)
static fsm_rt_t print_Orange(void)
{
    static enum {
        START,
        PRINT
    }s_tState = START;
    static print_str_t s_tOrange;
    switch(s_tState){
        case START:
            printf_string_init(&s_tOrange,"orange\r\n");
            s_tState = PRINT;
            break;
        case PRINT:
            if(fsm_rt_cpl == printf_string(&s_tOrange)){
                PRINT_ORANGE_RESET_FSM();
                return fsm_rt_cpl;
            }
            break;
    }
    return fsm_rt_on_going;
}

#define PRINT_WORLD_RESET_FSM()  \
    do{                    \
        s_tState = START;  \
    }while(0)
static fsm_rt_t print_World(void)
{
    static enum {
        START,
        PRINT
    }s_tState = START;
    static print_str_t s_tWorld;
    switch(s_tState){
        case START:
            printf_string_init(&s_tWorld,"world\r\n");
            s_tState = PRINT;
            break;
        case PRINT:
            if(fsm_rt_cpl == printf_string(&s_tWorld)){
                PRINT_ORANGE_RESET_FSM();
                return fsm_rt_cpl;
            }
            break;
    }
    return fsm_rt_on_going;
}

#define TASK_D_RESET_FSM()  \
    do{                    \
        s_tState = START;  \
    }while(0)
static fsm_rt_t task_d(void)
{
    static enum {
        START,
        WAIT_EVENT,
        PRINT
    }s_tState = START;
    switch(s_tState){
        case START:
            s_tState = WAIT_EVENT;
     //       break;
        case WAIT_EVENT:
            if(WAIT_EVENT(&s_tAppleEvent)){
                s_tState = PRINT;
            }
            break;
        case PRINT:
//            if(fsm_rt_cpl == print_Apple()){
            if(fsm_rt_cpl == call_fsm(print_Apple,&s_fsmPrintApple)){
                TASK_D_RESET_FSM();
                return fsm_rt_cpl;
            }
            break;
    }
    return fsm_rt_on_going;
}


#define TASK_E_RESET_FSM()  \
    do{                    \
        s_tState = START;  \
    }while(0)
static fsm_rt_t task_e(void)
{
    static enum {
        START,
        WAIT_EVENT,
        PRINT
    }s_tState = START;
    switch(s_tState){
        case START:
            s_tState = WAIT_EVENT;
 //           break;
        case WAIT_EVENT:
            if(WAIT_EVENT(&s_tOrangeEvent)){
                s_tState = PRINT;
            }            
            break;
        case PRINT:
            if(fsm_rt_cpl == print_Orange()){
                TASK_E_RESET_FSM();
                return fsm_rt_cpl;
            }
            break;
    }
    return fsm_rt_on_going;
}

#define TASK_F_RESET_FSM()  \
    do{                    \
        s_tState = START;  \
    }while(0)
static fsm_rt_t task_f(void)
{
    static enum {
        START,
        WAIT_EVENT,
        PRINT
    }s_tState = START;
    switch(s_tState){
	case START:
        s_tState = WAIT_EVENT;
 //       break;
    case WAIT_EVENT:
        if(WAIT_EVENT(&s_tWorldEvent)){
            s_tState = PRINT;
        }            
        break;
    case PRINT:
        if(fsm_rt_cpl == print_World()){
            TASK_F_RESET_FSM();
            return fsm_rt_cpl;
        }
        break;
    }
    return fsm_rt_on_going;
} 

#define PRINT_F1_RESET_FSM()  \
    do{  \
        s_tState = START;   \
    }while(0)
static fsm_rt_t print_f1_pressed(void)
{
    static enum {
        START,
        PRINT
    }s_tState = START; 
    static print_str_t s_tF1;
    switch(s_tState){
        case START:
            printf_string_init(&s_tF1,"F1 Pressed\r\n");
            s_tState = PRINT;
//            break;
        case PRINT:
            if(fsm_rt_cpl == printf_string(&s_tF1)){	
                PRINT_F1_RESET_FSM();
                return fsm_rt_cpl;
            }
            break;
    }
    return fsm_rt_on_going;
}

#define PRINT_UP_RESET_FSM()  \
    do{  \
        s_tState = START;   \
    }while(0)
static fsm_rt_t print_up_pressed(void)
{
    static enum {
        START,
        PRINT
    }s_tState = START; 
    static print_str_t s_tUP;
    switch(s_tState){
        case START:
            printf_string_init(&s_tUP,"UP Pressed\r\n");
            s_tState = PRINT;
//            break;
        case PRINT:
            if(fsm_rt_cpl == printf_string(&s_tUP)){	
                PRINT_UP_RESET_FSM();
                return fsm_rt_cpl;
            }
            break;
    }
    return fsm_rt_on_going;
}


#define PRINT_DOWN_RESET_FSM()  \
    do{  \
        s_tState = START;   \
    }while(0)
static fsm_rt_t print_down_pressed(void)
{
    static enum {
        START,
        PRINT
    }s_tState = START; 
    static print_str_t s_tDOWN;
    switch(s_tState){
        case START:
            printf_string_init(&s_tDOWN,"DOWN Pressed\r\n");
            s_tState = PRINT;
//            break;
        case PRINT:
            if(fsm_rt_cpl == printf_string(&s_tDOWN)){	
                PRINT_DOWN_RESET_FSM();
                return fsm_rt_cpl;
            }
            break;
    }
    return fsm_rt_on_going;
}


#define TASK_A_RESET_FSM()  \
    do{                    \
        s_tState = START;  \
    }while(0)
static fsm_rt_t task_a(void)
{
    static enum {
        START,
        WAIT_EVENT,
        PRINT
    }s_tState = START;
    switch(s_tState){
        case START:
            s_tState = WAIT_EVENT;
     //       break;
        case WAIT_EVENT:
            if(WAIT_EVENT(&s_tF1Event)){
                s_tState = PRINT;
            }
            break;
        case PRINT:
            if(fsm_rt_cpl == print_f1_pressed()){
                TASK_A_RESET_FSM();
                return fsm_rt_cpl;
            }
            break;
    }
    return fsm_rt_on_going;
}

#define TASK_B_RESET_FSM()  \
    do{                    \
        s_tState = START;  \
    }while(0)
static fsm_rt_t task_b(void)
{
    static enum {
        START,
        WAIT_EVENT,
        PRINT
    }s_tState = START;
    switch(s_tState){
        case START:
            s_tState = WAIT_EVENT;
     //       break;
        case WAIT_EVENT:
            if(WAIT_EVENT(&s_tDOWNEvent)){
                s_tState = PRINT;
            }
            break;
        case PRINT:
            if(fsm_rt_cpl == print_down_pressed()){
                TASK_B_RESET_FSM();
                return fsm_rt_cpl;
            }
            break;
    }
    return fsm_rt_on_going;
}


#define TASK_C_RESET_FSM()  \
    do{                    \
        s_tState = START;  \
    }while(0)
static fsm_rt_t task_c(void)
{
    static enum {
        START,
        WAIT_EVENT,
        PRINT
    }s_tState = START;
    switch(s_tState){
        case START:
            s_tState = WAIT_EVENT;
     //       break;
        case WAIT_EVENT:
            if(WAIT_EVENT(&s_tUPEvent)){
                s_tState = PRINT;
            }
            break;
        case PRINT:
            if(fsm_rt_cpl == print_up_pressed()){
                TASK_C_RESET_FSM();
                return fsm_rt_cpl;
            }
            break;
    }
    return fsm_rt_on_going;
}

#define F1_HANDER_TASK_RESET_FSM()      \
    do {\
        s_tState = START;\
    } while(0);
fsm_rt_t msg_f1_handler(const msg_t *ptMSG)
{
    static enum{
        START = 0,
        CHECK,
    }s_tState = START;
    if(NULL == ptMSG){
        return fsm_rt_err;
    }
    switch(s_tState){
        case START:
            s_tState = CHECK;
            break;
        case CHECK:
            SET_EVENT(&s_tF1Event);
            F1_HANDER_TASK_RESET_FSM();
            return fsm_rt_cpl;        
    }
    return fsm_rt_on_going;
}


#define UP_HANDER_TASK_RESET_FSM()      \
    do {\
        s_tState = START;\
    } while(0);
bool msg_up_handler(const msg_t *ptMSG)
{
    static enum{
        START = 0,
        CHECK,
    }s_tState = START;
    if(NULL == ptMSG){
        return fsm_rt_err;
    }
    switch(s_tState){
        case START:
            s_tState = CHECK;
            break;
        case CHECK:
            SET_EVENT(&s_tUPEvent);
            UP_HANDER_TASK_RESET_FSM();
            return fsm_rt_cpl;        
    }
    return fsm_rt_on_going;
}

#define DOWN_HANDER_TASK_RESET_FSM()      \
    do {\
        s_tState = START;\
    } while(0);
bool msg_down_handler(const msg_t *ptMSG)
{
    static enum{
        START = 0,
        CHECK,
    }s_tState = START;
    if(NULL == ptMSG){
        return fsm_rt_err;
    }
    switch(s_tState){
        case START:
            s_tState = CHECK;
            break;
        case CHECK:
            SET_EVENT(&s_tDOWNEvent);
            DOWN_HANDER_TASK_RESET_FSM();
            return fsm_rt_cpl;        
    }
    return fsm_rt_on_going;
}

void system_init(void)
{
    uart_init();
    Key_gpio_init();
    led_gpio_init();
}

static void set_led_gradation(uint16_t hwLevel)
{
    static uint16_t s_hwCounter = 0;
    
    if (hwLevel >= s_hwCounter) {
        LED1_ON();
    } else {
        LED1_OFF();
    }
    s_hwCounter++;
    s_hwCounter &= TOP;
}

static void breath_led(void)   //???
{
    static uint16_t s_hwCounter = 0;
    static int16_t s_nGray = (TOP >> 1);
    s_hwCounter++;
    if (!(s_hwCounter & (_BV(10)-1))) {
        s_nGray++; 
        if (s_nGray == TOP) {
            s_nGray = 0;
        }
    }
    set_led_gradation(ABS(s_nGray - (TOP >> 1)));
}

static uint8_t s_chXmodemRecBuffer[5120];
uint8_t* xmodem_report(xmodem_event_t ptEvent,uint8_t *ptBuffer)
{
    static uint16_t s_hwIndex;
    switch(ptEvent){
        case XMODEM_START:
            return &s_chXmodemRecBuffer[s_hwIndex];
        case XMODEM_EOT:
            s_hwIndex = 0;
            break;
        case TIMER_OUT_1S:
            return ptBuffer;
        case TIMER_OUT_10S:
            return ptBuffer;
        case CHECK_ERR:
            return ptBuffer;
        case INVALID_SN:
            return ptBuffer;
        case PACKAGE_OK:
            s_hwIndex += 128;
            if(s_hwIndex >= (UBOUND(s_chXmodemRecBuffer))){
                s_hwIndex = 0;
            }
            return &s_chXmodemRecBuffer[s_hwIndex];
    }
    return NULL;
}

#define DEALY_10S_RESRT_FSM()    \
    do {\
        s_tState = START;\
    } while(0);
fsm_rt_t delay_10s(void)
{
    static enum{
        START = 0,
        WAIT,
        CHECK,
    }s_tState = START;
    static uint16_t s_chCount = 0;
    switch(s_tState){
        case START:
            s_tXmodem.hwTimerCount = 50000ul;
            s_tState = WAIT;
//            break;
        case WAIT:
            xmodem_insert_1ms_timer_handler(&s_tXmodem);
            if(s_tXmodem.hwTimerCount == 0){
                s_chCount++;
                s_tState = CHECK;
            }
            break;
        case CHECK:
            if(s_chCount >= 10){
                s_chCount = 0;
                xmodem_start(&s_tXmodem);
                DEALY_10S_RESRT_FSM();
                return fsm_rt_cpl;
            }else{
                s_tXmodem.hwTimerCount = 50000ul;
                s_tState = WAIT;
            }
            break;
    }
    return fsm_rt_on_going;
}

static checker_item_t s_tOrangeItem;
static checker_item_t s_tAppleItem;
static checker_item_t s_tWorldItem;
static checker_item_t s_tCheckMapItem;
static checker_item_t s_tXmodemItem;

#define CHECK_XMODEM_RESET_FSM()      \
    do {\
        s_tState = START;\
    } while(0);
fsm_rt_t check_xmodem(stream_in_t *fnByteIn,bool *pbIsRequestDrop)
{
    static enum{
        START = 0,
        GET_BYTE,
    }s_tState = START;

//    static xmodem_t s_tXmodem;

    if((NULL == fnByteIn) || (NULL == pbIsRequestDrop)){
        return fsm_rt_err;
    }    
    switch(s_tState){
        case START:
            s_tState = GET_BYTE;
//            break;
        case GET_BYTE:    
            *pbIsRequestDrop = false;
            if(fsm_rt_cpl == xmodem_task(&s_tXmodem,fnByteIn,pbIsRequestDrop)){
                CHECK_XMODEM_RESET_FSM();
                return fsm_rt_cpl;
            }
            if(*pbIsRequestDrop){
                CHECK_XMODEM_RESET_FSM();
            }
            break;
    }
    return fsm_rt_on_going;
}
//search msg
#define TASK_CHECK_USE_PEEK_RESET_FSM()      \
    do {\
        s_tState = START;\
    } while(0);
fsm_rt_t check_msg(stream_in_t *fnByteIn,bool *pbIsRequestDrop)  
{
    static enum{
        START = 0,
		SEARCH,
        PRINT,
    }s_tState = START;
	static msg_t *s_tMsg;
    
    if((NULL == fnByteIn) || (NULL == pbIsRequestDrop)){
        return fsm_rt_err;
    }

	switch(s_tState){
        case START:
            s_tState = SEARCH;
            *pbIsRequestDrop = false;
//            break;
	    case SEARCH:
            s_tMsg = (msg_t *)search_msg_map_stream(fnByteIn,pbIsRequestDrop);  
            if(NULL == s_tMsg){
                break;
            }
//            s_tState = PRINT;
//            break;
        case PRINT:
            if(NULL == s_tMsg->fnHandler){
                TASK_CHECK_USE_PEEK_RESET_FSM();
                break;
            }
            if(fsm_rt_cpl == (*(s_tMsg->fnHandler))(s_tMsg)){
                TASK_CHECK_USE_PEEK_RESET_FSM();
                return fsm_rt_cpl;
            }
            break;
    }
    return fsm_rt_on_going;
}

#define CHECKER_APPLE_RESET_FSM()    \
    do {\
        s_tState = START;\
    } while(0);
fsm_rt_t check_apple(stream_in_t *fnByteIn,bool *pbIsRequestDrop)
{
    static enum{
        START = 0,
        CHECK,
    }s_tState = START;
    static check_str_t s_ptApple; 

    if((NULL == fnByteIn) || (NULL == pbIsRequestDrop)){
        return fsm_rt_err;
    }

    switch(s_tState){
        case START:
            s_tState = CHECK;
            *pbIsRequestDrop = false;
            init_check_string(&s_ptApple,"apple",fnByteIn);
//          break;
        case CHECK:
            if(fsm_rt_cpl == check_string(&s_ptApple,pbIsRequestDrop)){
                SET_EVENT(&s_tAppleEvent);
                CHECKER_APPLE_RESET_FSM();
                return fsm_rt_cpl;
            }
            break;
    }
    return fsm_rt_on_going;
}

#define CHECKER_ORANGE_RESET_FSM()    \
    do {\
        s_tState = START;\
    } while(0);
static fsm_rt_t check_orange(stream_in_t *fnByteIn,bool *pbIsRequestDrop)
{
    static enum{
        START = 0,
        CHECK,
    }s_tState = START;
    static check_str_t s_tOrange; 

    if((NULL == fnByteIn) || (NULL == pbIsRequestDrop)){
        return fsm_rt_err;
    }

    switch(s_tState){
        case START:
            s_tState = CHECK;
            *pbIsRequestDrop = false;
            init_check_string(&s_tOrange,"orange",fnByteIn);
//          break;
        case CHECK:
            if(fsm_rt_cpl == check_string(&s_tOrange,pbIsRequestDrop)){
                SET_EVENT(&s_tOrangeEvent);
                CHECKER_ORANGE_RESET_FSM();
                return fsm_rt_cpl;
            }
            break;
    }
    return fsm_rt_on_going;
}

#define CHECKER_WORLD_RESET_FSM()    \
    do {\
        s_tState = START;\
    } while(0);
static fsm_rt_t check_world(stream_in_t *fnByteIn,bool *pbIsRequestDrop)
{
    static enum{
        START = 0,
        CHECK,
    }s_tState = START;
    static check_str_t s_tWorld; 

    if((NULL == fnByteIn) || (NULL == pbIsRequestDrop)){
        return fsm_rt_err;
    }
    switch(s_tState){
        case START:
            s_tState = CHECK;
            *pbIsRequestDrop = false;
            init_check_string(&s_tWorld,"world",fnByteIn);
//          break;
        case CHECK:
            if(fsm_rt_cpl == check_string(&s_tWorld,pbIsRequestDrop)){
                SET_EVENT(&s_tWorldEvent);
                CHECKER_WORLD_RESET_FSM();
                return fsm_rt_cpl;
            }
            break;
    }
    return fsm_rt_on_going;
}

//static checker_list_t s_tCheckList;
static fsm(batch_check_task) s_fsmBatchCheck;    //定义本地状态机控制块

int main(void)
{
    system_init();
    INIT_EVENT(&s_tF1Event,false,false);
    INIT_EVENT(&s_tUPEvent,false,false);
    INIT_EVENT(&s_tDOWNEvent,false,false);
    INIT_EVENT(&s_tAppleEvent,false,false);
    INIT_EVENT(&s_tOrangeEvent,false,false);
    INIT_EVENT(&s_tWorldEvent,false,false);
    
    INIT_BYTE_QUEUE(&s_tFIFOin,s_chInBuffer,UBOUND(s_chInBuffer));
    INIT_BYTE_QUEUE(&s_tFIFOout,s_chOutBuffer,UBOUND(s_chOutBuffer));
    
    CHECK_LIST.Init(&s_tCheckList);
     
    CHECK_LIST.Item.Init(&s_tOrangeItem,&check_orange);
    CHECK_LIST.Item.Init(&s_tAppleItem,&check_apple);
    CHECK_LIST.Item.Init(&s_tWorldItem,&check_world);
    CHECK_LIST.Item.Init(&s_tCheckMapItem,&check_msg);
    CHECK_LIST.Item.Init(&s_tXmodemItem,&check_xmodem);
    
    CHECK_LIST.Item.Add(&s_tCheckList,&s_tOrangeItem);
    CHECK_LIST.Item.Add(&s_tCheckList,&s_tCheckMapItem);
    CHECK_LIST.Item.Add(&s_tCheckList,&s_tAppleItem);
    CHECK_LIST.Item.Add(&s_tCheckList,&s_tWorldItem);
    
 //   s_tCheckList.base__i_check_list_t.Item.Add(&s_tCheckList,&s_tXmodemItem);
    
    interface_init(&s_tPipe);
    xmodem_check_sum_mode_t tMode = XMODEM_CHECK_CRC;
    xmodem_init(&s_tXmodem,tMode,128,&xmodem_report,&s_tPipe);   
    
    init_fsm(batch_check_task,&s_fsmBatchCheck,args(s_tCheckList,&s_tPipe,&s_tFIFOin));
    
    while(1){
        breath_led();
        serial_in_task();
        serial_out_task();
        
        CHECK_LIST.Task(&s_fsmBatchCheck);
        
        task_a();
        task_b();
        task_c();
        task_d();
        task_e();
        task_f();
        delay_10s();
    }
}
