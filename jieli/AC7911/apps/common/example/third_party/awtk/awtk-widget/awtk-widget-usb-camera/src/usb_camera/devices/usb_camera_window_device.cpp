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

#include "usb_camera_devices.h"
#include <DShow.h>
#include <strsafe.h>
#include <assert.h>
#include <uuids.h>
#include <comdef.h>
#include <stdio.h>
#include <stdlib.h>
#include "color_format_conversion.inc"

#if defined(__MINGW32__) && defined(__MINGW64__)
namespace _com_util
{
char *WINAPI ConvertBSTRToString(BSTR pSrc)
{
    DWORD cb, cwch;
    char *szOut = NULL;

    if (!pSrc) {
        return NULL;
    }

    /* Retrieve the size of the BSTR with the NULL terminator */
    cwch = ::SysStringLen(pSrc) + 1;

    /* Compute the needed size with the NULL terminator */
    cb = ::WideCharToMultiByte(CP_ACP, 0, pSrc, cwch, NULL, 0, NULL, NULL);
    if (cb == 0) {
        cwch = ::GetLastError();
        ::_com_issue_error(!IS_ERROR(cwch) ? HRESULT_FROM_WIN32(cwch) : cwch);
        return NULL;
    }

    /* Allocate the string */
    szOut = (char *)::operator new (cb * sizeof(char));
    if (!szOut) {
        ::_com_issue_error(HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY));
        return NULL;
    }

    /* Convert the string and NULL-terminate */
    szOut[cb - 1] = '\0';
    if (::WideCharToMultiByte(CP_ACP, 0, pSrc, cwch, szOut, cb, NULL, NULL) == 0) {
        /* We failed, clean everything up */
        cwch = ::GetLastError();
        ::operator delete (szOut);
        szOut = NULL;
        ::_com_issue_error(!IS_ERROR(cwch) ? HRESULT_FROM_WIN32(cwch) : cwch);
    }

    return szOut;
}
}

#ifndef HEADER
#define HEADER(x) ((BITMAPINFOHEADER*)(&(x)->bmiHeader))
#endif
#endif

#define RELEASE_INTERFACE(x)                                                   \
  if (NULL != x) {                                                             \
    x->Release();                                                              \
    x = NULL;                                                                  \
  }

#define FAILED_PRINTF(hr, log)                                                 \
  if (FAILED(hr)) {                                                            \
    printf(log);                                                               \
  }

#define FAILED_PRINTF_FUN(hr, log, fun)                                        \
  if (FAILED(hr)) {                                                            \
    printf(log);                                                               \
    fun();                                                                     \
  }

#define FAILED_GOTO_DISPOSE(hr)                                                \
  if (FAILED(hr)) {                                                            \
    goto error;                                                                \
  }

struct ISampleGrabberCB : public IUnknown {
    virtual STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample) = 0;
    virtual STDMETHODIMP BufferCB(double SampleTime, BYTE *pBuffer,
                                  long BufferLen) = 0;
};

static const IID IID_ISampleGrabberCB = {
    0x0579154A,
    0x2B53,
    0x4994,
    {0xB0, 0xD0, 0xE7, 0x73, 0x14, 0x8E, 0xFF, 0x85}
};

struct ISampleGrabber : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE SetOneShot(BOOL OneShot) = 0;
    virtual HRESULT STDMETHODCALLTYPE
    SetMediaType(const AM_MEDIA_TYPE *pType) = 0;
    virtual HRESULT STDMETHODCALLTYPE
    GetConnectedMediaType(AM_MEDIA_TYPE *pType) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(BOOL BufferThem) = 0;
    virtual HRESULT STDMETHODCALLTYPE
    GetCurrentBuffer(long *pBufferSize, long *pBuffer) = 0;
    virtual HRESULT STDMETHODCALLTYPE
    GetCurrentSample(IMediaSample **ppSample) = 0;
    virtual HRESULT STDMETHODCALLTYPE
    SetCallback(ISampleGrabberCB *pCallback, long WhichMethodToCallback) = 0;
};

static const IID IID_ISampleGrabber = {
    0x6B652FFF,
    0x11FE,
    0x4fce,
    {0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F}
};

static const CLSID CLSID_SampleGrabber = {
    0xC1F400A0,
    0x3F08,
    0x11d3,
    {0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37}
};

static const CLSID CLSID_NullRenderer = {
    0xC1F400A4,
    0x3F08,
    0x11d3,
    {0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37}
};

static const CLSID CLSID_VideoEffects1Category = {
    0xcc7bfb42,
    0xf175,
    0x11d1,
    {0xa3, 0x92, 0x0, 0xe0, 0x29, 0x1f, 0x39, 0x59}
};

static const CLSID CLSID_VideoEffects2Category = {
    0xcc7bfb43,
    0xf175,
    0x11d1,
    {0xa3, 0x92, 0x0, 0xe0, 0x29, 0x1f, 0x39, 0x59}
};

static const CLSID CLSID_AudioEffects1Category = {
    0xcc7bfb44,
    0xf175,
    0x11d1,
    {0xa3, 0x92, 0x0, 0xe0, 0x29, 0x1f, 0x39, 0x59}
};

