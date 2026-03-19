#include "my_platform_http.h"

int my_http_get_request(char *url, char **response)
{
    int error = 0;
    http_body_obj http_body_buf;
    MY_LOG_I("wt get url: %s\n", url);
    httpcli_ctx *ctx = (httpcli_ctx *)calloc(1, sizeof(httpcli_ctx));
    if (!ctx) {
        MY_LOG_E("calloc faile\n");
        return -1;
    }
    memset(&http_body_buf, 0x0, sizeof(http_body_obj));
    http_body_buf.recv_len = 0;
    http_body_buf.buf_len = 2 * 1024;
    http_body_buf.buf_count = 1;
    http_body_buf.p = (char *)malloc(http_body_buf.buf_len * http_body_buf.buf_count);
    if (!http_body_buf.p) {
        free(ctx);
        ctx = NULL;
        return -1;
    }
    ctx->url = url;
    ctx->timeout_millsec = 8000;
    ctx->priv = &http_body_buf;
    ctx->connection = "close";
    ctx->data_format = "application/json";
    error = httpcli_get(ctx);
    if (error != HERROR_OK) {
        MY_LOG_E("get failed\n");
    } else {

        if (http_body_buf.recv_len > 0) {
            MY_LOG_I("\nrecieve %d bytes from(%s)\n", http_body_buf.recv_len, url);
            MY_LOG_I("%s\n", http_body_buf.p);
            if (response != NULL) {
                *response = malloc(http_body_buf.recv_len + 1);
                if (*response == NULL) {
                    MY_LOG_E("Memory allocation failed for response\n");
                } else {
                    snprintf(*response, http_body_buf.recv_len + 1, "%.*s",
                             (int)http_body_buf.recv_len, http_body_buf.p);
                    MY_LOG_I("Response: %s\n", *response);
                }
            }
        }
    }
    httpcli_close(ctx);
    if (http_body_buf.p) {
        free(http_body_buf.p);
    }
    if (ctx) {
        free(ctx);
    }
    return error;
}


int my_http_post_request(char *url, char **response)
{
    int ret = 0;
    http_body_obj http_body_buf;
    httpcli_ctx *ctx = (httpcli_ctx *)calloc(1, sizeof(httpcli_ctx));
    if (!ctx) {
        MY_LOG_E("calloc failed\n");
        return -1;
    }

    memset(&http_body_buf, 0x0, sizeof(http_body_obj));

    http_body_buf.recv_len = 0;
    http_body_buf.buf_len = 4 * 1024;
    http_body_buf.buf_count = 1;
    http_body_buf.p = (char *)malloc(http_body_buf.buf_len * http_body_buf.buf_count);
    if (!http_body_buf.p) {
        free(ctx);
        return -1;
    }
    ctx->url = url;
    ctx->timeout_millsec = 5000;
    ctx->priv = &http_body_buf;
    ctx->connection = "close";

    ctx->data_format = "application/json";
    ret = httpcli_post(ctx);

    if (ret != HERROR_OK) {
        MY_LOG_E("HTTP POST request failed\n");
    } else {
        if (http_body_buf.recv_len > 0) {
            MY_LOG_I("\nReceived %d Bytes from (%s)\n", http_body_buf.recv_len, url);
            if (response != NULL) {
                *response = malloc(http_body_buf.recv_len + 1);
                if (*response == NULL) {
                    MY_LOG_E("Memory allocation failed for response\n");
                } else {
                    snprintf(*response, http_body_buf.recv_len + 1, "%.*s",
                             (int)http_body_buf.recv_len, http_body_buf.p);
                    MY_LOG_I("Response: %s\n", *response);
                }
            }
        }
    }
    httpcli_close(ctx);
    if (http_body_buf.p) {
        free(http_body_buf.p);
    }
    if (ctx) {
        free(ctx);
    }
    return ret;
}


static int header_callback(void *ctx, void *buf, unsigned int size,
                           void *priv, httpin_status status)
{
    httpcli_ctx *http_ctx = (httpcli_ctx *)ctx;
    int *p_length = (int *)priv;

    if (status == HTTPIN_HEADER) {
        *p_length = http_ctx->content_length;
    }
    return 0;
}
int my_http_get_url_data_size(char *url)
{
    printf(">>>zwz info: %s %d %s\n", __FUNCTION__, __LINE__, __FILE__);
    int content_length = -1;
    httpcli_ctx ctx = {0};
    ctx.url = url;
    ctx.cb = header_callback;
    ctx.priv = &content_length;
    ctx.wait_content_length = 1;
    ctx.timeout_millsec = 5000;
    httpin_error err = httpcli_get(&ctx);
    if (err == HERROR_CALLBACK && content_length >= 0) {
        printf("zwz Success! Content-Length: %d bytes\n", content_length);
        return content_length;  // 成功返回实际大小
    }
    printf("zwz Error fetching Content-Length: %d\n", err);
    return -1;  // 失败返回-1
}

/*
int main() {
    const char *target_url = "https://pic.ibaotu.com/20/01/06/91e888piCsnS.mp3";
    long content_length;
    if (get_content_length(target_url, &content_length) == 0) {
        printf("Content-Length: %ld bytes\n", content_length);
    } else {
        printf("Failed to get Content-Length\n");
    }
    return 0;
}
*/
