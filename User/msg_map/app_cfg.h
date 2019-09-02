#ifndef __MSG_MAP_CFG_H__
#define __MSG_MAP_CFG_H__

#include "../app_cfg.h"

static uint8_t uchCodeF1[4] = {0x1B,0x4F,0x50,'\0'};
static uint8_t uchCodeUP[4] = {0x1B,0x5B,0x41,'\0'};
static uint8_t uchCodeDOWN[4] = {0x1B,0x5B,0x42,'\0'};

#define INSERT_MSG_MAP_FUNC_EXRERN                                          \
    extern bool msg_f1_handler(const msg_t *ptMSG);                         \
    extern bool msg_up_handler(const msg_t *ptMSG);                         \
    extern bool msg_down_handler(const msg_t *ptMSG);
//消息和对应的消息处理
#define INSERT_MSG_MAP_CMD  {uchCodeF1,&msg_f1_handler},                  \
                            {uchCodeUP,&msg_up_handler},                  \
                            {uchCodeDOWN,&msg_down_handler},

#endif

                            