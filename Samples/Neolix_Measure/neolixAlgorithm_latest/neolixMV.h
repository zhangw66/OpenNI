#ifndef __MEOLIX_MV_H__
#define __MEOLIX_MV_H__


/*
	unit:mm
*/


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
	 //internalCoefficient 内参 3个
   /*
   if (setArea()) 
   	if (backgroundReconstruction())  //OUT__ double parameter[]需要使用者分配空间.
   	 	setParameter()
   */
   bool setArea(rect safeZone, rect planeZone,IN__ double internalCoefficient[],int n);
   bool backgroundReconstruction(depthData depth,OUT__ double parameter[],int &n, int method = 0);
   void setParameter(IN__ double backgroundParameter[], int method = 0);
   bool measureVol(depthData depth,vol &v, int method = 0);
   void getDepthColor(depthData depth,void *buff);
   //微调长宽高
   void dajustVol(short fix_lenght = 0, short fix_width = 0, short fix_height = 0);
   void hardebug(depthData depth_, int rec[]);
#ifdef __cplusplus
}
#endif // __cplusplus
}
#endif // __MEOLIX_MV_H__
