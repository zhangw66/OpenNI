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

#include <iostream>
#include <stdio.h>
#include <XnLog.h>

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "DepthLetv.h"

using namespace std;

DepthLetv::DepthLetv() : 
	m_bGenerating(FALSE),
	m_bDataAvailable(FALSE),
	m_pDepthMap(NULL),
	m_nFrameID(0),
	m_nTimestamp(0),
	m_hScheduler(NULL),
	m_bMirror(FALSE)
{
	
}
#define nullptr NULL
typedef unsigned char BYTE;
DepthLetv::~DepthLetv()
{
	printf("DepthLetv::~DepthLetv() \n");
	delete[] m_pDepthMap;

#if TEST_CASE == 0
	pCam->CloseCamera();
	if (pCam)
		delete pCam;

	if (imgData)
		delete imgData;
#elif TEST_CASE == 1
	// Stop and terminate depth streams 
	CInuError ret = eOK;
	if (m_depthStream_Inu != nullptr)
	{
		ret = m_depthStream_Inu->Stop();
		if (ret != InuDev::eOK)
		{
			cout << "Failed to stop depth stream: " << string(ret) << endl;
		}

		ret = m_depthStream_Inu->Terminate();
		if (ret != InuDev::eOK)
		{
			cout << "Failed to finalize depth stream: " << string(ret) << endl;
		}
		m_depthStream_Inu = nullptr;
	}

	// Stop and terminate InuSensor
	if (m_device_Inu != nullptr)
	{
		ret = m_device_Inu->Stop();
		if (ret != InuDev::eOK)
		{
			cout << "Failed to stop Sensor: " << string(ret) << endl;
		}

		ret = m_device_Inu->Terminate();
		if (ret != InuDev::eOK)
		{
			cout << "Failed to finalize Sensor: " << string(ret) << endl;
		}

		m_device_Inu = nullptr;
	}
#elif TEST_CASE == 2
	m_ADI_obj.Release();
#elif TEST_CASE == 3
	m_Value = 0;
	EtronDI_SetFWRegister(m_hEtronDI, &m_DevSelInfo, m_RegAddr, m_Value, m_flag);

	EtronDI_CloseDevice(m_hEtronDI, &m_DevSelInfo);
	EtronDI_Release(&m_hEtronDI);
	delete[] m_pDevInfo;
	delete[] m_pDepthImgBuf;
#elif TEST_CASE == 5
	LibTOF_DisConnect();
	if (frame_data != NULL)
	{
		free(frame_data);
	}

	if (frame_data_Rgb != NULL)
	{
		free(frame_data_Rgb);
	}
	if (PColorbuffer_s != NULL)
	{
		free(PColorbuffer_s);
	}
#elif TEST_CASE == TEST_TY
	if (hasOpen)
	{
		ASSERT_OK(TYStopCapture(hDevice));
		ASSERT_OK(TYCloseDevice(hDevice));
		ASSERT_OK(TYDeinitLib());
	}
	if (pVer != nullptr) delete pVer;
	if (pDeviceBaseInfo != nullptr) delete pDeviceBaseInfo;
	if (frameBuffer[0] != nullptr) delete[] frameBuffer[0];
	if (frameBuffer[1] != nullptr) delete[] frameBuffer[1];
	hasOpen = false;

#endif
}

