// Demo.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "libTof.h"

typedef int BOOL;

#define BYTES_PER_POINT     2
#define ROUND_UP(x, align) (x+(align-1))&(~(align-1))
#define FALSE 0
#define TRUE 1
#define TEXT 0
int rv;
static int DEPTHMAP_W;
static int DEPTHMAP_H;
static int DEPTHVIDEO_W;
static int DEPTHVIDEO_H;
static int DEPTHVIDEO_FRAME_SIZE;
BOOL IS_RGBD = FALSE;
BOOL IS_RGB = FALSE;
FrameData_t* frame_data;
short* grey_data;
FrameDataRgb_t* frame_data_Rgb;
unsigned char	*PColorbuffer_s;
BOOL IS_Running=FALSE;
BOOL IS_Record = FALSE;
BOOL IS_photo = FALSE;
BOOL YUV_photo = FALSE;
BOOL GET_IMU = FALSE;
FILE * depthRec_File;
FILE * depthPhoto_File;
static int Camera_Mode=1;//默认为双频模式0、1： 深度模式（默认为1双频） 2：灰度模式
BOOL P_HEAD=FALSE;

#if TEXT

FILE* m_Temperture = NULL;
FILE* m_DepthEndTime = NULL;
FILE* m_RgbEndTime = NULL;

void* get_Temperture_fuc(void* param)
{
    CLibTof* libtoftemp = (CLibTof*)param;
    while (1)
    {
        if (!IS_Running)
        {
            usleep(100);
            continue;
        }

        usleep(5000000);
        float t1,t2;
        int rv = libtoftemp->LibTOF_GetTemperture(&t1,&t2);
        if (rv == LTOF_SUCCESS)
        {
            printf("cpu temperture: %2.2f  sensor temperture: %2.2f \n", t1,t2);
            fprintf(m_Temperture, "cpu temperture: %2.2f  sensor temperture: %2.2f \n", t1,t2);
        }
        else
        {
            printf("can not get temperture \n");
            if(NULL != m_Temperture)
            {
                fclose(m_Temperture);
                break;
            }
            
        }
    }

}
#endif


