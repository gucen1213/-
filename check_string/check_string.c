//include "check_string.h"
#include "queue.h"
#include "ooc.h"
#include "simple_fsm.h"
#include <stdint.h>
#include <stdbool.h>

typedef bool stream_in_t(uint8_t *pchByte);

simple_fsm(check_str,
        def_params(
            uint8_t *pchStr;
            uint8_t chCount;
            stream_in_t *fnIn;
        )
    )

//bool init_check_string(check_str_t *ptThis,uint8_t *pchStr,stream_in_t* fnIn)
//{
//    if((NULL == ptThis)||(NULL == pchStr)||(NULL == fnIn)){
//        return false;
//    }
//    ptThis->chState = 0;
//    ptThis->chCount = 0;
//    ptThis->pchStr = pchStr;
//    ptThis->fnIn = fnIn;
//    return true;
//}

fsm_initialiser(check_str,
    args(
        uint8_t *pchStr,stream_in_t* fnIn
    ))
    init_body(
        if((NULL == ptThis)||(NULL == pchStr)||(NULL == fnIn)){
            abort_init();
        }
        ptThis->chCount = 0;
        ptThis->pchStr = pchStr;
        ptThis->fnIn = fnIn;
    )



implement_fsm(check_str,
    args(
        bool *pbIsRequestDrop
    ))
    
    def_states( CHECK_NULL )
    uint8_t chByte;
    body (
        on_start(
            this.chCount = 0;
            *pbIsRequestDrop = false;
            update_state_to(CHECK_NULL);            
        )

        state(CHECK_NULL,
            while(ptThis->fnIn(&chByte)){
                if(chByte != ptThis->pchStr[ptThis->chCount]){
                    *pbIsRequestDrop = true;  
                    break;
                }
                ptThis->chCount++;
                if('\0' == ptThis->pchStr[ptThis->chCount]){
                    fsm_cpl();
                }
            }
            reset_fsm();
            fsm_on_going();
        )
    )
//#define CHECK_STRING_RESET_FSM()      \
//    do {\
//        ptThis->chState = 0;\
//    } while(0);
//fsm_rt_t check_string(check_str_t *ptThis,bool *pbIsRequestDrop)
//{
//    enum{
//        START = 0,
//        CHECK_NULL,
//        CHECK_TAIL,
//        CHECK_CHAR,
//    };    
//    static uint8_t s_chByte;
//    if((NULL == ptThis) &&(NULL == pbIsRequestDrop)){
//        return fsm_rt_err;
//    }
//    switch(ptThis->chState){
//        case START:
//            ptThis->chCount = 0;
//            *pbIsRequestDrop = false;
//            ptThis->chState = CHECK_NULL;
//        case CHECK_NULL:	
//            while(ptThis->fnIn(&s_chByte)){
//                if(s_chByte != ptThis->pchStr[ptThis->chCount]){
//                    *pbIsRequestDrop = true;  
//                    break;
//                }
//                ptThis->chCount++;
//                if('\0' == ptThis->pchStr[ptThis->chCount]){
//                    CHECK_STRING_RESET_FSM();
//                    return fsm_rt_cpl;
//                }
//            }
//            CHECK_STRING_RESET_FSM();
//    }  
//    return fsm_rt_on_going;
//}
