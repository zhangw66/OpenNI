/*****************************************************************************
*                                                                            *
*  Copyright (C) 2017 Letv.                                                  *
*                                                                            *
*  This is afree software: you can redistribute it and/or modify             *
*  it under the terms of the GNU Lesser General Public License as published  *
*  by the Free Software Foundation, either version 3 of the License, or      *
*  (at your option) any later version.                                       *
*                                                                            *
*  This is distributed in the hope that it will be useful,                   *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
*  GNU Lesser General Public License for more details.                       *
*                                                                            *
*  You should see a copy of the GNU Lesser General Public License			 *
*  in <http://www.gnu.org/licenses/>.										 *
*                                                                            *
*****************************************************************************/


#include <XnModuleCppInterface.h>
#include <XnEvent.h>
#include <XnOS.h>

#include <math.h>
#include <iostream>
#include <stdio.h>
//#include <windows.h>  //platform specific


#include "base.h"


#if TEST_CASE == TEST_HUMANPLUS
#include "MoveSenseCamera.h"
#include "CameraCtrl.h"
using namespace movesense;

#elif TEST_CASE == TEST_INUITIVE
#include "InuSensor.h"
#include "DepthStream.h"
#include "InuError.h"
using namespace InuDev;

#elif TEST_CASE == TEST_ADI
#include "types.h"
#include "ADI_TOF.h"

#elif TEST_CASE == TEST_ESPDI
#include "eSPDI.h"

#elif TEST_CASE == TEST_SUNNYMARS
#include "libTof.h"

#elif TEST_CASE == TEST_TY
#include "common.hpp"

#endif
#define ROUNDUP(x,n) ((x+n-1)&(~(n-1)))

//---------------------------------------------------------------------------
// Depth Specific Properties
//---------------------------------------------------------------------------
/** Integer */ 
#define XN_STREAM_PROPERTY_GAIN						"Gain"
/** Integer */ 
#define XN_STREAM_PROPERTY_MIN_DEPTH				"MinDepthValue"
/** Integer */ 
#define XN_STREAM_PROPERTY_MAX_DEPTH				"MaxDepthValue"

class DepthLetv : 
	public virtual xn::ModuleDepthGenerator,
	public virtual xn::ModuleMirrorInterface
	//public virtual xn::ModuleCroppingInterface
	//public virtual xn::ModuleProductionNode
{
public:
	DepthLetv();
	virtual ~DepthLetv();

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

	// DepthGenerator methods
	virtual XnDepthPixel* GetDepthMap();
	virtual XnDepthPixel GetDeviceMaxDepth();
	virtual void GetFieldOfView(XnFieldOfView& FOV);
	virtual XnStatus RegisterToFieldOfViewChange(XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback);
	virtual void UnregisterFromFieldOfViewChange(XnCallbackHandle hCallback);

	// Fun珲es que retornam as propriedades da c鈓era
	virtual XnStatus GetIntProperty(const XnChar* strName, XnUInt64& nValue) const;
	virtual XnStatus GetRealProperty(const XnChar* strName, XnDouble& dValue) const;
	virtual XnStatus GetStringProperty(const XnChar* strName, XnChar* csValue, XnUInt32 nBufSize) const;
	virtual XnStatus GetGeneralProperty(const XnChar* strName, XnUInt32 nBufferSize, void* pBuffer) const;

	//virtual void DisparityToDepth(const unsigned char* disparity, XnUInt32 rowNumber, XnUInt32 colNumber, XnDepthPixel* pDepth);

private:

#if TEST_CASE == TEST_HUMANPLUS
	// HumanPlus define
	movesense::MoveSenseCamera *pCam;
#elif TEST_CASE == TEST_INUITIVE
	// Inuitive define
	std::shared_ptr<InuDev::CInuSensor>  m_device_Inu;
	std::shared_ptr<InuDev::CDepthStream>  m_depthStream_Inu;
	InuDev::CImageFrame m_depthFrame_Inu;
#elif TEST_CASE == TEST_ADI
	ADI_TOF m_ADI_obj;
#elif TEST_CASE == TEST_ESPDI
	void*				m_hEtronDI;
	DEVSELINFO			m_DevSelInfo;
	DEVINFORMATION		*m_pDevInfo;
	int					m_EtronDevNum;
	int					m_EtronStreamMode;
	int					m_flag;
	unsigned short		m_RegAddr;
	unsigned short		m_Value;
	unsigned char		*m_pDepthImgBuf;
	unsigned long		m_depthSize;
	int					m_DepthSerialNumber;
#elif TEST_CASE == TEST_SUNNYMARS
	int rv;
	int DEPTHMAP_W;
	int DEPTHMAP_H;
	int DEPTHVIDEO_W;
	int DEPTHVIDEO_H;
	int DEPTHVIDEO_FRAME_SIZE;
	FrameData_t* frame_data;
	short* grey_data;
	FrameDataRgb_t* frame_data_Rgb;
	unsigned char	*PColorbuffer_s;
	int Camera_Mode;  //默认为双频模式
#elif TEST_CASE == TEST_TY
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
#endif

	XnUInt32 width;
	XnUInt32 height;
	unsigned char * imgData;
	int imgLen;
	XnDepthPixel * depthImgData;

	XN_DECLARE_EVENT_0ARG(ChangeEvent, ChangeEventInterface);

	static XN_THREAD_PROC SchedulerThread(void* pCookie);
	void OnNewFrame();

	XnBool m_bGenerating;
	XnBool m_bDataAvailable;
	XnDepthPixel* m_pDepthMap;
	XnUInt32 m_nFrameID;
	XnUInt64 m_nTimestamp;
	XN_THREAD_HANDLE m_hScheduler;
	XnBool m_bMirror;
	ChangeEvent m_generatingEvent;
	ChangeEvent m_dataAvailableEvent;
	ChangeEvent m_mirrorEvent;
	
	// Cropping
	/*XnGeneralBuffer crop;
	XnCropping *crop2;*/
};
