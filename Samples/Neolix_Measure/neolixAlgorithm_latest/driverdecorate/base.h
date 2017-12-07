#ifndef NEOLIX_BASE_H
#define NEOLIX_BASE_H
#include <opencv2/opencv.hpp>
namespace neolix{

//==========测量平台到摄像头九个区域的距离=====
#define  AREA1 1610.0f
#define  AREA2 1609.0f
#define  AREA3 1603.0f
#define  AREA4 1624.0f
#define  AREA5 1618.0f
#define  AREA6 1614.0f
#define  AREA7 1638.0f
#define  AREA8 1637.0f
#define  AREA9 1630.0f

//==========测量平台到摄像头九个区域的中心=====

#define  CENTER1 53,38
#define  CENTER2 56,115
#define  CENTER3 56,192
#define  CENTER4 168,38
#define  CENTER5 168,115
#define  CENTER6 168,192
#define  CENTER7 279,38
#define  CENTER8 279,115
#define  CENTER9 279,192

//=========系统校准参数=========
#define MEADSURING_TABLE_STDDEV 0.1f//


	typedef enum neolix_status
	{
		NEOLIX_SUCCESS = 0,
		NEOLIX_FALSE,
		NEOLIX_CANOT_FIND_CONTOURS,

	}NEOLIX_STATUS_LIST;
#include <stdio.h>
  //  #define LOGD0(_str, ...) do{printf(_str , ## __VA_ARGS__); printf("\n");fflush(stdout);} while(0)
	#define LOGINFO(_str, ...) do{printf(_str, ## __VA_ARGS__); printf("\n");fflush(stdout);} while (0)
	#define LOGERROR(_str, ...) do{printf(_str, ## __VA_ARGS__); printf("\n");fflush(stdout);} while (0)
	#define LOGINFO_TO_FILE(_logFileStream,logLine) do \
		{\
		(_logFileStream)<<("LOG  INFO: ")<<logLine<<std::endl;\
	} while (0)

	#define LOGERROR_TO_FILE(_logFileStream,logLine) do \
		{\
		(_logFileStream)<<("LOG  ERROR: ")<<logLine<<std::endl;\
	} while (0)


	#define CHECK_OK(x) do \
	{\
		int status = x;\
		if(status != NEOLIX_SUCCESS) \
			LOGINFO("check fail, error code is: %d",status)\
	} while (0)



}
#endif

