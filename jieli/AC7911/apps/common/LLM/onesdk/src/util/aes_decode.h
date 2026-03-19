#ifndef _VEI_UTIL_AES_DECODE_H
#define _VEI_UTIL_AES_DECODE_H

#include "aws/common/byte_buf.h"
#include "aws/common/string.h"
#include "aws/common/encoding.h"

char *aes_decode(struct aws_allocator *allocator, const char *device_secret, const char *encrypt_data, bool partial_secret);

#endif //_VEI_UTIL_AES_DECODE_H