/*****************************************************************************
*                                                                            *
*  OpenNI 1.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "NeolixImage.h"


#define SUPPORTED_X_RES 640
#define SUPPORTED_Y_RES 480
#define SUPPORTED_FPS 30
#define MAX_DEPTH_VALUE	15000  //z resolution unit:mm

NeolixImage::NeolixImage() : 
	m_bGenerating(FALSE),
	m_bDataAvailable(FALSE),
	m_pImageMap(NULL),
	m_nFrameID(0),
	m_nTimestamp(0),
	m_hScheduler(NULL),
	m_bMirror(FALSE)
{
}

NeolixImage::~NeolixImage()
{
#ifdef USE_TY_SDK
	if (hasOpen)
	{
		ASSERT_OK(TYStopCapture(hDevice));
		ASSERT_OK(TYCloseDevice(hDevice));
		ASSERT_OK(TYDeinitLib());
	}
	if (pVer != NULL) delete pVer;
	if (pDeviceBaseInfo != NULL) delete pDeviceBaseInfo;
	if (frameBuffer[0] != NULL) delete[] frameBuffer[0];
	if (frameBuffer[1] != NULL) delete[] frameBuffer[1];
	hasOpen = false;
#endif
	delete[] m_pImageMap;
}

XnStatus NeolixImage::Init()
{
	m_pImageMap = new XnUInt8[SUPPORTED_X_RES * SUPPORTED_Y_RES * sizeof(XnRGB24Pixel)];
	if (m_pImageMap == NULL)
	{
		return XN_STATUS_ALLOC_FAILED;
	}
	
#ifdef USE_TY_SDK
	componentIDs = TY_COMPONENT_DEPTH_CAM;
	//CHECK(TYInitLib());//初始化API
	ASSERT_OK(TYInitLib());
	//获取API版本
	this->pVer = new TY_VERSION_INFO;
	ASSERT_OK(TYLibVersion(pVer));
	LOGD("     - lib version: %d.%d.%d", pVer->major, pVer->minor, pVer->patch);

	//获取设备数量
	ASSERT_OK(TYGetDeviceNumber(&deviceNumber));
	LOGD("     - device number %d", deviceNumber);

	//获得设备基本信息
	this->pDeviceBaseInfo = new TY_DEVICE_BASE_INFO[100];
	ASSERT_OK(TYGetDeviceList(pDeviceBaseInfo, 100, &deviceNumber));
	if (deviceNumber == 0)
		return XN_STATUS_ERROR;

	//打开设备
	ASSERT_OK(TYOpenDevice(pDeviceBaseInfo[0].id, &hDevice));

	TY_FEATURE_INFO info;
	TY_STATUS ty_status;
	int32_t frameSize;

	ASSERT_OK(TYEnableComponents(hDevice, componentIDs));
	//设置摄像头参数
	//TYSetEnum(hDevice,componentIDs,enum_model,enum_model_value);
	TYSetEnum(hDevice, TY_COMPONENT_RGB_CAM_LEFT, TY_ENUM_IMAGE_MODE, TY_IMAGE_MODE_1280x960);
	TYSetEnum(hDevice, TY_COMPONENT_DEPTH_CAM, TY_ENUM_IMAGE_MODE, TY_IMAGE_MODE_640x480);

// 	if (frameBuffer[0] != nullptr) delete[] frameBuffer[0];
// 	if (frameBuffer[1] != nullptr) delete[] frameBuffer[1];

	//获得一帧数据的大小
	TYGetFrameBufferSize(hDevice, &frameSize);
	frameBuffer[0] = new char[frameSize];
	frameBuffer[1] = new char[frameSize];
	//===Enqueue buffer =====
	TYEnqueueBuffer(hDevice, frameBuffer[0], frameSize);
	TYEnqueueBuffer(hDevice, frameBuffer[1], frameSize);
	//=====disable trigger model====
	ty_status = TYGetFeatureInfo(hDevice, TY_COMPONENT_DEVICE, TY_BOOL_TRIGGER_MODE, &info);
	if ((info.accessMode & TY_ACCESS_WRITABLE) && (ty_status == TY_STATUS_OK)) {
		ASSERT_OK(TYSetBool(hDevice, TY_COMPONENT_DEVICE, TY_BOOL_TRIGGER_MODE, false));
	}

	//=====start capture====
	TYStartCapture(hDevice);

	hasOpen = true;
#endif
	return (XN_STATUS_OK);
}

XnBool NeolixImage::IsCapabilitySupported( const XnChar* strCapabilityName )
{
	// we only support the mirror capability
	return (strcmp(strCapabilityName, XN_CAPABILITY_MIRROR) == 0);
}

XnStatus NeolixImage::StartGenerating()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	m_bGenerating = TRUE;

	// start scheduler thread
	nRetVal = xnOSCreateThread(SchedulerThread, this, &m_hScheduler);
	if (nRetVal != XN_STATUS_OK)
	{
		m_bGenerating = FALSE;
		return (nRetVal);
	}

	m_generatingEvent.Raise();

	return (XN_STATUS_OK);
}

XnBool NeolixImage::IsGenerating()
{
	return m_bGenerating;
}

void NeolixImage::StopGenerating()
{
	m_bGenerating = FALSE;

	// wait for thread to exit
	xnOSWaitForThreadExit(m_hScheduler, 100);

	m_generatingEvent.Raise();
}

XnStatus NeolixImage::RegisterToGenerationRunningChange( XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback )
{
	return m_generatingEvent.Register(handler, pCookie, hCallback);
}

void NeolixImage::UnregisterFromGenerationRunningChange( XnCallbackHandle hCallback )
{
	m_generatingEvent.Unregister(hCallback);
}

XnStatus NeolixImage::RegisterToNewDataAvailable( XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback )
{
	return m_dataAvailableEvent.Register(handler, pCookie, hCallback);
}

void NeolixImage::UnregisterFromNewDataAvailable( XnCallbackHandle hCallback )
{
	m_dataAvailableEvent.Unregister(hCallback);
}

XnBool NeolixImage::IsNewDataAvailable( XnUInt64& nTimestamp )
{
	// return next timestamp
	nTimestamp = 1000000 / SUPPORTED_FPS;
	return m_bDataAvailable;
}

XnStatus NeolixImage::UpdateData()
{
	XnUInt8* pPixel = m_pImageMap;
#ifdef USE_TY_SDK
	int err = TYFetchFrame(hDevice, &frame, -1);
	for (int i = 0; i < frame.validCount; i++)
	{
		// get depth image
		if (frame.image[i].componentID == TY_COMPONENT_DEPTH_CAM)
		{
			memcpy(m_pImageMap, frame.image[i].buffer, sizeof(XnUInt8)*SUPPORTED_X_RES*SUPPORTED_Y_RES);
			break;
		}
	}

	TYEnqueueBuffer(hDevice, frame.userBuffer, frame.bufferSize);
#endif
	//for test 
	
#if 0
	static XnUInt8 color = 0;
	color++;
	xnOSMemSet(m_pImageMap, color, SUPPORTED_X_RES * SUPPORTED_Y_RES * sizeof(XnRGB24Pixel));
	// change our internal data, so that pixels go from frameID incrementally in both axes.
	for (XnUInt y = 0; y < SUPPORTED_Y_RES; ++y)
	{
		for (XnUInt x = 0; x < SUPPORTED_X_RES; ++x, ++pPixel)
		{
			*pPixel = (m_nFrameID + x + y) % MAX_DEPTH_VALUE;
		}
	}
#endif
	const RGBImage& image = webcam.frame();
	memcpy(m_pImageMap, image.data, sizeof(XnRGB24Pixel)*SUPPORTED_X_RES*SUPPORTED_Y_RES);
	// if needed, mirror the map
	if (m_bMirror)
	//if (false)
	{
		XnUInt8 temp;

		for (XnUInt y = 0; y < SUPPORTED_Y_RES; ++y)
		{
			XnUInt8* pUp = &m_pImageMap[y * SUPPORTED_X_RES];
			XnUInt8* pDown = &m_pImageMap[(y+1) * SUPPORTED_X_RES - 1];

			for (XnUInt x = 0; x < SUPPORTED_X_RES/2; ++x, ++pUp, --pDown)
			{
				temp = *pUp;
				*pUp = *pDown;
				*pDown = temp;
			}
		}
	}
	int cmp_len = 0;
	cmp_len = memcmp(m_pImageMap, image.data, sizeof(XnRGB24Pixel)*SUPPORTED_X_RES*SUPPORTED_Y_RES);
	if (cmp_len != 0) 
		printf("mem is diff:%d\n", cmp_len);
	 else 
		printf("mem is same\n");
	
	m_nFrameID++;
	m_nTimestamp += 1000000 / SUPPORTED_FPS;

	// mark that data is old
	m_bDataAvailable = FALSE;
	
	return (XN_STATUS_OK);
}


const void* NeolixImage::GetData()
{
	return m_pImageMap;
}

XnUInt32 NeolixImage::GetDataSize()
{
	return (SUPPORTED_X_RES * SUPPORTED_Y_RES * sizeof(XnUInt8));
}

XnUInt64 NeolixImage::GetTimestamp()
{
	return m_nTimestamp;
}

XnUInt32 NeolixImage::GetFrameID()
{
	return m_nFrameID;
}

xn::ModuleMirrorInterface* NeolixImage::GetMirrorInterface()
{
	return this;
}

XnStatus NeolixImage::SetMirror( XnBool bMirror )
{
	m_bMirror = bMirror;
	m_mirrorEvent.Raise();
	printf("%s:mirroe:%d\n", __func__, bMirror);
	return (XN_STATUS_OK);
}

XnBool NeolixImage::IsMirrored()
{
	return m_bMirror;
}

XnStatus NeolixImage::RegisterToMirrorChange( XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback )
{
	return m_mirrorEvent.Register(handler, pCookie, hCallback);
}

void NeolixImage::UnregisterFromMirrorChange( XnCallbackHandle hCallback )
{
	m_mirrorEvent.Unregister(hCallback);
}

XnUInt32 NeolixImage::GetSupportedMapOutputModesCount()
{
	// we only support a single mode
	return 1;
}

XnStatus NeolixImage::GetSupportedMapOutputModes( XnMapOutputMode aModes[], XnUInt32& nCount )
{
	if (nCount < 1)
	{
		return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
	}

	aModes[0].nXRes = SUPPORTED_X_RES;
	aModes[0].nYRes = SUPPORTED_Y_RES;
	aModes[0].nFPS = SUPPORTED_FPS;

	return (XN_STATUS_OK);
}

XnStatus NeolixImage::SetMapOutputMode( const XnMapOutputMode& Mode )
{
	// make sure this is our supported mode
	if (Mode.nXRes != SUPPORTED_X_RES ||
		Mode.nYRes != SUPPORTED_Y_RES ||
		Mode.nFPS != SUPPORTED_FPS)
	{
		return (XN_STATUS_BAD_PARAM);
	}

	return (XN_STATUS_OK);
}

XnStatus NeolixImage::GetMapOutputMode( XnMapOutputMode& Mode )
{
	Mode.nXRes = SUPPORTED_X_RES;
	Mode.nYRes = SUPPORTED_Y_RES;
	Mode.nFPS = SUPPORTED_FPS;

	return (XN_STATUS_OK);
}

XnStatus NeolixImage::RegisterToMapOutputModeChange( XnModuleStateChangedHandler /*handler*/, void* /*pCookie*/, XnCallbackHandle& hCallback )
{
	// no need. we only allow one mode
	hCallback = this;
	return XN_STATUS_OK;
}