static const CLSID CLSID_AudioEffects2Category = {
    0xcc7bfb45,
    0xf175,
    0x11d1,
    {0xa3, 0x92, 0x0, 0xe0, 0x29, 0x1f, 0x39, 0x59}
};

DEFINE_GUID(MEDIASUBTYPE_AYUV, 0x56555941, 0x0000, 0x0010, 0x80, 0x00, 0x00,
            0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_IYUV, 0x56555949, 0x0000, 0x0010, 0x80, 0x00, 0x00,
            0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_RGB24, 0xe436eb7d, 0x524f, 0x11ce, 0x9f, 0x53, 0x00,
            0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(MEDIASUBTYPE_RGB32, 0xe436eb7e, 0x524f, 0x11ce, 0x9f, 0x53, 0x00,
            0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(MEDIASUBTYPE_RGB555, 0xe436eb7c, 0x524f, 0x11ce, 0x9f, 0x53, 0x00,
            0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(MEDIASUBTYPE_RGB565, 0xe436eb7b, 0x524f, 0x11ce, 0x9f, 0x53, 0x00,
            0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(MEDIASUBTYPE_I420, 0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00,
            0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_UYVY, 0x59565955, 0x0000, 0x0010, 0x80, 0x00, 0x00,
            0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_Y211, 0x31313259, 0x0000, 0x0010, 0x80, 0x00, 0x00,
            0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_Y411, 0x31313459, 0x0000, 0x0010, 0x80, 0x00, 0x00,
            0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_Y41P, 0x50313459, 0x0000, 0x0010, 0x80, 0x00, 0x00,
            0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_YUY2, 0x32595559, 0x0000, 0x0010, 0x80, 0x00, 0x00,
            0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_YUYV, 0x56595559, 0x0000, 0x0010, 0x80, 0x00, 0x00,
            0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_YV12, 0x32315659, 0x0000, 0x0010, 0x80, 0x00, 0x00,
            0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_YVU9, 0x39555659, 0x0000, 0x0010, 0x80, 0x00, 0x00,
            0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_YVYU, 0x55595659, 0x0000, 0x0010, 0x80, 0x00, 0x00,
            0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_MJPG, 0x47504A4D, 0x0000, 0x0010, 0x80, 0x00, 0x00,
            0xaa, 0x00, 0x38, 0x9b, 0x71); // MGB
DEFINE_GUID(MEDIATYPE_Interleaved, 0x73766169, 0x0000, 0x0010, 0x80, 0x00, 0x00,
            0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIATYPE_Video, 0x73646976, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa,
            0x00, 0x38, 0x9b, 0x71);

class failed_printf_fun
{
private:
    int failed_number;

public:
    failed_printf_fun()
    {
        failed_number = 0;
    }
    void operator()()
    {
        failed_number++;
    }
    const int get_failed_number()
    {
        return failed_number;
    }
};

class SampleGrabberCallback : public ISampleGrabberCB
{
public:
    SampleGrabberCallback()
    {
        InitializeCriticalSection(&critSection);
        freezeCheck = 0;

        bufferSetup = false;
        newFrame = false;
        latestBufferLength = 0;

        hEvent = CreateEvent(NULL, true, false, NULL);
    }

    virtual ~SampleGrabberCallback()
    {
        ptrBuffer = NULL;
        DeleteCriticalSection(&critSection);
        CloseHandle(hEvent);
        if (bufferSetup) {
            delete[] pixels;
        }
    }

    bool setupBuffer(int numBytesIn)
    {
        if (bufferSetup) {
            return false;
        } else {
            numBytes = numBytesIn;
            pixels = new unsigned char[numBytes];
            bufferSetup = true;
            newFrame = false;
            latestBufferLength = 0;
        }
        return true;
    }

    STDMETHODIMP_(ULONG) AddRef()
    {
        return 1;
    }
    STDMETHODIMP_(ULONG) Release()
    {
        return 2;
    }

    STDMETHODIMP QueryInterface(REFIID, void **ppvObject)
    {
        *ppvObject = static_cast<ISampleGrabberCB *>(this);
        return S_OK;
    }

    STDMETHODIMP SampleCB(double, IMediaSample *pSample)
    {
        if (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0) {
            return S_OK;
        }

        HRESULT hr = pSample->GetPointer(&ptrBuffer);

        if (hr == S_OK) {
            latestBufferLength = pSample->GetActualDataLength();
            if (latestBufferLength == numBytes) {
                EnterCriticalSection(&critSection);
                memcpy(pixels, ptrBuffer, latestBufferLength);
                newFrame = true;
                freezeCheck = 1;
                LeaveCriticalSection(&critSection);
                SetEvent(hEvent);
            } else {
                printf("ERROR: SampleCB() - buffer sizes do not match, numBytes:%d, "
                       "latestBufferLength:%d \n",
                       numBytes, latestBufferLength);
            }
        }
        return S_OK;
    }

    STDMETHODIMP BufferCB(double, BYTE *, long)
    {
        return E_NOTIMPL;
    }

    int freezeCheck;

    int latestBufferLength;
    int numBytes;
    bool newFrame;
    bool bufferSetup;
    unsigned char *pixels;
    unsigned char *ptrBuffer;
    CRITICAL_SECTION critSection;
    HANDLE hEvent;
};

typedef struct _usb_camera_device_t {
    usb_camera_device_base_t base;

    IGraphBuilder *pGraph;
    IMediaControl *pControl;
    ISampleGrabber *pGrabber;
    IAMStreamConfig *streamConf;

    IBaseFilter *pGrabberF;
    IBaseFilter *pDestFilter;
    IBaseFilter *pVideoInputFilter;

    SampleGrabberCallback *sgCallback;

    ICaptureGraphBuilder2 *pCaptureGraph;

} usb_camera_device_t;

#define VI_NUM_TYPES 16

const static GUID mediaSubtypes[VI_NUM_TYPES] = {
    MEDIASUBTYPE_RGB24,  MEDIASUBTYPE_RGB32, MEDIASUBTYPE_RGB555,
    MEDIASUBTYPE_RGB565, MEDIASUBTYPE_YUY2,  MEDIASUBTYPE_YVYU,
    MEDIASUBTYPE_YUYV,   MEDIASUBTYPE_IYUV,  MEDIASUBTYPE_UYVY,
    MEDIASUBTYPE_YV12,   MEDIASUBTYPE_YVU9,  MEDIASUBTYPE_Y411,
    MEDIASUBTYPE_Y41P,   MEDIASUBTYPE_Y211,  MEDIASUBTYPE_AYUV,
    MEDIASUBTYPE_MJPG,
    // MEDIASUBTYPE_Y800,
    // MEDIASUBTYPE_Y8,
    // MEDIASUBTYPE_GREY,
    // MEDIASUBTYPE_I420
};

static ret_t usb_camera_bind_filter(uint32_t device_id,
                                    IBaseFilter **p_base_filter,
                                    usb_camera_device_info_t *device_info);

ret_t usb_camera_get_all_devices_info(slist_t *devices_list)
{
    uint32_t id = 0;
    ULONG cFetched = 0;
    IMoniker *pMoniker;
    IEnumMoniker *pEnumMon;
    ICreateDevEnum *pDevEnum;
    HRESULT hr =
        CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                         IID_ICreateDevEnum, (LPVOID *)&pDevEnum);
    if (SUCCEEDED(hr)) {
        hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
                                             &pEnumMon, 0);
        if (hr == S_FALSE) {
            hr = VFW_E_NOT_FOUND;
            return RET_NOT_FOUND;
        }
        pEnumMon->Reset();
        while ((hr = pEnumMon->Next(1, &pMoniker, &cFetched)) == S_OK) {
            IPropertyBag *pProBag;
            hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (LPVOID *)&pProBag);
            if (SUCCEEDED(hr)) {
                VARIANT varTemp;
                varTemp.vt = VT_BSTR;
                hr = pProBag->Read(L"FriendlyName", &varTemp, NULL);
                if (SUCCEEDED(hr)) {
                    usb_camera_device_info_t *device_info =
                        TKMEM_ZALLOC(usb_camera_device_info_t);
                    device_info->camera_id = id++;

                    _bstr_t b(varTemp.bstrVal);
                    memcpy(device_info->camera_name, (const char *)b,
                           b.length() < USB_CAMERA_NAME_MAX_LENGTH
                           ? b.length()
                           : USB_CAMERA_NAME_MAX_LENGTH);
                    memcpy(device_info->w_camera_name, (const wchar_t *)b,
                           b.length() * 2 < USB_CAMERA_NAME_MAX_LENGTH
                           ? b.length() * 2
                           : USB_CAMERA_NAME_MAX_LENGTH);

                    slist_append(devices_list, device_info);
                }
                pProBag->Release();
            }
            pMoniker->Release();
        }
        pEnumMon->Release();
    }

    return RET_OK;
}

ret_t usb_camera_get_devices_ratio_list_for_device_id(uint32_t device_id,
        slist_t *ratio_list)
{
    ret_t ret = RET_OK;
    HRESULT hr = NOERROR;
    IAMStreamConfig *streamConfTest = NULL;
    GUID CAPTURE_MODE = PIN_CATEGORY_CAPTURE;
    usb_camera_device_t *usb_camera_device = TKMEM_ZALLOC(usb_camera_device_t);

    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
                          IID_ICaptureGraphBuilder2,
                          (void **)&usb_camera_device->pCaptureGraph);
    FAILED_GOTO_DISPOSE(hr);

    hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER,
                          IID_IGraphBuilder, (void **)&usb_camera_device->pGraph);
    FAILED_GOTO_DISPOSE(hr);
    hr = usb_camera_device->pCaptureGraph->SetFiltergraph(
             usb_camera_device->pGraph);
    FAILED_GOTO_DISPOSE(hr);
    hr = usb_camera_device->pGraph->QueryInterface(
             IID_IMediaControl, (void **)&usb_camera_device->pControl);
    FAILED_GOTO_DISPOSE(hr);

    if (usb_camera_bind_filter(device_id, &usb_camera_device->pVideoInputFilter,
                               &usb_camera_device->base.device_info) == RET_OK) {
        hr = usb_camera_device->pGraph->AddFilter(
                 usb_camera_device->pVideoInputFilter,
                 usb_camera_device->base.device_info.w_camera_name);
        FAILED_GOTO_DISPOSE(hr);
        hr = usb_camera_device->pCaptureGraph->FindInterface(
                 &PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
                 usb_camera_device->pVideoInputFilter, IID_IAMStreamConfig,
                 (void **)&streamConfTest);
        if (SUCCEEDED(hr)) {
            CAPTURE_MODE = PIN_CATEGORY_PREVIEW;
            streamConfTest->Release();
            streamConfTest = NULL;
        }

        hr = usb_camera_device->pCaptureGraph->FindInterface(
                 &CAPTURE_MODE, &MEDIATYPE_Video, usb_camera_device->pVideoInputFilter,
                 IID_IAMStreamConfig, (void **)&usb_camera_device->streamConf);
        FAILED_GOTO_DISPOSE(hr);

        ret = usb_camera_get_devices_ratio_list(usb_camera_device, ratio_list);
        usb_camera_close_device(usb_camera_device);
        return ret;
    }
