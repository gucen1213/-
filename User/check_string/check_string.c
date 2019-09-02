#include <stdint.h>
#include <stdbool.h>

#include "system.h"
#include ".\app_cfg.h"

typedef bool stream_in_t(uint8_t *pchByte);

DECLARE_CLASS(check_str_t);
DEF_CLASS(check_str_t)
    uint8_t chState;
    uint8_t *pchStr;
    uint8_t chCount;
    stream_in_t *fnIn;
END_DEF_CLASS(check_str_t)

bool init_check_string(check_str_t *ptFSM,uint8_t *pchStr,stream_in_t* fnIn)
{
    CLASS(check_str_t) *ptThis = (CLASS(check_str_t) *)ptFSM;

    if((NULL == ptThis)||(NULL == pchStr)||(NULL == fnIn)){
        return false;
    }

    ptThis->chCount = 0;
    ptThis->pchStr = pchStr;
    ptThis->fnIn = fnIn;
    return true;
}

#define CHECK_STRING_RESET_FSM()      \
    do {\
        ptThis->chState = 0;\
    } while(0);
fsm_rt_t check_string(check_str_t *ptFSM,bool *pbIsRequestDrop)
{
    enum{
        START = 0,
        CHECK_NULL,
        CHECK_TAIL,
        CHECK_CHAR,
    };    
    CLASS(check_str_t) *ptThis = (CLASS(check_str_t) *)ptFSM;
    uint8_t chByte;

    if((NULL == ptThis) &&(NULL == pbIsRequestDrop)){
        return fsm_rt_err;
    }
    switch(ptThis->chState){
        case START:
            ptThis->chCount = 0;
            *pbIsRequestDrop = false;
            ptThis->chState = CHECK_NULL;
        case CHECK_NULL:	
            while(ptThis->fnIn(&chByte)){
                if(chByte != ptThis->pchStr[ptThis->chCount]){
                    *pbIsRequestDrop = true;  
                    break;
                }
                ptThis->chCount++;
                if('\0' == ptThis->pchStr[ptThis->chCount]){
                    CHECK_STRING_RESET_FSM();
                    return fsm_rt_cpl;
                }
            }
            CHECK_STRING_RESET_FSM();
    }  
    return fsm_rt_on_going;
}

