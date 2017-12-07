#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "libTof.h"

using namespace std;

#define BYTES_PER_POINT     2
#define ROUND_UP(x, align) (x+(align-1))&(~(align-1))
#define FALSE 0
#define TRUE 1
#define TEST_TOF 1
#define TEST_LOG 1
#define TEST_capture 1
typedef int BOOL;

FrameData_t* frame_data;
FrameDataRgb_t* frame_data_Rgb;
FrameData_t* frame_data2;
FrameDataRgb_t* frame_data_Rgb2;
static int DEPTHMAP_W;
static int DEPTHMAP_H;
static int DEPTHVIDEO_W;
static int DEPTHVIDEO_H;
static int DEPTHVIDEO_FRAME_SIZE;
bool IS_Running=false;
bool P_HEAD=true;
BOOL YUV_photo = FALSE;
BOOL YUV_photo2 = FALSE;
BOOL IS_RGBD = FALSE;
BOOL IS_RGB = FALSE;

#if TEST_LOG
    FILE* fp_log = fopen("log.txt", "wb");
#endif

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

void* get_depthdata_fuc1(void* param)
{
    CLibTof* libtoftemp = (CLibTof*)param;
    int rs;

    frame_data=(FrameData_t*)malloc(sizeof(FrameData_t)*DEPTHMAP_W*DEPTHMAP_H+DEPTHMAP_W);
    memset(frame_data,0,sizeof(FrameData_t)*DEPTHMAP_W*DEPTHMAP_H+DEPTHMAP_W);
    frame_data_Rgb = (FrameDataRgb_t*)malloc(sizeof(FrameDataRgb_t)*DEPTHMAP_W*DEPTHMAP_H+DEPTHMAP_W);
    memset(frame_data_Rgb,0,sizeof(FrameDataRgb_t)*DEPTHMAP_W*DEPTHMAP_H+DEPTHMAP_W);

    while (1)
    {
        if (!IS_Running )
        {
            usleep(100);
            continue;
        }
        FrameData_t* frame_data_p;
        FrameDataRgb_t* frame_data_Rgb_tmp;
        unsigned char	*PColorbuffer_d;

        frame_data_p = frame_data;
        frame_data_Rgb_tmp = frame_data_Rgb;//MRAS04�豸���Ҷ�ģʽʱ����ŵ��ǻҶ����ݣ�ÿ������ռ2�ֽ�
        IMU_data_t IMU_data;

        rs = libtoftemp->LibTOF_RcvDepthFrame2(frame_data_p, frame_data_Rgb_tmp,DEPTHMAP_W, DEPTHMAP_H);

        #if TEST_LOG
        static int count = 0;
        #endif
        if(rs != LTOF_SUCCESS) 
        {
            #if TEST_LOG
            if(0 == count)
            {
                time_t timep;
                time(&timep);
                fprintf(fp_log, "depth1 faile = [%s]", ctime(&timep));
            }
            count++;
            #endif

            usleep(100);
            continue;
        }
        #if TEST_LOG
        else
        {
            count = 0;
        }
        #endif

        if (P_HEAD)//打印帧头信息
        {
            static int num = 0;
            if(num > 20)
            {
                num = 0;
                S_MetaData depth_head;
                S_MetaData drgb_head;
                memcpy(&depth_head,(unsigned char*)(frame_data+DEPTHMAP_W*DEPTHMAP_H),sizeof(S_MetaData));
                memcpy(&drgb_head,(unsigned char*)(frame_data_Rgb+DEPTHMAP_W*DEPTHMAP_H),sizeof(S_MetaData));
                printf("depth1:[%4d] [%2d %2d]  %d\n",depth_head.frameCount,depth_head.width,depth_head.height,depth_head.getFrameTime);
                printf("rgbd1 :[%4d] [%2d %2d]  %d\n",drgb_head.frameCount,drgb_head.width,drgb_head.height,depth_head.getFrameTime);
            }
            else
            {
                num++;
            }

            #if TEST_capture
            static int captureNum = 0;
            if(captureNum > 5000)
            {
                captureNum = 0;
                char fileName[64] = {0};
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
                sprintf(fileName, "depth1_%04d%02d%02d%02d%02d%02d.smda",year,month,day,hour,min,sec);
                FILE* fp = fopen(fileName, "wb");

                int i,j;
                //每帧数据前面要加上32字节头，用于回复时识别分辨率。目前只使用了前4个字节，宽高分别占两字节
                char head[32]={0};
                char *p=head;
                char * data_w=p;
                char * data_h=p+2;
                *(short*)data_w = DEPTHMAP_W;
                *(short*)data_h = DEPTHMAP_H;
                int len=sizeof(head);
                fwrite (head, sizeof(head), 1, fp);
                //按x,y,z顺序保存深度数据
                for (i = 0; i<172;i++)
                {
                    for(j = 0; j<224;j++)
                    {
                        fwrite (&(frame_data[i*224+j].x) , sizeof(float), 1, fp);
                        fwrite (&(frame_data[i*224+j].y) , sizeof(float), 1, fp);
                        fwrite (&(frame_data[i*224+j].z) , sizeof(float), 1, fp);
                    }
                }
                fclose(fp);
            }
            else
            {
                captureNum++;
            }
            #endif
        }
    }
    return NULL;
}

