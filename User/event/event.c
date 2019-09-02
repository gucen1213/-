#include "system.h"

struct event;
typedef struct event event_t;
struct event{
    bool bAutoReset;
    bool bIsSet;
};

void init_event(event_t *ptEvent,bool bInitValue,bool bManual)  
{                     
    if(NULL == ptEvent){ 
        return; 
    }    
    if(bManual){      
        ptEvent->bAutoReset = false;   //ÊÖ¶¯¸´Î»
    }else{     
        ptEvent->bAutoReset = true;    //????
    }     
    ptEvent->bIsSet = bInitValue;  
}


bool wait_event(event_t *ptEvent)
{
    if(NULL == ptEvent){
        return false;
    }
    if(ptEvent->bIsSet){
        if(ptEvent->bAutoReset){
            ptEvent->bIsSet = false;
        }
        return true;
    }
    return false;
}

void set_event(event_t *ptEvent)
{
    if(NULL == ptEvent){
        return;
    }
    ptEvent->bIsSet = true;
}

void reset_event(event_t *ptEvent)
{
    if(NULL == ptEvent){
        return;
    }
    if(ptEvent->bIsSet){
        ptEvent->bIsSet = false;
    }
}
