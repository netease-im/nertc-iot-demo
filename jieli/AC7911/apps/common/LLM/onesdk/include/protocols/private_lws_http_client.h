#ifndef ONESDK_LWS_HTTP_CLIENT_H
#define ONESDK_LWS_HTTP_CLIENT_H

#include "libwebsockets.h"
#include "http.h"


int lws_http_client_init(http_request_context_t *http_ctx);

int lws_http_client_connect(http_request_context_t *http_ctx);

int lws_http_client_request(http_request_context_t *http_ctx);

int lws_http_client_send_request_body(http_request_context_t *http_ctx);

http_response_t *lws_http_client_send_request(http_request_context_t *http_ctx);

int lws_http_client_receive_response(http_request_context_t *http_ctx);
int lws_http_client_receive_response_body(http_request_context_t *http_ctx);


void lws_http_client_disconnect(http_request_context_t *http_ctx);

int lws_http_client_deinit(http_request_context_t *http_ctx);

// helper methods
int lws_http_client_init_client_connect_info(http_request_context_t *http_ctx, struct lws_client_connect_info *ccinfo);

int lws_http_client_parse_url(char *uri, const char **protocol, const char **host, int *port, const char **path);

// internal interfaces
void _http_client_data_free(http_client_data_t *data);
void _http_client_data_sse_free(sse_context_t *sse);
#endif //ONESDK_LWS_HTTP_CLIENT_H