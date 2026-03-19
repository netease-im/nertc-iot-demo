#ifndef _MY_PLATFORM_URL_LIST_H_
#define _MY_PLATFORM_URL_LIST_H_

#include "my_platform_common.h"

#include "list.h"
#include <stdlib.h>
#include <string.h>

typedef struct url_node {
    struct list_head list;
    char *url;
} url_node_t;

extern int my_url_is_last(void);
extern void my_url_list_init(void);
extern void my_url_list_destroy(void);
extern void my_url_set(char *url);
extern void my_print_urls(void);
extern char *my_get_url_by_index(int index);
extern char *my_url_get_first(void);
extern char *my_url_get_next(void);
extern void my_url_reset_iterator(void);
extern int my_url_get_count(void);



#endif