error:
    usb_camera_close_device(usb_camera_device);
    return RET_FAIL;
}

ret_t usb_camera_get_devices_ratio_list(void *p_usb_camera_device,
                                        slist_t *ratio_list)
{
    int piSize = 0;
    int piCount = 0;
    HRESULT hr = NOERROR;
    AM_MEDIA_TYPE *mt = NULL;
    VIDEO_STREAM_CONFIG_CAPS scc;
    IAMStreamConfig *streamConf = NULL;

    return_value_if_fail(p_usb_camera_device != NULL, RET_BAD_PARAMS);

    streamConf = ((usb_camera_device_t *)p_usb_camera_device)->streamConf;
    hr = streamConf->GetNumberOfCapabilities(&piCount, &piSize);

    if (SUCCEEDED(hr)) {
        if (sizeof(VIDEO_STREAM_CONFIG_CAPS) == piSize) {
            for (int i = 0; i < piCount; i++) {
                mt = NULL;
                ZeroMemory(&scc, sizeof(VIDEO_STREAM_CONFIG_CAPS));
                hr = streamConf->GetStreamCaps(i, &mt,
                                               reinterpret_cast<unsigned char *>(&scc));

                if (SUCCEEDED(hr)) {
                    VIDEOINFOHEADER *pVih =
                        reinterpret_cast<VIDEOINFOHEADER *>(mt->pbFormat);

                    usb_camera_ratio_t *usb_camera_ratio =
                        TKMEM_ZALLOC(usb_camera_ratio_t);
                    usb_camera_ratio->id = i;
                    usb_camera_ratio->width = HEADER(pVih)->biWidth;
                    usb_camera_ratio->height = HEADER(pVih)->biHeight;
                    usb_camera_ratio->bpp = HEADER(pVih)->biBitCount / 8;

                    slist_append(ratio_list, usb_camera_ratio);
                }
            }
        }
    }

    return RET_OK;
}

