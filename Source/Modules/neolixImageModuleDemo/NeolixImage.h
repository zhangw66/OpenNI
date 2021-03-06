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
#include <XnModuleCppInterface.h>
#include <XnEventT.h>
//#include <XnOS.h>
#include <iostream>
#include "webcam.h"
using namespace std;
class NeolixImage : 
	//public virtual xn::ModuleImageGenerator,
	public virtual xn::ModuleImageGenerator,
	public virtual xn::ModuleMirrorInterface
{
public:
	NeolixImage();
	virtual ~NeolixImage();

	XnStatus Init();

	// ProductionNode methods
	virtual XnBool IsCapabilitySupported(const XnChar* strCapabilityName);

	// Generator methods
	virtual XnStatus StartGenerating();
	virtual XnBool IsGenerating();
	virtual void StopGenerating();
	virtual XnStatus RegisterToGenerationRunningChange(XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback);
	virtual void UnregisterFromGenerationRunningChange(XnCallbackHandle hCallback);
	virtual XnStatus RegisterToNewDataAvailable(XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback);
	virtual void UnregisterFromNewDataAvailable(XnCallbackHandle hCallback);
	virtual XnBool IsNewDataAvailable(XnUInt64& nTimestamp);
	virtual XnStatus UpdateData();
	virtual const void* GetData();
	virtual XnUInt32 GetDataSize();
	virtual XnUInt64 GetTimestamp();
	virtual XnUInt32 GetFrameID();
	virtual xn::ModuleMirrorInterface* GetMirrorInterface();

	// Mirror methods
	virtual XnStatus SetMirror(XnBool bMirror);
	virtual XnBool IsMirrored();
	virtual XnStatus RegisterToMirrorChange(XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback);
	virtual void UnregisterFromMirrorChange(XnCallbackHandle hCallback);

	// MapGenerator methods
	virtual XnUInt32 GetSupportedMapOutputModesCount();
	virtual XnStatus GetSupportedMapOutputModes(XnMapOutputMode aModes[], XnUInt32& nCount);
	virtual XnStatus SetMapOutputMode(const XnMapOutputMode& Mode);
	virtual XnStatus GetMapOutputMode(XnMapOutputMode& Mode);
	virtual XnStatus RegisterToMapOutputModeChange(XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback);
	virtual void UnregisterFromMapOutputModeChange(XnCallbackHandle hCallback);

	// ImageGenerator methods
#if 0
	virtual XnImagePixel* GetImageMap();
	virtual XnImagePixel GetDeviceMaxImage();
	virtual void GetFieldOfView(XnFieldOfView& FOV);
	virtual XnStatus RegisterToFieldOfViewChange(XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback);
	virtual void UnregisterFromFieldOfViewChange(XnCallbackHandle hCallback);
#endif
	virtual XnUInt8* GetImageMap();
	virtual XnBool IsPixelFormatSupported(XnPixelFormat Format);
	virtual XnStatus SetPixelFormat(XnPixelFormat Format);
	virtual XnPixelFormat GetPixelFormat();
	virtual XnStatus RegisterToPixelFormatChange(XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback);
	virtual void UnregisterFromPixelFormatChange(XnCallbackHandle hCallback);

private:
#ifdef USE_TY_SDK
	//TY data
	int deviceNumber;//设备数量
	TY_VERSION_INFO * pVer;//api版本
	TY_DEVICE_BASE_INFO* pDeviceBaseInfo;//设备的基础信息
	TY_DEV_HANDLE hDevice;//设备的句柄
	int32_t componentIDs;//组件IDs
	int32_t enum_model;
	int32_t enum_model_value;
	bool printLog;
	std::string logFile;
	bool hasOpen;
	char* frameBuffer[2];
	TY_FRAME_DATA frame;
    //TY data end
#endif
	//webcam device
	Webcam webcam;
	static XN_THREAD_PROC SchedulerThread(void* pCookie);
	void OnNewFrame();
	XnBool m_bGenerating;
	XnBool m_bDataAvailable;
	XnUInt8* m_pImageMap;
	XnUInt32 m_nFrameID;
	XnUInt64 m_nTimestamp;
	XN_THREAD_HANDLE m_hScheduler;
	XnBool m_bMirror;
	XnEventNoArgs m_generatingEvent;
	XnEventNoArgs m_dataAvailableEvent;
	XnEventNoArgs m_mirrorEvent;
};
