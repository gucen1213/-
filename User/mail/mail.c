#include "..\event\event.h"
#define this   (*ptThis)

typedef struct {
    event_t tIsNewMail;
    void * pObj;
} mailbox_t;

bool init_mailbox(mailbox_t *ptThis)
{
    if (NULL == ptThis) {
        return false;
    }
    this.pObj = NULL;
    INIT_EVENT(&this.tIsNewMail,false,false);
    return true;
}

bool send_mail(mailbox_t *ptThis,void * pObj)
{
    if ((ptThis != NULL)
        &&(pObj != NULL)) {
        SET_EVENT(&this.tIsNewMail);
        this.pObj = pObj;
        return true;
    }
    return false;
}

void * open_mail(mailbox_t *ptThis)
{
    if(ptThis == NULL){
        return NULL;
    }
    if(WAIT_EVENT(&this.tIsNewMail)){
        return this.pObj;
    }
    return NULL;
}

