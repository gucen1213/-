#include "stm32f10x.h"
#include "system.h"
#include "stdlib.h"
#include "critical.h"
#include "event.h"
#include "mail.h"

//#define DELAY_1S   (200000)
static event_t s_tPrintEvent;
static event_t s_tEchoFinshEvent;
static mailbox_t s_tMailBox;

void system_init(void)
{
    uart_init();
    Key_gpio_init();
    led_gpio_init();

    INIT_EVENT(&s_tPrintEvent,false,true);
    INIT_EVENT(&s_tEchoFinshEvent,true,false);
    INIT_MAILBOX(&s_tMailBox);
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

#define PRINT_RESET_FSM()  \
    do{  \
        s_tState = PRINT_START;   \
    }while(0)		
	
static fsm_rt_t print_hello(void)   
{
    static enum {
        PRINT_START = 0,
        PRINT_H,
        PRINT_E,
        PRINT_L,
        PRINT_LL,
        PRINT_O
    }s_tState = PRINT_START;
    switch(s_tState){
        case PRINT_START:
            s_tState = PRINT_H;
            break;
        case PRINT_H:
            if(serial_out('H')){
                s_tState = PRINT_E;
            }
            break;
        case PRINT_E:
            if(serial_out('E')){
                s_tState = PRINT_L;
            }            
            break;
        case PRINT_L:
            if(serial_out('L')){
                s_tState = PRINT_LL;
            }            
            break;
        case PRINT_LL:
            if(serial_out('L')){
                s_tState = PRINT_O;
            }            
            break;
        case PRINT_O:
            if(serial_out('O')){
                PRINT_RESET_FSM();
                return fsm_rt_cpl;
            }            
            break;
    }
    return fsm_rt_on_going;
}   


//#define DELAY_RESET_FSM()  \
//    do{  \
//        s_tState = START;   \
//    }while(0)	
//static fsm_rt_t delay(void)
//{
//    static uint32_t s_wCnt;
//    static enum {
//        START = 0,
//        DELAY
//    }s_tState = START;
//    switch(s_tState){
//        case START:
//            s_tState = DELAY;
//            s_wCnt = DELAY_1S;
//            break;
//        case DELAY:
//            if(s_wCnt > 0){
//                s_wCnt--;
//            }else{
//                DELAY_RESET_FSM();
//                return fsm_rt_cpl;
//            }
//            break;
//    }
//    return fsm_rt_on_going;
//}

//#define TEST1A_RESET_FSM()  \
//    do{  \
//        s_tState = START;   \
//    }while(0)	
//static fsm_rt_t testla(void)
//{
//    static enum {
//        START = 0,
//        DELAY,
//        PRINT
//    }s_tState = START;
//    switch(s_tState){
//        case START:
//            s_tState = DELAY;
//            break;
//        case DELAY:
//            if(fsm_rt_cpl == delay()){
//                s_tState =  PRINT;
//            }
//            break;
//        case PRINT:
//            if(fsm_rt_cpl == print_hello()){
//                TEST1A_RESET_FSM();
//                return fsm_rt_cpl;
//            }
//            break;
//    }
//    return fsm_rt_on_going;
//}

//#define PANEL_RESET_FSM()  \
//    do{  \
//        s_tState = START;   \
//    }while(0)	
//static fsm_rt_t panel_task(void)
//{
//    static enum {
//        START = 0,
//        WAIT
//    }s_tState = START;
//    static uint8_t chVar;
//    switch(s_tState){
//        case START:
//            s_tState = WAIT;
//            break;
//        case WAIT:
//            if(serial_in(&chVar)){
//                if(chVar == 's'){
//                    SET_EVENT(&s_tSevent);  
//                }
//                if(chVar == 'e'){
//                    SET_EVENT(&s_tEevent);
//                }
//                PANEL_RESET_FSM();
//                return fsm_rt_cpl;
//            }
//            break;
//    }
//    return fsm_rt_on_going;
//}

//#define BACK_RESET_FSM()  \
//    do{  \
//        s_tState = START;   \
//    }while(0)	
//static fsm_rt_t background_task(void)
//{
//    static enum {
//        START = 0,
//        WAIT_S,
//        PRINT,
//        WAIT_E
//    }s_tState = START;   
//    switch(s_tState){
//        case START:
//            s_tState = WAIT_S;
//            break;
//        case WAIT_S:
//            if(WAIT_EVENT(&s_tSevent)){
//                s_tState = PRINT;
//            }
//            break;
//        case PRINT:
//            if(fsm_rt_cpl == testla()){
//                s_tState = WAIT_E;
//            }
//            break;
//        case WAIT_E:
//            if(WAIT_EVENT(&s_tEevent)){
//                BACK_RESET_FSM();
//                return fsm_rt_cpl;
//            }
//            s_tState = PRINT;
//            break;
//    }
//    return fsm_rt_on_going;
//}
//#define CHECK_RESET_FSM()  \
//    do{  \
//        s_tState = START;   \
//    }while(0)	
//static fsm_rt_t check_world(uint8_t chchar)
//{
//    static enum {
//        START = 0,
//        WAIT_W,
//        WAIT_O,
//        WAIT_R,
//        WAIT_L,
//        WAIT_D
//    }s_tState = START;  
//    switch(s_tState){
//        case START:
//            s_tState = WAIT_W;
//            break;
//        case WAIT_W:
//            if(serial_in(&chVar)){
//                if(chchar == 'w'){
//                    s_tState = WAIT_O;
//                }else{
//                    CHECK_RESET_FSM();
//                }
//            }
//            break;
//        case WAIT_O:
//            if(serial_in(&chVar)){
//                if(chchar == 'o'){
//                    s_tState = WAIT_R;
//                }else{
//                    CHECK_RESET_FSM();
//                }
//            }
//            break;
//        case WAIT_R:
//            if(serial_in(&chVar)){
//                if(chchar == 'r'){
//                    s_tState = WAIT_L;
//                }else{
//                    CHECK_RESET_FSM();
//                }
//            }
//            break;
//        case WAIT_L:
//            if(serial_in(&chVar)){
//                if(chchar == 'l'){
//                    s_tState = WAIT_D;
//                }else{
//                    CHECK_RESET_FSM();
//                }
//            }
//            break;
//        case WAIT_D:
//            if(serial_in(&chVar)){
//                if(chchar == 'd'){
//                    CHECK_RESET_FSM();
//                    return fsm_rt_cpl;
//                }else{
//                    CHECK_RESET_FSM();
//                }
//            }
//            break;
//    }
//    return fsm_rt_on_going;
//}


#define CHECK_RESET_FSM()  \
    do{  \
        s_tState = START;   \
    }while(0)	
static fsm_rt_t check_world(uint8_t chchar)
{
    static enum {
        START = 0,
        WAIT_W,
        WAIT_O,
        WAIT_R,
        WAIT_L,
        WAIT_D
    }s_tState = START;  
    switch(s_tState){
        case START:
            s_tState = WAIT_W;
            //break;
        case WAIT_W:
            if(chchar == 'w'){
                s_tState = WAIT_O;
             }else{
                CHECK_RESET_FSM();
             }
            break;
        case WAIT_O:
            if(chchar == 'o'){
                s_tState = WAIT_R;
            }else{
                CHECK_RESET_FSM();
            }
            break;
        case WAIT_R:
            if(chchar == 'r'){
                s_tState = WAIT_L;
            }else{
                CHECK_RESET_FSM();
            }
            break;
        case WAIT_L:
            if(chchar == 'l'){
                s_tState = WAIT_D;
            }else{
                CHECK_RESET_FSM();
            }
            break;
        case WAIT_D:
            if(chchar == 'd'){
                CHECK_RESET_FSM();
                return fsm_rt_cpl;
            }else{
                CHECK_RESET_FSM();
            }
            break;
    }
    return fsm_rt_on_going;
}

#define CHECK_TASK_RESET_FSM()  \
    do{  \
        s_tState = START;   \
    }while(0)	
static fsm_rt_t check_task(void)
{
    static enum {
        START = 0,
        WAIT_CHAR,
        WAIT_TASK_FREE,
        CHECK,
//        PRINT,
    }s_tState = START; 
    static uint8_t s_chByte;
    uint8_t chByte;     
 
    switch(s_tState){
        case START:
            s_tState = WAIT_CHAR;
            break;
        case WAIT_CHAR:
            if(!serial_in(&chByte)){
                break;
            }
            s_tState = WAIT_TASK_FREE;
            
            //break;
        case WAIT_TASK_FREE:
            if(WAIT_EVENT(&s_tEchoFinshEvent)){
                s_chByte = chByte;
                SEND_MAILBOX(&s_tMailBox,&s_chByte);
            }
            //break;
        case CHECK:
            if(fsm_rt_cpl == check_world(chByte)){
                SET_EVENT(&s_tPrintEvent);
                CHECK_TASK_RESET_FSM();
                return fsm_rt_cpl;
            }else{
                s_tState = WAIT_CHAR;
            }
            break;
    }
    return fsm_rt_on_going; 
}

#define PRINTF_RESET_FSM()  \
    do{  \
        s_tState = START;   \
    }while(0)	
static fsm_rt_t printf_task(void)
{
    static enum {
        START = 0,
        WAIT,
        PRINT,
    }s_tState = START;
    switch(s_tState){
        case START:
            s_tState = WAIT;
        case WAIT:
            if(WAIT_EVENT(&s_tPrintEvent)){
                s_tState = PRINT;
            }
            break;
        case PRINT:
            if(fsm_rt_cpl == print_hello()){
                PRINTF_RESET_FSM();
                RESET_EVENT(&s_tPrintEvent);
                return fsm_rt_cpl;
            }
            break;
    }
    return fsm_rt_on_going;
}

#define ECHO_RESET_FSM()  \
    do{  \
        s_tState = START;   \
    }while(0)	
static fsm_rt_t echo_task(void)
{
    static enum {
        START = 0,
        WAIT_ECHO_EVENT,
        WAIT_PRINT_EVENT,
        PRINT
    }s_tState = START;    
    uint8_t * ptTempObj;
    static uint8_t  s_chByte;
    switch(s_tState){
        case START:
            s_tState = WAIT_ECHO_EVENT;    
        case WAIT_ECHO_EVENT:
            ptTempObj = OPEN_MAILBOX(&s_tMailBox);
            if(ptTempObj != NULL){
                s_chByte = *ptTempObj;
                s_tState = WAIT_PRINT_EVENT;
            }
            break;
        case WAIT_PRINT_EVENT:
            if(WAIT_EVENT(&s_tPrintEvent)){
                break;
            }
            s_tState = PRINT;
        case PRINT:
            if(serial_out(s_chByte)){
                SET_EVENT(&s_tEchoFinshEvent); 
                ECHO_RESET_FSM();
                return fsm_rt_cpl;
            }else{
                s_tState = WAIT_PRINT_EVENT;
            }
            break;
    }
    return fsm_rt_on_going;
}

int main(void)
{
    system_init();
    while(1){
        breath_led();
        check_task();
        printf_task();
        echo_task();
    }
}

