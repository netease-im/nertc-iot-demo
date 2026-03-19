//
// Created by minwang7 on 2024/9/2.
//

#ifndef AITOY_E2E_H
#define AITOY_E2E_H

/**
 * @brief 初始化 E2E 语音交互模块
 *
 * @param appid 应用 ID
 * @param appkey 应用密钥
 * @param deviceId 设备 ID
 * @return int 成功返回 0，失败返回错误码
 */
int e2e_voice_create(const char *appid, const char *appkey, const char *deviceId);

/**
 * @brief 设置识别（IAT）相关参数
 *
 * 具体参数值可参考开放平台 WebSocket 接口，例如：
 * e2e_voice_set_iat_param("aue", "raw");
 *
 * @param key 参数名称
 * @param value 参数值
 * @return int 成功返回 0，失败返回错误码
 */
int e2e_voice_set_iat_param(const char *key, const char *value);

/**
 * @brief 设置大模型（LLM）相关参数
 *
 * 具体参数值可参考开放平台 WebSocket 接口
 *
 * @param key 参数名称
 * @param value 参数值
 * @return int 成功返回 0，失败返回错误码
 */
int e2e_voice_set_llm_param(const char *key, const char *value);

/**
 * @brief 设置文本转语音（TTS）相关参数
 *
 * 具体参数值可参考开放平台 WebSocket 接口
 *
 * @param key 参数名称
 * @param value 参数值
 * @return int 成功返回 0，失败返回错误码
 */
int e2e_voice_set_tts_param(const char *key, const char *value);

/**
 * @brief 开始识别，每次交互前调用
 *
 * @return char* 返回 WebSocket 连接地址
 */
char *e2e_voice_start();

/**
 * @brief 传入识别音频数据，返回需要 WebSocket 发送的文本数据
 *
 * @param audioData 音频数据
 * @param dataLen 音频数据长度
 * @return char* WebSocket 需要发送的文本数据
 */
char *e2e_voice_write(const char *audioData, unsigned int dataLen);

/**
 * @brief 停止识别，并等待返回 TTS 结果
 *
 * @return char* 返回 TTS 结果
 */
char *e2e_voice_commit();

/**
 * @brief 解析 WebSocket 接收的数据
 *
 * @param webrecv WebSocket 接收到的原始数据
 * @param result_code 解析后的状态码（0 代表成功）
 * @param result_len 解析出的数据长度
 * @return char* 返回解析后的文本数据
 */
char *e2e_voice_prase_result(char *webrecv, int *result_code, int *result_len);

/**
 * @brief 释放 E2E 语音交互模块的资源
 *
 * @return int 成功返回 0，失败返回错误码
 */
int e2e_voice_destroy();

#endif // AITOY_E2E_H

