#include "check_list.h"
#define this (*ptThis)


//查找下一个
checker_item_t* check_list_next_item(checker_list_t *ptThis)
{
    checker_item_t *ptPtr = NULL;
    if(NULL == ptThis){
        return NULL;
    }
    ptPtr = this.ptCurrent;
    if(this.ptCurrent == NULL){
        this.ptCurrent = this.ptRoot;
    }else{
        this.ptCurrent = this.ptCurrent->ptNext;
    }
    return ptPtr;
}   

bool check_list_add_item(checker_list_t *ptThis,checker_item_t *ptItem)
{
    checker_item_t *ptPtr = NULL;
    
    if((NULL == ptThis) || (NULL == ptItem)){
        return false;
    }
    if(this.ptRoot == NULL){
        this.ptRoot = ptItem;
        this.ptCurrent = ptItem;
        ptItem->ptNext = NULL;
        return true;
    }
    
    ptPtr = this.ptCurrent;
    while(NULL != ptPtr->ptNext){
        ptPtr = ptPtr->ptNext;
    }

    ptPtr->ptNext = ptItem;
    ptItem->ptNext = NULL;

    if(NULL == this.ptCurrent){
        this.ptCurrent = ptItem;
    }
    return true;
}

checker_item_t*  check_list_remove_item(checker_list_t *ptThis,checker_item_t *ptItem)
{
    checker_item_t* ptItemCheck;
    checker_item_t* ptItemCheckPrevious = NULL;
    
    if(NULL == ptThis || NULL == ptItem){
        return NULL;
    }

    ptItemCheck = this.ptRoot;
    ptItemCheckPrevious = this.ptRoot;

    while((NULL != ptItemCheck) && (ptItemCheck != ptItem)){
        ptItemCheckPrevious = ptItemCheck;
        ptItemCheck = ptItemCheck->ptNext;
    }
    
    if(NULL != ptItemCheck){
        if(this.ptCurrent == ptItem){
            this.ptCurrent = ptItem->ptNext;
        }
        
        if(this.ptRoot == ptItem){
            this.ptRoot = ptItem->ptNext;
        }

        ptItemCheckPrevious->ptNext = ptItem->ptNext;
        return ptItem;
    }
    
    return NULL;
}

//init check list
bool check_list_init(checker_list_t *ptThis,stream_in_t *fnIn)
{
    if(NULL == ptThis){
        return false;
    }

    this.ptRoot = NULL;
    this.ptCurrent = NULL;
    this.fnIn = fnIn;
    return true;
}



