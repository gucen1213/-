#ifndef _CHECK_LIST_H_
#define _CHECK_LIST_H_

#   include "./app_cfg.h"
#   include "../queue/queue.h"

typedef bool stream_in_t(uint8_t *pchByte);
typedef bool stream_out_t(uint8_t chByte);

#if 0
typedef struct{
    stream_in_t *fnByteIn;
    stream_out_t *fnByteOut;
}pipe_io_t;
#else
DEF_INTERFACE(pipe_io_t)
    stream_in_t *fnByteIn;
    stream_out_t *fnByteOut;
END_DEF_INTERFACE(pipe_io_t)
#endif

typedef fsm_rt_t checker_t(stream_in_t *fnByteIn,bool *pbIsRequestDrop);

DECLARE_CLASS(checker_list_t)
DECLARE_CLASS(checker_item_t)

declare_simple_fsm(batch_check_task);
extern_fsm_implementation(batch_check_task);

#if 0
typedef struct i_check_list i_check_list_t;
struct i_check_list{
    bool                    (*Init)   (checker_list_t *ptThis);
    batch_check_task_fn     *Task; 
    struct{
        bool                (*Add)    (checker_list_t *ptThis,checker_item_t *ptItem);
        checker_item_t *    (*Remove) (checker_list_t *ptThis,checker_item_t *ptItem);
    }Item;
};
#else
DEF_INTERFACE(i_check_list_t)
    bool      (*Init)   (checker_list_t *ptThis);
    batch_check_task_fn  *Task; 
    struct{
        bool                (*Add)    (checker_list_t *ptThis,checker_item_t *ptItem);
        checker_item_t *    (*Remove) (checker_list_t *ptThis,checker_item_t *ptItem);
        bool                (*Init)   (checker_item_t * ptItemTmp,checker_t * ptCheck);
    }Item;
END_DEF_INTERFACE(i_check_list_t)
#endif

extern_fsm_implementation(batch_check_task);
extern_fsm_initialiser(batch_check_task,
    args(checker_list_t tCheckList,
        pipe_io_t *ptPipe,
        byte_queue_t *ptQueue)
    );
    
//定义链表元素类
EXTERN_CLASS(checker_item_t)
    checker_t      *ptItem;
    checker_item_t *ptNext;
END_EXTERN_CLASS(checker_item_t)

//定义链表类
EXTERN_CLASS(checker_list_t /*,IMPLEMENT(i_check_list_t)*/)
    checker_item_t *ptRoot;
    checker_item_t *ptCurrent;
END_EXTERN_CLASS(checker_list_t /*,IMPLEMENT(i_check_list_t)*/)

extern_simple_fsm(batch_check_task,
    def_params(
        checker_list_t tCheckList;
        checker_item_t *ptItem;
        byte_queue_t *ptQueue;
        bool bIsRequestDropTemp;
        stream_in_t *fnByteIn;
        stream_out_t *fnByteOut;
    )
);

extern checker_item_t* check_list_next_item(checker_list_t *ptThis);
extern bool check_list_add_item(checker_list_t *ptThis,checker_item_t *ptItem);
extern checker_item_t*  check_list_remove_item(checker_list_t *ptThis,checker_item_t *ptItem);
extern bool check_list_init(checker_list_t *ptThis);
extern bool check_item_init(checker_item_t * ptCheckItem,checker_t * ptCheck);

extern const i_check_list_t CHECK_LIST;
#endif