static ret_t usb_camera_bind_filter(uint32_t device_id,
                                    IBaseFilter **p_base_filter,
                                    usb_camera_device_info_t *device_info)
{
    int index = 0;
    ULONG cFetched = 0;
    IMoniker *pMoniker;
    IEnumMoniker *pEnumMon;
    ICreateDevEnum *pDevEnum;
    HRESULT hr =
        CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                         IID_ICreateDevEnum, (LPVOID *)&pDevEnum);
    if (SUCCEEDED(hr)) {
        hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
                                             &pEnumMon, 0);
        if (hr == S_FALSE) {
            hr = VFW_E_NOT_FOUND;
            return RET_NOT_FOUND;
        }
        pEnumMon->Reset();
        while ((hr = pEnumMon->Next(1, &pMoniker, &cFetched)) == S_OK &&
               index <= (int)device_id) {
            IPropertyBag *pProBag;
            hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (LPVOID *)&pProBag);
            if (SUCCEEDED(hr)) {
                if (index == device_id) {
                    VARIANT varName;
                    VariantInit(&varName);
                    hr = pProBag->Read(L"FriendlyName", &varName, 0);

                    _bstr_t b(varName.bstrVal);
                    memcpy(device_info->camera_name, (const char *)b,
                           b.length() < USB_CAMERA_NAME_MAX_LENGTH
                           ? b.length()
                           : USB_CAMERA_NAME_MAX_LENGTH);
                    memcpy(device_info->w_camera_name, (const wchar_t *)b,
                           b.length() * 2 < USB_CAMERA_NAME_MAX_LENGTH
                           ? b.length() * 2
                           : USB_CAMERA_NAME_MAX_LENGTH);
                    device_info->camera_id = device_id;

                    pMoniker->BindToObject(0, 0, IID_IBaseFilter,
                                           (LPVOID *)p_base_filter);
                }
            }
            pMoniker->Release();
            index++;
        }
        pEnumMon->Release();
    }
    return RET_OK;
}

