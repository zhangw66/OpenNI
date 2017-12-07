/*
 ============================================================================
 Name        : libdepthview.h
 Author      : yfhong@sunnyoptical.com
 Version     : 2017年9月20日 上午9:14:05
 Copyright   : sunnyoptical.com
 Description :
 History     :
 ============================================================================
 */
#ifndef LIBTOF_H
#define LIBTOF_H

#ifdef _MSC_VER
/* on MS environments, the inline keyword is available in C++ only */
#if !defined(__cplusplus)
#define inline __inline
#endif
/* ssize_t is also not available (copy/paste from MinGW) */
#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
#undef ssize_t
#ifdef _WIN64
  typedef __int64 ssize_t;
#else
  typedef int ssize_t;
#endif /* _WIN64 */
#endif /* _SSIZE_T_DEFINED */
#endif /* _MSC_VER */

/* stdint.h is not available on older MSVC */
#if defined(_MSC_VER) && (_MSC_VER < 1600) && (!defined(_STDINT)) && (!defined(_STDINT_H))
typedef unsigned __int8   uint8_t;
typedef unsigned __int16  uint16_t;
typedef unsigned __int32  uint32_t;
typedef unsigned __int64  uint64_t;
#else
#include <stdint.h>
#endif

#if !defined(_WIN32_WCE)
#include <sys/types.h>
#endif

#if defined(__linux) || defined(__APPLE__) || defined(__CYGWIN__) || defined(__HAIKU__)
#include <sys/time.h>
#endif

#include <time.h>
#include <limits.h>
// #include <libusb.h>
#include "libusb.h"
#include <stdio.h>
#include <string>
using namespace std;


/* 'interface' might be defined as a macro on Windows, so we need to
* undefine it so as not to break the current libusb API, because
* libusb_config_descriptor has an 'interface' member
* As this can be problematic if you include windows.h after libusb.h
* in your sources, we force windows.h to be included first. */
#if defined(_WIN32) || defined(__CYGWIN__) || defined(_WIN32_WCE)
// #include <windows.h>
#if defined(interface)
#undef interface
#endif
#if !defined(__CYGWIN__)
// #include <winsock.h>
#endif
#endif

/** \def LibTOF_CALL*/
#if defined(_WIN32) || defined(__CYGWIN__) || defined(_WIN32_WCE)
#define LIBTOF_CALL WINAPI
#else
#define LIBTOF_CALL
#endif

#ifndef NULL
#define NULL 0
#endif

/* Internally, LIBDEPTH_API_VERSION is defined as follows:
* (major << 24) | ( minor << 16) | (16 bit incremental)
*/
#define LIBTOF_API_VERSION 0x00000001
typedef int BOOL;

typedef enum tag_ErrorCode_TOF
{
    /** Success (no error) */
    LTOF_SUCCESS = 0,
    /** Input/output error */
    LTOF_ERROR_IO = -1,
    /** Invalid parameter */
    LTOF_ERROR_INVALID_PARAM  = -2,
    /** Access denied (insufficient permissions) */
    LTOF_ERROR_ACCESS = -3,
    /** No such device (it may have been disconnected) */
    LTOF_ERROR_NO_DEVICE  =  -4,
    /** Operation timed out */
    LTOF_ERROR_TIMEOUT = -5,
    /** Overflow */
    LTOF_ERROR_OVERFLOW = -6,
    /** Insufficient memory */
    LTOF_ERROR_NO_MEM = -7,
    /** Operation not supported or unimplemented on this platform */
    LTOF_ERROR_NOT_SUPPORTED = -8,
    LDV_ERROR_USB_WRITE = -9,
    LDV_ERROR_USB_READ = -10,

    /** Other error */
    LTOF_ERROR_OTHER = -99,

}TOF_ErrorCode_t;
    typedef struct tag_FRAMEDATA
    {
        float        x;
        float        y;
        float        z;
    }FrameData_t;
    typedef struct tag_FRAMEDATA_RGB
    {
        unsigned char        r;
        unsigned char        g;
        unsigned char        b;
    }FrameDataRgb_t;
typedef struct tag_DeviceInfo
{
    uint32_t  DeviceVersion;
    uint32_t  DeviceType;
    uint16_t  DepthFrameWidth;
    uint16_t  DepthFrameHeight;
    uint16_t  BitsPerPoint;
    uint16_t  VisibleFrameWidth;
    uint16_t  VisibleFrameHeight;
    uint16_t  BitsPerPixel;
    uint16_t  BlockSizeIn;
    uint16_t BlockSizeOut;
    char    TofAlgVersion[20];
    char    DeviceId[19];
    uint8_t RgbdEn;
}DeviceInfo_t;
typedef enum CameraControlCommand
{
    /** Set the working mode of TOF camera,0：Mono-Frequency,1：Multi-Frequencies 2：GREY_SCALE */
    SET_CAMERA_MODE = (1<<16) |17,
    /** Command: Setting the frame rate */
    SET_CAMERA_FPS = (1<<16) |18,
    /** Command: Set the exposure time */
    SET_CAMERA_EXP = (1<<16) |19,
    /** Command: Set grey data  exposure time,range: 1-2000 */
    SET_GREY_EXP = 26,
}CameraControlCommand_e;

