/**
 * File:   usb_camera_linux_device_v4l.cpp
 * Author: AWTK Develop Team
 * Brief:  Windows 系统的 USB 摄像头驱动代码
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

// #define HAVE_MSMF_DXVA 1

#include "usb_camera_devices.h"
#include "color_format_conversion.inc"

#include <windows.h>
#include <guiddef.h>
#include <mfidl.h>
#include <mfapi.h>
#include <mfplay.h>
#include <mfobjects.h>
#include <tchar.h>
#include <strsafe.h>
#include <mfreadwrite.h>
#include <mutex>
#ifdef HAVE_MSMF_DXVA
#include <d3d11.h>
#include <d3d11_4.h>
#endif
#include <new>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef _MSC_VER
#pragma warning(disable:4503)
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mf")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "Strmiids")
#pragma comment(lib, "Mfreadwrite")
#ifdef HAVE_MSMF_DXVA
#pragma comment(lib, "d3d11")
// MFCreateDXGIDeviceManager() is available since Win8 only.
// To avoid OpenCV loading failure on Win7 use dynamic detection of this symbol.
// Details: https://github.com/opencv/opencv/issues/11858
typedef HRESULT(WINAPI *FN_MFCreateDXGIDeviceManager)(UINT *resetToken, IMFDXGIDeviceManager **ppDeviceManager);
static bool pMFCreateDXGIDeviceManager_initialized = false;
static FN_MFCreateDXGIDeviceManager pMFCreateDXGIDeviceManager = NULL;
static void init_MFCreateDXGIDeviceManager()
{
    HMODULE h = LoadLibraryExA("mfplat.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (h) {
        pMFCreateDXGIDeviceManager = (FN_MFCreateDXGIDeviceManager)GetProcAddress(h, "MFCreateDXGIDeviceManager");
    }
    pMFCreateDXGIDeviceManager_initialized = pMFCreateDXGIDeviceManager != NULL;
}
#endif
#pragma comment(lib, "Shlwapi.lib")
#endif

#include <mferror.h>

#include <comdef.h>

#include <shlwapi.h>  // QISearch

struct MediaType;
struct IMFMediaType;
struct IMFActivate;
struct IMFMediaSource;
struct IMFAttributes;
typedef struct _usb_camera_device_t usb_camera_device_t;

#define CV_FOURCC_MACRO(c1, c2, c3, c4) (((c1) & 255) + (((c2) & 255) << 8) + (((c3) & 255) << 16) + (((c4) & 255) << 24))

#define CV_CAP_MODE_BGR CV_FOURCC_MACRO('B','G','R','3')
#define CV_CAP_MODE_RGB CV_FOURCC_MACRO('R','G','B','3')
#define CV_CAP_MODE_GRAY CV_FOURCC_MACRO('G','R','E','Y')
#define CV_CAP_MODE_YUYV CV_FOURCC_MACRO('Y', 'U', 'Y', 'V')

template <class T>
class ComPtr
{
public:
    ComPtr() { }

    ComPtr(T *lp)
    {
        p = lp;
    }

    ComPtr(_In_ const ComPtr<T> &lp)
    {
        p = lp.p;
    }
    virtual ~ComPtr() { }

    T **operator&()
    {
        assert(p == NULL);
        return p.operator & ();
    }

    T *operator->() const
    {
        assert(p != NULL);
        return p.operator->();
    }
    operator bool()
    {
        return p.operator != (NULL);
    }

    T *Get() const
    {
        return p;
    }

    void Release()
    {
        if (p) {
            p.Release();
        }
    }

    // query for U interface
    template<typename U>
    HRESULT As(_Out_ ComPtr<U> &lp) const
    {
        lp.Release();
        return p->QueryInterface(__uuidof(U), reinterpret_cast<void **>((T **)&lp));
    }
private:
    _COM_SMARTPTR_TYPEDEF(T, __uuidof(T));
    TPtr p;
};

#define _ComPtr ComPtr

static void close(usb_camera_device_t *usb_camera_device);
static bool LogAttributeValueByIndexNew(IMFAttributes *pAttr, DWORD index, MediaType &out);

// Structure for collecting info about types of video, which are supported by current video device
struct MediaType {
    unsigned int MF_MT_FRAME_SIZE;
    UINT32 height;
    UINT32 width;
    unsigned int MF_MT_YUV_MATRIX;
    unsigned int MF_MT_VIDEO_LIGHTING;
    int MF_MT_DEFAULT_STRIDE; // stride is negative if image is bottom-up
    unsigned int MF_MT_VIDEO_CHROMA_SITING;
    GUID MF_MT_AM_FORMAT_TYPE;
    unsigned int MF_MT_FIXED_SIZE_SAMPLES;
    unsigned int MF_MT_VIDEO_NOMINAL_RANGE;
    UINT32 MF_MT_FRAME_RATE_NUMERATOR;
    UINT32 MF_MT_FRAME_RATE_DENOMINATOR;
    UINT32 MF_MT_PIXEL_ASPECT_RATIO_NUMERATOR;
    UINT32 MF_MT_PIXEL_ASPECT_RATIO_DENOMINATOR;
    unsigned int MF_MT_ALL_SAMPLES_INDEPENDENT;
    UINT32 MF_MT_FRAME_RATE_RANGE_MIN_NUMERATOR;
    UINT32 MF_MT_FRAME_RATE_RANGE_MIN_DENOMINATOR;
    unsigned int MF_MT_SAMPLE_SIZE;
    unsigned int MF_MT_VIDEO_PRIMARIES;
    unsigned int MF_MT_INTERLACE_MODE;
    UINT32 MF_MT_FRAME_RATE_RANGE_MAX_NUMERATOR;
    UINT32 MF_MT_FRAME_RATE_RANGE_MAX_DENOMINATOR;
    GUID MF_MT_MAJOR_TYPE;
    GUID MF_MT_SUBTYPE;
    LPCWSTR pMF_MT_MAJOR_TYPEName;
    LPCWSTR pMF_MT_SUBTYPEName;
    MediaType()
    {
        pMF_MT_MAJOR_TYPEName = NULL;
        pMF_MT_SUBTYPEName = NULL;
        Clear();
    }
    MediaType(IMFMediaType *pType)
    {
        pMF_MT_MAJOR_TYPEName = NULL;
        pMF_MT_SUBTYPEName = NULL;
        Clear();
        UINT32 count = 0;
        if (SUCCEEDED(pType->GetCount(&count)) &&
            SUCCEEDED(pType->LockStore())) {
            for (uint32_t i = 0; i < count; i++) {
                if (!LogAttributeValueByIndexNew(pType, i, *this)) {
                    break;
                }
            }
            pType->UnlockStore();
        }
    }
    ~MediaType()
    {
        Clear();
    }
    void Clear()
    {
        MF_MT_FRAME_SIZE = 0;
        height = 0;
        width = 0;
        MF_MT_YUV_MATRIX = 0;
        MF_MT_VIDEO_LIGHTING = 0;
        MF_MT_DEFAULT_STRIDE = 0;
        MF_MT_VIDEO_CHROMA_SITING = 0;
        MF_MT_FIXED_SIZE_SAMPLES = 0;
        MF_MT_VIDEO_NOMINAL_RANGE = 0;
        MF_MT_FRAME_RATE_NUMERATOR = 0;
        MF_MT_FRAME_RATE_DENOMINATOR = 0;
        MF_MT_PIXEL_ASPECT_RATIO_NUMERATOR = 0;
        MF_MT_PIXEL_ASPECT_RATIO_DENOMINATOR = 0;
        MF_MT_ALL_SAMPLES_INDEPENDENT = 0;
        MF_MT_FRAME_RATE_RANGE_MIN_NUMERATOR = 0;
        MF_MT_FRAME_RATE_RANGE_MIN_DENOMINATOR = 0;
        MF_MT_SAMPLE_SIZE = 0;
        MF_MT_VIDEO_PRIMARIES = 0;
        MF_MT_INTERLACE_MODE = 0;
        MF_MT_FRAME_RATE_RANGE_MAX_NUMERATOR = 0;
        MF_MT_FRAME_RATE_RANGE_MAX_DENOMINATOR = 0;
        memset(&MF_MT_MAJOR_TYPE, 0, sizeof(GUID));
        memset(&MF_MT_AM_FORMAT_TYPE, 0, sizeof(GUID));
        memset(&MF_MT_SUBTYPE, 0, sizeof(GUID));
    }
};

#ifndef IF_GUID_EQUAL_RETURN
#define IF_GUID_EQUAL_RETURN(val) if(val == guid) return L#val
#endif
static LPCWSTR GetGUIDNameConstNew(const GUID &guid)
{
    IF_GUID_EQUAL_RETURN(MF_MT_MAJOR_TYPE);
    IF_GUID_EQUAL_RETURN(MF_MT_SUBTYPE);
    IF_GUID_EQUAL_RETURN(MF_MT_ALL_SAMPLES_INDEPENDENT);
    IF_GUID_EQUAL_RETURN(MF_MT_FIXED_SIZE_SAMPLES);
    IF_GUID_EQUAL_RETURN(MF_MT_COMPRESSED);
    IF_GUID_EQUAL_RETURN(MF_MT_SAMPLE_SIZE);
    IF_GUID_EQUAL_RETURN(MF_MT_WRAPPED_TYPE);
    IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_NUM_CHANNELS);
    IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_SAMPLES_PER_SECOND);
    IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_FLOAT_SAMPLES_PER_SECOND);
    IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_AVG_BYTES_PER_SECOND);
    IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_BLOCK_ALIGNMENT);
    IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_BITS_PER_SAMPLE);
    IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_VALID_BITS_PER_SAMPLE);
    IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_SAMPLES_PER_BLOCK);
    IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_CHANNEL_MASK);
    IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_FOLDDOWN_MATRIX);
    IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_WMADRC_PEAKREF);
    IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_WMADRC_PEAKTARGET);
    IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_WMADRC_AVGREF);
    IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_WMADRC_AVGTARGET);
    IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_PREFER_WAVEFORMATEX);
    IF_GUID_EQUAL_RETURN(MF_MT_AAC_PAYLOAD_TYPE);
    IF_GUID_EQUAL_RETURN(MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION);
    IF_GUID_EQUAL_RETURN(MF_MT_FRAME_SIZE);
    IF_GUID_EQUAL_RETURN(MF_MT_FRAME_RATE);
    IF_GUID_EQUAL_RETURN(MF_MT_FRAME_RATE_RANGE_MAX);
    IF_GUID_EQUAL_RETURN(MF_MT_FRAME_RATE_RANGE_MIN);
    IF_GUID_EQUAL_RETURN(MF_MT_PIXEL_ASPECT_RATIO);
    IF_GUID_EQUAL_RETURN(MF_MT_DRM_FLAGS);
    IF_GUID_EQUAL_RETURN(MF_MT_PAD_CONTROL_FLAGS);
    IF_GUID_EQUAL_RETURN(MF_MT_SOURCE_CONTENT_HINT);
    IF_GUID_EQUAL_RETURN(MF_MT_VIDEO_CHROMA_SITING);
    IF_GUID_EQUAL_RETURN(MF_MT_INTERLACE_MODE);
    IF_GUID_EQUAL_RETURN(MF_MT_TRANSFER_FUNCTION);
    IF_GUID_EQUAL_RETURN(MF_MT_VIDEO_PRIMARIES);
    IF_GUID_EQUAL_RETURN(MF_MT_CUSTOM_VIDEO_PRIMARIES);
    IF_GUID_EQUAL_RETURN(MF_MT_YUV_MATRIX);
    IF_GUID_EQUAL_RETURN(MF_MT_VIDEO_LIGHTING);
    IF_GUID_EQUAL_RETURN(MF_MT_VIDEO_NOMINAL_RANGE);
    IF_GUID_EQUAL_RETURN(MF_MT_GEOMETRIC_APERTURE);
    IF_GUID_EQUAL_RETURN(MF_MT_MINIMUM_DISPLAY_APERTURE);
    IF_GUID_EQUAL_RETURN(MF_MT_PAN_SCAN_APERTURE);
    IF_GUID_EQUAL_RETURN(MF_MT_PAN_SCAN_ENABLED);
    IF_GUID_EQUAL_RETURN(MF_MT_AVG_BITRATE);
    IF_GUID_EQUAL_RETURN(MF_MT_AVG_BIT_ERROR_RATE);
    IF_GUID_EQUAL_RETURN(MF_MT_MAX_KEYFRAME_SPACING);
    IF_GUID_EQUAL_RETURN(MF_MT_DEFAULT_STRIDE);
    IF_GUID_EQUAL_RETURN(MF_MT_PALETTE);
    IF_GUID_EQUAL_RETURN(MF_MT_USER_DATA);
    IF_GUID_EQUAL_RETURN(MF_MT_AM_FORMAT_TYPE);
    IF_GUID_EQUAL_RETURN(MF_MT_MPEG_START_TIME_CODE);
    IF_GUID_EQUAL_RETURN(MF_MT_MPEG2_PROFILE);
    IF_GUID_EQUAL_RETURN(MF_MT_MPEG2_LEVEL);
    IF_GUID_EQUAL_RETURN(MF_MT_MPEG2_FLAGS);
    IF_GUID_EQUAL_RETURN(MF_MT_MPEG_SEQUENCE_HEADER);
    IF_GUID_EQUAL_RETURN(MF_MT_DV_AAUX_SRC_PACK_0);
    IF_GUID_EQUAL_RETURN(MF_MT_DV_AAUX_CTRL_PACK_0);
    IF_GUID_EQUAL_RETURN(MF_MT_DV_AAUX_SRC_PACK_1);
    IF_GUID_EQUAL_RETURN(MF_MT_DV_AAUX_CTRL_PACK_1);
    IF_GUID_EQUAL_RETURN(MF_MT_DV_VAUX_SRC_PACK);
    IF_GUID_EQUAL_RETURN(MF_MT_DV_VAUX_CTRL_PACK);
    IF_GUID_EQUAL_RETURN(MF_MT_ARBITRARY_HEADER);
    IF_GUID_EQUAL_RETURN(MF_MT_ARBITRARY_FORMAT);
    IF_GUID_EQUAL_RETURN(MF_MT_IMAGE_LOSS_TOLERANT);
    IF_GUID_EQUAL_RETURN(MF_MT_MPEG4_SAMPLE_DESCRIPTION);
    IF_GUID_EQUAL_RETURN(MF_MT_MPEG4_CURRENT_SAMPLE_ENTRY);
    IF_GUID_EQUAL_RETURN(MF_MT_ORIGINAL_4CC);
    IF_GUID_EQUAL_RETURN(MF_MT_ORIGINAL_WAVE_FORMAT_TAG);
    // Media types
    IF_GUID_EQUAL_RETURN(MFMediaType_Audio);
    IF_GUID_EQUAL_RETURN(MFMediaType_Video);
    IF_GUID_EQUAL_RETURN(MFMediaType_Protected);
#ifdef MFMediaType_Perception
    IF_GUID_EQUAL_RETURN(MFMediaType_Perception);
#endif
    IF_GUID_EQUAL_RETURN(MFMediaType_Stream);
    IF_GUID_EQUAL_RETURN(MFMediaType_SAMI);
    IF_GUID_EQUAL_RETURN(MFMediaType_Script);
    IF_GUID_EQUAL_RETURN(MFMediaType_Image);
    IF_GUID_EQUAL_RETURN(MFMediaType_HTML);
    IF_GUID_EQUAL_RETURN(MFMediaType_Binary);
    IF_GUID_EQUAL_RETURN(MFMediaType_FileTransfer);
    IF_GUID_EQUAL_RETURN(MFVideoFormat_AI44); //     FCC('AI44')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_ARGB32); //   D3DFMT_A8R8G8B8
    IF_GUID_EQUAL_RETURN(MFVideoFormat_AYUV); //     FCC('AYUV')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_DV25); //     FCC('dv25')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_DV50); //     FCC('dv50')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_DVH1); //     FCC('dvh1')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_DVC);
    IF_GUID_EQUAL_RETURN(MFVideoFormat_DVHD);
    IF_GUID_EQUAL_RETURN(MFVideoFormat_DVSD); //     FCC('dvsd')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_DVSL); //     FCC('dvsl')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_H264); //     FCC('H264')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_I420); //     FCC('I420')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_IYUV); //     FCC('IYUV')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_M4S2); //     FCC('M4S2')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_MJPG);
    IF_GUID_EQUAL_RETURN(MFVideoFormat_MP43); //     FCC('MP43')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_MP4S); //     FCC('MP4S')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_MP4V); //     FCC('MP4V')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_MPG1); //     FCC('MPG1')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_MSS1); //     FCC('MSS1')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_MSS2); //     FCC('MSS2')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_NV11); //     FCC('NV11')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_NV12); //     FCC('NV12')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_P010); //     FCC('P010')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_P016); //     FCC('P016')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_P210); //     FCC('P210')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_P216); //     FCC('P216')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_RGB24); //    D3DFMT_R8G8B8
    IF_GUID_EQUAL_RETURN(MFVideoFormat_RGB32); //    D3DFMT_X8R8G8B8
    IF_GUID_EQUAL_RETURN(MFVideoFormat_RGB555); //   D3DFMT_X1R5G5B5
    IF_GUID_EQUAL_RETURN(MFVideoFormat_RGB565); //   D3DFMT_R5G6B5
    IF_GUID_EQUAL_RETURN(MFVideoFormat_RGB8);
    IF_GUID_EQUAL_RETURN(MFVideoFormat_UYVY); //     FCC('UYVY')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_v210); //     FCC('v210')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_v410); //     FCC('v410')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_WMV1); //     FCC('WMV1')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_WMV2); //     FCC('WMV2')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_WMV3); //     FCC('WMV3')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_WVC1); //     FCC('WVC1')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_Y210); //     FCC('Y210')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_Y216); //     FCC('Y216')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_Y410); //     FCC('Y410')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_Y416); //     FCC('Y416')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_Y41P);
    IF_GUID_EQUAL_RETURN(MFVideoFormat_Y41T);
    IF_GUID_EQUAL_RETURN(MFVideoFormat_YUY2); //     FCC('YUY2')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_YV12); //     FCC('YV12')
    IF_GUID_EQUAL_RETURN(MFVideoFormat_YVYU);
#ifdef MFVideoFormat_H263
    IF_GUID_EQUAL_RETURN(MFVideoFormat_H263);
#endif
#ifdef MFVideoFormat_H265
    IF_GUID_EQUAL_RETURN(MFVideoFormat_H265);
#endif
#ifdef MFVideoFormat_H264_ES
    IF_GUID_EQUAL_RETURN(MFVideoFormat_H264_ES);
#endif
#ifdef MFVideoFormat_HEVC
    IF_GUID_EQUAL_RETURN(MFVideoFormat_HEVC);
#endif
#ifdef MFVideoFormat_HEVC_ES
    IF_GUID_EQUAL_RETURN(MFVideoFormat_HEVC_ES);
#endif
#ifdef MFVideoFormat_MPEG2
    IF_GUID_EQUAL_RETURN(MFVideoFormat_MPEG2);
#endif
#ifdef MFVideoFormat_VP80
    IF_GUID_EQUAL_RETURN(MFVideoFormat_VP80);
#endif
#ifdef MFVideoFormat_VP90
    IF_GUID_EQUAL_RETURN(MFVideoFormat_VP90);
#endif
#ifdef MFVideoFormat_420O
    IF_GUID_EQUAL_RETURN(MFVideoFormat_420O);
#endif
#ifdef MFVideoFormat_Y42T
    IF_GUID_EQUAL_RETURN(MFVideoFormat_Y42T);
#endif
#ifdef MFVideoFormat_YVU9
    IF_GUID_EQUAL_RETURN(MFVideoFormat_YVU9);
#endif
#ifdef MFVideoFormat_v216
    IF_GUID_EQUAL_RETURN(MFVideoFormat_v216);
#endif
#ifdef MFVideoFormat_L8
    IF_GUID_EQUAL_RETURN(MFVideoFormat_L8);
#endif
#ifdef MFVideoFormat_L16
    IF_GUID_EQUAL_RETURN(MFVideoFormat_L16);
#endif
#ifdef MFVideoFormat_D16
    IF_GUID_EQUAL_RETURN(MFVideoFormat_D16);
#endif
#ifdef D3DFMT_X8R8G8B8
    IF_GUID_EQUAL_RETURN(D3DFMT_X8R8G8B8);
#endif
#ifdef D3DFMT_A8R8G8B8
    IF_GUID_EQUAL_RETURN(D3DFMT_A8R8G8B8);
#endif
#ifdef D3DFMT_R8G8B8
    IF_GUID_EQUAL_RETURN(D3DFMT_R8G8B8);
#endif
#ifdef D3DFMT_X1R5G5B5
    IF_GUID_EQUAL_RETURN(D3DFMT_X1R5G5B5);
#endif
#ifdef D3DFMT_A4R4G4B4
    IF_GUID_EQUAL_RETURN(D3DFMT_A4R4G4B4);
#endif
#ifdef D3DFMT_R5G6B5
    IF_GUID_EQUAL_RETURN(D3DFMT_R5G6B5);
#endif
#ifdef D3DFMT_P8
    IF_GUID_EQUAL_RETURN(D3DFMT_P8);
#endif
#ifdef D3DFMT_A2R10G10B10
    IF_GUID_EQUAL_RETURN(D3DFMT_A2R10G10B10);
#endif
#ifdef D3DFMT_A2B10G10R10
    IF_GUID_EQUAL_RETURN(D3DFMT_A2B10G10R10);
#endif
#ifdef D3DFMT_L8
    IF_GUID_EQUAL_RETURN(D3DFMT_L8);
#endif
#ifdef D3DFMT_L16
    IF_GUID_EQUAL_RETURN(D3DFMT_L16);
#endif
#ifdef D3DFMT_D16
    IF_GUID_EQUAL_RETURN(D3DFMT_D16);
#endif
#ifdef MFVideoFormat_A2R10G10B10
    IF_GUID_EQUAL_RETURN(MFVideoFormat_A2R10G10B10);
#endif
#ifdef MFVideoFormat_A16B16G16R16F
    IF_GUID_EQUAL_RETURN(MFVideoFormat_A16B16G16R16F);
#endif
    IF_GUID_EQUAL_RETURN(MFAudioFormat_PCM); //              WAVE_FORMAT_PCM
    IF_GUID_EQUAL_RETURN(MFAudioFormat_Float); //            WAVE_FORMAT_IEEE_FLOAT
    IF_GUID_EQUAL_RETURN(MFAudioFormat_DTS); //              WAVE_FORMAT_DTS
    IF_GUID_EQUAL_RETURN(MFAudioFormat_Dolby_AC3_SPDIF); //  WAVE_FORMAT_DOLBY_AC3_SPDIF
    IF_GUID_EQUAL_RETURN(MFAudioFormat_DRM); //              WAVE_FORMAT_DRM
    IF_GUID_EQUAL_RETURN(MFAudioFormat_WMAudioV8); //        WAVE_FORMAT_WMAUDIO2
    IF_GUID_EQUAL_RETURN(MFAudioFormat_WMAudioV9); //        WAVE_FORMAT_WMAUDIO3
    IF_GUID_EQUAL_RETURN(MFAudioFormat_WMAudio_Lossless); // WAVE_FORMAT_WMAUDIO_LOSSLESS
    IF_GUID_EQUAL_RETURN(MFAudioFormat_WMASPDIF); //         WAVE_FORMAT_WMASPDIF
    IF_GUID_EQUAL_RETURN(MFAudioFormat_MSP1); //             WAVE_FORMAT_WMAVOICE9
    IF_GUID_EQUAL_RETURN(MFAudioFormat_MP3); //              WAVE_FORMAT_MPEGLAYER3
    IF_GUID_EQUAL_RETURN(MFAudioFormat_MPEG); //             WAVE_FORMAT_MPEG
    IF_GUID_EQUAL_RETURN(MFAudioFormat_AAC); //              WAVE_FORMAT_MPEG_HEAAC
    IF_GUID_EQUAL_RETURN(MFAudioFormat_ADTS); //             WAVE_FORMAT_MPEG_ADTS_AAC
#ifdef MFAudioFormat_ALAC
    IF_GUID_EQUAL_RETURN(MFAudioFormat_ALAC);
#endif
#ifdef MFAudioFormat_AMR_NB
    IF_GUID_EQUAL_RETURN(MFAudioFormat_AMR_NB);
#endif
#ifdef MFAudioFormat_AMR_WB
    IF_GUID_EQUAL_RETURN(MFAudioFormat_AMR_WB);
#endif
#ifdef MFAudioFormat_AMR_WP
    IF_GUID_EQUAL_RETURN(MFAudioFormat_AMR_WP);
#endif
#ifdef MFAudioFormat_Dolby_AC3
    IF_GUID_EQUAL_RETURN(MFAudioFormat_Dolby_AC3);
#endif
#ifdef MFAudioFormat_Dolby_DDPlus
    IF_GUID_EQUAL_RETURN(MFAudioFormat_Dolby_DDPlus);
#endif
#ifdef MFAudioFormat_FLAC
    IF_GUID_EQUAL_RETURN(MFAudioFormat_FLAC);
#endif
#ifdef MFAudioFormat_Opus
    IF_GUID_EQUAL_RETURN(MFAudioFormat_Opus);
#endif
#ifdef MEDIASUBTYPE_RAW_AAC1
    IF_GUID_EQUAL_RETURN(MEDIASUBTYPE_RAW_AAC1);
#endif
#ifdef MFAudioFormat_Float_SpatialObjects
    IF_GUID_EQUAL_RETURN(MFAudioFormat_Float_SpatialObjects);
#endif
#ifdef MFAudioFormat_QCELP
    IF_GUID_EQUAL_RETURN(MFAudioFormat_QCELP);
#endif

    return NULL;
}

static bool LogAttributeValueByIndexNew(IMFAttributes *pAttr, DWORD index, MediaType &out)
{
    PROPVARIANT var;
    PropVariantInit(&var);
    GUID guid = { 0 };
    if (SUCCEEDED(pAttr->GetItemByIndex(index, &guid, &var))) {
        if (guid == MF_MT_DEFAULT_STRIDE && var.vt == VT_INT) {
            out.MF_MT_DEFAULT_STRIDE = var.intVal;
        } else if (guid == MF_MT_FRAME_RATE && var.vt == VT_UI8) {
            Unpack2UINT32AsUINT64(var.uhVal.QuadPart, &out.MF_MT_FRAME_RATE_NUMERATOR, &out.MF_MT_FRAME_RATE_DENOMINATOR);
        } else if (guid == MF_MT_FRAME_RATE_RANGE_MAX && var.vt == VT_UI8) {
            Unpack2UINT32AsUINT64(var.uhVal.QuadPart, &out.MF_MT_FRAME_RATE_RANGE_MAX_NUMERATOR, &out.MF_MT_FRAME_RATE_RANGE_MAX_DENOMINATOR);
        } else if (guid == MF_MT_FRAME_RATE_RANGE_MIN && var.vt == VT_UI8) {
            Unpack2UINT32AsUINT64(var.uhVal.QuadPart, &out.MF_MT_FRAME_RATE_RANGE_MIN_NUMERATOR, &out.MF_MT_FRAME_RATE_RANGE_MIN_DENOMINATOR);
        } else if (guid == MF_MT_PIXEL_ASPECT_RATIO && var.vt == VT_UI8) {
            Unpack2UINT32AsUINT64(var.uhVal.QuadPart, &out.MF_MT_PIXEL_ASPECT_RATIO_NUMERATOR, &out.MF_MT_PIXEL_ASPECT_RATIO_DENOMINATOR);
        } else if (guid == MF_MT_YUV_MATRIX && var.vt == VT_UI4) {
            out.MF_MT_YUV_MATRIX = var.ulVal;
        } else if (guid == MF_MT_VIDEO_LIGHTING && var.vt == VT_UI4) {
            out.MF_MT_VIDEO_LIGHTING = var.ulVal;
        } else if (guid == MF_MT_DEFAULT_STRIDE && var.vt == VT_UI4) {
            out.MF_MT_DEFAULT_STRIDE = (int)var.ulVal;
        } else if (guid == MF_MT_VIDEO_CHROMA_SITING && var.vt == VT_UI4) {
            out.MF_MT_VIDEO_CHROMA_SITING = var.ulVal;
        } else if (guid == MF_MT_VIDEO_NOMINAL_RANGE && var.vt == VT_UI4) {
            out.MF_MT_VIDEO_NOMINAL_RANGE = var.ulVal;
        } else if (guid == MF_MT_ALL_SAMPLES_INDEPENDENT && var.vt == VT_UI4) {
            out.MF_MT_ALL_SAMPLES_INDEPENDENT = var.ulVal;
        } else if (guid == MF_MT_FIXED_SIZE_SAMPLES && var.vt == VT_UI4) {
            out.MF_MT_FIXED_SIZE_SAMPLES = var.ulVal;
        } else if (guid == MF_MT_SAMPLE_SIZE && var.vt == VT_UI4) {
            out.MF_MT_SAMPLE_SIZE = var.ulVal;
        } else if (guid == MF_MT_VIDEO_PRIMARIES && var.vt == VT_UI4) {
            out.MF_MT_VIDEO_PRIMARIES = var.ulVal;
        } else if (guid == MF_MT_INTERLACE_MODE && var.vt == VT_UI4) {
            out.MF_MT_INTERLACE_MODE = var.ulVal;
        } else if (guid == MF_MT_AM_FORMAT_TYPE && var.vt == VT_CLSID) {
            out.MF_MT_AM_FORMAT_TYPE = *var.puuid;
        } else if (guid == MF_MT_MAJOR_TYPE && var.vt == VT_CLSID) {
            out.pMF_MT_MAJOR_TYPEName = GetGUIDNameConstNew(out.MF_MT_MAJOR_TYPE = *var.puuid);
        } else if (guid == MF_MT_SUBTYPE && var.vt == VT_CLSID) {
            out.pMF_MT_SUBTYPEName = GetGUIDNameConstNew(out.MF_MT_SUBTYPE = *var.puuid);
        } else if (guid == MF_MT_FRAME_SIZE && var.vt == VT_UI8) {
            Unpack2UINT32AsUINT64(var.uhVal.QuadPart, &out.width, &out.height);
            out.MF_MT_FRAME_SIZE = out.width * out.height;
        }
        PropVariantClear(&var);
        return true;
    }
    return false;
}


class SourceReaderCB : public IMFSourceReaderCallback
{
public:
    SourceReaderCB() :
        m_nRefCount(0), m_hEvent(CreateEvent(NULL, FALSE, FALSE, NULL)), m_bEOS(FALSE), m_hrStatus(S_OK), m_reader(NULL), m_dwStreamIndex(0)
    { }

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void **ppv) override
    {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4838)
#endif
        static const QITAB qit[] = {
            QITABENT(SourceReaderCB, IMFSourceReaderCallback),
            { 0 },
        };
#ifdef _MSC_VER
#pragma warning(pop)
#endif
        return QISearch(this, qit, iid, ppv);
    }
    STDMETHODIMP_(ULONG) AddRef() override
    {
        return InterlockedIncrement(&m_nRefCount);
    }
    STDMETHODIMP_(ULONG) Release() override
    {
        ULONG uCount = InterlockedDecrement(&m_nRefCount);
        if (uCount == 0) {
            delete this;
        }
        return uCount;
    }

    STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample *pSample) override
    {
        HRESULT hr = 0;
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (SUCCEEDED(hrStatus)) {
            if (pSample) {
                // IMFSample* prev = m_lastSample.Get();
                // if (prev) {
                //   log_warn("videoio(MSMF): drop frame (not processed)");
                // }
                m_lastSample = pSample;
            }
        } else {
            log_warn("videoio(MSMF): OnReadSample() is called with error status");
        }

        if (MF_SOURCE_READERF_ENDOFSTREAM & dwStreamFlags) {
            // Reached the end of the stream.
            m_bEOS = true;
        }
        m_hrStatus = hrStatus;

        if (FAILED(hr = m_reader->ReadSample(dwStreamIndex, 0, NULL, NULL, NULL, NULL))) {
            log_warn("videoio(MSMF): async ReadSample() call is failed with error status");
            m_bEOS = true;
        }

        if (pSample || m_bEOS) {
            SetEvent(m_hEvent);
        }
        return S_OK;
    }

    STDMETHODIMP OnEvent(DWORD, IMFMediaEvent *) override
    {
        return S_OK;
    }
    STDMETHODIMP OnFlush(DWORD) override
    {
        return S_OK;
    }

    HRESULT Wait(DWORD dwMilliseconds, _ComPtr<IMFSample> &videoSample, BOOL &bEOS)
    {
        bEOS = FALSE;
        DWORD dwResult = WaitForSingleObject(m_hEvent, dwMilliseconds);
        if (dwResult == WAIT_TIMEOUT) {
            return E_PENDING;
        } else if (dwResult != WAIT_OBJECT_0) {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        bEOS = m_bEOS;
        if (!bEOS) {
            std::lock_guard<std::recursive_mutex> lock(m_mutex);
            videoSample = m_lastSample;
            assert(videoSample);
            m_lastSample.Release();
            ResetEvent(m_hEvent);  // event is auto-reset, but we need this forced reset due time gap between wait() and mutex hold.
        }

        return m_hrStatus;
    }

private:
    // Destructor is private. Caller should call Release.
    virtual ~SourceReaderCB()
    {
        log_warn("terminating async callback");
    }

public:
    long m_nRefCount;        // Reference count.
    std::recursive_mutex m_mutex;
    HANDLE m_hEvent;
    BOOL m_bEOS;
    HRESULT m_hrStatus;

    IMFSourceReader *m_reader;
    DWORD m_dwStreamIndex;
    _ComPtr<IMFSample>  m_lastSample;
};


typedef enum {
    MODE_SW = 0,
    MODE_HW = 1
} MSMFCapture_Mode;

struct _usb_camera_device_t {
    usb_camera_device_base_t base;

    int camid;
    bool convertFormat;
    uint32_t aspectN, aspectD;
    uint32_t outputFormat;
    uint32_t requestedWidth, requestedHeight;

    DWORD dwStreamIndex;
    MediaType nativeFormat;
    MediaType captureFormat;
    MSMFCapture_Mode captureMode;
    _ComPtr<IMFSample> videoSample;
    _ComPtr<IMFSourceReader> videoFileSource;
    _ComPtr<IMFSourceReaderCallback> readCallback;
#ifdef HAVE_MSMF_DXVA
    _ComPtr<ID3D11Device> D3DDev;
    _ComPtr<IMFDXGIDeviceManager> D3DMgr;
#endif

};


static bool configureHW(usb_camera_device_t *usb_camera_device, bool enable)
{
#ifdef HAVE_MSMF_DXVA
    if ((enable && usb_camera_device->D3DMgr && usb_camera_device->D3DDev) || (!enable && !usb_camera_device->D3DMgr && !usb_camera_device->D3DDev)) {
        return true;
    }
    if (!pMFCreateDXGIDeviceManager_initialized) {
        init_MFCreateDXGIDeviceManager();
    }
    if (enable && !pMFCreateDXGIDeviceManager) {
        return false;
    }

    bool reopen = isOpen;
    int prevcam = camid;
    cv::String prevfile = filename;
    close(usb_camera_device);
    if (enable) {
        D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
                                       D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
                                       D3D_FEATURE_LEVEL_9_3,  D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1
                                     };
        if (SUCCEEDED(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_VIDEO_SUPPORT,
                                        levels, sizeof(levels) / sizeof(*levels), D3D11_SDK_VERSION, &usb_camera_device->D3DDev, NULL, NULL))) {
            // NOTE: Getting ready for multi-threaded operation
            _ComPtr<ID3D11Multithread> D3DDevMT;
            UINT mgrRToken;
            if (SUCCEEDED(usb_camera_device->D3DDev->QueryInterface(IID_PPV_ARGS(&D3DDevMT)))) {
                D3DDevMT->SetMultithreadProtected(TRUE);
                D3DDevMT.Release();
                if (SUCCEEDED(pMFCreateDXGIDeviceManager(&mgrRToken, &usb_camera_device->D3DMgr))) {
                    if (SUCCEEDED(usb_camera_device->D3DMgr->ResetDevice(usb_camera_device->D3DDev.Get(), mgrRToken))) {
                        captureMode = MODE_HW;
                        return reopen ? (prevcam >= 0 ? open(prevcam) : open(prevfile.c_str())) : true;
                    }
                    usb_camera_device->D3DMgr.Release();
                }
            }
            usb_camera_device->D3DDev.Release();
        }
        return false;
    } else {
        if (usb_camera_device->D3DMgr) {
            usb_camera_device->D3DMgr.Release();
        }
        if (usb_camera_device->D3DDev) {
            usb_camera_device->D3DDev.Release();
        }
        usb_camera_device->captureMode = MODE_SW;
        return reopen ? (prevcam >= 0 ? open(prevcam) : open(prevfile.c_str())) : true;
    }
#else
    return !enable;
#endif
}

#define UDIFF(res, ref) (ref == 0 ? 0 : res > ref ? res - ref : ref - res)
static UINT32 resolutionDiff(MediaType &mType, UINT32 refWidth, UINT32 refHeight)
{
    return UDIFF(mType.width, refWidth) + UDIFF(mType.height, refHeight);
}
#undef UDIFF

static double getFramerate(MediaType MT)
{
    if (MT.MF_MT_SUBTYPE == MFVideoFormat_MP43) { //Unable to estimate FPS for MP43
        return 0;
    }
    return MT.MF_MT_FRAME_RATE_DENOMINATOR != 0 ? ((double)MT.MF_MT_FRAME_RATE_NUMERATOR) / ((double)MT.MF_MT_FRAME_RATE_DENOMINATOR) : 0;
}

static bool configureOutput(usb_camera_device_t *usb_camera_device, uint32_t width, uint32_t height, double prefFramerate, uint32_t aspectRatioN, uint32_t aspectRatioD, uint32_t outFormat, bool convertToFormat)
{
    if (width != 0 && height != 0 &&
        width == usb_camera_device->captureFormat.width && height == usb_camera_device->captureFormat.height && prefFramerate == getFramerate(usb_camera_device->nativeFormat) &&
        aspectRatioN == usb_camera_device->aspectN && aspectRatioD == usb_camera_device->aspectD && outFormat == usb_camera_device->outputFormat && convertToFormat == usb_camera_device->convertFormat) {
        return true;
    }

    usb_camera_device->requestedWidth = width;
    usb_camera_device->requestedHeight = height;

    HRESULT hr = S_OK;
    int dwStreamBest = -1;
    MediaType MTBest;
    uint32_t id = 0;
    DWORD dwMediaTypeTest = 0;
    DWORD dwStreamTest = 0;
    while (SUCCEEDED(hr)) {
        _ComPtr<IMFMediaType> pType;
        hr = usb_camera_device->videoFileSource->GetNativeMediaType(dwStreamTest, dwMediaTypeTest, &pType);
        if (hr == MF_E_NO_MORE_TYPES) {
            hr = S_OK;
            ++dwStreamTest;
            dwMediaTypeTest = 0;
        } else if (SUCCEEDED(hr)) {
            MediaType MT(pType.Get());
            if (MT.MF_MT_MAJOR_TYPE == MFMediaType_Video) {
                if (dwStreamBest < 0 ||
                    resolutionDiff(MT, width, height) < resolutionDiff(MTBest, width, height) ||
                    (resolutionDiff(MT, width, height) == resolutionDiff(MTBest, width, height) && MT.width > MTBest.width) ||
                    (resolutionDiff(MT, width, height) == resolutionDiff(MTBest, width, height) && MT.width == MTBest.width && MT.height > MTBest.height) ||
                    (MT.width == MTBest.width && MT.height == MTBest.height && (getFramerate(MT) > getFramerate(MTBest) && (prefFramerate == 0 || getFramerate(MT) <= prefFramerate)))) {
                    dwStreamBest = (int)dwStreamTest;
                    MTBest = MT;
                }
            }
            ++dwMediaTypeTest;
        }
        id++;
    }
    if (dwStreamBest >= 0)  {
        GUID outSubtype = GUID_NULL;
        uint32_t outStride = 0;
        uint32_t outSize = 0;
        if (convertToFormat) {
            switch (outFormat) {
            case CV_CAP_MODE_BGR:
            case CV_CAP_MODE_RGB:
                outSubtype = usb_camera_device->captureMode == MODE_HW ? MFVideoFormat_RGB32 : MFVideoFormat_RGB24; // HW accelerated mode support only RGB32
                outStride = (usb_camera_device->captureMode == MODE_HW ? 4 : 3) * MTBest.width;
                outSize = outStride * MTBest.height;
                break;
            case CV_CAP_MODE_GRAY:
                outSubtype = MFVideoFormat_NV12;
                outStride = MTBest.width;
                outSize = outStride * MTBest.height * 3 / 2;
                break;
            case CV_CAP_MODE_YUYV:
                outSubtype = MFVideoFormat_YUY2;
                outStride = 2 * MTBest.width;
                outSize = outStride * MTBest.height;
                break;
            default:
                return false;
            }
        }
        _ComPtr<IMFMediaType>  mediaTypeOut;
        if (// Set the output media type.
            SUCCEEDED(MFCreateMediaType(&mediaTypeOut)) &&
            SUCCEEDED(mediaTypeOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video)) &&
            SUCCEEDED(mediaTypeOut->SetGUID(MF_MT_SUBTYPE, convertToFormat ? outSubtype : MTBest.MF_MT_SUBTYPE)) &&
            SUCCEEDED(mediaTypeOut->SetUINT32(MF_MT_INTERLACE_MODE, convertToFormat ? MFVideoInterlace_Progressive : MTBest.MF_MT_INTERLACE_MODE)) &&
            SUCCEEDED(MFSetAttributeRatio(mediaTypeOut.Get(), MF_MT_PIXEL_ASPECT_RATIO, aspectRatioN, aspectRatioD)) &&
            SUCCEEDED(MFSetAttributeSize(mediaTypeOut.Get(), MF_MT_FRAME_SIZE, MTBest.width, MTBest.height)) &&
            SUCCEEDED(mediaTypeOut->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, convertToFormat ? 1 : MTBest.MF_MT_FIXED_SIZE_SAMPLES)) &&
            SUCCEEDED(mediaTypeOut->SetUINT32(MF_MT_SAMPLE_SIZE, convertToFormat ? outSize : MTBest.MF_MT_SAMPLE_SIZE)) &&
            SUCCEEDED(mediaTypeOut->SetUINT32(MF_MT_DEFAULT_STRIDE, convertToFormat ? outStride : MTBest.MF_MT_DEFAULT_STRIDE))) { //Assume BGR24 input
            if (SUCCEEDED(usb_camera_device->videoFileSource->SetStreamSelection((DWORD)MF_SOURCE_READER_ALL_STREAMS, false)) &&
                SUCCEEDED(usb_camera_device->videoFileSource->SetStreamSelection((DWORD)dwStreamBest, true)) &&
                SUCCEEDED(usb_camera_device->videoFileSource->SetCurrentMediaType((DWORD)dwStreamBest, NULL, mediaTypeOut.Get()))
               ) {
                usb_camera_device->base.device_ratio.id = id;
                usb_camera_device->base.device_ratio.bpp = outStride / MTBest.width;
                usb_camera_device->base.device_ratio.width = MTBest.width;
                usb_camera_device->base.device_ratio.height = MTBest.height;
                usb_camera_device->dwStreamIndex = (DWORD)dwStreamBest;
                usb_camera_device->nativeFormat = MTBest;
                usb_camera_device->aspectN = aspectRatioN;
                usb_camera_device->aspectD = aspectRatioD;
                usb_camera_device->outputFormat = outFormat;
                usb_camera_device->convertFormat = convertToFormat;
                usb_camera_device->captureFormat = MediaType(mediaTypeOut.Get());
                return true;
            }
            close(usb_camera_device);
        }
    }
    return false;
}

static usb_camera_device_t *usb_camera_device_create()
{
    usb_camera_device_t *usb_camera_device = TKMEM_ZALLOC(usb_camera_device_t);
    return_value_if_fail(usb_camera_device != NULL, NULL);
    usb_camera_device->convertFormat = true;
    usb_camera_device->outputFormat = CV_CAP_MODE_BGR;
    usb_camera_device->aspectN = 1;
    usb_camera_device->aspectD = 1;
    usb_camera_device->camid = -1;
    usb_camera_device->captureMode = MODE_SW;

    configureHW(usb_camera_device, true);
    return usb_camera_device;
}

static usb_camera_device_t *open(int _index, uint32_t width, uint32_t height)
{
    _ComPtr<IMFAttributes> msAttr = NULL;
    usb_camera_device_t *usb_camera_device = usb_camera_device_create();
    return_value_if_fail(usb_camera_device != NULL, NULL);

    if (SUCCEEDED(MFCreateAttributes(&msAttr, 1)) && SUCCEEDED(msAttr->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID))) {
        uint32_t count;
        IMFActivate **ppDevices = NULL;
        if (SUCCEEDED(MFEnumDeviceSources(msAttr.Get(), &ppDevices, &count))) {
            if (count > 0) {
                for (int ind = 0; ind < (int)count; ind++) {
                    if (ind == _index && ppDevices[ind]) {
                        // Set source reader parameters
                        _ComPtr<IMFMediaSource> mSrc;
                        _ComPtr<IMFAttributes> srAttr;
                        if (SUCCEEDED(ppDevices[ind]->ActivateObject(__uuidof(IMFMediaSource), (void **)&mSrc)) && mSrc &&
                            SUCCEEDED(MFCreateAttributes(&srAttr, 10)) &&
                            SUCCEEDED(srAttr->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE)) &&
                            SUCCEEDED(srAttr->SetUINT32(MF_SOURCE_READER_DISABLE_DXVA, FALSE)) &&
                            SUCCEEDED(srAttr->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, FALSE)) &&
                            SUCCEEDED(srAttr->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, TRUE))) {
#ifdef HAVE_MSMF_DXVA
                            if (D3DMgr) {
                                srAttr->SetUnknown(MF_SOURCE_READER_D3D_MANAGER, D3DMgr.Get());
                            }
#endif
                            usb_camera_device->readCallback = ComPtr<IMFSourceReaderCallback>(new SourceReaderCB());
                            HRESULT hr = srAttr->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, (IMFSourceReaderCallback *)usb_camera_device->readCallback.Get());
                            if (FAILED(hr))  {
                                usb_camera_device->readCallback.Release();
                                continue;
                            }

                            if (SUCCEEDED(MFCreateSourceReaderFromMediaSource(mSrc.Get(), srAttr.Get(), &usb_camera_device->videoFileSource))) {
                                if (configureOutput(usb_camera_device, width, height, 0, usb_camera_device->aspectN, usb_camera_device->aspectD, usb_camera_device->outputFormat, usb_camera_device->convertFormat)) {
                                    usb_camera_device->camid = _index;
                                }
                            }
                        }
                    }
                    if (ppDevices[ind]) {
                        ppDevices[ind]->Release();
                    }
                }
            }
        }
    }
    if (usb_camera_device->camid == -1) {
        close(usb_camera_device);
        TKMEM_FREE(usb_camera_device);
        usb_camera_device = NULL;
    }
    return usb_camera_device;
}

static void close(usb_camera_device_t *usb_camera_device)
{
    usb_camera_device->videoSample.Release();
    usb_camera_device->videoFileSource.Release();
    usb_camera_device->readCallback.Release();
    usb_camera_device->camid = -1;
    usb_camera_device->readCallback.Release();
}

static bool grabFrame(usb_camera_device_t *usb_camera_device)
{
    if (usb_camera_device->readCallback != NULL) { // async "live" capture mode
        HRESULT hr = 0;
        SourceReaderCB *reader = ((SourceReaderCB *)usb_camera_device->readCallback.Get());
        if (!reader->m_reader) {
            // Initiate capturing with async callback
            reader->m_reader = usb_camera_device->videoFileSource.Get();
            reader->m_dwStreamIndex = usb_camera_device->dwStreamIndex;
            if (FAILED(hr = usb_camera_device->videoFileSource->ReadSample(usb_camera_device->dwStreamIndex, 0, NULL, NULL, NULL, NULL))) {
                log_error("videoio(MSMF): can't grab frame - initial async ReadSample() call failed: %ll", hr);
                reader->m_reader = NULL;
                return false;
            }
        }
        BOOL bEOS = false;
        if (FAILED(hr = reader->Wait(10000, usb_camera_device->videoSample, bEOS))) { // 10 sec
            log_warn("videoio(MSMF): can't grab frame. Error: %ll", hr);
            return false;
        }
        if (bEOS) {
            log_warn("videoio(MSMF): EOS signal. Capture stream is lost");
            return false;
        }
        return true;
    }
    return false;
}

static ret_t color_format_conver(unsigned char *src, unsigned int width, unsigned int height,
                                 unsigned int line_length,
                                 unsigned int bpp, unsigned char *dst,
                                 bitmap_format_t format, unsigned int dst_len,
                                 unsigned char is_mirror,
                                 unsigned char is_reverse)
{
    uint32_t error = -1;
    switch (format) {
    case BITMAP_FMT_BGRA8888:
        error = color_format_conversion_data_fun(COLOR_FORMAT_BGR2BGRA, src, width,
                height, line_length, bpp, dst, dst_len,
                is_mirror, is_reverse);
        break;
    case BITMAP_FMT_RGBA8888:
        error = color_format_conversion_data_fun(COLOR_FORMAT_BGR2RGBA, src, width,
                height, line_length, bpp, dst, dst_len,
                is_mirror, is_reverse);
        break;
    case BITMAP_FMT_BGR565:
        error = color_format_conversion_data_fun(COLOR_FORMAT_BGR2BGR565, src,
                width, height, line_length, bpp, dst, dst_len,
                is_mirror, is_reverse);
        break;
    case BITMAP_FMT_RGB565:
        error = color_format_conversion_data_fun(COLOR_FORMAT_BGR2RGB565, src,
                width, height, line_length, bpp, dst, dst_len,
                is_mirror, is_reverse);
        break;
    default:
        printf("color_format_conversion fail \r\n");
        error = 1;
        break;
    }

    return error == 0 ? RET_OK : RET_FAIL;
}

static bool retrieveFrame(usb_camera_device_t *usb_camera_device, unsigned char *data, bitmap_format_t format, uint32_t data_len)
{
    do {
        if (!usb_camera_device->videoSample != NULL) {
            break;
        }

        _ComPtr<IMFMediaBuffer> buf = NULL;
        if (!SUCCEEDED(usb_camera_device->videoSample->ConvertToContiguousBuffer(&buf))) {
            DWORD bcnt = 0;
            if (!SUCCEEDED(usb_camera_device->videoSample->GetBufferCount(&bcnt))) {
                break;
            }
            if (bcnt == 0) {
                break;
            }
            if (!SUCCEEDED(usb_camera_device->videoSample->GetBufferByIndex(0, &buf))) {
                break;
            }
        }

        bool lock2d = false;
        BYTE *ptr = NULL;
        LONG pitch = 0;

        // "For 2-D buffers, the Lock2D method is more efficient than the Lock method"
        // see IMFMediaBuffer::Lock method documentation: https://msdn.microsoft.com/en-us/library/windows/desktop/bb970366(v=vs.85).aspx
        _ComPtr<IMF2DBuffer> buffer2d;
        if (usb_camera_device->convertFormat) {
            if (SUCCEEDED(buf.As<IMF2DBuffer>(buffer2d))) {
                if (SUCCEEDED(buffer2d->Lock2D(&ptr, &pitch)))  {
                    lock2d = true;
                }
            }
        }
        if (!ptr) {
            break;
        }

        color_format_conver(ptr,
                            usb_camera_device->base.device_ratio.width,
                            usb_camera_device->base.device_ratio.height, pitch,
                            usb_camera_device->base.device_ratio.bpp, data, format, data_len, usb_camera_device->base.is_mirror, FALSE);

        if (lock2d) {
            buffer2d->Unlock2D();
        } else {
            buf->Unlock();
        }
        return true;
    } while (0);
    return false;
}


static ret_t usb_camera_get_all_devices_info(slist_t *devices_list)
{
    _ComPtr<IMFAttributes> msAttr = NULL;
    return_value_if_fail(devices_list != NULL, RET_BAD_PARAMS);
    if (SUCCEEDED(MFCreateAttributes(&msAttr, 1)) && SUCCEEDED(msAttr->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID))) {
        IMFActivate **ppDevices = NULL;
        uint32_t count;
        if (SUCCEEDED(MFEnumDeviceSources(msAttr.Get(), &ppDevices, &count))) {
            for (uint32_t i = 0; i < count; i++) {
                if (ppDevices[i] != NULL) {
                    WCHAR *pszName = NULL;
                    if (ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &pszName, NULL) == S_OK) {
                        usb_camera_device_info_t *device_info = TKMEM_ZALLOC(usb_camera_device_info_t);
                        device_info->camera_id = i;
                        WideCharToMultiByte(CP_ACP, 0, pszName, -1, device_info->camera_name, sizeof(device_info->camera_name), NULL, FALSE);
                        wcscpy(device_info->w_camera_name, pszName);
                        slist_append(devices_list, device_info);
                        CoTaskMemFree(pszName);
                    }
                }
            }
            CoTaskMemFree(ppDevices);
        }
    }

    return RET_OK;
}

static int usb_camera_ratio_list_on_compare(const void *a, const void *b)
{
    usb_camera_ratio_t *usb_camera_ratio_a = (usb_camera_ratio_t *)a;
    usb_camera_ratio_t *usb_camera_ratio_b = (usb_camera_ratio_t *)b;
    if (usb_camera_ratio_a->width == usb_camera_ratio_b->width && usb_camera_ratio_a->height == usb_camera_ratio_b->height) {
        return 0;
    }
    return 1;
}

static ret_t usb_camera_get_devices_ratio_list_for_device_id(uint32_t device_id, slist_t *ratio_list)
{
    ret_t ret = RET_FAIL;
    _ComPtr<IMFAttributes> msAttr = NULL;
    return_value_if_fail(ratio_list != NULL, RET_BAD_PARAMS);
    if (SUCCEEDED(MFCreateAttributes(&msAttr, 1)) && SUCCEEDED(msAttr->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID))) {
        IMFActivate **ppDevices = NULL;
        uint32_t count = 0, dwMediaTypeTest = 0, dwStreamTest = 0;
        if (SUCCEEDED(MFEnumDeviceSources(msAttr.Get(), &ppDevices, &count))) {
            for (int ind = 0; ind < (int)count; ind++) {
                if (ind == device_id && ppDevices[ind] != NULL) {
                    // Set source reader parameters
                    _ComPtr<IMFMediaSource> mSrc;
                    _ComPtr<IMFAttributes> srAttr;
                    _ComPtr<IMFSourceReader> videoFileSource;
                    if (SUCCEEDED(ppDevices[ind]->ActivateObject(__uuidof(IMFMediaSource), (void **)&mSrc)) && mSrc &&
                        SUCCEEDED(MFCreateAttributes(&srAttr, 10)) &&
                        SUCCEEDED(srAttr->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE)) &&
                        SUCCEEDED(srAttr->SetUINT32(MF_SOURCE_READER_DISABLE_DXVA, FALSE)) &&
                        SUCCEEDED(srAttr->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, FALSE)) &&
                        SUCCEEDED(srAttr->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, TRUE))) {
#ifdef HAVE_MSMF_DXVA
                        if (D3DMgr) {
                            srAttr->SetUnknown(MF_SOURCE_READER_D3D_MANAGER, D3DMgr.Get());
                        }
#endif

                        if (SUCCEEDED(MFCreateSourceReaderFromMediaSource(mSrc.Get(), srAttr.Get(), &videoFileSource))) {
                            uint32_t i = 0;
                            HRESULT hr = S_OK;
                            while (SUCCEEDED(hr)) {
                                _ComPtr<IMFMediaType> pType;
                                hr = videoFileSource->GetNativeMediaType(dwStreamTest, dwMediaTypeTest, &pType);
                                if (hr == MF_E_NO_MORE_TYPES) {
                                    hr = S_OK;
                                    ++dwStreamTest;
                                    dwMediaTypeTest = 0;
                                } else if (SUCCEEDED(hr))  {
                                    MediaType MT(pType.Get());
                                    if (MT.MF_MT_MAJOR_TYPE == MFMediaType_Video) {
                                        tk_compare_t compare = ratio_list->compare;
                                        usb_camera_ratio_t *usb_camera_ratio = TKMEM_ZALLOC(usb_camera_ratio_t);
                                        usb_camera_ratio->id = i;
                                        usb_camera_ratio->width = MT.width;
                                        usb_camera_ratio->height = MT.height;
#ifdef HAVE_MSMF_DXVA
                                        usb_camera_ratio->bpp = 4;
#else
                                        usb_camera_ratio->bpp = 3;
#endif

                                        ratio_list->compare = usb_camera_ratio_list_on_compare;
                                        if (slist_find(ratio_list, usb_camera_ratio) == NULL) {
                                            slist_append(ratio_list, usb_camera_ratio);
                                        } else {
                                            TKMEM_FREE(usb_camera_ratio);
                                        }
                                        ratio_list->compare = compare;
                                    }
                                    ++dwMediaTypeTest;
                                }
                                i++;
                            }
                        }
                    }
                    ret = RET_OK;
                    break;
                }
                if (ppDevices[ind]) {
                    ppDevices[ind]->Release();
                }
            }
            CoTaskMemFree(ppDevices);
        }
    }

    return ret;
}

static ret_t usb_camera_get_devices_ratio_list(void *p_usb_camera_device, slist_t *ratio_list)
{
    HRESULT hr = S_OK;
    uint32_t i = 0, dwStreamTest = 0, dwMediaTypeTest = 0;
    usb_camera_device_t *usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;
    return_value_if_fail(usb_camera_device != NULL && ratio_list != NULL, RET_BAD_PARAMS);
    while (SUCCEEDED(hr)) {
        _ComPtr<IMFMediaType> pType;
        hr = usb_camera_device->videoFileSource->GetNativeMediaType(dwStreamTest, dwMediaTypeTest, &pType);
        if (hr == MF_E_NO_MORE_TYPES) {
            hr = S_OK;
            ++dwStreamTest;
            dwMediaTypeTest = 0;
        } else if (SUCCEEDED(hr))  {
            MediaType MT(pType.Get());
            if (MT.MF_MT_MAJOR_TYPE == MFMediaType_Video) {
                tk_compare_t compare = ratio_list->compare;
                usb_camera_ratio_t *usb_camera_ratio = TKMEM_ZALLOC(usb_camera_ratio_t);
                usb_camera_ratio->id = i;
                usb_camera_ratio->width = MT.width;
                usb_camera_ratio->height = MT.height;
#ifdef HAVE_MSMF_DXVA
                usb_camera_ratio->bpp = 4;
#else
                usb_camera_ratio->bpp = 3;
#endif
                ratio_list->compare = usb_camera_ratio_list_on_compare;
                if (slist_find(ratio_list, usb_camera_ratio) == NULL) {
                    slist_append(ratio_list, usb_camera_ratio);
                } else {
                    TKMEM_FREE(usb_camera_ratio);
                }
                ratio_list->compare = compare;
            }
            ++dwMediaTypeTest;
        }
        i++;
    }
    return RET_OK;
}

static void *usb_camera_open_device_with_width_and_height(uint32_t device_id,
        bool_t is_mirror,
        uint32_t width,
        uint32_t height)
{
    usb_camera_device_t *usb_camera_device = open(device_id, width, height);
    return_value_if_fail(usb_camera_device != NULL, NULL);
    usb_camera_device->base.is_mirror = is_mirror;
    return usb_camera_device;
}

static void *usb_camera_open_device(uint32_t device_id, bool_t is_mirror)
{
    return usb_camera_open_device_with_width_and_height(device_id, is_mirror, 0,
            0);
}

static void *usb_camera_set_ratio(void *p_usb_camera_device, uint32_t width,
                                  uint32_t height)
{
    HRESULT hr = S_OK;
    ret_t ret = RET_FAIL;
    uint32_t i = 0, dwStreamTest = 0, dwMediaTypeTest = 0;
    usb_camera_device_t *usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;
    return_value_if_fail(usb_camera_device != NULL, NULL);
    while (SUCCEEDED(hr)) {
        _ComPtr<IMFMediaType> pType;
        hr = usb_camera_device->videoFileSource->GetNativeMediaType(dwStreamTest, dwMediaTypeTest, &pType);
        if (hr == MF_E_NO_MORE_TYPES) {
            hr = S_OK;
            ++dwStreamTest;
            dwMediaTypeTest = 0;
        } else if (SUCCEEDED(hr))  {
            MediaType MT(pType.Get());
            if (MT.MF_MT_MAJOR_TYPE == MFMediaType_Video && MT.MF_MT_SAMPLE_SIZE > 0 && width == MT.width && height == MT.height) {
                bool su = configureOutput(usb_camera_device, width, 480, height, usb_camera_device->aspectN, usb_camera_device->aspectD, usb_camera_device->outputFormat, usb_camera_device->convertFormat);
                ret = su ? RET_OK : RET_FAIL;
                break;
            }
            ++dwMediaTypeTest;
        }
        i++;
    }
    return usb_camera_device;
}

static ret_t usb_camera_close_device(void *p_usb_camera_device)
{
    usb_camera_device_t *usb_camera_device = NULL;
    return_value_if_fail(p_usb_camera_device != NULL, RET_BAD_PARAMS);

    usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;

    close(usb_camera_device);

    configureHW(usb_camera_device, false);
    TKMEM_FREE(usb_camera_device);

    return RET_OK;
}

static ret_t usb_camera_get_video_image_data(void *p_usb_camera_device,
        unsigned char *data,
        bitmap_format_t format,
        uint32_t data_len)
{
    usb_camera_device_t *usb_camera_device = NULL;
    return_value_if_fail(p_usb_camera_device != NULL, RET_BAD_PARAMS);
    usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;

    if (grabFrame(usb_camera_device)) {
        return retrieveFrame(usb_camera_device, data, format, data_len) ? RET_OK : RET_FAIL;
    }
    return RET_FAIL;
}

const usb_camera_device_base_vt_t msmf_vt = {
    usb_camera_get_all_devices_info,
    usb_camera_get_devices_ratio_list_for_device_id,
    usb_camera_get_devices_ratio_list,
    usb_camera_open_device_with_width_and_height,
    usb_camera_open_device,
    usb_camera_set_ratio,
    usb_camera_close_device,
    usb_camera_get_video_image_data,
};

extern "C" const usb_camera_drive_info_t msmd_usb_camera_drive_info = {
    "MSMF",
    &msmf_vt,
};