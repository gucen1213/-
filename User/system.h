#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "usart.h"
#include "LED.h"
#include "key.h"

#define ABS(__N)    ((__N) < 0 ? -(__N) : (__N))
#define TOP         (0x01FF)
#define _BV(__N)	((uint32_t)1<<(__N))

// °´¼ü±àÂë
#define KEY_CODE_ESC    0x00001BUL
#define KEY_CODE_F1     0x1B4F50UL
#define KEY_CODE_F3     0x1B4F52UL
#define KEY_CODE_UP     0x1B5B41UL
#define KEY_CODE_DOWN   0x1B5B42UL

//typedef enum{
//	fsm_rt_err = -1,
//	fsm_rt_cpl = 0,
//	fsm_rt_on_going = 1,
//}fsm_rt_t;

#define Log(__STR)  \
    do{       \
        uint8_t *pchSTR = (__STR);  \
        do{          \
            while(!serial_out(*pchSTR)); \
            pchSTR++;         \
            if('\0' == *pchSTR){    \
                break;    \
            }   \
        }while(1); \
    }while(0)  

#define UBOUND(_STR)   sizeof(_STR)/sizeof(_STR[0])

#define DELAY_1000MS  0x4FFFF
	
#endif
