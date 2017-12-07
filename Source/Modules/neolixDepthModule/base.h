#pragma once
#include <iostream>
#include <stdio.h>
//#include <windows.h>   //platform specific
#include <math.h>
//#include <fstream>
//using namespace std;

#include <XnCppWrapper.h>
#include <XnModuleCppInterface.h>
#include <XnEvent.h>
#include <XnLog.h>
#include <XnOS.h>
using namespace xn;

//#include <chrono>
//using namespace std::chrono;



#define HRESULT long //zhangw add
#define S_OK ((HRESULT)0l) //zhangw add
#define S_FALSE ((HRESULT)1l) //zhangw add
//#include "opencv2/opencv.hpp"

struct XnStatusException {
	const XnStatus nStatus;
	const HRESULT hResult;

	XnStatusException(XnStatus n, HRESULT hr = S_OK) : nStatus(n), hResult(hr) {}
};

#define CHECK_XN_STATUS(statement) \
{ \
	XnStatus __status = (statement); \
	if (__status != XN_STATUS_OK) { xnPrintError(__status, #statement); throw XnStatusException(__status); } \
}
inline void printHResult(HRESULT hr, const char* statement)
{
	 
	fprintf(stderr, "Failed: [%08x] (%s)\n", hr, statement);
}

#define CHECK_HRESULT(statement) \
{ \
	HRESULT __hr = (statement); \
	if (FAILED(__hr)) { printHResult(__hr, #statement); throw XnStatusException(XN_STATUS_ERROR, __hr); } \
}

#define LOG(format, ...) fprintf(stderr, format, __VA_ARGS__)


#define MAX_USERS		 8
#define PI 3.1415926f
#define DUMMY_FOV 1.35f
#define SUPPORTED_FPS 30
#define MAX_DEPTH_VALUE	10000

#define TEST_HUMANPLUS	0
#define TEST_INUITIVE	1
#define TEST_ADI		2
#define TEST_ESPDI		3
#define TEST_PRIMESENSE	4
#define TEST_SUNNYMARS	5
#define TEST_TY			6

#define TEST_CASE 6//TEST_SUNNYMARS



#if TEST_CASE == TEST_HUMANPLUS
#define SUPPORTED_X_RES 752 
#define SUPPORTED_Y_RES 480
#define HFOV DUMMY_FOV
#define VFOV DUMMY_FOV
 
#elif TEST_CASE == TEST_INUITIVE
#define SUPPORTED_X_RES 1264
#define SUPPORTED_Y_RES 752
#define HFOV ( 57.5f * PI / 180.0f )
#define VFOV ( 43.5f * PI / 180.0f )
 
#elif TEST_CASE == TEST_ADI
#define SUPPORTED_X_RES 640
#define SUPPORTED_Y_RES 480
#define HFOV DUMMY_FOV
#define VFOV DUMMY_FOV

#elif TEST_CASE == TEST_ESPDI
#define SUPPORTED_X_RES 1280
#define SUPPORTED_Y_RES 720
#define HFOV DUMMY_FOV
#define VFOV DUMMY_FOV

#elif TEST_CASE == TEST_PRIMESENSE
#define SUPPORTED_X_RES 320
#define SUPPORTED_Y_RES 240
#define HFOV DUMMY_FOV
#define VFOV DUMMY_FOV
#elif TEST_CASE == TEST_SUNNYMARS
#define SUPPORTED_X_RES 224
#define SUPPORTED_Y_RES 172
#define HFOV DUMMY_FOV
#define VFOV DUMMY_FOV
#elif TEST_CASE == TEST_TY
#define SUPPORTED_X_RES 640
#define SUPPORTED_Y_RES 480
#define HFOV DUMMY_FOV
#define VFOV DUMMY_FOV
#endif

inline int GetXRes() { return SUPPORTED_X_RES; }
inline int GetYRes() { return SUPPORTED_Y_RES; }
inline int GetFPS() { return SUPPORTED_FPS; }
