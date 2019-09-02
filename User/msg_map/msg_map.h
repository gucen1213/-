#ifndef _MSG_MAP_H_
#define _MSG_MAP_H_

#include ".\app_cfg.h"
typedef bool stream_in_t(uint8_t *pchByte);

typedef struct _msg_t msg_t;
typedef bool MSG_HANDLER(const msg_t *ptMSG);

struct _msg_t {
    uint8_t* chMSG;                 
    MSG_HANDLER *fnHandler;          
};

extern const msg_t *search_msg_map(void);
extern const msg_t *search_msg_map_stream(stream_in_t *fnIn,bool *pbIsRequestDrops);
#endif
