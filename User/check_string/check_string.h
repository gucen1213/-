#ifndef _CHECK_STRING_H_
#define _CHECK_STRING_H_

#include ".\app_cfg.h"

typedef bool stream_in_t(uint8_t *pchByte);

DECLARE_CLASS(check_str_t);
EXTERN_CLASS(check_str_t)
    uint8_t chState;
    uint8_t *pchStr;
    uint8_t chCount;
    stream_in_t *fnIn;
END_EXTERN_CLASS(check_str_t)

extern bool init_check_string(check_str_t *ptThis,uint8_t *pchStr,stream_in_t* fnIn);

extern fsm_rt_t check_string(check_str_t *ptThis,bool *pbIsRequestDrop);

#endif
