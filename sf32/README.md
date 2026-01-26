# nertc-sf32-demo
SF32 demo for NERtc SDK

## 使用说明
- 使用app下的nertc_sdk进行接入测试
- 原有小智demo代码中，用NERTC_AI区分nertc和小智

## 代码说明
- 启动nertc sdk：nertc_start
- 初始化nertc sdk：nertc_init_ui，同时填入自己的device id和appkey
- 链接回调OnJoin：打开ai和asr
- 接收远端音频数据OnAudioData
- 音频数据发送nertc_audio_send，aec参考数据发送nertc_audio_speaker
- 接收ai数据OnAiData

