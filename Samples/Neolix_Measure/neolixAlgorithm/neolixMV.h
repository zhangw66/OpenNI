#ifndef __MEOLIX_MV_H__
#define __MEOLIX_MV_H__
#define DLLTEST_API
#ifdef DLLTEST_API  
#else   
#define DLLTEST_API _declspec(dllimport)   
#endif 



namespace neolix
{

#define OUT__
#define IN__


	typedef  struct RECT
	{
		int left_x;
		int left_y;
		int width;
		int height;
	}rect;

	typedef struct VOLUME
	{
		float length;
		float width;
		float height;
	}vol;
	typedef struct DEPTHDADA
	{
		void* data;//(short*)
		int width;
		int height;
	}depthData;

#ifdef __cplusplus

	extern "C"
	{
#endif // __cplusplus
		DLLTEST_API  bool setArea(rect safeZone, rect planeZone, IN__ double internalCoefficient[], int n);
		DLLTEST_API  bool backgroundReconstruction(depthData depth, OUT__ double parameter[], int &n, int method = 0);
		DLLTEST_API  void setParameter(IN__ double backgroundParameter[], int method = 0);
		DLLTEST_API  bool measureVol(depthData depth, vol &v, int method = 0);
		DLLTEST_API  void hardebug(depthData depth_, int rec[]);
#ifdef __cplusplus
	}
#endif // __cplusplus
}
#endif // __MEOLIX_MV_H__
