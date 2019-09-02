#ifndef _CRITICAL_H_
#define _CRITICAL_H_

#include "system.h"
#include ".\app_cfg.h"
typedef struct critical_sector critical_sector_t;
struct critical_sector{
//    event_t tUnlocked;
};

extern void init_critical_sector(critical_sector_t *ptMutex);
extern bool enter_critical_sector(critical_sector_t *ptMutex);
extern bool leave_critical_sector(critical_sector_t *ptMutex);

#define INIT_CRITICAL_SECTOR(__MUTEX)   init_critical_sector(__MUTEX)
#define ENTER_CRITICAL_SECTOR(__MUTEX)  enter_critical_sector(__MUTEX)
#define LEAVE_CRITICAL_SECTOR(__MUTEX)  leave_critical_sector(__MUTEX)

#endif

