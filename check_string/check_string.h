#ifndef _CHECK_STRING_H_
#define _CHECK_STRING_H_

#include <stdbool.h>
#include <stdint.h>
#include "ooc.h"

typedef bool stream_in_t(uint8_t *pchByte);


extern_simple_fsm( check_str,
    def_params(
        uint8_t * pchStr;
        uint8_t chCount;
        stream_in_t *fnIn;
    )
)

extern_fsm_initialiser( check_str,
    args(           
        uint8_t *pchStr,stream_in_t* fnIn
    ))

#endif






 