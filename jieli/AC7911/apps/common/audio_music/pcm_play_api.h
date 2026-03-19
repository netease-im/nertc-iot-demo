#include "generic/typedef.h"

void *audio_pcm_play_open(int sample_rate, u32 frame_size, u32 drop_points, u8 channel, u8 volume, u8 block);

int audio_pcm_play_start(void *priv);

int audio_pcm_play_pause(void *priv, int clear_cache);

int audio_pcm_play_set_volume(void *priv, u8 volume);

int audio_pcm_play_stop(void *priv);

int audio_pcm_play_data_write(void *priv, void *data, u32 size);