void* get_depthdata_fuc2(void* param)
{
    CLibTof* libtoftemp = (CLibTof*)param;
    int rs;

    frame_data2=(FrameData_t*)malloc(sizeof(FrameData_t)*DEPTHMAP_W*DEPTHMAP_H+DEPTHMAP_W);
    memset(frame_data2,0,sizeof(FrameData_t)*DEPTHMAP_W*DEPTHMAP_H+DEPTHMAP_W);
    frame_data_Rgb2 = (FrameDataRgb_t*)malloc(sizeof(FrameDataRgb_t)*DEPTHMAP_W*DEPTHMAP_H+DEPTHMAP_W);
    memset(frame_data_Rgb2,0,sizeof(FrameDataRgb_t)*DEPTHMAP_W*DEPTHMAP_H+DEPTHMAP_W);

    while (1)
    {
        if (!IS_Running )
        {
            usleep(100);
            continue;
        }
        FrameData_t* frame_data_p;
        FrameDataRgb_t* frame_data_Rgb_tmp;
        unsigned char	*PColorbuffer_d;

        frame_data_p = frame_data2;
        frame_data_Rgb_tmp = frame_data_Rgb2;//MRAS04�豸���Ҷ�ģʽʱ����ŵ��ǻҶ����ݣ�ÿ������ռ2�ֽ�
        IMU_data_t IMU_data;

        rs = libtoftemp->LibTOF_RcvDepthFrame2(frame_data_p, frame_data_Rgb_tmp,DEPTHMAP_W, DEPTHMAP_H);

        #if TEST_LOG
        static int count = 0;
        #endif
        if(rs != LTOF_SUCCESS) 
        {
            #if TEST_LOG
            if(0 == count)
            {
                time_t timep;
                time(&timep);
                fprintf(fp_log, "depth2 faile = [%s]", ctime(&timep));
            }
            count++;
            #endif
            
            usleep(100);
            continue;
        }
        #if TEST_LOG
        else
        {
            count = 0;
        }
        #endif

        if (P_HEAD)//打印帧头信息
        {
            static int num = 0;
            if(num > 15)
            {
                num = 0;
                S_MetaData depth_head;
                S_MetaData drgb_head;
                memcpy(&depth_head,(unsigned char*)(frame_data2+DEPTHMAP_W*DEPTHMAP_H),sizeof(S_MetaData));
                memcpy(&drgb_head,(unsigned char*)(frame_data_Rgb2+DEPTHMAP_W*DEPTHMAP_H),sizeof(S_MetaData));
                printf("depth2:[%4d] [%2d %2d]  %d\n",depth_head.frameCount,depth_head.width,depth_head.height,depth_head.getFrameTime);
                printf("rgbd2:[%4d] [%2d %2d]  %d\n",drgb_head.frameCount,drgb_head.width,drgb_head.height,depth_head.getFrameTime);
            }
            else
            {
                num++;
            }

            #if TEST_capture
            static int captureNum2 = 0;
            if(captureNum2 > 5000)
            {
                captureNum2 = 0;
                char fileName[64] = {0};
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
                sprintf(fileName, "depth2_%04d%02d%02d%02d%02d%02d.smda",year,month,day,hour,min,sec);
                FILE* fp = fopen(fileName, "wb");

                int i,j;
                //每帧数据前面要加上32字节头，用于回复时识别分辨率。目前只使用了前4个字节，宽高分别占两字节
                char head[32]={0};
                char *p=head;
                char * data_w=p;
                char * data_h=p+2;
                *(short*)data_w = DEPTHMAP_W;
                *(short*)data_h = DEPTHMAP_H;
                int len=sizeof(head);
                fwrite (head, sizeof(head), 1, fp);
                //按x,y,z顺序保存深度数据
                for (i = 0; i<172;i++)
                {
                    for(j = 0; j<224;j++)
                    {
                        fwrite (&(frame_data2[i*224+j].x) , sizeof(float), 1, fp);
                        fwrite (&(frame_data2[i*224+j].y) , sizeof(float), 1, fp);
                        fwrite (&(frame_data2[i*224+j].z) , sizeof(float), 1, fp);
                    }
                }
                fclose(fp);
            }
            else
            {
                captureNum2++;
            }
            #endif

        }
    }
    return NULL;
}

