#ifndef _EVENT_H_
#define _EVENT_H_

#include "system.h"
#include "./app_cfg.h"

struct event;
typedef struct event event_t;
struct event{
    bool bAutoReset;
    bool bIsSet;
};

extern void init_event(event_t *ptEvent,bool bInitValue,bool bManual);  
extern bool wait_event(event_t *ptEvent);
extern void set_event(event_t *ptEvent);
extern void reset_event(event_t *ptEvent);

#define INIT_EVENT(__EVENT,__INIT_VALUE,__MANUAL)  init_event(__EVENT,__INIT_VALUE,__MANUAL)
#define SET_EVENT(__EVENT)                        set_event(__EVENT)
#define WAIT_EVENT(__EVENT)                       wait_event(__EVENT)
#define RESET_EVENT(__EVENT)                      reset_event(__EVENT)


#endif

