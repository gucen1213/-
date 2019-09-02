#include "system.h"
#include "check_string.h"
#include ".\app_cfg.h"

typedef struct _msg_t msg_t;
typedef bool MSG_HANDLER(const msg_t *ptMSG);

struct _msg_t {
    uint8_t* chMSG;                 
    MSG_HANDLER *fnHandler;          
};

typedef bool MSG_HANDLER(const msg_t *ptMSG);

INSERT_MSG_MAP_FUNC_EXRERN
const static msg_t c_tMSGMap[] = {
    INSERT_MSG_MAP_CMD
};

#define SEARCH_MSG_MAP_RESET()  \
    do {\
        s_tState = START;\
    } while(0);
const msg_t * search_msg_map_stream(stream_in_t *fnIn,bool *pbIsRequestDrops)
{
    static enum{
        START = 0,
        END_LOOP,
        BODY_START,
        BODY_END,
    }s_tState = START;
    static uint16_t s_hwLoop;
    static check_str_t s_ptItem;
    static bool s_bIsRequestDropTemp;
    static bool s_bIsRequestDrop;
    switch(s_tState){
        case START:
            s_tState = END_LOOP;
            s_bIsRequestDropTemp = true;
            s_hwLoop = 0;
            *pbIsRequestDrops = false;
       //     break;
        case END_LOOP:
            if(s_hwLoop >= UBOUND(c_tMSGMap)){
                if(s_bIsRequestDropTemp){
                    *pbIsRequestDrops = true;
                }
                SEARCH_MSG_MAP_RESET();
            }else{
                s_tState = BODY_START;
            }
            break;
        case BODY_START:
            if(!init_check_string(&s_ptItem,(uint8_t *)c_tMSGMap[s_hwLoop].chMSG,fnIn)){
                SEARCH_MSG_MAP_RESET();
                break;
            }
            if(fsm_rt_cpl == check_string(&s_ptItem,&s_bIsRequestDrop)){           
                SEARCH_MSG_MAP_RESET();
                return (msg_t *)(&c_tMSGMap[s_hwLoop]);
            }
            s_bIsRequestDropTemp &= s_bIsRequestDrop;
            s_tState = BODY_END;
//            break;
        case BODY_END:
            s_hwLoop++;
            s_tState = END_LOOP;
            break;
    }
    return NULL;
}
