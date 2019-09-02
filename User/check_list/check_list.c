#include "system.h"
#include "./app_cfg.h"
#include "../queue/queue.h"

#define this (*ptThis)

typedef bool stream_in_t(uint8_t *pchByte);
typedef bool stream_out_t(uint8_t chByte);
#if 0
DEF_STRUCTURE(pipe_io_t)
    stream_in_t *fnByteIn;
    stream_out_t *fnByteOut;
END_DEF_STRUCTURE(pipe_io_t)
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
    bool      (*Init)   (checker_list_t *ptThis);
    batch_check_task_fn  *Task; 
    struct{
        bool  (*Add)        (checker_list_t *ptThis,checker_item_t *ptItem);
        checker_item_t *    (*Remove) (checker_list_t *ptThis,checker_item_t *ptItem);
        bool                (*Init)   (checker_item_t * ptItemTmp,checker_t * ptCheck);
    }Item;
};
#else
DEF_INTERFACE(i_check_list_t)
    //bool      (*Init)   (checker_list_t *ptThis);
    batch_check_task_fn  *Task; 
    struct{
        bool                (*Add)    (checker_list_t *ptThis,checker_item_t *ptItem);
        checker_item_t *    (*Remove) (checker_list_t *ptThis,checker_item_t *ptItem);
        bool                (*Init)   (checker_item_t * ptItemTmp,checker_t * ptCheck);
    }Item;
END_DEF_INTERFACE(i_check_list_t)
#endif

DEF_CLASS(checker_item_t)
    checker_t      *ptItem;
    checker_item_t *ptNext;
END_DEF_CLASS(checker_item_t)

DEF_CLASS(checker_list_t /*, IMPLEMENT(i_check_list_t) */)
    checker_item_t *ptRoot;
    checker_item_t *ptCurrent;
END_DEF_CLASS(checker_list_t /*, IMPLEMENT(i_check_list_t)*/)

static bool check_list_init(checker_list_t *ptList);
    
/*! /brief define fsm batch_check_task
 *!        list all the parameters
 */
simple_fsm(batch_check_task,
    def_params(
        checker_list_t tCheckList;
        checker_item_t *ptItem;
        byte_queue_t *ptQueue;
        bool bIsRequestDropTemp;
        stream_in_t *fnByteIn;
        stream_out_t *fnByteOut;
//        pipe_io_t *ptPipe;
    )
)

//查找下一个
checker_item_t* check_list_next_item(checker_list_t *ptList)
{
    CLASS(checker_list_t) *ptThis = (CLASS(checker_list_t) *)ptList;
    CLASS(checker_item_t) *ptPtr = NULL;
    if(NULL == ptThis){
        return NULL;
    }
    ptPtr = (CLASS(checker_item_t)*)this.ptCurrent;
    if(this.ptCurrent == NULL){
        this.ptCurrent = this.ptRoot;
    }else{
        this.ptCurrent = ((CLASS(checker_item_t)*)this.ptCurrent)->ptNext;
    }
    return (checker_item_t*)ptPtr;
}

bool check_list_add_item(checker_list_t *ptList,checker_item_t *ptItemTmp)
{
    CLASS(checker_list_t) *ptThis = (CLASS(checker_list_t) *)ptList;
    
    CLASS(checker_item_t) *ptPtr = NULL;
    
    if((NULL == ptThis) || (NULL == ptItemTmp)){
        return false;
    }
    if(this.ptRoot == NULL){
        this.ptRoot = ptItemTmp;
        this.ptCurrent = ptItemTmp;
        ((CLASS(checker_item_t)*)ptItemTmp)->ptNext = NULL;
        return true;
    }
    
    ptPtr = (CLASS(checker_item_t)*)this.ptCurrent;
    while(NULL != ptPtr->ptNext){
        ptPtr = (CLASS(checker_item_t)*)ptPtr->ptNext;
    }

    ptPtr->ptNext = ptItemTmp;
    ((CLASS(checker_item_t)*)ptItemTmp)->ptNext = NULL;

    if(NULL == this.ptCurrent){
        this.ptCurrent = ptItemTmp;
    }
    return true;
}

