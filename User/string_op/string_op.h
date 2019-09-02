#ifndef __STRING_OP_H__
#define __STRING_OP_H__
#   include "system.h"
#include "oopc.h"

DEF_CLASS(check_str_t)
    uint8_t chState;
    uint8_t * pchStr;
    uint8_t chCount;
    struct{
        fsm_rt_t (*check_string)(check_str_t *ptThis,uint8_t chByte);
    };
END_DEF_CLASS(check_str_t)


DEF_STRUCT(check_str_item_t)
    check_str_t tThis;
    check_str_item_t * ptNext;
END_DEF_STRUCT(check_str_item_t)

//extern void init_check_string(check_str_t *,uint8_t *);
extern check_str_t * new_check_str(uint8_t * pchStr);
extern fsm_rt_t check_string(check_str_t *,uint8_t );
extern bool check_init_heap(void *pBlock,uint16_t hwSize);


#endif