//获取相机的YUV数据                                        
short * Video_data;                                     

void* get_visiabledata_fuc1(void* param)
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
            if (0)//打印帧头信息
            {
                S_MetaData head;
                //memset(head,0,sizeof(S_MetaData));
                memcpy(&head,(unsigned char*)Video_data+DEPTHVIDEO_W*DEPTHVIDEO_H*3/BYTES_PER_POINT,sizeof(S_MetaData));
                printf("rgb  :[%4d] [%2d %2d]  %d\n",head.frameCount,head.width,head.height,head.getFrameTime);
                //free(head);
            }
    
            #if TEST_capture
            static int rgbNum1 = 0;
            if(rgbNum1 > 5000)
            {
                YUV_photo = TRUE;
                rgbNum1 = 0;
            }
            else
            {
                rgbNum1++;
            }
            #endif
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
				    sprintf(outfile_name, "rgb_%dx%d_%04d%02d%02d%02d%02d%02d.I420",DEPTHVIDEO_W,DEPTHVIDEO_H,year,month,day,hour,min,sec);
				    fp = fopen(outfile_name,"wb");
					fwrite(Video_data,DEPTHVIDEO_W*DEPTHVIDEO_H*3/2,1,fp);
					fclose(fp);
					printf("photo finish \n");
				}
				else
				{
					sprintf(outfile_name, "rgb_%dx%d_%04d%02d%02d%02d%02d%02d.YUYV",DEPTHVIDEO_W,DEPTHVIDEO_H,year,month,day,hour,min,sec);
				   fp = fopen(outfile_name,"wb");
					fwrite(Video_data,DEPTHVIDEO_W*DEPTHVIDEO_H*2,1,fp);
					fclose(fp);
					printf("photo finish \n");
				}
			}
	     }
    }
    return NULL;
}

//获取相机的YUV数据                                        
short * Video_data2;                                     