void* get_depthdata_fuc(void* param)
{
    CLibTof* libtoftemp = (CLibTof*)param;
    int rs;

    frame_data=(FrameData_t*)malloc(sizeof(FrameData_t)*DEPTHMAP_W*DEPTHMAP_H+DEPTHMAP_W);
    memset(frame_data,0,sizeof(FrameData_t)*DEPTHMAP_W*DEPTHMAP_H+DEPTHMAP_W);

    frame_data_Rgb = (FrameDataRgb_t*)malloc(sizeof(FrameDataRgb_t)*DEPTHMAP_W*DEPTHMAP_H+DEPTHMAP_W);
    memset(frame_data_Rgb,0,sizeof(FrameDataRgb_t)*DEPTHMAP_W*DEPTHMAP_H+DEPTHMAP_W);

    if (NULL == PColorbuffer_s)
    {
        PColorbuffer_s = (unsigned char *) malloc(DEPTHMAP_W*DEPTHMAP_H*4);
    }
    while (1)
    {
        if (!IS_Running)
        {
            usleep(100);
            continue;
        }
        FrameData_t* frame_data_p;
        FrameDataRgb_t* frame_data_Rgb_tmp;
        unsigned char	*PColorbuffer_d;

        frame_data_p = frame_data;
        frame_data_Rgb_tmp = frame_data_Rgb;
        PColorbuffer_d =  PColorbuffer_s;
		if(GET_IMU)
		{
			IMU_data_t imu_data;
		     rs = libtoftemp->LibTOF_RcvDepthFrame2(frame_data_p, frame_data_Rgb_tmp,DEPTHMAP_W, DEPTHMAP_H,&imu_data);
printf(">>%ld %f %f %f %ld %f %f %f \n",imu_data.ACC_timeStamp,imu_data.ACC_x,imu_data.ACC_y,imu_data.ACC_z,
imu_data.GYR_timeStamp,imu_data.GYR_x,imu_data.GYR_y,imu_data.GYR_z);
		}
		else
		{
        	rs = libtoftemp->LibTOF_RcvDepthFrame2(frame_data_p, frame_data_Rgb_tmp,DEPTHMAP_W, DEPTHMAP_H);
		}
        if(rs != LTOF_SUCCESS)
        {
            #if TEXT
            if(LDV_ERROR_USB_READ == rs)
            {
                
                time_t timep;
                time (&timep);
                if(NULL != m_DepthEndTime)
                {
                    fprintf(m_DepthEndTime, "depth %s\n",ctime(&timep));
                    fclose(m_DepthEndTime);
                    m_DepthEndTime = NULL;
                    break;
                } 
            }

            static int num = 1;
            if(num > 3)
            {
                num = 0;
                time_t timep;
                time (&timep);
                if(NULL != m_DepthEndTime)
                {
                    fprintf(m_DepthEndTime, "depth %s\n",ctime(&timep));
                    fclose(m_DepthEndTime);
                    m_DepthEndTime = NULL;
                    break;
                } 
            }
            num++;
#endif
            usleep(100);
            continue;
        }

        #if 0
        static int count = 0;
        struct timeval tv;
        gettimeofday(&tv,NULL);
        uint64_t val;
        uint64_t val2;
        val = tv.tv_sec*1000 + tv.tv_usec/1000;
        
        if(0 == count)
        {
            val2 = val;
            count++;
        }
        printf("time interval = [%u]\n", val-val2);
        val2 = val;
        #endif

        if (P_HEAD)//打印针信息
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
        if(IS_Record)
        {
            int i,j;
            //每帧数据前面要加上32字节头，用于回复时识别分辨率。目前只使用了前4个字节，宽高分别占两字节
            char head[32]={0};
            char *p=head;
            char * data_w=p;
            char * data_h=p+2;
            *(short*)data_w = DEPTHMAP_W;
            *(short*)data_h = DEPTHMAP_H;
            int len=sizeof(head);
            fwrite (head, sizeof(head), 1, depthRec_File);
            //按x,y,z顺序保存深度数据
            for (i = 0; i<172;i++)
            {
                for(j = 0; j<224;j++)
                {
                    fwrite (&(frame_data[i*224+j].x) , sizeof(float), 1, depthRec_File);
                    fwrite (&(frame_data[i*224+j].y) , sizeof(float), 1, depthRec_File);
                    fwrite (&(frame_data[i*224+j].z) , sizeof(float), 1, depthRec_File);
                }
            }
        }
        if(IS_photo)
        {
            IS_photo=FALSE;
            int i,j;
            //每帧数据前面要加上32字节头，用于回复时识别分辨率。目前只使用了前4个字节，宽高分别占两字节
            char head[32]={0};
            char *p=head;
            char * data_w=p;
            char * data_h=p+2;
            *(short*)data_w = DEPTHMAP_W;
            *(short*)data_h = DEPTHMAP_H;
            int len=sizeof(head);
            fwrite (head, sizeof(head), 1, depthPhoto_File);
            //按x,y,z顺序保存深度数据
            for (i = 0; i<172;i++)
            {
                for(j = 0; j<224;j++)
                {
                    fwrite (&(frame_data[i*224+j].x) , sizeof(float), 1, depthPhoto_File);
                    fwrite (&(frame_data[i*224+j].y) , sizeof(float), 1, depthPhoto_File);
                    fwrite (&(frame_data[i*224+j].z) , sizeof(float), 1, depthPhoto_File);
                }
            }
            fclose(depthPhoto_File);

            if (2 == Camera_Mode)//深度数据中带有的灰度数据
            {
                time_t nowtime;
                struct tm *timeinfo;
                int year,month,day,hour,min,sec;
                char szFileName[32];
                time(&nowtime);
                timeinfo=localtime(&nowtime);
                year=timeinfo->tm_year+1900;
                month=timeinfo->tm_mon+1;
                day=timeinfo->tm_mday;
                hour=timeinfo->tm_hour;
                min=timeinfo->tm_min;
                sec=timeinfo->tm_sec;
                sprintf(szFileName, "photo_%04d%02d%02d%02d%02d%02d.raw",
                    year,month,day,hour,min,sec);

                FILE * pFile;
                pFile = fopen(szFileName,"wb");
                fwrite (frame_data_Rgb, 2, 224*172, pFile);
                fclose (pFile);
            }
        }
        //伪彩转换
        //LibTOF_PseudoColor(frame_data, DEPTHMAP_W , DEPTHMAP_H,PColorbuffer_d,1,6);
    }
    return NULL;
}                                                                                                                                                                                                                                                                                                                                                                                                                                               

