#include "my_platform_url_list.h"
static struct list_head url_list = LIST_HEAD_INIT(url_list);
static url_node_t *current_iter = NULL;
static int url_count = 0;
/*====== 接口实现 ======*/

// 初始化URL链表
void my_url_list_init(void)
{
    if (!list_empty(&url_list)) {
        my_url_list_destroy();
    } else {
        INIT_LIST_HEAD(&url_list);
    }
    url_count = 0;
    current_iter = NULL;
}

// 销毁URL链表并释放所有资源
void my_url_list_destroy(void)
{
    url_node_t *pos, *n;

    list_for_each_entry_safe(pos, n, &url_list, list) {
        if (pos->url) {
            free(pos->url);
        }
        list_del(&pos->list);
        free(pos);
    }

    INIT_LIST_HEAD(&url_list);
    url_count = 0;
    current_iter = NULL;
}

// 添加URL到链表
void my_url_set(char *url)
{
    url_node_t *new_node = malloc(sizeof(url_node_t));
    if (!new_node) {
        return;
    }

    new_node->url = strdup(url);
    if (!new_node->url) {
        free(new_node);
        return;
    }
    list_add_tail(&new_node->list, &url_list);
    url_count++;
}

// 打印所有URL
void my_print_urls(void)
{
    if (list_empty(&url_list)) {
        printf("URL list is empty\n");
        return;
    }

    url_node_t *node;
    printf("URL List (%d items):\n", url_count);
    list_for_each_entry(node, &url_list, list) {
        printf("%s\n", node->url);
    }
}

// 通过索引获取URL
char *my_get_url_by_index(int index)
{
    if (index < 0 || index >= url_count) {
        return NULL;
    }

    url_node_t *node;
    int i = 0;
    list_for_each_entry(node, &url_list, list) {
        if (i == index) {
            return node->url;
        }
        i++;
    }
    return NULL;
}

// 获取第一个URL
char *my_url_get_first(void)
{
    if (list_empty(&url_list)) {
        return NULL;
    }

    url_node_t *first = list_entry(url_list.next, url_node_t, list);
    current_iter = first;
    return first->url;
}

// 获取下一个URL
char *my_url_get_next(void)
{
    if (!current_iter) {
        return NULL;
    }
    struct list_head *current_node = &current_iter->list;
    if (current_node->next == &url_list) {
        current_iter = NULL;
        return NULL;
    }
    url_node_t *next = list_entry(current_node->next, url_node_t, list);
    current_iter = next;
    return next->url;
}

// 重置迭代器
void my_url_reset_iterator(void)
{
    current_iter = NULL;
}

// 获取URL计数
int my_url_get_count(void)
{
    return url_count;
}

int my_url_is_last(void)
{
    if (!current_iter) {
        return 0;
    }
    return (current_iter->list.next == &url_list);
}


#if 0
void test_url_management()
{
    mem_stats();
    printf("URL List Manager Test\n");
    printf("=====================\n");

    // 1. 初始化URL链表
    my_url_list_init();
    printf("1. List initialized\n");

    // 2. 添加URL
    my_url_set("https://www.example.com");
    /* my_url_set("https://www.github.com"); */
    /* my_url_set("https://www.openai.com"); */
    /* my_url_set("https://www.kernel.org"); */
    /* my_url_set("https://www.python.org"); */
    printf("2. URLs added\n");

    // 3. 打印所有URL
    my_print_urls();

    // 4. 按索引获取URL
    printf("\n4. Access by index:");
    for (int i = 0; i < my_url_get_count(); i++) {
        char *url = my_get_url_by_index(i);
        printf("\n  [%d] %s", i, url);
    }
    printf("\n");

    // 5. 迭代器测试
    printf("\n5. Iterator test:");
    int count = 0;
    for (char *url = my_url_get_first();
         url != NULL;
         url = my_url_get_next()) {
        printf("\n  Iteration %d: %s", ++count, url);
    }

    // 6. 重置迭代器
    my_url_reset_iterator();

    // 7. 边缘测试：越界访问
    printf("\n\n7. Boundary tests:");
    printf("\n  Index -1: %s",
           my_get_url_by_index(-1) ? "Found" : "NULL");
    printf("\n  Index %d: %s",
           my_url_get_count(),
           my_get_url_by_index(my_url_get_count()) ? "Found" : "NULL");

    // 8. 销毁链表
    my_url_list_destroy();
    printf("\n\n8. List destroyed\n");

    // 9. 销毁后访问
    printf("\n9. Post-destruction access:");
    printf("\n  Count: %d", my_url_get_count());
    my_print_urls();

    // 10. 重新初始化并添加新URL
    my_url_list_init();
    my_url_set("https://www.new-start.com");
    printf("\n10. Reinitialized list:\n");
    my_print_urls();
    my_url_list_destroy();

    mem_stats();
    return;
}
#endif
