#include "my_platform_http_api.h"

#define MY_HTTP_TEST	0

#if MY_HTTP_TEST
//选择其中一种进行测试
/* #define HTTP_GET_TEST */
#define HTTP_POST_TEST
//#define HTTP_PUT_TEST

#define HTTP_DEMO_URL  "http://www.baidu.com/search/error.html"
#define HTTP_DEMO_DATA "{\"name\":\"xiao ming\", \"age\":12}"

#define LOG_HTTP_HEAD_OPTION        \
    "POST /search/error.html HTTP/1.1\r\n"\
    "Host: www.baidu.com\r\n"\
    "Connection: close\r\n"\
    "Cache-Control: no-store\r\n"\
    "Content-type: application/json\r\n"\
    "Content-length: 30\r\n"\
    "Accept: */*\r\n"\
    "Accept-Language: zh-CN,zh,q=0.8\r\n"\
	"Cookie: loginType=hardware; appId=1000000001; deviceSN=A4W0H02B000001; credential=56fb1f8df901f13fea8bf12ac2109c58; randNum=2dcf4629\r\n\r\n"


httpin_error httpcli_put(httpcli_ctx *ctx)
{
    return httpcli_post(ctx);
}

static void http_test_task(void *priv)
{
    int ret = 0;
    http_body_obj http_body_buf;
    httpcli_ctx *ctx = (httpcli_ctx *)calloc(1, sizeof(httpcli_ctx));
    if (!ctx) {
        MY_LOG_E("calloc faile\n");
        return;
    }

    memset(&http_body_buf, 0x0, sizeof(http_body_obj));

    http_body_buf.recv_len = 0;
    http_body_buf.buf_len = 4 * 1024;
    http_body_buf.buf_count = 1;
    http_body_buf.p = (char *)malloc(http_body_buf.buf_len * http_body_buf.buf_count);
    if (!http_body_buf.p) {
        free(ctx);
        return;
    }

    ctx->url = HTTP_DEMO_URL;
    ctx->timeout_millsec = 5000;
    ctx->priv = &http_body_buf;
    ctx->connection = "close";

#if (defined HTTP_POST_TEST)
    MY_LOG_I("http post test start\n\n");
    ctx->data_format = "application/json";
    ctx->post_data = HTTP_DEMO_DATA;
    ctx->data_len = strlen(HTTP_DEMO_DATA);
    ctx->user_http_header = LOG_HTTP_HEAD_OPTION;	//添加该参数, 则自定义申请头部内容
    ret = httpcli_post(ctx);
#elif (defined HTTP_GET_TEST)
    MY_LOG_I("http get test start\n\n");
    ret = httpcli_get(ctx);
#elif (defined HTTP_PUT_TEST)
    MY_LOG_I("http put test start\n\n");
    ret = httpcli_put(ctx);
#endif

    if (ret != HERROR_OK) {
        MY_LOG_E("http client test faile\n");
    } else {
        if (http_body_buf.recv_len > 0) {
            MY_LOG_I("\nReceive %d Bytes from (%s)\n", http_body_buf.recv_len, HTTP_DEMO_URL);
            MY_LOG_I("%s\n", http_body_buf.p);
        }
    }
    httpcli_close(ctx);

    if (http_body_buf.p) {
        free(http_body_buf.p);
        http_body_buf.p = NULL;
    }

    if (ctx) {
        free(ctx);
        ctx = NULL;
    }
}

static void http_test_start(void *priv)
{
    while (1) {
        http_test_task(NULL);
        os_time_dly(200);
    }
}

void http_test_main()
{
    os_task_create(http_test_start, NULL, 3, 512, 0, "http_test_start");
}

#endif