typedef enum SceneMode
{
    Scene_No  = 0,
    Scene_30cm  = 1,
    /** */
    Scene_75cm  = 2, 
}SceneMode_e;

typedef enum CameraUseCase
{
    /** Indoor room reconstruction*/
    MODE_1_5FPS_1200 = 0,
    /** Room scanning  indoor navigation*/
    MODE_2_10FPS_650,
    /** 3D object reconstruction*/
    MODE_3_15FPS_850,
    /** Medium size object recognition,face reconstruction*/
    MODE_4_30FPS_380, 
    /** Remote collaboration,step by step instruction,table-top gaming*/
    MODE_5_45FPS_250,
}CameraUseCase_e;

typedef struct{ 
    int  version[3];         /*reserve byte*/ 
    int cameraNum;      /*camera number*/ 
    int width;                   /*image width*/ 
    int height;                  /*image height*/ 
    int  bpp;                      /*bpp  bits per pixel*/ 
    int dataFormat;       /*data format*/ 
    int frameCount; 
    int getFrameTime; 
    int sendFrameTime; 
    int headerLength; 
}S_MetaData;

typedef enum ExpourseMode
{
    Manual = 1, 
    Auto = 2,
}ExpourseMode_e;

enum{
    MARS03_TOF = 1,
    MARS03_TOF_RGB,    //have rgb sensor
    MAR04_TOF,
    MARS04_TOF_RGB
};
typedef struct tag_IMU_data
{
	uint64_t ACC_timeStamp;
	float ACC_x;
	float ACC_y;
	float ACC_z;
	uint64_t GYR_timeStamp;
	float GYR_x;
	float GYR_y;
	float GYR_z;
}IMU_data_t;

typedef struct tag_CmdBuffer
{
    uint16_t cmdlen;
    uint16_t size;
    unsigned char * buffer;
}CmdBuffer_t;

#define ROUND_UP(x, align) (x+(align-1))&(~(align-1))

class CLibTof
{
public:
    CLibTof();
    ~CLibTof();
    
public:
    TOF_ErrorCode_t  LibTOF_Connect();
    TOF_ErrorCode_t  LibTOF_DisConnect();
    TOF_ErrorCode_t  LibTOF_DepthCommand(CmdBuffer_t *CmdBuffer,CmdBuffer_t *RecvBuffer);
    TOF_ErrorCode_t  LibTOF_RcvDepthFrame2(FrameData_t* frame_data,FrameDataRgb_t* frame_data_Rgb, int width, int height,IMU_data_t* IMU_data=NULL);
    TOF_ErrorCode_t  LibTOF_RcvVideoFrame(short* video_data,unsigned int frame_len);
    TOF_ErrorCode_t  LibTOF_GetTemperture(float* Cpu_t, float* Sensor_t);
    TOF_ErrorCode_t  LibTOF_PseudoColor(FrameData_t* buffer,int width ,int height, unsigned char *Colorbuffer,float depth_min,float depth_max);
    TOF_ErrorCode_t  LibTOF_GetDeviceInfo(DeviceInfo_t *deviceinfo);
    TOF_ErrorCode_t  LibTOF_UpdataFalsh();
    TOF_ErrorCode_t  LibTOF_SetUseCase(CameraUseCase_e UseCase);
    TOF_ErrorCode_t  LibTOF_SetSceneMode(SceneMode_e SceneMode);
    TOF_ErrorCode_t  LibTOF_SetExpourseMode(ExpourseMode_e ExpMode);
    TOF_ErrorCode_t  LibTOF_RgbdEnSet(int RgbdEn);
	int  LibTOF_Setparam(CameraControlCommand_e Cmd, int value);

private:
    float *drictx;
    float *dricty;
    float *drictz;
    libusb_device_handle* mUsbHandle; /* opened device handle */
    libusb_context* mCtx;
    libusb_device** mDevs;

    int cur_scene;
    bool IS_running;
    float *gpLensparam;
    int cur_scence;

    int RgbdEnable;

    int camera_lastMode;  //指记录最后一次模式单双频  0 单频  1 双频
    int camera_mode;      //记录灰度和rgbd模式的变量 0 rgbd  2 灰度
    bool camera_mode2;    //灰度模式下切换单双频变量  false 没有切换  

    char serial_name[254];
    int  RcvDateFiledCount;
    int last_usecase;
    bool m_RecvDepthPause;
    bool m_RecvRgbPause;

};

#endif // LIBTOF_H
