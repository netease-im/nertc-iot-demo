/**
 * File:   color_format_conversion.inc
 * Author: AWTK Develop Team
 * Brief:  软件计算颜色格式转换的通用代码层
 *
 * Copyright (c) 2020 - 2020 Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2020-06-17 Luo ZhiMing <luozhiming@zlg.cn> created
 *
 */

typedef enum _color_format_t {
  COLOR_FORMAT_BGR2BGRA = 0,
  COLOR_FORMAT_RGB2RGBA = COLOR_FORMAT_BGR2BGRA,
  COLOR_FORMAT_BGR2RGBA,
  COLOR_FORMAT_RGB2BGRA = COLOR_FORMAT_BGR2RGBA,

  COLOR_FORMAT_BGR2BGR565,
  COLOR_FORMAT_RGB2RGB565 = COLOR_FORMAT_BGR2BGR565,
  COLOR_FORMAT_BGR2RGB565,
  COLOR_FORMAT_RGB2BGR565 = COLOR_FORMAT_BGR2RGB565,

  COLOR_FORMAT_BGR5652BGRA,
  COLOR_FORMAT_RGB5652RGBA = COLOR_FORMAT_BGR5652BGRA,
  COLOR_FORMAT_BGR5652RGBA,
  COLOR_FORMAT_RGB5652BGRA = COLOR_FORMAT_BGR5652RGBA,

  COLOR_FORMAT_BGR5652RGB565,
  COLOR_FORMAT_RGB5652BGR565 = COLOR_FORMAT_BGR5652RGB565,

  COLOR_FORMAT_YVU4202BGRA,
  COLOR_FORMAT_YVU4202RGBA,
  COLOR_FORMAT_YVU4202BGR565,
  COLOR_FORMAT_YVU4202RGB565,

  COLOR_FORMAT_YUV411P2BGRA,
  COLOR_FORMAT_YUV411P2RGBA,
  COLOR_FORMAT_YUV411P2BGR565,
  COLOR_FORMAT_YUV411P2RGB565,

  COLOR_FORMAT_YUYV2BGRA,
  COLOR_FORMAT_YUYV2RGBA,
  COLOR_FORMAT_YUYV2BGR565,
  COLOR_FORMAT_YUYV2RGB565,

  COLOR_FORMAT_UYVY2BGRA,
  COLOR_FORMAT_UYVY2RGBA,
  COLOR_FORMAT_UYVY2BGR565,
  COLOR_FORMAT_UYVY2RGB565,

  COLOR_FORMAT_SBGGR82BGRA,
  COLOR_FORMAT_SBGGR82RGBA,
  COLOR_FORMAT_SBGGR82BGR565,
  COLOR_FORMAT_SBGGR82RGB565,

  COLOR_FORMAT_SN9C10X2BGRA,
  COLOR_FORMAT_SN9C10X2RGBA,
  COLOR_FORMAT_SN9C10X2BGR565,
  COLOR_FORMAT_SN9C10X2RGB565,

  COLOR_FORMAT_MJPEG2BGRA,
  COLOR_FORMAT_MJPEG2RGBA,
  COLOR_FORMAT_MJPEG2BGR565,
  COLOR_FORMAT_MJPEG2RGB565,

  COLOR_FORMAT_SGBRG2BGRA,
  COLOR_FORMAT_SGBRG2RGBA,
  COLOR_FORMAT_SGBRG2BGR565,
  COLOR_FORMAT_SGBRG2RGB565,
} color_format_t;

#define SAT(a) (a < 0 ? 0 : (a > 255 ? 255 : a))

#if 0
#define LIMIT(x) ((x) > 0xffffff ? 0xff : ((x) <= 0xffff ? 0 : ((x) >> 16)))

static inline void move_420_block(int yTL, int yTR, int yBL, int yBR, int u, int v,
               										int rowPixels, unsigned char * rgb)
{
    const int rvScale = 91881;
    const int guScale = -22553;
    const int gvScale = -46801;
    const int buScale = 116129;
    const int yScale  = 65536;
    int r, g, b;

    g = guScale * u + gvScale * v;
		r = rvScale * v;
		b = buScale * u;

    yTL *= yScale; yTR *= yScale;
    yBL *= yScale; yBR *= yScale;

    /* Write out top two pixels */
    rgb[0] = LIMIT(b+yTL); rgb[1] = LIMIT(g+yTL);
    rgb[2] = LIMIT(r+yTL);

    rgb[3] = LIMIT(b+yTR); rgb[4] = LIMIT(g+yTR);
    rgb[5] = LIMIT(r+yTR);

    /* Skip down to next line to write out bottom two pixels */
    rgb += 3 * rowPixels;
    rgb[0] = LIMIT(b+yBL); rgb[1] = LIMIT(g+yBL);
    rgb[2] = LIMIT(r+yBL);

    rgb[3] = LIMIT(b+yBR); rgb[4] = LIMIT(g+yBR);
    rgb[5] = LIMIT(r+yBR);
}