void* get_visiabledata_fuc2(void* param)
{
    CLibTof* libtoftemp = (CLibTof*)param;
    Video_data2=(short *) malloc(DEPTHVIDEO_FRAME_SIZE);

    while (IS_RGB)
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
        if (!IS_Running)
        {	
            usleep(100);
            continue;
        }	

        /* here we request device to give a frame */
        int rs = libtoftemp->LibTOF_RcvVideoFrame(Video_data2,DEPTHVIDEO_FRAME_SIZE);//目前获取到的数据格式为YUYV
        if(rs == LTOF_SUCCESS) 
        {
            ;//数据处理部分
            if (0)//打印帧头信息
            {
                S_MetaData head;
                //memset(head,0,sizeof(S_MetaData));
                memcpy(&head,(unsigned char*)Video_data2+DEPTHVIDEO_W*DEPTHVIDEO_H*3/BYTES_PER_POINT,sizeof(S_MetaData));
                printf("rgb  :[%4d] [%2d %2d]  %d\n",head.frameCount,head.width,head.height,head.getFrameTime);
                //free(head);
            }
    
            #if TEST_capture
            static int rgbNum2 = 0;
            if(rgbNum2 > 5000)
            {
                YUV_photo2 = TRUE;
                rgbNum2 = 0;
            }
            else
            {
                rgbNum2++;
            }
            #endif
			if(YUV_photo2)
			{
				YUV_photo2 = FALSE;
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
				    sprintf(outfile_name, "rgb2_%dx%d_%04d%02d%02d%02d%02d%02d.I420",DEPTHVIDEO_W,DEPTHVIDEO_H,year,month,day,hour,min,sec);
				    fp = fopen(outfile_name,"wb");
					fwrite(Video_data,DEPTHVIDEO_W*DEPTHVIDEO_H*3/2,1,fp);
					fclose(fp);
					printf("photo2 finish \n");
				}
				else
				{
					sprintf(outfile_name, "rgb2_%dx%d_%04d%02d%02d%02d%02d%02d.YUYV",DEPTHVIDEO_W,DEPTHVIDEO_H,year,month,day,hour,min,sec);
				   fp = fopen(outfile_name,"wb");
					fwrite(Video_data,DEPTHVIDEO_W*DEPTHVIDEO_H*2,1,fp);
					fclose(fp);
					printf("photo2 finish \n");
				}
			}
	     }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    printf("-------------多线程支持多设备---------------------\n");
    printf("---------SampleCode  Version  V2.0.1------------\n");
    printf("------------------------------------------------\n");
    CLibTof* libtof1 = new CLibTof();
    int rv = libtof1->LibTOF_Connect();
    if (rv<0)
    {
        printf(" LibTOF_Connect Failed\n");
        system("pause");
        return 0;
    }

    //获取设备信息，图像宽高，设备版本信息
    DeviceInfo_t *deviceinfo = new DeviceInfo_t;//(DeviceInfo_t *)malloc(sizeof(DeviceInfo_t));
    memset(deviceinfo,0,sizeof(DeviceInfo_t));
    rv = libtof1->LibTOF_GetDeviceInfo(deviceinfo);
    if (rv<0)
    {
        printf(" LibTOF_GetDeviceInfo Failed\n");
        system("pause");
        return 0;
    }
    else
    {
        DEPTHMAP_W = deviceinfo->DepthFrameWidth;
        DEPTHMAP_H = deviceinfo->DepthFrameHeight-1;//减去一行头长度
        DEPTHVIDEO_W = deviceinfo->VisibleFrameWidth;
        DEPTHVIDEO_H = deviceinfo->VisibleFrameHeight;
    }
    DEPTHVIDEO_FRAME_SIZE = ROUND_UP(DEPTHVIDEO_W*DEPTHVIDEO_H*3/2+DEPTHVIDEO_W,1024);//β����һ��֡��Ϣ

    printf("deviceID1:%s\n",deviceinfo->DeviceId);

    //判断设备版本，设置设备支持的功能
    if (MARS03_TOF_RGB == deviceinfo->DeviceVersion || MARS04_TOF_RGB == deviceinfo->DeviceVersion)
    {
        IS_RGBD=TRUE;
        IS_RGB=TRUE;
    }

    //奇瑞项目定制参数设置
    libtof1->LibTOF_SetUseCase(MODE_4_30FPS_380);
    libtof1->LibTOF_SetExpourseMode(Auto);

    pthread_t pth_depth;
    pthread_create(&pth_depth,NULL,get_depthdata_fuc1,libtof1);

    pthread_t pth_visiable;
    pthread_create(&pth_visiable,NULL,get_visiabledata_fuc1,libtof1);

#if TEST_TOF
    CLibTof* libtof2 = new CLibTof();
    int rv2 = libtof2->LibTOF_Connect();
    if (rv2<0)
    {
        printf(" LibTOF_Connect2 Failed\n");
        system("pause");
        return 0;
    }

    DeviceInfo_t *deviceinfo2 = new DeviceInfo_t;//(DeviceInfo_t *)malloc(sizeof(DeviceInfo_t));
    memset(deviceinfo2,0,sizeof(DeviceInfo_t));
    rv2 = libtof2->LibTOF_GetDeviceInfo(deviceinfo2);
    if (rv2<0)
    {
        printf(" LibTOF_GetDeviceInfo2 Failed\n");
        system("pause");
        return 0;
    }

    printf("deviceID2:%s\n",deviceinfo2->DeviceId);

    //奇瑞项目定制参数设置
    libtof2->LibTOF_SetUseCase(MODE_4_30FPS_380);
    libtof2->LibTOF_SetExpourseMode(Auto);

    pthread_t pth_depth2;
    pthread_create(&pth_depth2,NULL,get_depthdata_fuc2,libtof2);

    pthread_t pth_visiable2;
    pthread_create(&pth_visiable2,NULL,get_visiabledata_fuc2,libtof2);
    
#endif

    IS_Running = TRUE;

    while(1)
    {
        char cmd[25];

        printf("\n===========================\n");

        printf("q: Stop\n");

        printf("\n===========================\n\n");

        get_string(cmd);
        if(strstr(cmd,"q")!=NULL)
        {
            //断开设备连接，释放内存
            IS_Running=FALSE;
            usleep(50000);//等待线程中的操作完成
            
            libtof1->LibTOF_DisConnect();
            delete deviceinfo;
            delete libtof1;

#if TEST_TOF
            libtof2->LibTOF_DisConnect();
            delete deviceinfo2;
            delete libtof2;
#endif

#if TEST_LOG
            fclose(fp_log);
#endif
            printf("disconnect \n");
            break;
        }
    }
	return 0;
}
