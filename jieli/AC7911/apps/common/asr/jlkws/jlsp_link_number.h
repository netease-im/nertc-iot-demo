#ifndef __LINK_NUMBER_WAKE_WORD_H_
#define __LINK_NUMBER_WAKE_WORD_H_

int jl_kws_model_get_heap_size(int model, int *model_size, int *private_heap_size, int *share_heap_size);
void *jl_kws_model_init(int model, unsigned char *private_heap_buffer, int private_heap_size, unsigned char *share_heap_buffer, int share_heap_size, int model_size, const float *confidence, int online);
int jl_kws_model_reset(void *m);
int jl_kws_model_process(void *m, int model, unsigned char *pcm, int size);
int jl_kws_model_free(void *m);

#endif
