#include ".\event\event.h"
#include <stdint.h>
#include <stdbool.h>
#include "system.h"
#include ".\app_cfg.h"

DECLARE_CLASS(print_str_t)
DEF_CLASS(print_str_t)
    uint8_t chState;
    uint8_t * pchStr;   //×Ö·û´®Ö¸Õë
END_DEF_CLASS(print_str_t)

bool printf_string_init(print_str_t * ptFSM,uint8_t *pchStr)
{
    CLASS(print_str_t) *ptThis = (CLASS(print_str_t) *)ptFSM;
    if((NULL == ptThis)||(NULL == pchStr)){
        return false;
    }
    ptThis->chState = 0;
    ptThis->pchStr = pchStr;
    return true;
}

#define PRINT_STRING_RESET_FSM()  \
    do{  \
        ptThis->chState = START;   \
    }while(0)
fsm_rt_t printf_string(print_str_t * ptFSM)
{
    enum {
        START,
        CHECK_TAIL,
        PRINT
    }; 
    CLASS(print_str_t) *ptThis = (CLASS(print_str_t) *)ptFSM;
    if(ptThis == NULL){
        return fsm_rt_err;
    }
    switch(ptThis->chState){
        case START:
            ptThis->chState = CHECK_TAIL;
        //  break;
        case CHECK_TAIL:
            if(*ptThis->pchStr == '\0'){
                PRINT_STRING_RESET_FSM();
                return fsm_rt_cpl;
            }
       //   break;
        case PRINT:
            if(serial_out(*ptThis->pchStr)){
                ptThis->pchStr++;
                ptThis->chState = CHECK_TAIL;
            }
            break;
    }
    return fsm_rt_on_going;
}
