#include "stm32f10x.h"
#include "system.h"
#include "stdlib.h"
#include "critical.h"
#include "event.h"
#include "mail.h"
#include "queue.h"
#include "xmodem.h"
#include "msg_map.h"
#include "print_string.h"
#include "stream_in_out.h"
#include "check_list.h"

byte_queue_t g_tFIFOin;
byte_queue_t g_tFIFOout;

static event_t s_tAppleEvent;
static event_t s_tOrangeEvent;
static event_t s_tWorldEvent;

static uint8_t s_chInBuffer[16] = {0};
static uint8_t s_chOutBuffer[16] = {0};

#define PRINT_APPLE_RESET_FSM()  \
    do{  \
        s_tState = START;   \
    }while(0)
static fsm_rt_t print_Apple(void)
{
    static enum {
        START,
        PRINT
    }s_tState = START; 
    static print_str_t s_tApple;
    switch(s_tState){
        case START:
            printf_string_init(&s_tApple,"apple\r\n");
            s_tState = PRINT;
            break;
        case PRINT:
            if(fsm_rt_cpl == printf_string(&s_tApple)){	
                PRINT_APPLE_RESET_FSM();
                return fsm_rt_cpl;
            }
            break;
    }
    return fsm_rt_on_going;
}

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
            if(WAIT_EVENT(&s_tAppleEvent)){
                s_tState = PRINT;
            }
            break;
        case PRINT:
            if(fsm_rt_cpl == print_Apple()){
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
 //           break;
        case WAIT_EVENT:
            if(WAIT_EVENT(&s_tOrangeEvent)){
                s_tState = PRINT;
            }            
            break;
        case PRINT:
            if(fsm_rt_cpl == print_Orange()){
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
        if(WAIT_EVENT(&s_tWorldEvent)){
            s_tState = PRINT;
        }            
        break;
    case PRINT:
        if(fsm_rt_cpl == print_World()){
            TASK_C_RESET_FSM();
            return fsm_rt_cpl;
        }
        break;
    }
    return fsm_rt_on_going;
}  

#define APPLE_HANDER_TASK_RESET_FSM()      \
    do {\
        s_tState = START;\
    } while(0);
bool msg_apple_handler(const msg_t *ptMSG)
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
            SET_EVENT(&s_tAppleEvent);
            APPLE_HANDER_TASK_RESET_FSM();
            return fsm_rt_cpl;        
    }
    return fsm_rt_on_going;
}

#define HELLO_HANDER_TASK_RESET_FSM()      \
    do {\
        s_tState = START;\
    } while(0);
bool msg_hello_handler(const msg_t *ptMSG)
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
            SET_EVENT(&s_tWorldEvent);
            HELLO_HANDER_TASK_RESET_FSM();
            return fsm_rt_cpl;
    }
    return fsm_rt_on_going;
}

#define ORANGE_HANDER_TASK_RESET_FSM()      \
    do {\
        s_tState = START;\
    } while(0);
bool msg_orange_handler(const msg_t *ptMSG)
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
            SET_EVENT(&s_tOrangeEvent);
            ORANGE_HANDER_TASK_RESET_FSM();
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

//#define SERIAL_IN_TASK_RESET_FSM()      \
//    do {\
//        s_tState = START;\
//    } while(0);
//fsm_rt_t serial_in_task(void)
//{
//    static enum{
//        START = 0,
//        INPUT,
//    }s_tState = START;    
//    uint8_t chChar = 0;    
//    switch(s_tState) {
//        case START:
//            s_tState = INPUT;
////             break;        
//        case INPUT:          
//            if(serial_in(&chChar)) {      
//                ENQUEUE_BYTE(&g_tFIFOin, chChar);
//                SERIAL_IN_TASK_RESET_FSM();
//                return fsm_rt_cpl;               
//            }
//    }   
//    return fsm_rt_on_going;
//}


#define CHECKER_APPLE_RESET_FSM()    \
    do {\
        s_tState = START;\
    } while(0);