static usb_camera_device_t *usb_camera_device_init(bool_t is_mirror)
{
    usb_camera_device_t *usb_camera_device = TKMEM_ZALLOC(usb_camera_device_t);

    usb_camera_device->sgCallback = new SampleGrabberCallback();
    usb_camera_device->base.is_mirror = is_mirror;

    return usb_camera_device;
}

static void free_media_type(AM_MEDIA_TYPE &mt)
{
    if (mt.cbFormat != 0) {
        CoTaskMemFree((PVOID)mt.pbFormat);
        mt.cbFormat = 0;
        mt.pbFormat = NULL;
    }
    if (mt.pUnk != NULL) {
        // Unecessary because pUnk should not be used, but safest.
        mt.pUnk->Release();
        mt.pUnk = NULL;
    }
}

static void delete_media_type(AM_MEDIA_TYPE *pmt)
{
    if (pmt != NULL) {
        free_media_type(*pmt);
        CoTaskMemFree(pmt);
    }
}

static bool setSizeAndSubtype(usb_camera_device_t *usb_camera_device,
                              AM_MEDIA_TYPE *pAmMediaType, int attemptWidth,
                              int attemptHeight, GUID mediatype)
{

    VIDEOINFOHEADER *pVih =
        reinterpret_cast<VIDEOINFOHEADER *>(pAmMediaType->pbFormat);

    AM_MEDIA_TYPE *tmpType = NULL;

    HRESULT hr = usb_camera_device->streamConf->GetFormat(&tmpType);
    if (hr != S_OK) {
        return false;
    }

    // set new size:
    // width and height
    HEADER(pVih)->biWidth = attemptWidth;
    HEADER(pVih)->biHeight = attemptHeight;

    pAmMediaType->formattype = FORMAT_VideoInfo;
    pAmMediaType->majortype = MEDIATYPE_Video;
    pAmMediaType->subtype = mediatype;

    // buffer size
    if (mediatype == MEDIASUBTYPE_RGB24) {
        pAmMediaType->lSampleSize = attemptWidth * attemptHeight * 3;
    } else {
        pAmMediaType->lSampleSize = 0;
    }

    // okay lets try new size
    hr = usb_camera_device->streamConf->SetFormat(pAmMediaType);
    if (hr == S_OK) {
        if (tmpType != NULL) {
            delete_media_type(tmpType);
        }
        return true;
    } else {
        usb_camera_device->streamConf->SetFormat(tmpType);
        if (tmpType != NULL) {
            delete_media_type(tmpType);
        }
    }

    return false;
}

static ret_t
usb_camera_set_setup_device_ratio(usb_camera_device_t *usb_camera_device,
                                  uint32_t width, uint32_t height)
{
    ret_t ret = RET_FAIL;
    HRESULT hr = NOERROR;
    AM_MEDIA_TYPE *pAmMediaType;
    hr = usb_camera_device->streamConf->GetFormat(&pAmMediaType);
    if (SUCCEEDED(hr)) {
        if (width == 0 || height == 0) {
            VIDEOINFOHEADER *pVih =
                reinterpret_cast<VIDEOINFOHEADER *>(pAmMediaType->pbFormat);
            usb_camera_device->base.device_ratio.width = HEADER(pVih)->biWidth;
            usb_camera_device->base.device_ratio.height = HEADER(pVih)->biHeight;
            usb_camera_device->base.device_ratio.bpp = 3;
            ret = RET_OK;
        } else {
            for (int i = 0; i < VI_NUM_TYPES; i++) {
                if (setSizeAndSubtype(usb_camera_device, pAmMediaType, width, height,
                                      mediaSubtypes[i])) {
                    VIDEOINFOHEADER *pVih =
                        reinterpret_cast<VIDEOINFOHEADER *>(pAmMediaType->pbFormat);
                    usb_camera_device->base.device_ratio.width = HEADER(pVih)->biWidth;
                    usb_camera_device->base.device_ratio.height = HEADER(pVih)->biHeight;
                    usb_camera_device->base.device_ratio.bpp = 3;
                    ret = RET_OK;
                    break;
                }
            }
        }
        return ret;
    } else {
        printf("ERROR: Could not GetFormat\n");
    }

    return ret;
}