checker_item_t*  check_list_remove_item(checker_list_t *ptList,checker_item_t *ptItemTmp)
{
    CLASS(checker_list_t) *ptThis = (CLASS(checker_list_t) *)ptList;
    
    checker_item_t* ptItemCheck;
    checker_item_t* ptItemCheckPrevious = NULL;
    
    if(NULL == ptThis || NULL == ptItemTmp){
        return NULL;
    }

    ptItemCheck = this.ptRoot;
    ptItemCheckPrevious = this.ptRoot;

    while((NULL != ptItemCheck) && (ptItemCheck != ptItemTmp)){
        ptItemCheckPrevious = ptItemCheck;
        ptItemCheck = ((CLASS(checker_item_t)*)ptItemCheck)->ptNext;
    }
    
    if(NULL != ptItemCheck){
        if(this.ptCurrent == ptItemTmp){
            this.ptCurrent = ((CLASS(checker_item_t)*)ptItemTmp)->ptNext;
        }
        
        if(this.ptRoot == ptItemTmp){
            this.ptRoot = ((CLASS(checker_item_t)*)ptItemTmp)->ptNext;
        }

        ((CLASS(checker_item_t)*)ptItemCheckPrevious)->ptNext = ((CLASS(checker_item_t)*)ptItemTmp)->ptNext;
        return (checker_item_t*)ptItemTmp;
    }
    return NULL;
}


bool check_item_init(checker_item_t * ptItemTmp,checker_t * ptCheck)
{
    CLASS(checker_item_t) *ptItem = (CLASS(checker_item_t) *)ptItemTmp;
    if((NULL == ptItem) || (NULL == ptCheck)){
        return false;
    }
    ptItem->ptItem = ptCheck;
    return true;
}

fsm_initialiser(batch_check_task,
    args(checker_list_t tCheckList,
        pipe_io_t *ptPipe,
        byte_queue_t *ptQueue))

    init_body(
        if(NULL == ptPipe){
            abort_init();
        }
        check_list_init(&(this.tCheckList));
        this.fnByteIn = ptPipe->fnByteIn;
        this.fnByteOut = ptPipe->fnByteOut;
        this.ptQueue = ptQueue;
    )
/*! /brief Implement the fsm: batch_check_task
 *         This fsm only contains one state.
 */
fsm_implementation(batch_check_task)
    
    def_states(CHECK,RUN_ITEM)

    uint8_t chByte;
    bool bIsRequestDrop;
    CLASS(checker_item_t) *ptItemTmp = (CLASS(checker_item_t) *)this.ptItem;

    body(
        on_start(
            this.bIsRequestDropTemp = true;
            update_state_to(CHECK);
        )
        state(CHECK,
            this.ptItem = check_list_next_item(&this.tCheckList);
            if(NULL == this.ptItem){
                if(this.bIsRequestDropTemp){                   
                    DEQUEUE_BYTE(this.ptQueue,&chByte);
                }
                fsm_cpl(); 
            }
            transfer_to(RUN_ITEM);
        )
        state(RUN_ITEM,
            RESET_PEEK_BYTE(this.ptQueue);
            bIsRequestDrop = false;
            if(fsm_rt_cpl == ptItemTmp->ptItem(this.fnByteIn,&bIsRequestDrop)){
                GET_ALL_PEEK_BYTE(this.ptQueue);
                fsm_cpl();
            }
            this.bIsRequestDropTemp &= bIsRequestDrop;
            transfer_to(CHECK);
        )
    )


const i_check_list_t CHECK_LIST = {
    //.Init = &check_list_init,
    .Task = &batch_check_task,
    .Item = {
        .Add = &check_list_add_item,
        .Remove = &check_list_remove_item,
        .Init = &check_item_init,
    },
};

bool check_list_init(checker_list_t *ptList)
{
    CLASS(checker_list_t) *ptThis = (CLASS(checker_list_t) *)ptList;
    if(NULL == ptThis){
        return false;
    }
    
    //OBJ_CONVERT_AS(this, i_check_list_t) = CHECK_LIST;
    
    this.ptRoot      = NULL;
    this.ptCurrent   = NULL;
    return true;
}