fsm_rt_t check_apple(byte_queue_t *ptQueue,bool *pbIsRequestDrop)
{
    static enum{
        START = 0,
        CHECK,
    }s_tState = START;
    static check_str_t s_ptApple; 

    if((NULL == ptQueue)||(NULL == pbIsRequestDrop)){
        return fsm_rt_err;
    }

    switch(s_tState){
        case START:
            s_tState = CHECK;
            init_check_string(&s_ptApple,"apple",ptQueue);
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
static fsm_rt_t check_orange(byte_queue_t *ptQueue,bool *pbIsRequestDrop)
{
    static enum{
        START = 0,
        CHECK,
    }s_tState = START;
    static check_str_t s_tOrange; 

    if((NULL == ptQueue)||(NULL == pbIsRequestDrop)){
        return fsm_rt_err;
    }

    switch(s_tState){
        case START:
            s_tState = CHECK;
            init_check_string(&s_tOrange,"orange",ptQueue);
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
static fsm_rt_t check_world(byte_queue_t *ptQueue,bool *pbIsRequestDrop)
{
    static enum{
        START = 0,
        CHECK,
    }s_tState = START;
    static check_str_t s_tWorld; 

    if((NULL == ptQueue)||(NULL == pbIsRequestDrop)){
        return fsm_rt_err;
    }

    switch(s_tState){
        case START:
            s_tState = CHECK;
            init_check_string(&s_tWorld,"world",ptQueue);
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

static xmodem_t s_tXmodem;
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
            if(s_chCount >= 20){
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

static checker_item_t s_tOrange_item;
static checker_item_t s_tApple_item;
static checker_item_t s_tWorld_item;
static checker_list_t s_tCheck_list;

bool check_item_init(checker_item_t * ptCheckItem,checker_t * ptCheck)
{
    if((NULL == ptCheckItem) || (NULL == ptCheck)){
        return false;
    }
    ptCheckItem->ptItem = ptCheck;
    return true;
}

#define ECHO_RESRT_FSM()    \
    do {\
        s_tState = START;\
    } while(0);

static fsm_rt_t echo(void)
{
    static enum{
        START = 0,
        WAIT,
    }s_tState = START;
    uint8_t chChar;
    switch(s_tState){
        case START:
            s_tState = WAIT;
//            break;
        case WAIT:
            if(DEQUEUE_BYTE(&g_tFIFOin,&chChar)){
                if(9 < ((chChar >> 4) & 0x0f)){
                    ENQUEUE_BYTE(&g_tFIFOout,((chChar >> 4) & 0x0f) - 10 + 'a');
                }else{
                    ENQUEUE_BYTE(&g_tFIFOout,((chChar >> 4) & 0x0f) + '0');
                }
                if(9 < (chChar & 0x0f)){
                    ENQUEUE_BYTE(&g_tFIFOout,((chChar) & 0x0f) - 10 + 'a');
                }else{
                    ENQUEUE_BYTE(&g_tFIFOout, (chChar & 0x0f) + '0');
                }
                ENQUEUE_BYTE(&g_tFIFOout,' ');
                ECHO_RESRT_FSM();
                return fsm_rt_cpl;
            }
            break;
    }
    return fsm_rt_on_going;
}


int main(void)
{
    system_init();

    INIT_EVENT(&s_tAppleEvent,false,false);
    INIT_EVENT(&s_tOrangeEvent,false,false);
    INIT_EVENT(&s_tWorldEvent,false,false);

    INIT_BYTE_QUEUE(&g_tFIFOin,s_chInBuffer,UBOUND(s_chInBuffer));
    INIT_BYTE_QUEUE(&g_tFIFOout,s_chOutBuffer,UBOUND(s_chOutBuffer));
//    xmodem_check_sum_mode_t tMode = 1;
//    xmodem_init(&s_tXmodem,tMode,128,&xmodem_report,&get_byte,&serial_out);

    check_item_init(&s_tOrange_item,&check_orange);
    check_item_init(&s_tApple_item,&check_apple);
    check_item_init(&s_tWorld_item,&check_world);
    check_list_init(&s_tCheck_list);
    check_list_add_item(&s_tCheck_list,&s_tOrange_item);
    check_list_add_item(&s_tCheck_list,&s_tApple_item);
    check_list_add_item(&s_tCheck_list,&s_tWorld_item);
    while(1){
        breath_led();
        serial_in_task();
        echo();
//        batch_check_task(&s_tCheck_list,&g_tFIFOin);
//        task_a();
//        task_c();
//        task_b();
        serial_out_task();
    }
}
