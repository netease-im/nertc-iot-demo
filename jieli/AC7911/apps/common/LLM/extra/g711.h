#ifndef __g711_h__
#define __g711_h__

#ifdef __cplusplus
extern "C" {
#endif

unsigned char linear2alaw(short pcm_val);	/* 2's complement (16-bit range) */
short alaw2linear(unsigned char	a_val);

unsigned char linear2ulaw(short pcm_val);	/* 2's complement (16-bit range) */
short ulaw2linear(unsigned char	u_val);

#ifdef __cplusplus
}
#endif

#endif
