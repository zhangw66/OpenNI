#include"hardDebug.h"
#include<math.h>
#include"config.h"
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<iostream>
#include"../driverdecorate/base.h"
#include"../imagepro/Utils.h"
#include"../measure/measure.h"
#include"../common/DepthRender.hpp"
#include"../neolixMV.h"
 
namespace neolix
{

#define WINDOW_NAME "testWindow"
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))




void on_MouseHandle(int event, int x, int y, int flag, void* param);

cv::Rect g_rectangle;
cv::Mat showImg;
int left_top_point_x, left_top_point_y;
int anchor_x,anchor_y;
int width, height;
bool exit_main = false;
int rectmax[4] = { 0 };
int rectmin[4] = { 0 };

extern cv::Mat g_rect_para;
extern cv::Mat g_plane_para_least;
extern cv::Mat g_plane_ransca;
extern cv::Mat g_cammer_intrinic;
extern cv::Mat g_plane_rect;


static cv::Mat depth;


void saveRect()
{
	rectmax[0] = left_top_point_x;
	rectmax[1] = left_top_point_y;
	rectmax[2] = width;
	rectmax[3] = height;
	std::cout << "OK_Max" << std::endl;

}
void extractIn()
{

}
void saveplane()
{
	rectmin[0] = left_top_point_x;
	rectmin[1] = left_top_point_y;
	rectmin[2] = width;
	rectmin[3] = height;
	std::cout << "OK_Min" << std::endl;
}
void initConfigFile()
{
}
void fitRancas()
{
    
}

void fitLeast()
{
   
}



void cvWait_t(bool &exit_main)
{

    int key = cv::waitKey(20);
        switch (key & 0xff)
        {
        case'q':
            exit_main = true;
			destroyAllWindows();
            break;
        case's':
            saveRect();
            break;
        case'i':
            extractIn();
            break;
        case 'a':
            initConfigFile();
            break;
        case 'p':
            saveplane();
            break;
        case 'b':
            fitRancas();
            break;
        case 'c':
            fitLeast();
            break;
        case 'h':
            std::cout<<"command:\n\tq:exit\n\ts:Set the safe area\n\ti:Reading device parameters\n\ta:Initialize a configuration file (configure the first step)\n\tp:Set up the measuring platform surface\n\tc:The least squares reconstruction background\n\tb:Background of random sampling conformance reconstruction\n";
            break;
        default:
             break;
        }

}
void hardebug(depthData depth_, int rec[])
{

    std::cout<<"command:\n\tq:exit\n\ts:Set the safe area\n\ti:Reading device parameters\n\ta:Initialize a configuration file (configure the first step)\n\tp:Set up the measuring platform surface\n\tc:The least squares reconstruction background\n\tb:Background of random sampling conformance reconstruction\n";
    g_rectangle = cv::Rect(-1,-1,0,0);
    DepthRender render = DepthRender();
	void* data = malloc(depth_.width*depth_.height*sizeof(short));
	memcpy(data, (void*)depth_.data, depth_.width*depth_.height*sizeof(short));
	cv::Mat depth = cv::Mat(depth_.height, depth_.width, CV_16UC1, data);
    cv::Mat srcImage =  render.Compute(depth);
    srcImage.copyTo(showImg);
    cv::namedWindow(WINDOW_NAME);
    cv::imshow(WINDOW_NAME,srcImage);
    cv::waitKey(20);
    setMouseCallback(WINDOW_NAME,on_MouseHandle,&srcImage);

    while(!exit_main)

    { 
		cv::Mat srcCopy = srcImage.clone();
       g_rectangle = cv::Rect(left_top_point_x, left_top_point_y, width, height);
	   cv::rectangle(srcCopy, g_rectangle, cv::Scalar(128, 255, 25));
	   cv::imshow(WINDOW_NAME, srcCopy);
       cvWait_t(exit_main);
    }

	rec[0] = rectmax[0];
	rec[1] = rectmax[1];
	rec[2] = rectmax[2];
	rec[3] = rectmax[3];

	rec[4] = rectmin[0];
	rec[5] = rectmin[1];
	rec[6] = rectmin[2];
	rec[7] = rectmin[3];
	free(data);
}

void on_MouseHandle(int event, int x, int y, int flag, void* img)
{


    if(CV_EVENT_LBUTTONDOWN == event)///×ó¼ü°´ÏÂ
    {
       anchor_x = x;
       anchor_y = y;
    }else if(CV_EVENT_MOUSEMOVE == event &&(CV_EVENT_FLAG_LBUTTON & flag))
    {
        ((cv::Mat*)img)->copyTo(showImg);
        width  = abs(anchor_x - x);
        height = abs(anchor_y - y);
        left_top_point_x = min(anchor_x,x);
        left_top_point_y = min(anchor_y,y);
        g_rectangle = cv::Rect(left_top_point_x, left_top_point_y, width, height);
        cv::rectangle(showImg,g_rectangle,cv::Scalar(128,255,25));
        cv::imshow(WINDOW_NAME,showImg);
        cv::waitKey(10);
    }
}


void debug_rancas()
    {}

}
