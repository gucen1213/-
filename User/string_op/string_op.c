#include "string_op.h"
#   include "system.h"
#define this   (*ptThis)


static check_str_item_t * s_tCheckList = NULL;

///////////////////////////////////////////////////////////

//从堆中分配
static check_str_t * check_str_new(void)
{
    if(NULL == s_tCheckList){
        return NULL;
    }
    check_str_item_t * ptHeapItem = s_tCheckList;
    s_tCheckList = ptHeapItem->ptNext;
    ptHeapItem->ptNext = NULL;
    return ((check_str_t *) ptHeapItem);
}

//释放到堆
static void  check_str_free(check_str_t * ptItem)
{
    check_str_item_t * ptHeapItem = (check_str_item_t *) ptItem;
    if(NULL == ptItem){
        return;
    }
    ptHeapItem->ptNext = s_tCheckList;
    s_tCheckList = ptHeapItem;
}


//堆初始化
bool check_init_heap(void *pBlock,uint16_t hwSize)
{
    uint16_t hwCount = 0;
    check_str_item_t * ptTYPE = (check_str_item_t *)pBlock;

    if(NULL == ptTYPE){
        return false;
    }
    hwCount = hwSize/sizeof(check_str_item_t);
    if(0 == hwCount){
        return false;
    }
    do{
        check_str_free((check_str_t *)ptTYPE);
        ptTYPE++;
        hwCount--;
    }while(hwCount);
    return true;    
}


bool init_check_string(check_str_t *ptThis,uint8_t *pchStr)
{
    if((NULL == ptThis)||(NULL == pchStr)){
        return false;
    }
    this.chState = 0;
    this.chCount = 0;
    this.pchStr = pchStr;
    this.check_string = &check_string;   //赋值按键检测状态机
    return true;
} 



#define CHECK_STRING_RESET_FSM()      \
    do {\
        this.chState = 0;\
    } while(0);
fsm_rt_t check_string(check_str_t *ptThis,uint8_t chByte)
{
    enum{
        START = 0,
        CHECK_NULL,
        CHECK_TAIL,
        CHECK_CHAR,
    };    
    switch(this.chState){
        case START:
            this.chCount = 0;
            this.chState = CHECK_NULL;
//            break;
        case CHECK_NULL:
            if(this.pchStr[0] == '\0'){
                CHECK_STRING_RESET_FSM();
                return fsm_rt_cpl;
            }
            this.chState = CHECK_CHAR;
 //           break;
        case CHECK_CHAR:
            if(chByte != this.pchStr[this.chCount]){
                CHECK_STRING_RESET_FSM();
                break;
            }
            this.chCount++;
            this.chState = CHECK_TAIL;
//            break;
        case CHECK_TAIL:
            if('\0' == this.pchStr[this.chCount]){
                CHECK_STRING_RESET_FSM();
                return fsm_rt_cpl;
            }
            this.chState = CHECK_CHAR;
            break;
    }
    return fsm_rt_on_going;
}


check_str_t * new_check_str(uint8_t * pchStr)
{
    if(NULL == pchStr){
        return NULL;
    }
    check_str_t *ptHeapItem = check_str_new();
    if(NULL == ptHeapItem){
        return NULL;
    }
    if(init_check_string(ptHeapItem,pchStr)){
        return ((check_str_t *) ptHeapItem);
    }
    return NULL;
}
