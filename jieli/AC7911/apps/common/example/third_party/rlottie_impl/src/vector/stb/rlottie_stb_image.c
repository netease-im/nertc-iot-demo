/*
 * configure stb_image about
 * the image we will support
 */

#ifdef HAS_AWTK_CONFIG //AWTK 内部有实现, 重定义
#include "stb_image.h"
#else
#define STB_IMAGE_IMPLEMENTATION
#include "rlottie_stb_image.h"
#endif

#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_NO_GIF
#define STBI_NO_PIC


#if defined _WIN32 || defined __CYGWIN__
#ifdef RLOTTIE_BUILD
#define RLOTTIE_API __declspec(dllexport)
#else
#define RLOTTIE_API __declspec(dllimport)
#endif
#else
#ifdef RLOTTIE_BUILD
#define RLOTTIE_API __attribute__ ((visibility ("default")))
#else
#define RLOTTIE_API
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * exported function wrapper from the library
 */

RLOTTIE_API unsigned char *lottie_image_load(char const *filename, int *x,
        int *y, int *comp, int req_comp)
{
#if 0
    return stbi_load(filename, x, y, comp, req_comp);
#else
#ifdef CONFIG_JLFAT_ENABLE
    char fn[128];
    extern int long_file_name_encode(const char *input, char *output, unsigned int output_buf_len);
    fn[long_file_name_encode(filename, fn, sizeof(fn))] = '\0';
    FILE *file = fopen(fn, "r");
#else
    FILE *file = fopen(filename, "r");
#endif
    unsigned int img_size = flen(file);
    unsigned char *buffer = malloc(img_size);
    if (buffer == NULL) {
        fclose(file);
        printf("LOTTIE Failed to load %s - malloc fail\r\n", filename);
        return NULL;
    }
    fread(buffer, img_size, 1, file);
    fclose(file);
    unsigned char *img = stbi_load_from_memory(buffer, img_size, x, y, comp, req_comp);
    free(buffer);
    return img;
#endif
}

RLOTTIE_API unsigned char *lottie_image_load_from_data(const char *imageData,
        int len, int *x, int *y,
        int *comp, int req_comp)
{
    unsigned char *data = (unsigned char *)imageData;
    return stbi_load_from_memory(data, len, x, y, comp, req_comp);
}

RLOTTIE_API void lottie_image_free(unsigned char *data)
{
    stbi_image_free(data);
}

#ifdef __cplusplus
}
#endif
