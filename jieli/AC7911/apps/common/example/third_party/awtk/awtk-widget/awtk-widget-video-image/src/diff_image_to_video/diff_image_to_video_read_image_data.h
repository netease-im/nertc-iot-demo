#ifndef __DIFF_IMAGE_TO_VIDEO_READ_IMAGE_DATA_H__
#define __DIFF_IMAGE_TO_VIDEO_READ_IMAGE_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

int diff_image_to_video_read_image_init(const unsigned char *file_data,
                                        unsigned int file_data_length,
                                        unsigned char **max_data_decompress);

void diff_image_to_video_read_image_dispose(unsigned char **max_data_decompress);

int diff_image_to_video_read_image_data(
    const unsigned char *file_data, unsigned int file_data_length,
    unsigned int frame_number, unsigned char *last_image_data, unsigned int line_length,
    unsigned int width, unsigned int height, unsigned int channels, unsigned char **max_data_decompress);

#ifdef __cplusplus
}
#endif

#endif // !__DIFF_IMAGE_TO_VIDEO_READ_IMAGE_DATA_H__
