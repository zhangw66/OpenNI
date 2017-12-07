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
#include "NeolixDepth.h"
XnUInt32 NeolixDepth::g_xRes = SUPPORTED_X_RES;
XnUInt32 NeolixDepth::g_yRes = SUPPORTED_Y_RES;
XnUInt32 NeolixDepth::g_fps = SUPPORTED_FPS;
#if (CHOSEN_SDK == USE_PMD_MARS04_SDK)
int NeolixDepth::getMars04DepthData(XnDepthPixel *& nidata)
{
	//printf("get Depth data\n");
	if (nidata == NULL) 
		return -1;
	static 	CLibTof* libtoftemp = libtof;
    static int rs;
	if (frame_data == NULL) {
		printf("getMars04DepthData: malloc for depth\n");
		frame_data=(FrameData_t*)malloc(sizeof(FrameData_t)*DEPTHMAP_W*DEPTHMAP_H+DEPTHMAP_W);
		if (NULL == frame_data) {
			printf("getMars04DepthData malloc frame_data fail\n");
			return -1;
		}
	}
    memset(frame_data,0,sizeof(FrameData_t)*DEPTHMAP_W*DEPTHMAP_H+DEPTHMAP_W);
	if (frame_data_Rgb == NULL) {
		printf("getMars04DepthData: malloc for rgb\n");
		frame_data_Rgb = (FrameDataRgb_t*)malloc(sizeof(FrameDataRgb_t)*DEPTHMAP_W*DEPTHMAP_H+DEPTHMAP_W);
		if (NULL == frame_data_Rgb) {
		printf("getMars04DepthData malloc  frame_data_Rgb fail\n");
		return -1;
		}
	}
	memset(frame_data_Rgb,0,sizeof(FrameDataRgb_t)*DEPTHMAP_W*DEPTHMAP_H+DEPTHMAP_W);
	if (frame_data == NULL || frame_data_Rgb == NULL) {
		printf("mull pointer\n");
		return -1;
	}
	//  if (NULL == PColorbuffer_s)
 //   {
 //       PColorbuffer_s = (unsigned char *) malloc(DEPTHMAP_W*DEPTHMAP_H*4);
 //   }
static	 FrameData_t* frame_data_p;
static     	FrameDataRgb_t* frame_data_Rgb_tmp;
 //       unsigned char	*PColorbuffer_d;

        frame_data_p = frame_data;
        frame_data_Rgb_tmp = frame_data_Rgb;
    //    PColorbuffer_d =  PColorbuffer_s;
		
        rs = libtoftemp->LibTOF_RcvDepthFrame2(frame_data_p, frame_data_Rgb_tmp,DEPTHMAP_W, DEPTHMAP_H);
	
        if(rs != LTOF_SUCCESS)
        {
		    usleep(100);
			return rs;
			//continue;
        }
		 if (0/*P_HEAD*/)//获取每帧图像附加在后边的信息.
        {
            S_MetaData depth_head;
            S_MetaData drgb_head;
            //memset(head,0,sizeof(S_MetaData));
            memcpy(&depth_head,(unsigned char*)(frame_data+DEPTHMAP_W*DEPTHMAP_H),sizeof(S_MetaData));
            memcpy(&drgb_head,(unsigned char*)(frame_data_Rgb+DEPTHMAP_W*DEPTHMAP_H),sizeof(S_MetaData));
            printf("depth:[%4d] [%2d %2d]  %d\n",depth_head.frameCount,depth_head.width,depth_head.height,depth_head.getFrameTime);
            printf("rgbd :[%4d] [%2d %2d]  %d\n",drgb_head.frameCount,drgb_head.width,drgb_head.height,depth_head.getFrameTime);
            //free(head);
        }
        if(1)   //可以写入文件,此处可以将深度数据提取出来.
        {
         static int i,j;
        #if 0
           
            //每帧数据前面要加上32字节头，用于回复时识别分辨率。目前只使用了前4个字节，宽高分别占两字节
            char head[32]={0};
            char *p=head;
            char * data_w=p;
            char * data_h=p+2;
            *(short*)data_w = DEPTHMAP_W;
            *(short*)data_h = DEPTHMAP_H;
            int len=sizeof(head);
         //   fwrite (head, sizeof(head), 1, depthRec_File);
		#endif
		 //按x,y,z顺序保存深度数据
            for (i = 0; i<172;i++)
            {
                for(j = 0; j<224;j++)
                {
                	nidata[i*224+j] = (XnDepthPixel )(frame_data[i*224+j].z*1000);  //原生的数据是m,最好转换为mm
                }
            }
        }
		return 0;
}
#endif
NeolixDepth::NeolixDepth() : 
#if (CHOSEN_SDK == USE_PMD_MARS04_SDK)
	deviceinfo(NULL),
	libtof(NULL),
	frame_data(NULL),
	frame_data_Rgb(NULL),