//获取相机的YUV数据                                        
short * Video_data;                                     

void* get_visiabledata_fuc(void* param)
{
    CLibTof* libtoftemp = (CLibTof*)param;
    Video_data=(short *) malloc(DEPTHVIDEO_FRAME_SIZE);

    while (IS_RGB)
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
        if (!IS_Running)
        {	
            usleep(100);
            continue;
        }	

        /* here we request device to give a frame */
        int rs = libtoftemp->LibTOF_RcvVideoFrame(Video_data,DEPTHVIDEO_FRAME_SIZE);//目前获取到的数据格式为YUYV
        if(rs == LTOF_SUCCESS) 
        {
            ;//数据处理部分
            if (P_HEAD)//打印帧头信息
            {
                S_MetaData head;
                //memset(head,0,sizeof(S_MetaData));
                memcpy(&head,(unsigned char*)Video_data+DEPTHVIDEO_W*DEPTHVIDEO_H*3/BYTES_PER_POINT,sizeof(S_MetaData));
                printf("rgb  :[%4d] [%2d %2d]  %d\n",head.frameCount,head.width,head.height,head.getFrameTime);
                //free(head);
            }
	
			if(YUV_photo)
			{
				YUV_photo = FALSE;
				char outfile_name[32]={0};
				FILE* fp=NULL;
			    time_t nowtime;
			    int year,month,day,hour,min,sec;
			    struct tm *timeinfo;
			    time(&nowtime);
			    timeinfo=localtime(&nowtime);
			    year=timeinfo->tm_year+1900;
			    month=timeinfo->tm_mon+1;
			    day=timeinfo->tm_mday;
			    hour=timeinfo->tm_hour;
			    min=timeinfo->tm_min;
			    sec=timeinfo->tm_sec;
				if(IS_RGBD)
				{
				    sprintf(outfile_name, "%dx%d_%04d%02d%02d%02d%02d%02d.I420",DEPTHVIDEO_W,DEPTHVIDEO_H,year,month,day,hour,min,sec);
				    fp = fopen(outfile_name,"wb");
					fwrite(Video_data,DEPTHVIDEO_W*DEPTHVIDEO_H*3/2,1,fp);
					fclose(fp);
					printf("photo finish \n");
				}
				else
				{
					sprintf(outfile_name, "%dx%d_%04d%02d%02d%02d%02d%02d.YUYV",DEPTHVIDEO_W,DEPTHVIDEO_H,year,month,day,hour,min,sec);
				   fp = fopen(outfile_name,"wb");
					fwrite(Video_data,DEPTHVIDEO_W*DEPTHVIDEO_H*2,1,fp);
					fclose(fp);
					printf("photo finish \n");
				}
			}
	     }
        else
        {
            #if TEXT
            if(LDV_ERROR_USB_READ == rs)
            {
                time_t timep;
                time (&timep);
                if(NULL != m_RgbEndTime)
                {
                    fprintf(m_RgbEndTime, "rgb %s\n",ctime(&timep));
                    fclose(m_RgbEndTime);
                    m_RgbEndTime = NULL;
                    break;
                }
            }
            
            static int num = 1;
            if(num > 3)
            {
                num = 0;
                time_t timep;
                time (&timep);
                if(NULL != m_RgbEndTime)
                {
                    fprintf(m_RgbEndTime, "rgb %s\n",ctime(&timep));
                    fclose(m_RgbEndTime);
                    m_RgbEndTime = NULL;
                    break;
                }
            }
            num++;
            #endif
        }
    }
    return NULL;
}

