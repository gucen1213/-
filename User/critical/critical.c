#include "..\event\event.h"

typedef struct critical_sector critical_sector_t;
struct critical_sector{
    event_t tUnlocked;
};
void init_critical_sector(critical_sector_t *ptMutex)
{
    if(NULL == ptMutex){
        return;
    }
    INIT_EVENT(&(ptMutex->tUnlocked),true,false);
}

bool enter_critical_sector(critical_sector_t *ptMutex)
{
    if(NULL == ptMutex){
        return false;
    }
    return WAIT_EVENT(&(ptMutex->tUnlocked));
}

bool leave_critical_sector(critical_sector_t *ptMutex)
{
    if(NULL == ptMutex){
        return false;
    }
    SET_EVENT(&ptMutex->tUnlocked);   //¸´Î»Unlocked
    return true;
}