#endif
	m_bGenerating(FALSE),
	m_bDataAvailable(FALSE),
	m_pDepthMap(NULL),
	m_nFrameID(0),
	m_nTimestamp(0),
	m_hScheduler(NULL),
	m_bMirror(FALSE)

{
}

NeolixDepth::~NeolixDepth()
{
#if (CHOSEN_SDK == USE_TY_SDK)
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
#if (CHOSEN_SDK == USE_PMD_MARS04_SDK)
	if (frame_data != NULL)
		free(frame_data);
	if (frame_data_Rgb != NULL)
		free(frame_data_Rgb);
	if (PColorbuffer_s != NULL)
		free(PColorbuffer_s);
	if (libtof != NULL)
		delete libtof;
	if (deviceinfo != NULL)
		delete deviceinfo;
#endif
	delete[] m_pDepthMap;
}

XnStatus NeolixDepth::Init()
{
#if (CHOSEN_SDK == USE_PMD_MARS04_SDK)
    int rv = -1;
	libtof = new CLibTof();
	printf("============%#x\n",frame_data);
	printf("============%p\n",frame_data_Rgb);
    printf("------------------------------------------------\n");
    printf("---------SampleCode  Version  V2.0.1------------\n");
    printf("------------------------------------------------\n");
//连接相机，初始化相机
    rv = libtof->LibTOF_Connect();
    if (rv<0)
    {
        printf(" LibTOF_Connect Failed\n");
        return -1;
    }
    //获取设备信息，图像宽高，设备版本信息
    deviceinfo = new DeviceInfo_t;//(DeviceInfo_t *)malloc(sizeof(DeviceInfo_t));
    memset(deviceinfo,0,sizeof(DeviceInfo_t));
    rv = libtof->LibTOF_GetDeviceInfo(deviceinfo);
    if (rv<0)
    {
        printf(" LibTOF_GetDeviceInfo Failed\n");
        //system("pause");
        return -1 ;
    }
    else
    {
        DEPTHMAP_W = deviceinfo->DepthFrameWidth;
        DEPTHMAP_H = deviceinfo->DepthFrameHeight-1;//减去一行头长度
        DEPTHVIDEO_W = deviceinfo->VisibleFrameWidth;
        DEPTHVIDEO_H = deviceinfo->VisibleFrameHeight;
        //DEPTHVIDEO_FRAME_SIZE =DEPTHVIDEO_W*DEPTHVIDEO_H*BYTES_PER_POINT;

        printf("device ID:%s \ndeviceInfo:%s\nDepth Data:w-%d h-%d \nvisiableData:w-%d h-%d \n"
            ,deviceinfo->DeviceId,deviceinfo->TofAlgVersion,DEPTHMAP_W,DEPTHMAP_H,DEPTHVIDEO_W,DEPTHVIDEO_H);

        //判断设备版本，设置设备支持的功能
        if (MARS03_TOF_RGB == deviceinfo->DeviceVersion || MARS04_TOF_RGB == deviceinfo->DeviceVersion)
        {
            IS_RGBD=TRUE;
            IS_RGB=TRUE;
        }

        DEPTHVIDEO_FRAME_SIZE = ROUND_UP(DEPTHVIDEO_W*DEPTHVIDEO_H*3/2+DEPTHVIDEO_W,1024);//尾部带一行帧信息
    }

    if (0 == deviceinfo->RgbdEn)
    {
        libtof->LibTOF_RgbdEnSet(0);
    }
    else
    {
        libtof->LibTOF_RgbdEnSet(1);
    }

    //奇瑞项目定制参数设置(需要设置下面2个接口函数)
    libtof->LibTOF_SetUseCase(MODE_1_5FPS_1200);
    libtof->LibTOF_SetExpourseMode(Auto);


#endif
	m_pDepthMap = new XnDepthPixel[g_xRes * g_yRes];
	if (m_pDepthMap == NULL)
	{
		return XN_STATUS_ALLOC_FAILED;
	}
#if (CHOSEN_SDK == USE_TY_SDK)
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

XnBool NeolixDepth::IsCapabilitySupported( const XnChar* strCapabilityName )
{


	// every depth camera device  support diffrent capability,the mirror is base capability which implement with code
	#if (CHOSEN_SDK == USE_PMD_MARS04_SDK)
		if (strcmp(strCapabilityName, NEOLIX_CAPABILITY_SET_USE_CASE) == 0)
			return true;
		if (strcmp(strCapabilityName, NEOLIX_CAPABILITY_SET_SCENE) == 0)
			return true;
	#endif
	if (strcmp(strCapabilityName, XN_CAPABILITY_MIRROR) == 0)
	return true;
	return false;
}
XnStatus NeolixDepth::SetGeneralProperty(const XnChar* strName, XnUInt32 nBufferSize, const void* pBuffer) 
{ 
	printf("set prop[%s]\n", strName);
	if ((strcmp(strName, NEOLIX_CAPABILITY_SET_USE_CASE) == 0)) {
		unsigned char choose = *((unsigned char *)pBuffer);
	 	int rv = libtof->LibTOF_SetUseCase((CameraUseCase_e)choose);
     	if(rv == LTOF_SUCCESS) {
			//根据fps设置线程调用时间.
			switch (choose) {
			case MODE_1_5FPS_1200:g_fps =5;cout <<"fps:5"<<endl;break;
			case MODE_2_10FPS_650:g_fps =10;cout <<"fps:10"<<endl;break;
			case MODE_3_15FPS_850:g_fps =15;cout <<"fps:15"<<endl;break;
			case MODE_4_30FPS_380:g_fps =30;cout <<"fps:30"<<endl;break;
			case MODE_5_45FPS_250:g_fps =45;cout <<"fps:45"<<endl;break;
			default:return XN_STATUS_ERROR; ;
			}
			 
			return XN_STATUS_OK;
     	}

	}	
	if (strcmp(strName, NEOLIX_CAPABILITY_SET_SCENE) == 0) {
		unsigned char scene = *((unsigned char *)pBuffer);
	   	int rv = libtof->LibTOF_SetSceneMode((SceneMode_e)scene);
     	if(rv == LTOF_SUCCESS) 
			return XN_STATUS_OK;
	}	
	return XN_STATUS_ERROR; 
}
XnStatus NeolixDepth::GetGeneralProperty(const XnChar* strName, XnUInt32 nBufferSize, void* pBuffer)const
{ 
	printf("get prop[%s]\n", strName);
	strncpy((XnChar *)pBuffer, "hello wrold", nBufferSize);
	((XnChar *)pBuffer)[nBufferSize] = '\0';
	return XN_STATUS_ERROR; 
}


XnStatus NeolixDepth::StartGenerating()
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

XnBool NeolixDepth::IsGenerating()
{
	return m_bGenerating;
}

void NeolixDepth::StopGenerating()
{
	m_bGenerating = FALSE;

	// wait for thread to exit
	xnOSWaitForThreadExit(m_hScheduler, 100);

	m_generatingEvent.Raise();
}

XnStatus NeolixDepth::RegisterToGenerationRunningChange( XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback )
{
	return m_generatingEvent.Register(handler, pCookie, hCallback);
}

void NeolixDepth::UnregisterFromGenerationRunningChange( XnCallbackHandle hCallback )
{
	m_generatingEvent.Unregister(hCallback);
}

XnStatus NeolixDepth::RegisterToNewDataAvailable( XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback )
{
	return m_dataAvailableEvent.Register(handler, pCookie, hCallback);
}

void NeolixDepth::UnregisterFromNewDataAvailable( XnCallbackHandle hCallback )
{
	m_dataAvailableEvent.Unregister(hCallback);
}

XnBool NeolixDepth::IsNewDataAvailable( XnUInt64& nTimestamp )
{
	// return next timestamp
	nTimestamp = 1000000 / g_fps;
	return m_bDataAvailable;
}

XnStatus NeolixDepth::UpdateData()
{
	XnDepthPixel* pPixel = m_pDepthMap;

#if (CHOSEN_SDK == USE_TY_SDK)
	int err = TYFetchFrame(hDevice, &frame, -1);
	for (int i = 0; i < frame.validCount; i++)
	{
		// get depth image
		if (frame.image[i].componentID == TY_COMPONENT_DEPTH_CAM)
		{
			memcpy(m_pDepthMap, frame.image[i].buffer, sizeof(XnDepthPixel)*g_xRes*g_yRes);
			break;
		}
	}

	TYEnqueueBuffer(hDevice, frame.userBuffer, frame.bufferSize);
#endif
#if (CHOSEN_SDK == USE_PMD_MARS04_SDK)
		getMars04DepthData(m_pDepthMap); //如果失败,将会使用上一帧的数据.
#endif

#if 0
	// change our internal data, so that pixels go from frameID incrementally in both axes.
	for (XnUInt y = 0; y < SUPPORTED_Y_RES; ++y)
	{
		for (XnUInt x = 0; x < SUPPORTED_X_RES; ++x, ++pPixel)
		{
			*pPixel = (m_nFrameID + x + y) % MAX_DEPTH_VALUE;
		}
	}
#endif
	// if needed, mirror the map
	if (m_bMirror)
	{
		XnDepthPixel temp;

		for (XnUInt y = 0; y < g_yRes; ++y)
		{
			XnDepthPixel* pUp = &m_pDepthMap[y * g_xRes];
			XnDepthPixel* pDown = &m_pDepthMap[(y+1) * g_xRes - 1];

			for (XnUInt x = 0; x < g_xRes/2; ++x, ++pUp, --pDown)
			{
				temp = *pUp;
				*pUp = *pDown;
				*pDown = temp;
			}
		}
	}

	m_nFrameID++;
	m_nTimestamp += 1000000 / g_fps;

	// mark that data is old
	m_bDataAvailable = FALSE;
	
	return (XN_STATUS_OK);
}


const void* NeolixDepth::GetData()
{
	return m_pDepthMap;
}

XnUInt32 NeolixDepth::GetDataSize()
{
	return (g_xRes * g_yRes * sizeof(XnDepthPixel));
}

XnUInt64 NeolixDepth::GetTimestamp()
{
	return m_nTimestamp;
}

XnUInt32 NeolixDepth::GetFrameID()
{
	return m_nFrameID;
}

xn::ModuleMirrorInterface* NeolixDepth::GetMirrorInterface()
{
	return this;
}

XnStatus NeolixDepth::SetMirror( XnBool bMirror )
{
	m_bMirror = bMirror;
	m_mirrorEvent.Raise();
	return (XN_STATUS_OK);
}

XnBool NeolixDepth::IsMirrored()
{
	return m_bMirror;
}

XnStatus NeolixDepth::RegisterToMirrorChange( XnModuleStateChangedHandler handler, void* pCookie, XnCallbackHandle& hCallback )
{
	return m_mirrorEvent.Register(handler, pCookie, hCallback);
}

void NeolixDepth::UnregisterFromMirrorChange( XnCallbackHandle hCallback )
{
	m_mirrorEvent.Unregister(hCallback);
}

XnUInt32 NeolixDepth::GetSupportedMapOutputModesCount()
{
	// we only support a single mode
	return 1;
}

XnStatus NeolixDepth::GetSupportedMapOutputModes( XnMapOutputMode aModes[], XnUInt32& nCount )
{
	if (nCount < 1)
	{
		return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
	}

	aModes[0].nXRes = g_xRes;
	aModes[0].nYRes = g_yRes;
	aModes[0].nFPS = g_fps;

	return (XN_STATUS_OK);
}

XnStatus NeolixDepth::SetMapOutputMode( const XnMapOutputMode& Mode )
{
	// make sure this is our supported mode
	if (Mode.nXRes != g_xRes ||
		Mode.nYRes != g_yRes ||
		Mode.nFPS != g_fps)
	{
		return (XN_STATUS_BAD_PARAM);
	}

	return (XN_STATUS_OK);
}

XnStatus NeolixDepth::GetMapOutputMode( XnMapOutputMode& Mode )
{
	Mode.nXRes = g_xRes;
	Mode.nYRes = g_yRes;
	Mode.nFPS = g_fps;

	return (XN_STATUS_OK);
}

XnStatus NeolixDepth::RegisterToMapOutputModeChange( XnModuleStateChangedHandler /*handler*/, void* /*pCookie*/, XnCallbackHandle& hCallback )
{
	// no need. we only allow one mode
	hCallback = this;
	return XN_STATUS_OK;
}

void NeolixDepth::UnregisterFromMapOutputModeChange( XnCallbackHandle /*hCallback*/ )
{
	// do nothing (we didn't really register)	
}

XnDepthPixel* NeolixDepth::GetDepthMap()
{
	return m_pDepthMap;
}

XnDepthPixel NeolixDepth::GetDeviceMaxDepth()
{
	return MAX_DEPTH_VALUE;
}

void NeolixDepth::GetFieldOfView( XnFieldOfView& FOV )
{
	// some numbers
	FOV.fHFOV = 1.35;
	FOV.fVFOV = 1.35;
}

XnStatus NeolixDepth::RegisterToFieldOfViewChange( XnModuleStateChangedHandler /*handler*/, void* /*pCookie*/, XnCallbackHandle& hCallback )
{
	// no need. it never changes
	hCallback = this;
	return XN_STATUS_OK;
}

void NeolixDepth::UnregisterFromFieldOfViewChange( XnCallbackHandle /*hCallback*/ )
{
	// do nothing (we didn't really register)	
}

XN_THREAD_PROC NeolixDepth::SchedulerThread( void* pCookie )
{
	NeolixDepth* pThis = (NeolixDepth*)pCookie;
	XnUInt32 delay;
	while (pThis->m_bGenerating)
	{
		delay = 1000000/g_fps/1000;
		// wait 33 ms (to produce 30 FPS)
		xnOSSleep(delay);

		pThis->OnNewFrame();
	}

	XN_THREAD_PROC_RETURN(0);
}

void NeolixDepth::OnNewFrame()
{
	m_bDataAvailable = TRUE;
	m_dataAvailableEvent.Raise();
}