static void *usb_camera_setup_device(uint32_t device_id, uint32_t width,
                                     uint32_t height, bool_t is_mirror)
{
    AM_MEDIA_TYPE mt;
    ret_t ret = RET_OK;
    HRESULT hr = NOERROR;
    failed_printf_fun failed_fun;
    IMediaFilter *pMediaFilter = NULL;
    IAMStreamConfig *streamConfTest = NULL;
    GUID CAPTURE_MODE = PIN_CATEGORY_CAPTURE;
    usb_camera_device_t *usb_camera_device = usb_camera_device_init(is_mirror);

    ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
    mt.majortype = MEDIATYPE_Video;
    mt.subtype = MEDIASUBTYPE_RGB24;
    mt.formattype = FORMAT_VideoInfo;

    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
                          IID_ICaptureGraphBuilder2,
                          (void **)&usb_camera_device->pCaptureGraph);
    FAILED_PRINTF_FUN(
        hr, "ERROR: Could not get IID_ICaptureGraphBuilder2 interface\n",
        failed_fun);

    hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER,
                          IID_IGraphBuilder, (void **)&usb_camera_device->pGraph);
    FAILED_PRINTF_FUN(hr, "ERROR: Could not get IID_IGraphBuilder interface\n",
                      failed_fun);
    hr = usb_camera_device->pCaptureGraph->SetFiltergraph(
             usb_camera_device->pGraph);
    FAILED_PRINTF_FUN(hr, "ERROR: Could not SetFiltergraph interface\n",
                      failed_fun);
    hr = usb_camera_device->pGraph->QueryInterface(
             IID_IMediaControl, (void **)&usb_camera_device->pControl);
    FAILED_PRINTF_FUN(hr, "ERROR: Could not query IID_IMediaControl interface\n",
                      failed_fun);

    ret = usb_camera_bind_filter(device_id, &usb_camera_device->pVideoInputFilter,
                                 &usb_camera_device->base.device_info);
    if (ret == RET_OK && failed_fun.get_failed_number() == 0) {
        hr = usb_camera_device->pGraph->AddFilter(
                 usb_camera_device->pVideoInputFilter,
                 usb_camera_device->base.device_info.w_camera_name);
        FAILED_PRINTF_FUN(hr, "ERROR: Could not addFilter VideoInputFilter \n",
                          failed_fun);

        hr = usb_camera_device->pCaptureGraph->FindInterface(
                 &PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
                 usb_camera_device->pVideoInputFilter, IID_IAMStreamConfig,
                 (void **)&streamConfTest);
        if (SUCCEEDED(hr)) {
            CAPTURE_MODE = PIN_CATEGORY_PREVIEW;
            streamConfTest->Release();
            streamConfTest = NULL;
        }

        hr = usb_camera_device->pCaptureGraph->FindInterface(
                 &CAPTURE_MODE, &MEDIATYPE_Video, usb_camera_device->pVideoInputFilter,
                 IID_IAMStreamConfig, (void **)&usb_camera_device->streamConf);
        FAILED_PRINTF_FUN(hr, "ERROR: Could not findInterface MEDIATYPE_Video for "
                          "VideoInputFilter \n",
                          failed_fun);

        if (usb_camera_set_setup_device_ratio(usb_camera_device, width, height) !=
            RET_OK) {
            printf("ERROR: Not Support this ratio (%d, %d) \n", width, height);
            failed_fun();
        }

        hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
                              IID_IBaseFilter,
                              (void **)&usb_camera_device->pGrabberF);
        FAILED_PRINTF_FUN(hr, "ERROR: Could not get IID_IBaseFilter Interface \n",
                          failed_fun);

        hr = usb_camera_device->pGraph->AddFilter(usb_camera_device->pGrabberF,
                L"Sample Grabber");
        FAILED_PRINTF_FUN(hr, "ERROR: Could not addFilter Sample Grabber \n",
                          failed_fun);

        hr = usb_camera_device->pGrabberF->QueryInterface(
                 IID_ISampleGrabber, (void **)&usb_camera_device->pGrabber);
        FAILED_PRINTF_FUN(hr,
                          "ERROR: Could not query IID_ISampleGrabber Interface \n",
                          failed_fun);

        hr = usb_camera_device->pGrabber->SetOneShot(FALSE);
        FAILED_PRINTF_FUN(hr, "ERROR: Could not SetOneShot \n", failed_fun);

        hr = usb_camera_device->pGrabber->SetBufferSamples(FALSE);
        FAILED_PRINTF_FUN(hr, "ERROR: Could not SetBufferSamples \n", failed_fun);

        hr = usb_camera_device->pGrabber->SetCallback(usb_camera_device->sgCallback,
                FALSE);
        FAILED_PRINTF_FUN(hr, "ERROR: Could not SetCallback \n", failed_fun);

        if (!usb_camera_device->sgCallback->setupBuffer(
                usb_camera_device->base.device_ratio.width *
                usb_camera_device->base.device_ratio.height *
                usb_camera_device->base.device_ratio.bpp)) {
            printf("ERROR: Could not Callback setupBuffer \n");
            failed_fun();
        }

        hr = usb_camera_device->pGrabber->SetMediaType(&mt);
        FAILED_PRINTF_FUN(hr, "ERROR: Could not SetMediaType \n", failed_fun);

        hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,
                              IID_IBaseFilter,
                              (void **)(&usb_camera_device->pDestFilter));
        FAILED_PRINTF_FUN(hr, "ERROR: Could not get IID_IBaseFilter interface \n",
                          failed_fun);

        hr = usb_camera_device->pGraph->AddFilter(usb_camera_device->pDestFilter,
                L"NullRenderer");
        FAILED_PRINTF_FUN(hr, "ERROR: Could not AddFilter NullRenderer \n",
                          failed_fun);

        hr = usb_camera_device->pCaptureGraph->RenderStream(
                 &PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
                 usb_camera_device->pVideoInputFilter, usb_camera_device->pGrabberF,
                 usb_camera_device->pDestFilter);
        FAILED_PRINTF_FUN(hr, "ERROR: Could not get RenderStream \n", failed_fun);

        hr = usb_camera_device->pGraph->QueryInterface(IID_IMediaFilter,
                (void **)&pMediaFilter);
        FAILED_PRINTF_FUN(hr, "ERROR: Could not get IID_IMediaFilter interface\n",
                          failed_fun)
        else {
            pMediaFilter->SetSyncSource(NULL);
            pMediaFilter->Release();
        }

        RELEASE_INTERFACE(usb_camera_device->pVideoInputFilter);
        RELEASE_INTERFACE(usb_camera_device->pGrabberF);
        RELEASE_INTERFACE(usb_camera_device->pDestFilter);

        if (failed_fun.get_failed_number() == 0) {
            return usb_camera_device;
        }
    } else {
        printf("ERROR: Could not bind usb camera \n");
    }

    usb_camera_close_device(usb_camera_device);
    return NULL;
}

