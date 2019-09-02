#ifndef     __MAIL_H__
#   define  __MAIL_H__

#   include ".\app_cfg.h"
typedef struct {
} mailbox_t;

extern bool init_mailbox(mailbox_t *);
extern bool send_mail(mailbox_t *, void * );
extern void * open_mail(mailbox_t *);

#   define INIT_MAILBOX(__MAIL)        init_mailbox(__MAIL)
#   define SEND_MAILBOX(__MAIL,__OBJ)  send_mail(__MAIL,__OBJ)
#   define OPEN_MAILBOX(__MAIL)        open_mail(__MAIL)

#endif