//zhudm, use this to replace gets 
void get_string(char  * cmd) 
{
    int i;
    for (i = 0; i < 10; i ++) {
        int ch = getchar();
        if (ch == 10) 
        { 
            break;
        }
        cmd[i] = ch;
    }

    cmd[i]= 0; 
}


#define CHAR_BUFFERSIZE 20
int MAX_Fps=30;//最大帧率默认30
ExpourseMode_e cur_expMode =Manual;
SceneMode_e cur_SceneMode = Scene_No;
CameraUseCase_e cur_UseCase =MODE_4_30FPS_380;
int main(int argc, char *argv[])
{
    CLibTof* libtof = new CLibTof();
    printf("------------------------------------------------\n");
    printf("---------SampleCode  Version  V2.0.1------------\n");
    printf("------------------------------------------------\n");
    #if TEXT
    m_Temperture = fopen("Temperture.txt","wb");
    if(NULL == m_Temperture)
    {
        printf("m_Temperture fopen failed\n");
    }

    m_DepthEndTime = fopen("DepthEndTime.txt","wb");
    if(NULL == m_DepthEndTime)
    {
        printf("m_DepthEndTime fopen failed\n");
    }

    m_RgbEndTime = fopen("RgbEndTime.txt","wb");
    if(NULL == m_RgbEndTime)
    {
        printf("m_RgbEndTime fopen failed\n");
    }
    #endif
    //连接相机，初始化相机
    rv = libtof->LibTOF_Connect();
    if (rv<0)
    {
        printf(" LibTOF_Connect Failed\n");
        //system("pause");
        return -1;
    }

    //获取设备信息，图像宽高，设备版本信息
    DeviceInfo_t *deviceinfo = new DeviceInfo_t;//(DeviceInfo_t *)malloc(sizeof(DeviceInfo_t));
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
    libtof->LibTOF_SetUseCase(MODE_4_30FPS_380);
    libtof->LibTOF_SetExpourseMode(Auto);

    IS_Running = TRUE;
    //创建取数据线程
    pthread_t pth_depth;
    pthread_create(&pth_depth,NULL,get_depthdata_fuc,libtof);

#if TEXT
	pthread_t pth_temperture;
	pthread_create(&pth_temperture,NULL,get_Temperture_fuc,libtof);
#endif

    if(IS_RGB)
    {
        pthread_t pth_visiable;
        pthread_create(&pth_visiable,NULL,get_visiabledata_fuc,libtof);
    }

    char sel[CHAR_BUFFERSIZE];

    while(1)
    {
        char cmd[25];

        printf("\n===========================\n");

        printf("q: Stop\n");
        printf("a: Set expourse mode\n");  
        printf("e: Set expourse time\n");
        printf("f: Set fps\n");
        printf("m: Set Camera mode\n");
        printf("t: Get Camera Temperture\n");
        printf("s: set Camera Scene Mode\n");
        printf("r: Record depth data\n");
        printf("u: set  UseCaese\n");
        printf("p: TakePhoto depth data\n");
        printf("g: get grey data\n");
        printf("d: open depth log\n");
        printf("v: close depth log\n");
        printf("x: upversion\n");
        printf("\n===========================\n\n");

        get_string(cmd);
        if (strstr(cmd,"m")!=NULL)  //设置模式
        {
            int Mode;
            printf("0：Mono-Frequency,1：Multi-Frequencies");
            get_string( sel);
            sscanf(sel,"%d", &Mode);
            if (Mode == 0 || Mode == 1|| Mode == 2)
            {
                rv = libtof->LibTOF_Setparam(SET_CAMERA_MODE,Mode);
                if (rv<0)
                {
                    printf("failed \n");
                }
                else
                {
                    Camera_Mode = Mode;
                    printf("success Camera_Mode=%d\n",Camera_Mode);
                }
            }
            else
            {
                printf("error input\n");
            }
        }
        else if(strstr(cmd,"g")!=NULL)
        {
            if(Camera_Mode!=2)
            {
                rv = libtof->LibTOF_Setparam(SET_CAMERA_MODE,2);
                if (rv<0)
                {
                    printf("set camera mode failed \n");
                }
                else
                    Camera_Mode=2;
            }
        }
        else if(strstr(cmd,"d")!=NULL)
        {
            P_HEAD = true;
        }
        else if(strstr(cmd,"v")!=NULL)
        {
            P_HEAD = false;
        }
        else if(strstr(cmd,"a")!=NULL)
        {
            int mode;
            printf("mode (1:manual 2:auto) :");
            get_string( sel);
            sscanf(sel,"%d", &mode);
            int rv = libtof->LibTOF_SetExpourseMode((ExpourseMode_e)mode);
            if(rv == LTOF_SUCCESS)
                cur_expMode =(ExpourseMode_e) mode;
        }
        else if(strstr(cmd,"s")!=NULL)
        {
            int mode;
            printf("mode ( 0:Scene_No, 1:Scene_30cm, 2:Scene_75cm ：");
            get_string( sel);
            sscanf(sel,"%d", &mode);
            int rv = libtof->LibTOF_SetSceneMode((SceneMode_e)mode);
            if(rv == LTOF_SUCCESS)
                cur_SceneMode = (SceneMode_e)mode;
        }
        else if(strstr(cmd,"u")!=NULL)
        {
            int usecase;
            printf("mode ( 0:	MODE_1_5FPS_1200 ,1:MODE_2_10FPS_650, 2:MODE_3_15FPS_850,3: MODE_4_30FPS_380, 4:MODE_5_45FPS_250,)  :");
            get_string( sel );
            sscanf(sel,"%d", &usecase);
            int rv = libtof->LibTOF_SetUseCase((CameraUseCase_e)usecase);
            if(rv == LTOF_SUCCESS)
                cur_UseCase = (CameraUseCase_e)usecase;
        }
        else if(strstr(cmd,"e")!=NULL)//设置曝光时间，和帧率的乘积不能超过7500
        {
            if(cur_expMode!=Manual||cur_SceneMode!=Scene_No)
            {
                printf ("current exp mode %d(1:manual 2:auto)   Scene Mode  %d(0:Scene_No, 1:Scene_30cm, 2:Scene_75cm)\n",cur_expMode,cur_SceneMode);
                continue;
            }
            int Exp;
            printf("Exp (1~2000)=");
            get_string( sel);
            sscanf(sel,"%d", &Exp);
            if(Exp<1)
                Exp = 1;
            if(Exp>2000)
                Exp = 2000;
            int cur_Exp = libtof->LibTOF_Setparam(SET_CAMERA_EXP, Exp);//返回值和设置值相同证明设置成功，否则返回支持的最大的曝光时间
            if(cur_Exp ==Exp)
                printf("set Exp %d success\n",cur_Exp);
            else
                printf("Support MAX Exp is  %d\n",cur_Exp);

        }
        else if(strstr(cmd,"f")!=NULL)
        {
            if(cur_expMode!=Manual||cur_SceneMode!=Scene_No)
            {
                printf ("current exp mode %d(1:manual 2:auto)   Scene Mode  %d(0:Scene_No, 1:Scene_30cm, 2:Scene_75cm)\n",cur_expMode,cur_SceneMode);
                continue;
            }
            int Fps;
            if (Camera_Mode==1 || Camera_Mode==2)//双频最大帧率30
            {
                printf("Fps (1~30)=");
                MAX_Fps=30;
            }else if (Camera_Mode==0)//单频最大帧率60
            {
                printf("Fps (1~60)=");
                MAX_Fps=60;
            }
            get_string( sel);
            sscanf(sel,"%d", &Fps);
            if(Fps<1)
                Fps = 1;
            if(Fps>MAX_Fps)
                Fps = MAX_Fps;
            int cur_fps = libtof->LibTOF_Setparam(SET_CAMERA_FPS, Fps);//返回值和设置值相同证明设置成功，否则返回支持的最大帧率
            if(cur_fps ==Fps)
                printf("set fps %d success\n",cur_fps);
            else
                printf("Support MAX FPS is  %d\n",cur_fps);

        }
        else if(strstr(cmd,"t")!=NULL)
        {
            float t1,t2;
            int rv = libtof->LibTOF_GetTemperture(&t1,&t2);
            if (rv ==LTOF_SUCCESS)
            {
                printf("cpu temperture: %2.2f  sensor temperture: %2.2f \n", t1,t2);
            }
            else
            {
                printf("can not get temperture \n");
            }
        }
        else if(strstr(cmd,"x")!=NULL)
        {
            libtof->LibTOF_UpdataFalsh();
        }
        else if(strstr(cmd,"r")!=NULL)
        {
            char depthRec_name[32]={0};
            time_t nowtime;
            int year,month,day,hour,min,sec;
            struct tm *timeinfo;
            time(&nowtime);
            timeinfo=localtime(&nowtime);
            year=timeinfo->tm_year+1900;
            month=timeinfo->tm_mon+1;
            day=timeinfo->tm_mday;
            hour=timeinfo->tm_hour;
            min=timeinfo->tm_min;
            sec=timeinfo->tm_sec;
            sprintf(depthRec_name, "video_%04d%02d%02d%02d%02d%02d.smda",year,month,day,hour,min,sec);
            depthRec_File = fopen(depthRec_name,"wb");

            IS_Record = TRUE;
            printf("\n===========================\n\n");

            printf("rq: stop Record\n");

            get_string(cmd);
            if(strstr(cmd,"rq")!=NULL)
            {
                IS_Record=FALSE;
                fclose(depthRec_File);
            }
        }
        else if(strstr(cmd,"p")!=NULL)
        {
            if(( ! IS_RGBD && (Camera_Mode==0 ||Camera_Mode==1)) || IS_RGBD)
            {
                char depthPhoto_name[32]={0};
                time_t nowtime;
                int year,month,day,hour,min,sec;
                struct tm *timeinfo;
                time(&nowtime);
                timeinfo=localtime(&nowtime);
                year=timeinfo->tm_year+1900;
                month=timeinfo->tm_mon+1;
                day=timeinfo->tm_mday;
                hour=timeinfo->tm_hour;
                min=timeinfo->tm_min;
                sec=timeinfo->tm_sec;
                sprintf(depthPhoto_name, "photo_%04d%02d%02d%02d%02d%02d.smda",year,month,day,hour,min,sec);
                depthPhoto_File = fopen(depthPhoto_name,"wb");
                IS_photo = TRUE;
            }
            else
            {
                IS_photo = TRUE;
            }
			if(IS_RGB)
			{
				YUV_photo = TRUE;
			}
        }
        else if(strstr(cmd,"q")!=NULL)
        {
            //断开设备连接，释放内存
            IS_Running=FALSE;
            usleep(50000);//等待线程中的操作完成
            libtof->LibTOF_DisConnect();
            if (frame_data!=NULL)
            {
                free(frame_data);
            }

            if (frame_data_Rgb!=NULL)
            {
                free(frame_data_Rgb);
            }
            if (PColorbuffer_s!=NULL)
            {
                free(PColorbuffer_s);
            }
            if (depthRec_File!=NULL)
            {
                fclose(depthRec_File);
            }
            printf("disconnect \n");
            break;
        }
    }
    //system("pause");
    return 0;

}
