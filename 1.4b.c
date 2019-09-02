#include "stm32f10x.h"
#include "system.h"
#include "stdlib.h"
#include "critical.h"
#include "event.h"

uint8_t chVar;
uint8_t chInput;
event_t t_PrintEvent;
event_t t_EchoEvent;
event_t t_EchoFree;

#define Log(__STR)  \
    do{       \
        uint8_t *pchSTR = (__STR);  \
        do{          \
            while(!serial_out(*pchSTR)); \
            pchSTR++;         \
            if('\0' == *pchSTR){    \
                break;    \
            }   \
        }while(1); \
    }while(0) 

void system_init(void)
{
    uart_init();
    Key_gpio_init();
    led_gpio_init();
    INIT_EVENT(&t_PrintEvent,false,true);
    INIT_EVENT(&t_EchoEvent,false,false);
    INIT_EVENT(&t_EchoFree,true,false);
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
        CHECK_TASK,    
    }s_tState = START; 
    switch(s_tState){
        case START:
            s_tState = WAIT_CHAR;
            break;
        case WAIT_CHAR:
            if(serial_in(&chVar)){
                s_tState = WAIT_TASK_FREE;
            }
            break;
        case WAIT_TASK_FREE:
            if(WAIT_EVENT(&t_EchoFree)){
                SET_EVENT(&t_EchoEvent);
                chInput = chVar;
            }
            s_tState = CHECK_TASK;
            break;
        case CHECK_TASK:
            if(fsm_rt_cpl == check_world(chVar)){
                SET_EVENT(&t_PrintEvent);
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
//            break;
        case WAIT:
            if(WAIT_EVENT(&t_PrintEvent)){
                s_tState = PRINT;
            }
            break;
        case PRINT:
            if(fsm_rt_cpl == print_hello()){
                RESET_EVENT(&t_PrintEvent);
                PRINTF_RESET_FSM();
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
    switch(s_tState){
        case START:
            s_tState = WAIT_ECHO_EVENT;
            break;
        case WAIT_ECHO_EVENT:
            if(WAIT_EVENT(&t_EchoEvent)){
                s_tState = WAIT_PRINT_EVENT;
            }
            break;
        case WAIT_PRINT_EVENT:
            if(WAIT_EVENT(&t_PrintEvent)){
                s_tState = WAIT_PRINT_EVENT;
                break;
            }
            s_tState = PRINT;
//            break;
        case PRINT:
            if(serial_out(chInput)){  //ªÿœ‘
                SET_EVENT(&t_EchoFree);
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
    Log("abc");
    while(1){
        breath_led();
        check_task();
        printf_task();
        echo_task();
    }
}

