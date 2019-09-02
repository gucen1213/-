#ifndef __OOPC_H__
#define __OOPC_H__


#define DEF_UNION(__NAME)  typedef union __##__NAME  __NAME; \
                           union __##__NAME{

#define END_DEF_UNION(__NAME)   \
                        };


#endif