static bool usb_camera_run_device(void *p_usb_camera_device)
{
    HRESULT hr = NOERROR;
    usb_camera_device_t *usb_camera_device =
        (usb_camera_device_t *)p_usb_camera_device;

    hr = usb_camera_device->pControl->Run();
    if (FAILED(hr)) {
        printf("ERROR: Could not start graph\n");
        return false;
    } else {
        return true;
    }
}

void *usb_camera_open_device_with_width_and_height(uint32_t device_id,
        bool_t is_mirror,
        uint32_t width,
        uint32_t height)
{
    void *usb_camera_device =
        usb_camera_setup_device(device_id, width, height, is_mirror);
    if (usb_camera_device != NULL) {
        if (!usb_camera_run_device(usb_camera_device)) {
            usb_camera_close_device(usb_camera_device);
            return NULL;
        }
    }

    return usb_camera_device;
}

void *usb_camera_open_device(uint32_t device_id, bool_t is_mirror)
{
    return usb_camera_open_device_with_width_and_height(device_id, is_mirror, 0,
            0);
}

void *usb_camera_set_ratio(void *p_usb_camera_device, uint32_t width,
                           uint32_t height)
{
    HRESULT hr = NOERROR;
    uint32_t device_id = 0;
    bool_t is_mirror = FALSE;
    usb_camera_device_t *usb_camera_device = NULL;
    return_value_if_fail(p_usb_camera_device != NULL, NULL);
    usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;

    return_value_if_fail(
        usb_camera_check_ratio(p_usb_camera_device, width, height) == RET_OK,
        NULL);

    is_mirror = usb_camera_device->base.is_mirror;
    device_id = usb_camera_device->base.device_info.camera_id;
    usb_camera_close_device(usb_camera_device);

    return usb_camera_open_device_with_width_and_height(device_id, is_mirror,
            width, height);
}

static void usb_camera_nuke_downstream(IGraphBuilder *pGraph,
                                       IBaseFilter *pBF)
{
    IPin *pP, *pTo;
    ULONG u;
    IEnumPins *pins = NULL;
    PIN_INFO pininfo;
    HRESULT hr = pBF->EnumPins(&pins);
    pins->Reset();
    while (hr == NOERROR) {
        hr = pins->Next(1, &pP, &u);
        if (hr == S_OK && pP) {
            pP->ConnectedTo(&pTo);
            if (pTo) {
                hr = pTo->QueryPinInfo(&pininfo);
                if (hr == NOERROR) {
                    if (pininfo.dir == PINDIR_INPUT) {
                        usb_camera_nuke_downstream(pGraph, pininfo.pFilter);
                        pGraph->Disconnect(pTo);
                        pGraph->Disconnect(pP);
                        pGraph->RemoveFilter(pininfo.pFilter);
                    }
                    pininfo.pFilter->Release();
                    pininfo.pFilter = NULL;
                }
                pTo->Release();
            }
            pP->Release();
        }
    }
    if (pins) {
        pins->Release();
    }
}

static void usb_camera_destroy_graph(IGraphBuilder *pGraph)
{
    HRESULT hr = 0;

    int i = 0;
    while (hr == NOERROR) {
        IEnumFilters *pEnum = 0;
        ULONG cFetched;

        hr = pGraph->EnumFilters(&pEnum);

        IBaseFilter *pFilter = NULL;
        if (pEnum->Next(1, &pFilter, &cFetched) == S_OK) {
            FILTER_INFO FilterInfo;
            memset(&FilterInfo, 0, sizeof(FilterInfo));
            hr = pFilter->QueryFilterInfo(&FilterInfo);
            FilterInfo.pGraph->Release();

            int count = 0;
            char buffer[255];
            memset(buffer, 0, 255 * sizeof(char));

            while (FilterInfo.achName[count] != 0x00) {
                buffer[count] = (char)FilterInfo.achName[count];
                count++;
            }

            hr = pGraph->RemoveFilter(pFilter);

            pFilter->Release();
            pFilter = NULL;
        } else {
            break;
        }
        pEnum->Release();
        pEnum = NULL;
        i++;
    }

    return;
}

