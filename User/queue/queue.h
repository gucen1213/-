#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "system.h"
#include "./app_cfg.h"

typedef volatile struct{
    uint8_t  *pchBuffer;
    uint16_t hwSize;
    uint16_t hwHead;
    uint16_t hwTail;
    uint16_t hwLength;
	uint16_t hwPeek;
	uint16_t hwPeekLength;
}byte_queue_t;

extern bool init_byte_queue(byte_queue_t *ptThis,uint8_t *pchFIFOBuff ,uint16_t hwSize);
extern bool enqueue_byte(byte_queue_t *ptThis,uint8_t chByte);
extern bool dequeue_byte(byte_queue_t *ptThis,uint8_t *pchByte);
extern bool is_byte_queue_empty(byte_queue_t *ptThis);
extern bool peek_byte_queue(byte_queue_t *ptThis,uint8_t *pchByte);
extern bool get_all_peeked_byte(byte_queue_t *ptThis);
extern bool reset_peek_byte(byte_queue_t *ptThis);

#define INIT_BYTE_QUEUE(__QUEUE, __BUFFER, __SIZE)   init_byte_queue(__QUEUE,__BUFFER,__SIZE)
#define ENQUEUE_BYTE(__QUEUE, __OBJ)                 enqueue_byte(__QUEUE,__OBJ)
#define DEQUEUE_BYTE(__QUEUE, __ADDR)                dequeue_byte(__QUEUE,__ADDR)
#define IS_BYTE_QUEUE_EMPTY(__QUEUE)                 is_byte_queue_empty(__QUEUE)

#define RESET_PEEK_BYTE(__QUEUE)                     reset_peek_byte(__QUEUE)
#define PEEK_BYTE_QUEUE(__QUEUE,__ADDR)              peek_byte_queue(__QUEUE,__ADDR)
#define GET_ALL_PEEK_BYTE(__QUEUE)                   get_all_peeked_byte(__QUEUE)                                

#endif