XnStatus DepthLetv::Init()
{
	printf("DepthLetv::Init() \n");

	// Aloca o mapa de profundidade
	m_pDepthMap = new XnDepthPixel[SUPPORTED_X_RES * SUPPORTED_Y_RES];

	if (m_pDepthMap == NULL)
	{
		return XN_STATUS_ALLOC_FAILED;
	}

#if TEST_CASE == 0
	// Init HumanPlus
	movesense::CameraMode sel = CAM_STEREO_752X480_LD_30FPS;
	pCam = new movesense::MoveSenseCamera(sel);

	if (!(movesense::MS_SUCCESS == pCam->OpenCamera()))
	{
		printf("DepthLetv::Init(), depth camera open failed \n");
		return XN_STATUS_DEVICE_NOT_CONNECTED;
	}
	pCam->SetUndistort(false);

	width = SUPPORTED_X_RES;
	height = SUPPORTED_Y_RES;
	imgLen = width*height * 2;
	imgData = new unsigned char[imgLen];
#elif TEST_CASE == 1
	m_device_Inu = InuDev::CInuSensor::Create();

	// Define parameters for initialization
	InuDev::CSensorParams params;
	params.FPS = SUPPORTED_FPS;
	params.SensorRes = InuDev::ESensorResolution::eFull;

	// Initiate the sensor - it must be call before any access to the sensor. Sensor will start working in low power.
	InuDev::CInuError m_retCode = m_device_Inu->Init(params);
	if (m_retCode != InuDev::eOK)
	{
		std::cout << "Failed to connect to Inuitive Sensor. Error: " << std::hex << int(m_retCode) << " - " << std::string(m_retCode) << std::endl;
		return XN_STATUS_ERROR;
	}
	std::cout << "Connected to Sensor !......" << std::endl;

	// Start acquiring frames - it must be call before starting acquiring any type of frames (depth, video, head, etc.)
	m_retCode = m_device_Inu->Start();
	if (m_retCode != InuDev::eOK)
	{
		std::cout << "Failed to start to Inuitive Sensor." << std::endl;
		return XN_STATUS_ERROR;
	}
	std::cout << "Sensor is started !" << std::endl;

	// Generate a Depth stream object. This object provides depth frames. 
	m_depthStream_Inu = m_device_Inu->CreateDepthStream(); // m_depthStream->Init(InuDev::eAccurate/eFastMode/eDisabled); 
	if (m_depthStream_Inu == nullptr)
	{
		std::cout << "Unexpected error, failed to get Depth Stream" << std::endl;
		return XN_STATUS_ERROR;
	}

	// Configure Depth parameters and start the depth service on device
	m_retCode = m_depthStream_Inu->Init();
	if (m_retCode != InuDev::eOK)
	{
		std::cout << "Depth initiation error: " << std::hex << int(m_retCode) << " - " << std::string(m_retCode) << std::endl;
		return XN_STATUS_ERROR;
	}
	std::cout << "Depth Stream is initialized !" << std::endl;

	// Start depth frames acquisition (operation mode)
	m_retCode = m_depthStream_Inu->Start();

	if (m_retCode != InuDev::eOK)
	{
		std::cout << "Start error: " << std::hex << int(m_retCode) << " - " << std::string(m_retCode) << std::endl;
		return XN_STATUS_ERROR;
	}
	std::cout << "Depth frames acquisition started!" << std::endl;

	// get depth frame
	m_retCode = m_depthStream_Inu->GetFrame(m_depthFrame_Inu);
	if (m_retCode != eOK)
	{
		cout << "Failed to acquire depth frame: " << string(m_retCode) << endl;
		return XN_STATUS_ERROR;
	}

	if (m_depthFrame_Inu.Valid)
	{
// 		m_DepthWidth = m_depthFrame_Inu.Width();
// 		m_DepthHeight = m_depthFrame_Inu.Height();
	}
	else
	{
		cout << "depth frame isn't valid!" << endl;
		return eInitError;
	}
#elif TEST_CASE == 2
	//Initialize the camera
	int status = m_ADI_obj.Init(0, 0);
	if (status == -1)
	{
		cout << "ADI Init Failed\n";
		return -1;
	}

	//Set the Depth Range, For Playback select the depth range data was captured
	//obj.setDepthIRMode(false);
	m_ADI_obj.setDepthRange((range)XNEAR_RANGE, 0);
	uint16 d = m_ADI_obj.getDepthRange();
	cout << "Depth Range=" << d << endl;
	m_ADI_obj.setPulseCount(700);
	d = m_ADI_obj.getPulseCount();
	cout << "pulse=" << d << endl;

	m_ADI_obj.getCameraIntrinsic();
	// Filter
	m_ADI_obj.setFilter(FILT_SEL_GAUS5X5X2);
	m_ADI_obj.setTemporalFilter();
	m_ADI_obj.setThreshold(50);
#elif TEST_CASE == 3
	// Etron Init
	if (0 > EtronDI_Init(&m_hEtronDI, false)) 
	{
		return (XN_STATUS_ERROR);
	}

// 	// Etron Find Device
// 	EtronDI_FindDevice(m_hEtronDI);

	// Etron Get Device Num
	m_EtronDevNum = EtronDI_GetDeviceNumber(m_hEtronDI);
	if (m_EtronDevNum == 0)
		return (XN_STATUS_ERROR);

	m_pDevInfo = new DEVINFORMATION[m_EtronDevNum];

	DEVSELINFO DevSelInfo;

	for (int i = 0; i < m_EtronDevNum; i++)
	{
		DevSelInfo.index = i;
		EtronDI_GetDeviceInfo(m_hEtronDI, &DevSelInfo, m_pDevInfo + i);
		
		printf("DepthLetv::DevName(%s) \n", m_pDevInfo[i].strDevName);
	}

	m_DevSelInfo.index = 0;

// 	// Etron Select Device
// 	int iRet = EtronDI_SelectDevice(m_hEtronDI, 1);
// 	m_DevSelInfo.index = 0;
// 	if (iRet != ETronDI_OK){
// 		return (XN_STATUS_ERROR);
// 	}

	WORD DepthDataType = 2;
	if (EtronDI_SetDepthDataType(m_hEtronDI, &m_DevSelInfo, DepthDataType) != ETronDI_OK)
	{
		printf("EtronDI_SetDepthDataType Failed !!");
		return (XN_STATUS_ERROR);
	}		

 	// Etron Open Device
	if (EtronDI_OpenDevice2(m_hEtronDI, &m_DevSelInfo, 0, 0, false, SUPPORTED_X_RES, SUPPORTED_Y_RES) != ETronDI_OK)
		return (XN_STATUS_ERROR);

	// Get FW register (Addr:E0 - IR on/off)
	m_RegAddr = 224;
	m_Value = -1;
	m_flag = 0;
	m_flag |= FG_Address_2Byte;
	m_flag |= FG_Value_2Byte;
	EtronDI_GetFWRegister(m_hEtronDI, &m_DevSelInfo, m_RegAddr, &m_Value, m_flag);

	m_Value = 4;
	EtronDI_SetFWRegister(m_hEtronDI, &m_DevSelInfo, m_RegAddr, m_Value, m_flag);

	m_pDepthImgBuf = new unsigned char[SUPPORTED_X_RES* SUPPORTED_Y_RES * 2];
#elif TEST_CASE == 5
	//连接相机，初始化相机
	rv = LibTOF_Connect();
	if (rv < 0)
	{
		printf(" LibTOF_Connect Failed\n");
		system("pause");
		return XN_STATUS_ERROR;
	}

	//获取设备信息，图像宽高，设备版本信息
	DeviceInfo_t *deviceinfo = new DeviceInfo_t;//(DeviceInfo_t *)malloc(sizeof(DeviceInfo_t));
	memset(deviceinfo, 0, sizeof(DeviceInfo_t));
	rv = LibTOF_GetDeviceInfo(deviceinfo);
	if (rv < 0)
	{
		printf(" LibTOF_GetDeviceInfo Failed\n");
		system("pause");
		return XN_STATUS_ERROR;
	}
	else
	{
		DEPTHMAP_W = deviceinfo->DepthFrameWidth;
		DEPTHMAP_H = deviceinfo->DepthFrameHeight - 1;//减去一行头长度
		DEPTHVIDEO_W = deviceinfo->VisibleFrameWidth;
		DEPTHVIDEO_H = deviceinfo->VisibleFrameHeight;

		printf("deviceID:%s \ndeviceInfo:%s\nDepth Data:w-%d h-%d \nvisiableData:w-%d h-%d \nDeviceVersion:\n"
			, deviceinfo->DeviceId, deviceinfo->TofAlgVersion, DEPTHMAP_W, DEPTHMAP_H, DEPTHVIDEO_W, DEPTHVIDEO_H);

		DEPTHVIDEO_FRAME_SIZE = ROUNDUP(DEPTHVIDEO_W*DEPTHVIDEO_H * 3 / 2 + DEPTHVIDEO_W, 1024);//尾部带一行帧信息

		frame_data = (FrameData_t*)malloc(sizeof(FrameData_t)*DEPTHMAP_W*DEPTHMAP_H + DEPTHMAP_W);
		memset(frame_data, 0, sizeof(FrameData_t)*DEPTHMAP_W*DEPTHMAP_H + DEPTHMAP_W);

		frame_data_Rgb = (FrameDataRgb_t*)malloc(sizeof(FrameDataRgb_t)*DEPTHMAP_W*DEPTHMAP_H + DEPTHMAP_W);
		memset(frame_data_Rgb, 0, sizeof(FrameDataRgb_t)*DEPTHMAP_W*DEPTHMAP_H + DEPTHMAP_W);
	}
#elif TEST_CASE == TEST_TY
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


XnStatus DepthLetv::UpdateData()
{
	//printf("DepthLetv::UpdateData() \n");
	XnStatus nRetVal = XN_STATUS_OK;
		
	XnDepthPixel* pPixel = m_pDepthMap;

#if TEST_CASE == 0
	int ret = pCam->GetImageData(imgData, imgLen);
	if (ret > 0)
	{
		unsigned char* dispData = new unsigned char[width*height];
		for (int i = 0; i < height; i++)
		{
			//memcpy(left.data + width*i, img_data + (2 * i)*width, width);
			memcpy(dispData + width*i, imgData + (2 * i + 1)*width, width);
		}
		DisparityToDepth(dispData, height, width, m_pDepthMap);

		delete[] dispData;
	}
	else if (ret == -1)
	{
		std::cout << "time out" << std::endl;
	}
#elif TEST_CASE == 1
	// get depth frame buffer
	CInuError ret = m_depthStream_Inu->GetFrame(m_depthFrame_Inu);
	if (ret != eOK)
	{
		cout << "Failed to acquire depth frame: " << string(ret) << endl;
		return XN_STATUS_ERROR;
	}

	// depth data
	const InuDev::byte* pDepthRow = (const InuDev::byte*)m_depthFrame_Inu.GetData();
	int rowSize = m_depthFrame_Inu.Width() * m_depthFrame_Inu.BytesPerPixel();

	memcpy(m_pDepthMap, pDepthRow, sizeof(InuDev::byte)*m_depthFrame_Inu.BytesPerPixel()*SUPPORTED_X_RES*SUPPORTED_Y_RES);

#elif TEST_CASE == 2
	m_ADI_obj.ProcFrame(/*m_pDepthMap*/);
	memcpy(m_pDepthMap, m_ADI_obj.punDepth, sizeof(uint16)*SUPPORTED_X_RES*SUPPORTED_Y_RES);
#elif TEST_CASE == 3
	int nRet = EtronDI_GetImage(m_hEtronDI, &m_DevSelInfo, m_pDepthImgBuf, &m_depthSize, &m_DepthSerialNumber);

	memcpy(m_pDepthMap, m_pDepthImgBuf, sizeof(BYTE)*SUPPORTED_X_RES*SUPPORTED_Y_RES*2);
#elif TEST_CASE == 5
	FrameData_t* frame_data_p;
	FrameDataRgb_t* frame_data_Rgb_tmp;

	frame_data_p = frame_data;
	frame_data_Rgb_tmp = frame_data_Rgb;//MRAS04设备，灰度模式时，存放的是灰度数据，每个像素占2字节

	int rs = LibTOF_RcvDepthFrame2(frame_data_p, frame_data_Rgb_tmp, DEPTHMAP_W, DEPTHMAP_H);

	if (rs != LTOF_SUCCESS)
		return XN_STATUS_ERROR;

	// Analysis Data
	for (int y = 0; y < DEPTHMAP_H; y++)
	{
		for (int x = 0; x < DEPTHVIDEO_W; x++)
		{
			m_pDepthMap[y*DEPTHMAP_W + x] = (int)(frame_data_p[y*DEPTHMAP_W + x].z * 1000.0f);
		}
	}
#elif TEST_CASE == TEST_TY
	int err = TYFetchFrame(hDevice, &frame, -1);
	for (int i = 0; i < frame.validCount; i++)
	{
		// get depth image
		if (frame.image[i].componentID == TY_COMPONENT_DEPTH_CAM)
		{
			memcpy(m_pDepthMap, frame.image[i].buffer, sizeof(BYTE)*SUPPORTED_X_RES*SUPPORTED_Y_RES * 2);
			break;
		}
	}

	TYEnqueueBuffer(hDevice, frame.userBuffer, frame.bufferSize);

#endif

	// if needed, mirror the map
	if (0)
	{
		XnDepthPixel temp;

		for (XnUInt y = 0; y < SUPPORTED_Y_RES; ++y)
		{
			XnDepthPixel* pUp = &m_pDepthMap[y * SUPPORTED_X_RES];
			XnDepthPixel* pDown = &m_pDepthMap[(y+1) * SUPPORTED_X_RES - 1];

			for (XnUInt x = 0; x < SUPPORTED_X_RES/2; ++x, ++pUp, --pDown)
			{
				temp = *pUp;
				*pUp = *pDown;
				*pDown = temp;
			}
		}
	}

	// Incrementa o n鷐ero do frame, aplica珲es utilizam um get para obter este valor
	m_nFrameID++;

	// Incrementa o n鷐ero do Timestamp, aplica珲es utilizam um get para obter este valor
	m_nTimestamp += 1000000 / SUPPORTED_FPS;

	// Mark that data is old
	m_bDataAvailable = FALSE;
	
	return (XN_STATUS_OK);
}

XnBool DepthLetv::IsCapabilitySupported( const XnChar* strCapabilityName )
{

	XnBool result;

	// we only support the mirror capability
	result =  (strcmp(strCapabilityName, XN_CAPABILITY_MIRROR) == 0);


	/*result = (strcmp(strCapabilityName, XN_CAPABILITY_USER_POSITION) == 0 ||
		strcmp(strCapabilityName, XN_CAPABILITY_ALTERNATIVE_VIEW_POINT) == 0 ||
		strcmp(strCapabilityName, XN_CAPABILITY_FRAME_SYNC) == 0 ||
		strcmp(strCapabilityName, XN_CAPABILITY_CROPPING) == 0 );*/

	//result =  (strcmp(strCapabilityName, XN_CAPABILITY_MIRROR) == 0 || 
		//strcmp(strCapabilityName, XN_CAPABILITY_CROPPING) == 0 ); 

	 return result;
}

XnStatus DepthLetv::StartGenerating()
{
	printf("DepthLetv::StartGenerating() \n");
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

XnBool DepthLetv::IsGenerating()
{
	return m_bGenerating;
}

void DepthLetv::StopGenerating()
{
	printf("DepthLetv::StopGenerating() \n");
	m_bGenerating = FALSE;

	// wait for thread to exit
	xnOSWaitForThreadExit(m_hScheduler, 100);

	m_generatingEvent.Raise();
}

XnStatus DepthLetv::RegisterToGenerationRunningChange( XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback )
{
	return m_generatingEvent.Register(handler, pCookie, &hCallback);
}

void DepthLetv::UnregisterFromGenerationRunningChange( XnCallbackHandle hCallback )
{
	m_generatingEvent.Unregister(hCallback);
}

XnStatus DepthLetv::RegisterToNewDataAvailable( XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback )
{
	return m_dataAvailableEvent.Register(handler, pCookie, &hCallback);
}

void DepthLetv::UnregisterFromNewDataAvailable( XnCallbackHandle hCallback )
{
	m_dataAvailableEvent.Unregister(hCallback);
}

XnBool DepthLetv::IsNewDataAvailable( XnUInt64& nTimestamp )
{
	return m_bDataAvailable;
}

XnUInt32 DepthLetv::GetDataSize()
{
	return (SUPPORTED_X_RES * SUPPORTED_Y_RES * sizeof(XnDepthPixel));
}

XnUInt64 DepthLetv::GetTimestamp()
{
	return m_nTimestamp;
}

XnUInt32 DepthLetv::GetFrameID()
{
	return m_nFrameID;
}

xn::ModuleMirrorInterface* DepthLetv::GetMirrorInterface()
{
	return this;
}

XnStatus DepthLetv::SetMirror( XnBool bMirror )
{
	m_bMirror = bMirror;
	m_mirrorEvent.Raise();
	return (XN_STATUS_OK);
}

XnBool DepthLetv::IsMirrored()
{
	return m_bMirror;
}

XnStatus DepthLetv::RegisterToMirrorChange( XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback )
{
	return m_mirrorEvent.Register(handler, pCookie, &hCallback);
}

void DepthLetv::UnregisterFromMirrorChange( XnCallbackHandle hCallback )
{
	m_mirrorEvent.Unregister(hCallback);
}

XnUInt32 DepthLetv::GetSupportedMapOutputModesCount()
{
	// we only support a single mode
	return 1;
}

XnStatus DepthLetv::GetSupportedMapOutputModes( XnMapOutputMode aModes[], XnUInt32& nCount )
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

XnStatus DepthLetv::SetMapOutputMode( const XnMapOutputMode& Mode )
{

	// make sure this is our supported mode
	if (Mode.nXRes != SUPPORTED_X_RES ||
		Mode.nYRes != SUPPORTED_Y_RES ||
		Mode.nFPS != SUPPORTED_FPS)
	{
		printf("XN_STATUS_BAD_PARAM\n");
		return (XN_STATUS_BAD_PARAM);
	}

	return (XN_STATUS_OK);
}

XnStatus DepthLetv::GetMapOutputMode( XnMapOutputMode& Mode )
{
	Mode.nXRes = SUPPORTED_X_RES;
	Mode.nYRes = SUPPORTED_Y_RES;
	Mode.nFPS = SUPPORTED_FPS;

	return (XN_STATUS_OK);
}

XnStatus DepthLetv::RegisterToMapOutputModeChange( XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback )
{

	// no need. we only allow one mode
	hCallback = this;
	return XN_STATUS_OK;
}

void DepthLetv::UnregisterFromMapOutputModeChange( XnCallbackHandle hCallback )
{
	// do nothing (we didn't really register)	
}

XnDepthPixel* DepthLetv::GetDepthMap()
{
	return m_pDepthMap;
}

XnDepthPixel DepthLetv::GetDeviceMaxDepth()
{
	//printf("GetDeviceMaxDepth\n");
	return MAX_DEPTH_VALUE;
}

void DepthLetv::GetFieldOfView( XnFieldOfView& FOV )
{
	FOV.fHFOV = HFOV;
	FOV.fVFOV = VFOV;	 
}

XnStatus DepthLetv::RegisterToFieldOfViewChange( XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback )
{
	// no need. it never changes
	hCallback = this;
	return XN_STATUS_OK;
}

void DepthLetv::UnregisterFromFieldOfViewChange( XnCallbackHandle hCallback )
{
	// do nothing (we didn't really register)	
}

XN_THREAD_PROC DepthLetv::SchedulerThread( void* pCookie )
{
	DepthLetv* pThis = (DepthLetv*)pCookie;

	while (pThis->m_bGenerating)
	{
		// wait 33 ms (to produce 30 FPS)
		xnOSSleep(1000000/SUPPORTED_FPS/1000);

		pThis->OnNewFrame();
	}

	XN_THREAD_PROC_RETURN(0);
}

void DepthLetv::OnNewFrame()
{
	m_bDataAvailable = TRUE;
	m_dataAvailableEvent.Raise();
}

// Esta fun玢o retorna as propriedades inteiras do m骴ulo, ?necess醨ia para o funcionamento das aplica珲es
XnStatus DepthLetv::GetIntProperty(const XnChar* strName, XnUInt64& nValue) const
{
	if (strcmp(strName, XN_STREAM_PROPERTY_MIN_DEPTH) == 0)
    {
		nValue = 0;
        return XN_STATUS_OK;
    }
	else if (strcmp(strName, XN_STREAM_PROPERTY_MAX_DEPTH) == 0)
    {
		nValue = MAX_DEPTH_VALUE;
        return XN_STATUS_OK;
    }	
    else
    {
        return xn::ModuleDepthGenerator::GetIntProperty(strName, nValue);
    }
}

// Esta fun玢o retorna as propriedades reais do m骴ulo, ?necess醨ia para o funcionamento das aplica珲es
XnStatus DepthLetv::GetRealProperty(const XnChar* strName, XnDouble& dValue) const
{
	return xn::ModuleDepthGenerator::GetRealProperty(strName, dValue);
}


// Esta fun玢o retorna as propriedades de string do m骴ulo
XnStatus DepthLetv::GetStringProperty(const XnChar* strName, XnChar* csValue, XnUInt32 nBufSize) const { 

	return xn::ModuleDepthGenerator::GetStringProperty(strName, csValue, nBufSize);

}

// Esta fun玢o retorna outras propriedades do m骴ulo (como as matrizes de convers鉶), ?necess醨ia para o funcionamento das aplica珲es
XnStatus DepthLetv::GetGeneralProperty(const XnChar* strName, XnUInt32 nBufferSize, void* pBuffer) const { 
	return xn::ModuleDepthGenerator::GetGeneralProperty(strName, nBufferSize, pBuffer);	
}

/*
XnStatus DepthLetv::SetCropping(const XnCropping &Cropping)
{
	//printf ("SetCropping\n");
	//printf("nXOffset: %d, nYOffset: %d, nXSize: %d, nYSize: %d\n\n", Cropping.nXOffset, Cropping.nYOffset, Cropping.nXSize, Cropping.nYSize);
	crop = XnGeneralBufferPack((void*)&Cropping, sizeof(Cropping));
	//crop2 = (XnCropping*)&Cropping;
	return XN_STATUS_OK; //m_pSensor->SetProperty(m_strModule, XN_STREAM_PROPERTY_CROPPING, gbValue);
}


XnStatus DepthLetv::GetCropping(XnCropping &Cropping)
{
	//printf ("GetCropping\n");
	//printf("nXOffset: %d, nYOffset: %d, nXSize: %d, nYSize: %d\n\n", Cropping.nXOffset, Cropping.nYOffset, Cropping.nXSize, Cropping.nYSize);
	XN_PACK_GENERAL_BUFFER(Cropping);
	Cropping = (XnCropping &)crop.pData;

	//Cropping = (XnCropping &)crop2;
	return XN_STATUS_OK; // m_pSensor->GetProperty(m_strModule, XN_STREAM_PROPERTY_CROPPING, XN_PACK_GENERAL_BUFFER(Cropping));
}

XnStatus DepthLetv::RegisterToCroppingChange(XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback)
{
	//printf ("RegisterToCroppingChange\n");
	const XnChar* aProps[] = 
	{
		XN_CAPABILITY_CROPPING,
		NULL
	};

	return XN_STATUS_OK; //RegisterToProps(handler, pCookie, hCallback, aProps);
}

void DepthLetv::UnregisterFromCroppingChange(XnCallbackHandle hCallback)
{
	//printf ("UnregisterFromCroppingChange\n");
	//UnregisterFromProps(hCallback);
}
*/

/*XnStatus XnPixelStream::CropImpl(XnStreamData* pStreamOutput, const XnCropping* pCropping)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnUChar* pPixelData = (XnUChar*)pStreamOutput->pData;
	XnUInt32 nCurDataSize = 0;

	for (XnUInt32 y = pCropping->nYOffset; y < XnUInt32(pCropping->nYOffset + pCropping->nYSize); ++y)
	{
		XnUChar* pOrigLine = &pPixelData[y * GetXRes() * GetBytesPerPixel()];

		// move line
		xnOSMemCopy(pPixelData + nCurDataSize, pOrigLine + pCropping->nXOffset * GetBytesPerPixel(), pCropping->nXSize * GetBytesPerPixel());
		nCurDataSize += pCropping->nXSize * GetBytesPerPixel();
	}

	// update size
	pStreamOutput->nDataSize = nCurDataSize;

	return XN_STATUS_OK;
}*/