ret_t usb_camera_close_device(void *p_usb_camera_device)
{
    HRESULT hr = NOERROR;
    usb_camera_device_t *usb_camera_device = NULL;

    return_value_if_fail(p_usb_camera_device != NULL, RET_BAD_PARAMS);

    usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;

    if (usb_camera_device->sgCallback != NULL &&
        usb_camera_device->pGrabber != NULL) {
        usb_camera_device->pGrabber->SetCallback(NULL, 1);
        usb_camera_device->sgCallback->Release();

        delete usb_camera_device->sgCallback;
        usb_camera_device->sgCallback = NULL;
    }

    if (usb_camera_device->pControl != NULL) {
        hr = usb_camera_device->pControl->Pause();
        hr = usb_camera_device->pControl->Stop();
    }

    if (usb_camera_device->pVideoInputFilter != NULL) {
        usb_camera_nuke_downstream(usb_camera_device->pGraph,
                                   usb_camera_device->pVideoInputFilter);
    }

    RELEASE_INTERFACE(usb_camera_device->pDestFilter);
    RELEASE_INTERFACE(usb_camera_device->pVideoInputFilter);
    RELEASE_INTERFACE(usb_camera_device->pGrabberF);
    RELEASE_INTERFACE(usb_camera_device->pGrabber);
    RELEASE_INTERFACE(usb_camera_device->pControl);
    RELEASE_INTERFACE(usb_camera_device->streamConf);

    if (usb_camera_device->pGraph != NULL) {
        usb_camera_destroy_graph(usb_camera_device->pGraph);
    }

    RELEASE_INTERFACE(usb_camera_device->pCaptureGraph);
    RELEASE_INTERFACE(usb_camera_device->pGraph);

    delete usb_camera_device->pDestFilter;
    delete usb_camera_device->pVideoInputFilter;
    delete usb_camera_device->pGrabberF;
    delete usb_camera_device->pGrabber;
    delete usb_camera_device->pControl;
    delete usb_camera_device->streamConf;
    delete usb_camera_device->pCaptureGraph;
    delete usb_camera_device->pGraph;

    TKMEM_FREE(usb_camera_device);

    return RET_OK;
}

static ret_t color_format_conver(unsigned char *src, unsigned int width, unsigned int height,
                                 unsigned int bpp, unsigned char *dst,
                                 bitmap_format_t format, unsigned int dst_len,
                                 unsigned char is_mirror,
                                 unsigned char is_reverse)
{
    uint32_t error = -1;
    switch (format) {
    case BITMAP_FMT_BGRA8888:
        error = color_format_conversion_data_fun(COLOR_FORMAT_BGR2BGRA, src, width,
                height, bpp, dst, dst_len,
                is_mirror, is_reverse);
        break;
    case BITMAP_FMT_RGBA8888:
        error = color_format_conversion_data_fun(COLOR_FORMAT_BGR2RGBA, src, width,
                height, bpp, dst, dst_len,
                is_mirror, is_reverse);
        break;
    case BITMAP_FMT_BGR565:
        error = color_format_conversion_data_fun(COLOR_FORMAT_BGR2BGR565, src,
                width, height, bpp, dst, dst_len,
                is_mirror, is_reverse);
        break;
    case BITMAP_FMT_RGB565:
        error = color_format_conversion_data_fun(COLOR_FORMAT_BGR2RGB565, src,
                width, height, bpp, dst, dst_len,
                is_mirror, is_reverse);
        break;
    default:
        printf("color_format_conversion fail \r\n");
        error = 1;
        break;
    }

    return error == 0 ? RET_OK : RET_FAIL;
}

ret_t usb_camera_get_video_image_data(void *p_usb_camera_device,
                                      unsigned char *data,
                                      bitmap_format_t format,
                                      uint32_t data_len)
{
    usb_camera_device_t *usb_camera_device = NULL;
    return_value_if_fail(p_usb_camera_device != NULL, RET_BAD_PARAMS);

    usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;

    return_value_if_fail(usb_camera_device != NULL, RET_BAD_PARAMS);

    DWORD result =
        WaitForSingleObject(usb_camera_device->sgCallback->hEvent, 1000);
    if (result != WAIT_OBJECT_0) {
        return RET_FAIL;
    }

    EnterCriticalSection(&usb_camera_device->sgCallback->critSection);

    color_format_conver(
        usb_camera_device->sgCallback->pixels,
        usb_camera_device->base.device_ratio.width,
        usb_camera_device->base.device_ratio.height,
        usb_camera_device->base.device_ratio.bpp, data, format, data_len,
        usb_camera_device->base.is_mirror, TRUE);

    usb_camera_device->sgCallback->newFrame = false;

    LeaveCriticalSection(&usb_camera_device->sgCallback->critSection);

    ResetEvent(usb_camera_device->sgCallback->hEvent);

    return RET_OK;
}
