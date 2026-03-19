#include "web_socket/websocket_api.h"

typedef WEBSOCKET_INFO coze_ws_info_t;

// websocket根据data[0]判别数据包类型    比如0x81 = 0x80 | 0x1 为一个txt类型数据包
// typedef enum {
//     WCT_SEQ     = 0x00,
//     WCT_TXTDATA = 0x01,      // 0x1：标识一个txt类型数据包
//     WCT_BINDATA = 0x02,      // 0x2：标识一个bin类型数据包
//     WCT_DISCONN = 0x08,      // 0x8：标识一个断开连接类型数据包
//     WCT_PING    = 0x09,     // 0x8：标识一个断开连接类型数据包
//     WCT_PONG    = 0x0a,     // 0xA：表示一个pong类型数据包
//     WCT_FIN     = 0x80,     //fin
//     WCT_END     = 0x10,
//     WCT_CLOSE_OK   = 0xaa,
//     WCT_INIT    = 0xff,
// } WS_CMD_Type;


void websocket_struct_dump(const WEBSOCKET_INFO *info);
