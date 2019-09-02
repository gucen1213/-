#ifndef _CHECK_LIST_H_
#define _CHECK_LIST_H_

#   include "system.h"
#   include "oopc.h"
#   include "queue.h"

typedef bool stream_in_t(uint8_t* pchChar);
typedef fsm_rt_t checker_t(stream_in_t *fnIn,bool *pbIsRequestDrop);

//定义链表元素
DEF_STRUCTURE(checker_item_t)
    checker_t      *ptItem;
    checker_item_t *ptNext;
END_DEF_STRUCTURE(check_item_t)
//定义链表
DEF_STRUCTURE(checker_list_t)
    checker_item_t *ptRoot;
    checker_item_t *ptCurrent;
    stream_in_t    *fnIn;
END_DEF_STRUCTURE(checker_list_t)

extern checker_item_t* check_list_next_item(checker_list_t *ptThis);
extern bool check_list_add_item(checker_list_t *ptThis,checker_item_t *ptItem);
extern checker_item_t*  check_list_remove_item(checker_list_t *ptThis,checker_item_t *ptItem);
extern bool check_list_init(checker_list_t *ptThis,stream_in_t *fnIn);
//extern fsm_rt_t batch_check_task(checker_list_t *ptThis,stream_in_t *fnIn);
#endif

