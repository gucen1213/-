#ifndef _PRINTF_STRING_H_
#define _PRINTF_STRING_H_

#include ".\app_cfg.h"

#define SERIAL_OUT_HANDLE   serial_out

DECLARE_CLASS(print_str_t)
EXTERN_CLASS(print_str_t)
    uint8_t chState;
    uint8_t * pchStr;   //×Ö·û´®Ö¸Õë
END_EXTERN_CLASS(print_str_t)

extern bool printf_string_init(print_str_t * ptThis,uint8_t *pchStr);
extern fsm_rt_t printf_string(print_str_t * ptThis);

#endif