static inline void move_411_block(int yTL, int yTR, int yBL, int yBR, int u, int v,
               										int /*rowPixels*/, unsigned char * rgb)
{
    const int rvScale = 91881;
    const int guScale = -22553;
    const int gvScale = -46801;
    const int buScale = 116129;
    const int yScale  = 65536;
    int r, g, b;

    g = guScale * u + gvScale * v;
		r = rvScale * v;
		b = buScale * u;

    yTL *= yScale; yTR *= yScale;
    yBL *= yScale; yBR *= yScale;

    /* Write out top two first pixels */
    rgb[0] = LIMIT(b+yTL); rgb[1] = LIMIT(g+yTL);
    rgb[2] = LIMIT(r+yTL);

    rgb[3] = LIMIT(b+yTR); rgb[4] = LIMIT(g+yTR);
    rgb[5] = LIMIT(r+yTR);

    /* Write out top two last pixels */
    rgb += 6;
    rgb[0] = LIMIT(b+yBL); rgb[1] = LIMIT(g+yBL);
    rgb[2] = LIMIT(r+yBL);

    rgb[3] = LIMIT(b+yBR); rgb[4] = LIMIT(g+yBR);
    rgb[5] = LIMIT(r+yBR);
}
#endif

static void color_format_conversion_data_BGR_to_BGRA(
    unsigned char *src, unsigned int width, unsigned int height, unsigned int line_length,
    unsigned int bpp, unsigned char *dst, unsigned int dst_len,
    unsigned char is_mirror, unsigned char is_reverse) {
  unsigned int widthInBytes = width * bpp;
  unsigned int numBytes = height * width * bpp;

  if (is_mirror) {
    src += (is_reverse ? (numBytes - bpp) : 0);
    for (unsigned int i = 0; i < numBytes; i += bpp) {
      *dst = *src;
      dst++;

      *dst = *(src + 1);
      dst++;

      *dst = *(src + 2);
      dst++;

      *dst = 255;
      dst++;

      src = is_reverse ? src - bpp : src + bpp;
    }
  } else {
    unsigned int x = 0;
    unsigned int y = is_reverse ? (height - 1) * widthInBytes : 0;
    src += y;

    for (unsigned int i = 0; i < numBytes; i += bpp) {
      if (x >= width && is_reverse) {
        x = 0;
        src -= widthInBytes * 2;
      }

      *dst = *src;
      dst++;

      *dst = *(src + 1);
      dst++;

      *dst = *(src + 2);
      dst++;

      *dst = 255;
      dst++;

      src += bpp;
      x++;
    }
  }
}

static void color_format_conversion_data_BGR_to_RGBA(
    unsigned char *src, unsigned int width, unsigned int height, unsigned int line_length,
    unsigned int bpp, unsigned char *dst, unsigned int dst_len,
    unsigned char is_mirror, unsigned char is_reverse) {
  unsigned int widthInBytes = width * bpp;
  unsigned int numBytes = height * width * bpp;

  if (is_mirror) {
    src += (is_reverse ? (numBytes - bpp) : 0);
    for (unsigned int i = 0; i < numBytes; i += bpp) {
      *dst = *(src + 2);
      dst++;

      *dst = *(src + 1);
      dst++;

      *dst = *src;
      dst++;

      *dst = 255;
      dst++;

      src = is_reverse ? src - bpp : src + bpp;
    }
  } else {
    unsigned int x = 0;
    unsigned int y = is_reverse ? (height - 1) * widthInBytes : 0;
    src += y;

    for (unsigned int i = 0; i < numBytes; i += bpp) {
      if (x >= width && is_reverse) {
        x = 0;
        src -= widthInBytes * 2;
      }

      *dst = *(src + 2);
      dst++;

      *dst = *(src + 1);
      dst++;

      *dst = *src;
      dst++;

      *dst = 255;
      dst++;

      src += bpp;
      x++;
    }
  }
}