void NeolixImage::UnregisterFromMapOutputModeChange( XnCallbackHandle /*hCallback*/ )
{
	// do nothing (we didn't really register)	
}

XnUInt8* NeolixImage::GetImageMap()
{
	return m_pImageMap;
}

XnBool NeolixImage::IsPixelFormatSupported(XnPixelFormat Format)
{
	//we only support rgb24
	if (Format != XN_PIXEL_FORMAT_RGB24)
		return false;
	else 
		return true;
}
XnStatus NeolixImage::SetPixelFormat(XnPixelFormat Format)
{

	return XN_STATUS_OK;
}
XnPixelFormat NeolixImage::GetPixelFormat()
{
	return XN_PIXEL_FORMAT_RGB24;
}
XnStatus NeolixImage::RegisterToPixelFormatChange(XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback)
{
	// no need. it never changes
	hCallback = this;
	return XN_STATUS_OK;

}
void NeolixImage::UnregisterFromPixelFormatChange(XnCallbackHandle hCallback)
{
	// do nothing (we didn't really register)	

}
#if 0
XnUInt8 NeolixImage::GetDeviceMaxImage()
{
	return MAX_DEPTH_VALUE;
}
void NeolixImage::GetFieldOfView( XnFieldOfView& FOV )
{
	// some numbers
	FOV.fHFOV = 1.35;
	FOV.fVFOV = 1.35;
}

XnStatus NeolixImage::RegisterToFieldOfViewChange( XnModuleStateChangedHandler /*handler*/, void* /*pCookie*/, XnCallbackHandle& hCallback )
{
	// no need. it never changes
	hCallback = this;
	return XN_STATUS_OK;
}

void NeolixImage::UnregisterFromFieldOfViewChange( XnCallbackHandle /*hCallback*/ )
{
	// do nothing (we didn't really register)	
}

#endif



XN_THREAD_PROC NeolixImage::SchedulerThread( void* pCookie )
{
	NeolixImage* pThis = (NeolixImage*)pCookie;

	while (pThis->m_bGenerating)
	{
		// wait 33 ms (to produce 30 FPS)
		xnOSSleep(1000000/SUPPORTED_FPS/1000);

		pThis->OnNewFrame();
	}

	XN_THREAD_PROC_RETURN(0);
}

void NeolixImage::OnNewFrame()
{
	m_bDataAvailable = TRUE;
	m_dataAvailableEvent.Raise();
}