static void color_format_conversion_data_BGR_to_BGR565(
    unsigned char *src, unsigned int width, unsigned int height, unsigned int line_length,
    unsigned int bpp, unsigned short *dst, unsigned int dst_len,
    unsigned char is_mirror, unsigned char is_reverse) {
  unsigned char r = 0;
  unsigned char g = 0;
  unsigned char b = 0;
  unsigned int widthInBytes = width * bpp;
  unsigned int numBytes = height * width * bpp;

  if (is_mirror) {
    src += (is_reverse ? (numBytes - bpp) : 0);
    for (unsigned int i = 0; i < numBytes; i += bpp) {
      b = *src;
      g = *(src + 1);
      r = *(src + 2);

      *dst = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
      dst++;

      src = is_reverse ? src - bpp : src + bpp;
    }
  } else {
    unsigned int x = 0;
    unsigned int y = is_reverse ? (height - 1) * widthInBytes : 0;
    src += y;

    for (unsigned int i = 0; i < numBytes; i += bpp) {
      if (x >= width && is_reverse) {
        x = 0;
        src -= widthInBytes * 2;
      }

      b = *src;
      g = *(src + 1);
      r = *(src + 2);

      *dst = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
      dst++;

      src += bpp;
      x++;
    }
  }
}

static void color_format_conversion_data_BGR_to_RGB565(
    unsigned char *src, unsigned int width, unsigned int height, unsigned int line_length,
    unsigned int bpp, unsigned short *dst, unsigned int dst_len,
    unsigned char is_mirror, unsigned char is_reverse) {
  unsigned char r = 0;
  unsigned char g = 0;
  unsigned char b = 0;
  unsigned int widthInBytes = width * bpp;
  unsigned int numBytes = height * width * bpp;

  if (is_mirror) {
    src += (is_reverse ? (numBytes - bpp) : 0);
    for (unsigned int i = 0; i < numBytes; i += bpp) {
      b = *src;
      g = *(src + 1);
      r = *(src + 2);

      *dst = ((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3);
      dst++;

      src = is_reverse ? src - bpp : src + bpp;
    }
  } else {
    unsigned int x = 0;
    unsigned int y = is_reverse ? (height - 1) * widthInBytes : 0;
    src += y;

    for (unsigned int i = 0; i < numBytes; i += bpp) {
      if (x >= width && is_reverse) {
        x = 0;
        src -= widthInBytes * 2;
      }

      b = *src;
      g = *(src + 1);
      r = *(src + 2);

      *dst = ((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3);
      dst++;

      src += bpp;
      x++;
    }
  }
}

#if 0

static void color_format_conversion_data_YUV420P_to_RGB(unsigned char* src, unsigned int width, unsigned int height, unsigned int bpp,
										unsigned char* dst, unsigned int dst_len, unsigned char is_mirror, unsigned char is_reverse)
{
    const int numpix = width * height;
    const int bytes = 24 >> 3;
    unsigned int i, j;
	int y00, y01, y10, y11, u, v;
    unsigned char *pY = src;
    unsigned char *pU = pY + numpix;
    unsigned char *pV = pU + numpix / 4;
    unsigned char *pOut = dst;

    for (j = 0; j <= height - 2; j += 2) 
		{
        for (i = 0; i <= width - 2; i += 2) 
				{
            y00 = *pY;
            y01 = *(pY + 1);
            y10 = *(pY + width);
            y11 = *(pY + width + 1);
            u = (*pU++) - 128;
            v = (*pV++) - 128;

            move_420_block(y00, y01, y10, y11, u, v, width, pOut);

            pY += 2;
            pOut += 2 * bytes;

        }
        pY += width;
        pOut += width * bytes;
    }
}

static void color_format_conversion_data_YUV420_to_RGB(unsigned char* src, unsigned int width, unsigned int height, unsigned int bpp,
										unsigned char* dst, unsigned int dst_len, unsigned char is_mirror, unsigned char is_reverse)
{
    const int bytes = 24 >> 3;
    unsigned int i, j;
	int y00, y01, y10, y11, u, v;
    unsigned char *pY = src;
    unsigned char *pU = pY + 4;
    unsigned char *pV = pU + width;
    unsigned char *pOut = dst;

    for (j = 0; j <= height - 2; j += 2)
		 {
        for (i = 0; i <= width - 4; i += 4) 
				{
            y00 = *pY;
            y01 = *(pY + 1);
            y10 = *(pY + width);
            y11 = *(pY + width + 1);
            u = (*pU++) - 128;
            v = (*pV++) - 128;

            move_420_block(y00, y01, y10, y11, u, v, width, pOut);

            pY += 2;
            pOut += 2 * bytes;

            y00 = *pY;
            y01 = *(pY + 1);
            y10 = *(pY + width);
            y11 = *(pY + width + 1);
            u = (*pU++) - 128;
            v = (*pV++) - 128;

            move_420_block(y00, y01, y10, y11, u, v,
                       width, pOut);

            pY += 4; // skip UV
            pOut += 2 * bytes;

        }
        pY += width;
        pOut += width * bytes;
    }
}

static void color_format_conversion_data_YUV411P_to_RGB(unsigned char* src, unsigned int width, unsigned int height, unsigned int bpp,
										unsigned char* dst, unsigned int dst_len, unsigned char is_mirror, unsigned char is_reverse)
{
    const int numpix = width * height;
    const int bytes = 24 >> 3;
    unsigned int i, j;
	int y00, y01, y10, y11, u, v;
    unsigned char *pY = src;
    unsigned char *pU = pY + numpix;
    unsigned char *pV = pU + numpix / 4;
    unsigned char *pOut = dst;

    for (j = 0; j <= height; j++)
		 {
        for (i = 0; i <= width - 4; i += 4) 
				{
            y00 = *pY;
            y01 = *(pY + 1);
            y10 = *(pY + 2);
            y11 = *(pY + 3);
            u = (*pU++) - 128;
            v = (*pV++) - 128;

            move_411_block(y00, y01, y10, y11, u, v, width, pOut);

            pY += 4;
            pOut += 4 * bytes;

        }
    }
}

#endif

static void color_format_conversion_data_YUYV_to_RGBA(
    unsigned char *src, unsigned int width, unsigned int height, unsigned int line_length,
    unsigned int bpp, unsigned char *dst, unsigned int dst_len,
    unsigned char is_mirror, unsigned char is_reverse) {
  int l = 0, c = 0;
  unsigned char *s = NULL;
  unsigned char *d = NULL;
  unsigned char r = 0, g = 0, b = 0;
  int cr = 0, cg = 0, cb = 0, y1 = 0, y2 = 0;

  s = src;
  d = dst;
  l = height;
  while (l--) {
    c = width >> 1;
    if (is_mirror) {
      d = dst + (width * 4 * (height - l));
      while (c--) {
        y1 = *s++;
        cb = ((*s - 128) * 454) >> 8;
        cg = (*s++ - 128) * 88;
        y2 = *s++;
        cr = ((*s - 128) * 359) >> 8;
        cg = (cg + (*s++ - 128) * 183) >> 8;

        r = SAT(y1 + cr);
        b = SAT(y1 + cb);
        g = SAT(y1 - cg);

        *(--d) = 255;
        *(--d) = b;
        *(--d) = g;
        *(--d) = r;

        r = SAT(y2 + cr);
        b = SAT(y2 + cb);
        g = SAT(y2 - cg);

        *(--d) = 255;
        *(--d) = b;
        *(--d) = g;
        *(--d) = r;
      }
    } else {
      while (c--) {
        y1 = *s++;
        cb = ((*s - 128) * 454) >> 8;
        cg = (*s++ - 128) * 88;
        y2 = *s++;
        cr = ((*s - 128) * 359) >> 8;
        cg = (cg + (*s++ - 128) * 183) >> 8;

        r = SAT(y1 + cr);
        b = SAT(y1 + cb);
        g = SAT(y1 - cg);

        *d++ = r;
        *d++ = g;
        *d++ = b;
        *d++ = 255;

        r = SAT(y2 + cr);
        b = SAT(y2 + cb);
        g = SAT(y2 - cg);

        *d++ = r;
        *d++ = g;
        *d++ = b;
        *d++ = 255;
      }
    }
  }
}

static void color_format_conversion_data_YUYV_to_RGB565(
    unsigned char *src, unsigned int width, unsigned int height, unsigned int line_length,
    unsigned int bpp, unsigned char *dst, unsigned int dst_len,
    unsigned char is_mirror, unsigned char is_reverse) {
  int l = 0, c = 0;
  unsigned char *s = NULL;
  unsigned short *d = NULL;
  unsigned char r = 0, g = 0, b = 0;
  int cr = 0, cg = 0, cb = 0, y1 = 0, y2 = 0;

  s = src;
  l = height;
  d = (unsigned short *)dst;
  while (l--) {
    c = width >> 1;
    if (is_mirror) {
      d = (unsigned short *)(dst + (width * 2 * (height - l)));
      while (c--) {
        y1 = *s++;
        cb = ((*s - 128) * 454) >> 8;
        cg = (*s++ - 128) * 88;
        y2 = *s++;
        cr = ((*s - 128) * 359) >> 8;
        cg = (cg + (*s++ - 128) * 183) >> 8;

        r = SAT(y1 + cr);
        b = SAT(y1 + cb);
        g = SAT(y1 - cg);

        *(--d) = ((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3);

        r = SAT(y2 + cr);
        b = SAT(y2 + cb);
        g = SAT(y2 - cg);

        *(--d) = ((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3);
      }
    } else {
      while (c--) {
        y1 = *s++;
        cb = ((*s - 128) * 454) >> 8;
        cg = (*s++ - 128) * 88;
        y2 = *s++;
        cr = ((*s - 128) * 359) >> 8;
        cg = (cg + (*s++ - 128) * 183) >> 8;

        r = SAT(y1 + cr);
        b = SAT(y1 + cb);
        g = SAT(y1 - cg);

        *(d++) = ((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3);

        r = SAT(y2 + cr);
        b = SAT(y2 + cb);
        g = SAT(y2 - cg);

        *(d++) = ((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3);
      }
    }
  }
}

static void color_format_conversion_data_YUYV_to_BGRA(
    unsigned char *src, unsigned int width, unsigned int height, unsigned int line_length,
    unsigned int bpp, unsigned char *dst, unsigned int dst_len,
    unsigned char is_mirror, unsigned char is_reverse) {
  int l = 0, c = 0;
  unsigned char *s = NULL;
  unsigned char *d = NULL;
  unsigned char r = 0, g = 0, b = 0;
  int cr = 0, cg = 0, cb = 0, y1 = 0, y2 = 0;

  l = height;
  s = src;
  d = dst;
  while (l--) {
    c = width >> 1;
    if (is_mirror) {
      d = dst + (width * 4 * (height - l));
      while (c--) {
        y1 = *s++;
        cb = ((*s - 128) * 454) >> 8;
        cg = (*s++ - 128) * 88;
        y2 = *s++;
        cr = ((*s - 128) * 359) >> 8;
        cg = (cg + (*s++ - 128) * 183) >> 8;

        r = SAT(y1 + cr);
        b = SAT(y1 + cb);
        g = SAT(y1 - cg);

        *(--d) = 255;
        *(--d) = r;
        *(--d) = g;
        *(--d) = b;

        r = SAT(y2 + cr);
        b = SAT(y2 + cb);
        g = SAT(y2 - cg);

        *(--d) = 255;
        *(--d) = r;
        *(--d) = g;
        *(--d) = b;
      }
    } else {
      while (c--) {
        y1 = *s++;
        cb = ((*s - 128) * 454) >> 8;
        cg = (*s++ - 128) * 88;
        y2 = *s++;
        cr = ((*s - 128) * 359) >> 8;
        cg = (cg + (*s++ - 128) * 183) >> 8;

        r = SAT(y1 + cr);
        b = SAT(y1 + cb);
        g = SAT(y1 - cg);

        *d++ = b;
        *d++ = g;
        *d++ = r;
        *d++ = 255;

        r = SAT(y2 + cr);
        b = SAT(y2 + cb);
        g = SAT(y2 - cg);

        *d++ = b;
        *d++ = g;
        *d++ = r;
        *d++ = 255;
      }
    }
  }
}

static void color_format_conversion_data_YUYV_to_BGR565(
    unsigned char *src, unsigned int width, unsigned int height, unsigned int line_length,
    unsigned int bpp, unsigned char *dst, unsigned int dst_len,
    unsigned char is_mirror, unsigned char is_reverse) {
  int l = 0, c = 0;
  unsigned char *s = NULL;
  unsigned short *d = NULL;
  unsigned char r = 0, g = 0, b = 0;
  int cr = 0, cg = 0, cb = 0, y1 = 0, y2 = 0;

  l = height;
  s = src;
  d = (unsigned short *)dst;
  while (l--) {
    c = width >> 1;
    if (is_mirror) {
      d = (unsigned short *)(dst + (width * 2 * (height - l)));
      while (c--) {
        y1 = *s++;
        cb = ((*s - 128) * 454) >> 8;
        cg = (*s++ - 128) * 88;
        y2 = *s++;
        cr = ((*s - 128) * 359) >> 8;
        cg = (cg + (*s++ - 128) * 183) >> 8;

        r = SAT(y1 + cr);
        b = SAT(y1 + cb);
        g = SAT(y1 - cg);

        *(--d) = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);

        r = SAT(y2 + cr);
        b = SAT(y2 + cb);
        g = SAT(y2 - cg);

        *(--d) = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
      }
    } else {
      while (c--) {
        y1 = *s++;
        cb = ((*s - 128) * 454) >> 8;
        cg = (*s++ - 128) * 88;
        y2 = *s++;
        cr = ((*s - 128) * 359) >> 8;
        cg = (cg + (*s++ - 128) * 183) >> 8;

        r = SAT(y1 + cr);
        b = SAT(y1 + cb);
        g = SAT(y1 - cg);

        *(d++) = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);

        r = SAT(y2 + cr);
        b = SAT(y2 + cb);
        g = SAT(y2 - cg);

        *(d++) = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
      }
    }
  }
}

static void color_format_conversion_data_UYVY_to_RGBA(
    unsigned char *src, unsigned int width, unsigned int height, unsigned int line_length,
    unsigned int bpp, unsigned char *dst, unsigned int dst_len,
    unsigned char is_mirror, unsigned char is_reverse) {
  unsigned char *s;
  unsigned char *d;
  int l, c;
  int r, g, b, cr, cg, cb, y1, y2;

  l = height;
  s = src;
  d = dst;
  while (l--) {
    c = width >> 1;
    if (is_mirror) {
      d = dst + (width * 4 * (height - l));
      while (c--) {
        cb = ((*s - 128) * 454) >> 8;
        cg = (*s++ - 128) * 88;
        y1 = *s++;
        cr = ((*s - 128) * 359) >> 8;
        cg = (cg + (*s++ - 128) * 183) >> 8;
        y2 = *s++;

        r = SAT(y1 + cr);
        b = SAT(y1 + cb);
        g = SAT(y1 - cg);

        *(--d) = b;
        *(--d) = g;
        *(--d) = r;
        *(--d) = 255;

        r = SAT(y2 + cr);
        b = SAT(y2 + cb);
        g = SAT(y2 - cg);

        *(--d) = b;
        *(--d) = g;
        *(--d) = r;
        *(--d) = 255;
      }
    } else {
      while (c--) {
        cb = ((*s - 128) * 454) >> 8;
        cg = (*s++ - 128) * 88;
        y1 = *s++;
        cr = ((*s - 128) * 359) >> 8;
        cg = (cg + (*s++ - 128) * 183) >> 8;
        y2 = *s++;

        r = SAT(y1 + cr);
        b = SAT(y1 + cb);
        g = SAT(y1 - cg);

        *d++ = b;
        *d++ = g;
        *d++ = r;
        *d++ = 255;

        r = SAT(y2 + cr);
        b = SAT(y2 + cb);
        g = SAT(y2 - cg);

        *d++ = b;
        *d++ = g;
        *d++ = r;
        *d++ = 255;
      }
    }
  }
}

static void color_format_conversion_data_UYVY_to_RGB565(
    unsigned char *src, unsigned int width, unsigned int height, unsigned int line_length,
    unsigned int bpp, unsigned char *dst, unsigned int dst_len,
    unsigned char is_mirror, unsigned char is_reverse) {
  unsigned char *s;
  unsigned short *d;
  int l, c;
  int r, g, b, cr, cg, cb, y1, y2;

  s = src;
  l = height;
  d = (unsigned short *)dst;
  while (l--) {
    c = width >> 1;
    if (is_mirror) {
      d = (unsigned short *)(dst + (width * 2 * (height - l)));
      while (c--) {
        cb = ((*s - 128) * 454) >> 8;
        cg = (*s++ - 128) * 88;
        y1 = *s++;
        cr = ((*s - 128) * 359) >> 8;
        cg = (cg + (*s++ - 128) * 183) >> 8;
        y2 = *s++;

        r = SAT(y1 + cr);
        b = SAT(y1 + cb);
        g = SAT(y1 - cg);

        *(--d) = ((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3);

        r = SAT(y2 + cr);
        b = SAT(y2 + cb);
        g = SAT(y2 - cg);

        *(--d) = ((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3);
      }
    } else {
      while (c--) {
        cb = ((*s - 128) * 454) >> 8;
        cg = (*s++ - 128) * 88;
        y1 = *s++;
        cr = ((*s - 128) * 359) >> 8;
        cg = (cg + (*s++ - 128) * 183) >> 8;
        y2 = *s++;

        r = SAT(y1 + cr);
        b = SAT(y1 + cb);
        g = SAT(y1 - cg);

        *(d++) = ((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3);

        r = SAT(y2 + cr);
        b = SAT(y2 + cb);
        g = SAT(y2 - cg);

        *(d++) = ((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3);
      }
    }
  }
}

static void color_format_conversion_data_UYVY_to_BGRA(
    unsigned char *src, unsigned int width, unsigned int height, unsigned int line_length,
    unsigned int bpp, unsigned char *dst, unsigned int dst_len,
    unsigned char is_mirror, unsigned char is_reverse) {
  unsigned char *s;
  unsigned char *d;
  int l, c;
  int r, g, b, cr, cg, cb, y1, y2;

  l = height;
  s = src;
  d = dst;
  while (l--) {
    c = width >> 1;
    if (is_mirror) {
      d = dst + (width * 4 * (height - l));
      while (c--) {
        cb = ((*s - 128) * 454) >> 8;
        cg = (*s++ - 128) * 88;
        y1 = *s++;
        cr = ((*s - 128) * 359) >> 8;
        cg = (cg + (*s++ - 128) * 183) >> 8;
        y2 = *s++;

        r = SAT(y1 + cr);
        b = SAT(y1 + cb);
        g = SAT(y1 - cg);

        *(--d) = r;
        *(--d) = g;
        *(--d) = b;
        *(--d) = 255;

        r = SAT(y2 + cr);
        b = SAT(y2 + cb);
        g = SAT(y2 - cg);

        *(--d) = r;
        *(--d) = g;
        *(--d) = b;
        *(--d) = 255;
      }
    } else {
      while (c--) {
        cb = ((*s - 128) * 454) >> 8;
        cg = (*s++ - 128) * 88;
        y1 = *s++;
        cr = ((*s - 128) * 359) >> 8;
        cg = (cg + (*s++ - 128) * 183) >> 8;
        y2 = *s++;

        r = SAT(y1 + cr);
        b = SAT(y1 + cb);
        g = SAT(y1 - cg);

        *d++ = r;
        *d++ = g;
        *d++ = b;
        *d++ = 255;

        r = SAT(y2 + cr);
        b = SAT(y2 + cb);
        g = SAT(y2 - cg);

        *d++ = r;
        *d++ = g;
        *d++ = b;
        *d++ = 255;
      }
    }
  }
}

static void color_format_conversion_data_UYVY_to_BGR565(
    unsigned char *src, unsigned int width, unsigned int height, unsigned int line_length,
    unsigned int bpp, unsigned char *dst, unsigned int dst_len,
    unsigned char is_mirror, unsigned char is_reverse) {
  unsigned char *s;
  unsigned short *d;
  int l, c;
  int r, g, b, cr, cg, cb, y1, y2;

  l = height;
  s = src;
  d = (unsigned short *)dst;
  while (l--) {
    c = width >> 1;
    if (is_mirror) {
      d = (unsigned short *)(dst + (width * 2 * (height - l)));
      while (c--) {
        cb = ((*s - 128) * 454) >> 8;
        cg = (*s++ - 128) * 88;
        y1 = *s++;
        cr = ((*s - 128) * 359) >> 8;
        cg = (cg + (*s++ - 128) * 183) >> 8;
        y2 = *s++;

        r = SAT(y1 + cr);
        b = SAT(y1 + cb);
        g = SAT(y1 - cg);

        *(--d) = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);

        r = SAT(y2 + cr);
        b = SAT(y2 + cb);
        g = SAT(y2 - cg);

        *(--d) = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
      }
    } else {
      while (c--) {
        cb = ((*s - 128) * 454) >> 8;
        cg = (*s++ - 128) * 88;
        y1 = *s++;
        cr = ((*s - 128) * 359) >> 8;
        cg = (cg + (*s++ - 128) * 183) >> 8;
        y2 = *s++;

        r = SAT(y1 + cr);
        b = SAT(y1 + cb);
        g = SAT(y1 - cg);

        *(d++) = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);

        r = SAT(y2 + cr);
        b = SAT(y2 + cb);
        g = SAT(y2 - cg);

        *(d++) = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
      }
    }
  }
}

static unsigned int color_format_conversion_data_fun(
    color_format_t format, unsigned char *src, unsigned int width,
    unsigned int height, unsigned int line_length, unsigned int bpp, unsigned char *dst,
    unsigned int dst_len, unsigned char is_mirror, unsigned char is_reverse) {
  unsigned int error = 0;
  switch (format) {
  case COLOR_FORMAT_BGR2BGRA:
    color_format_conversion_data_BGR_to_BGRA(src, width, height, line_length, bpp, dst,
                                             dst_len, is_mirror, is_reverse);
    break;
  case COLOR_FORMAT_BGR2RGBA:
    color_format_conversion_data_BGR_to_RGBA(src, width, height, line_length, bpp, dst,
                                             dst_len, is_mirror, is_reverse);
    break;
  case COLOR_FORMAT_BGR2BGR565:
    color_format_conversion_data_BGR_to_BGR565(src, width, height, line_length, bpp, (unsigned short *)dst,
                                               dst_len, is_mirror, is_reverse);
    break;
  case COLOR_FORMAT_BGR2RGB565:
    color_format_conversion_data_BGR_to_RGB565(src, width, height, line_length, bpp, (unsigned short *)dst,
                                               dst_len, is_mirror, is_reverse);
    break;
  case COLOR_FORMAT_BGR5652BGRA:
    error = 1;
    break;
  case COLOR_FORMAT_BGR5652RGBA:
    error = 1;
    break;
  case COLOR_FORMAT_BGR5652RGB565:
    error = 1;
    break;
  case COLOR_FORMAT_YVU4202BGRA:
    error = 1;
    break;
  case COLOR_FORMAT_YVU4202RGBA:
    error = 1;
    break;
  case COLOR_FORMAT_YVU4202BGR565:
    error = 1;
    break;
  case COLOR_FORMAT_YVU4202RGB565:
    error = 1;
    break;
  case COLOR_FORMAT_YUV411P2BGRA:
    error = 1;
    break;
  case COLOR_FORMAT_YUV411P2RGBA:
    error = 1;
    break;
  case COLOR_FORMAT_YUV411P2BGR565:
    error = 1;
    break;
  case COLOR_FORMAT_YUV411P2RGB565:
    error = 1;
    break;
  case COLOR_FORMAT_YUYV2BGRA:
    color_format_conversion_data_YUYV_to_BGRA(src, width, height, line_length, bpp, dst,
                                              dst_len, is_mirror, is_reverse);
    break;
  case COLOR_FORMAT_YUYV2RGBA:
    color_format_conversion_data_YUYV_to_RGBA(src, width, height, line_length, bpp, dst,
                                              dst_len, is_mirror, is_reverse);
    break;
  case COLOR_FORMAT_YUYV2BGR565:
    color_format_conversion_data_YUYV_to_BGR565(src, width, height, line_length, bpp, dst,
                                                dst_len, is_mirror, is_reverse);
    break;
  case COLOR_FORMAT_YUYV2RGB565:
    color_format_conversion_data_YUYV_to_RGB565(src, width, height, line_length, bpp, dst,
                                                dst_len, is_mirror, is_reverse);
    break;
  case COLOR_FORMAT_UYVY2BGRA:
    color_format_conversion_data_UYVY_to_BGRA(src, width, height, line_length, bpp, dst,
                                              dst_len, is_mirror, is_reverse);
    break;
  case COLOR_FORMAT_UYVY2RGBA:
    color_format_conversion_data_UYVY_to_RGBA(src, width, height, line_length, bpp, dst,
                                              dst_len, is_mirror, is_reverse);
    break;
  case COLOR_FORMAT_UYVY2BGR565:
    color_format_conversion_data_UYVY_to_BGR565(src, width, height, line_length, bpp, dst,
                                                dst_len, is_mirror, is_reverse);
    break;
  case COLOR_FORMAT_UYVY2RGB565:
    color_format_conversion_data_UYVY_to_RGB565(src, width, height, line_length, bpp, dst,
                                                dst_len, is_mirror, is_reverse);
    break;
  case COLOR_FORMAT_SBGGR82BGRA:
    error = 1;
    break;
  case COLOR_FORMAT_SBGGR82RGBA:
    error = 1;
    break;
  case COLOR_FORMAT_SBGGR82BGR565:
    error = 1;
    break;
  case COLOR_FORMAT_SBGGR82RGB565:
    error = 1;
    break;
  case COLOR_FORMAT_SN9C10X2BGRA:
    error = 1;
    break;
  case COLOR_FORMAT_SN9C10X2RGBA:
    error = 1;
    break;
  case COLOR_FORMAT_SN9C10X2BGR565:
    error = 1;
    break;
  case COLOR_FORMAT_SN9C10X2RGB565:
    error = 1;
    break;
  case COLOR_FORMAT_MJPEG2BGRA:
    error = 1;
    break;
  case COLOR_FORMAT_MJPEG2RGBA:
    error = 1;
    break;
  case COLOR_FORMAT_MJPEG2BGR565:
    error = 1;
    break;
  case COLOR_FORMAT_MJPEG2RGB565:
    error = 1;
    break;
  case COLOR_FORMAT_SGBRG2BGRA:
    error = 1;
    break;
  case COLOR_FORMAT_SGBRG2RGBA:
    error = 1;
    break;
  case COLOR_FORMAT_SGBRG2BGR565:
    error = 1;
    break;
  case COLOR_FORMAT_SGBRG2RGB565:
    error = 1;
    break;
  default:
    error = 1;
    break;
  }
  return error;
}
