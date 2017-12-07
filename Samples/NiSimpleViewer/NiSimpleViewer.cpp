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
#include <XnOS.h>
#if (XN_PLATFORM == XN_PLATFORM_MACOSX)
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif
#include <opencv2/opencv.hpp>
#include <math.h>

#include <XnCppWrapper.h>
using namespace xn;
//#define HAVE_IMAGE_NODE
#define HAVE_DEPTH_NODE
//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define SAMPLE_XML_PATH "../../../../Data/SamplesConfig.xml"

//#define GL_WIN_SIZE_X 1280
#define GL_WIN_SIZE_X 640
//#define GL_WIN_SIZE_Y 1024
#define GL_WIN_SIZE_Y 480

#define DISPLAY_MODE_OVERLAY	1
#define DISPLAY_MODE_DEPTH		2
#define DISPLAY_MODE_IMAGE		3
#define DEFAULT_DISPLAY_MODE	DISPLAY_MODE_IMAGE
typedef struct {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
}rgb_t;
const rgb_t pallet[] = {
	{0x00, 0xff, 0xff},
	{0x00, 0xff, 0xcc},
	{0x00, 0xff, 0x99},
	{0x00, 0xff, 0x66},
	{0x00, 0xff, 0x33},
	{0x00, 0xff, 0x00},
};
//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------
#ifdef HAVE_DEPTH_NODE
float* g_pDepthHist;
#endif
XnRGB24Pixel* g_pTexMap = NULL;
unsigned int g_nTexMapX = 0;
unsigned int g_nTexMapY = 0;
XnDepthPixel g_nZRes;

unsigned int g_nViewState = DEFAULT_DISPLAY_MODE;

Context g_context;
ScriptNode g_scriptNode;
#ifdef HAVE_DEPTH_NODE
DepthGenerator g_depth;
DepthMetaData g_depthMD;
#endif
#ifdef HAVE_IMAGE_NODE
ImageMetaData g_imageMD;
ImageGenerator g_image;
#endif
int nXRes = 0;
int nYRes = 0;
//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

void glutIdle (void)
{
	// Display the frame
	glutPostRedisplay();
}

void glutDisplay (void)
{
	XnStatus rc = XN_STATUS_OK;

	// Read a new frame
	rc = g_context.WaitAnyUpdateAll();
	if (rc != XN_STATUS_OK)
	{
		printf("Read failed: %s\n", xnGetStatusString(rc));
		return;
	}
#ifdef HAVE_DEPTH_NODE
	g_depth.GetMetaData(g_depthMD);
#endif
#ifdef HAVE_IMAGE_NODE
	g_image.GetMetaData(g_imageMD);
	//print frame ID and  timestamp
	printf("frame ID:%d, time stamp:%d\n", g_imageMD.FrameID(), g_imageMD.Timestamp());
#endif
#ifdef HAVE_DEPTH_NODE
	const XnDepthPixel* pDepth = g_depthMD.Data();
#endif
	// Copied from SimpleViewer
	// Clear the OpenGL buffers
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup the OpenGL viewpoint
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, GL_WIN_SIZE_X, GL_WIN_SIZE_Y, 0, -1.0, 1.0);
#ifdef HAVE_DEPTH_NODE
	// Calculate the accumulative histogram (the yellow display...)
	xnOSMemSet(g_pDepthHist, 0, g_nZRes*sizeof(float));
	XnDepthPixel maxDepth = 0;
	unsigned int nNumberOfPoints = 0;
	for (XnUInt y = 0; y < g_depthMD.YRes(); ++y)
	{
		for (XnUInt x = 0; x < g_depthMD.XRes(); ++x, ++pDepth)
		{
			if (*pDepth != 0)
			{
				g_pDepthHist[*pDepth]++;
				nNumberOfPoints++;
				if (*pDepth > maxDepth) 
					maxDepth = *pDepth;
			}
		}
	}
//	printf("==========MAX depth is:%d, nNumberOfPoints:%d\n", maxDepth, nNumberOfPoints);
	for (int nIndex=1; nIndex<g_nZRes; nIndex++)
	{
		g_pDepthHist[nIndex] += g_pDepthHist[nIndex-1];
	}
	if (nNumberOfPoints)
	{
		for (int nIndex=1; nIndex<g_nZRes; nIndex++)
		{
			g_pDepthHist[nIndex] = (unsigned int)((sizeof(pallet)/sizeof(pallet[0])) * (1.0f - (g_pDepthHist[nIndex] / nNumberOfPoints)));
		}
	}
#endif
	xnOSMemSet(g_pTexMap, 0, g_nTexMapX*g_nTexMapY*sizeof(XnRGB24Pixel));

#ifdef HAVE_IMAGE_NODE
	// check if we need to draw image frame to texture
	if (g_nViewState == DISPLAY_MODE_OVERLAY ||
		g_nViewState == DISPLAY_MODE_IMAGE)
	{
		const XnRGB24Pixel* pImageRow = g_imageMD.RGB24Data();
		XnRGB24Pixel* pTexRow = g_pTexMap + g_imageMD.YOffset() * g_nTexMapX;

		for (XnUInt y = 0; y < g_imageMD.YRes(); ++y)
		{
			const XnRGB24Pixel* pImage = pImageRow;
			XnRGB24Pixel* pTex = pTexRow + g_imageMD.XOffset();

			for (XnUInt x = 0; x < g_imageMD.XRes(); ++x, ++pImage, ++pTex)
			{
				*pTex = *pImage;
			}

			pImageRow += g_imageMD.XRes();
			pTexRow += g_nTexMapX;
		}
	}
	//use cv disp 
	const XnRGB24Pixel* pImageRow = g_imageMD.RGB24Data();
  	static cv::Mat image;
    cv::Mat rgb(480, 640, CV_8UC3, (void *)pImageRow);
    cv::cvtColor(rgb, image, cv::COLOR_RGB2BGR);
    if(!image.empty()){ cv::imshow("image", image); }
	cv::waitKey(3);
#endif
#ifdef HAVE_DEPTH_NODE
	/* g_pTexMap:pointer to the texture map,data type:RGB
	 * pDepthRow: pointer to the row of depth data map
	 * pTexRow:   pointer to the row of texture map 
	 * pTex: pointer to the pixel of Texture map
	 * pDepth: pointer to the pixel of row depth map
	 * */
	// check if we need to draw depth frame to texture
//	printf("the cropping area x offset:%d, y offset:%d\n", g_depthMD.XOffset(),g_depthMD.YOffset());
	if (g_nViewState == DISPLAY_MODE_OVERLAY ||
		g_nViewState == DISPLAY_MODE_DEPTH)
	{
		const XnDepthPixel* pDepthRow = g_depthMD.Data();
		XnRGB24Pixel* pTexRow = g_pTexMap + g_depthMD.YOffset() * g_nTexMapX;

		for (XnUInt y = 0; y < g_depthMD.YRes(); ++y)
		{
			const XnDepthPixel* pDepth = pDepthRow;
			XnRGB24Pixel* pTex = pTexRow + g_depthMD.XOffset();

			for (XnUInt x = 0; x < g_depthMD.XRes(); ++x, ++pDepth, ++pTex)
			{
				if (*pDepth != 0)
				{
					int color_index = g_pDepthHist[*pDepth];
					pTex->nRed = pallet[color_index].red;
					pTex->nGreen = pallet[color_index].green;
					pTex->nBlue = pallet[color_index].blue;
			//		pTex->nRed = 0xff;
		   //	    pTex->nGreen = 0xff;
			//		pTex->nBlue = 0;
				}
			}

			pDepthRow += g_depthMD.XRes();
			pTexRow += g_nTexMapX;
		}
	}
#endif
	// Create the OpenGL texture map
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_nTexMapX, g_nTexMapY, 0, GL_RGB, GL_UNSIGNED_BYTE, g_pTexMap);

	// Display the OpenGL texture map
	glColor4f(1,1,1,1);

	glBegin(GL_QUADS);
#ifdef HAVE_DEPTH_NODE
	int nXRes = g_depthMD.FullXRes();
	int nYRes = g_depthMD.FullYRes();
#endif
	// upper left
	glTexCoord2f(0, 0);
	glVertex2f(0, 0);
	// upper right
	glTexCoord2f((float)nXRes/(float)g_nTexMapX, 0);
	glVertex2f(GL_WIN_SIZE_X, 0);
	// bottom right
	glTexCoord2f((float)nXRes/(float)g_nTexMapX, (float)nYRes/(float)g_nTexMapY);
	glVertex2f(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	// bottom left
	glTexCoord2f(0, (float)nYRes/(float)g_nTexMapY);
	glVertex2f(0, GL_WIN_SIZE_Y);

	glEnd();

	// Swap the OpenGL display buffers
	glutSwapBuffers();
}

void glutKeyboard (unsigned char key, int /*x*/, int /*y*/)
{
	switch (key)
	{
		case 27:
			exit (1);
#if 1
		case '1':
			g_nViewState = DISPLAY_MODE_OVERLAY;
#ifdef HAVE_IMAGE_NODE
			g_depth.GetAlternativeViewPointCap().SetViewPoint(g_image);
#endif
			break;
		case '2':
			g_nViewState = DISPLAY_MODE_DEPTH;
			g_depth.GetAlternativeViewPointCap().ResetViewPoint();
			break;
		case '3':
			g_nViewState = DISPLAY_MODE_IMAGE;
			g_depth.GetAlternativeViewPointCap().ResetViewPoint();
			break;
#endif
		case 'm':
			g_context.SetGlobalMirror(!g_context.GetGlobalMirror());
			break;
	}
}

int main(int argc, char* argv[])
{
	XnStatus rc;

	EnumerationErrors errors;
	rc = g_context.InitFromXmlFile(SAMPLE_XML_PATH, g_scriptNode, &errors);
	if (rc == XN_STATUS_NO_NODE_PRESENT)
	{
		XnChar strError[1024];
		errors.ToString(strError, 1024);
		printf("%s\n", strError);
		return (rc);
	}
	else if (rc != XN_STATUS_OK)
	{
		printf("Open failed: %s\n", xnGetStatusString(rc));
		return (rc);
	}
#ifdef HAVE_DEPTH_NODE
	rc = g_context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_depth);
	if (rc != XN_STATUS_OK)
	{
		printf("No depth node exists! Check your XML.");
		return 1;
	}
	g_depth.GetMetaData(g_depthMD);
	nXRes = g_depthMD.FullXRes();
	nYRes = g_depthMD.FullYRes();
	g_nZRes = g_depthMD.ZRes();
	g_pDepthHist = (float*)malloc(g_nZRes * sizeof(float));
	printf("z resolution:%d, DepthHist addr:%#x\n", g_nZRes, g_pDepthHist);
#endif
#ifdef HAVE_IMAGE_NODE
	rc = g_context.FindExistingNode(XN_NODE_TYPE_IMAGE, g_image);
	if (rc != XN_STATUS_OK)
	{
		printf("No image node exists! Check your XML.");
		return 1;
	}
	g_image.GetMetaData(g_imageMD);
//	// Hybrid mode isn't supported in this sample
//	if (g_imageMD.FullXRes() != g_depthMD.FullXRes() || g_imageMD.FullYRes() != g_depthMD.FullYRes())
//	{
//		printf ("The device depth and image resolution must be equal!\n");
//		return 1;
//	}
	// RGB is the only image format supported.
	if (g_imageMD.PixelFormat() != XN_PIXEL_FORMAT_RGB24)
	{
		printf("The device image format must be RGB24\n");
		return 1;
	}
	nXRes = g_imageMD.FullXRes();
	nYRes = g_imageMD.FullYRes();
#endif
	// Texture map init
	g_nTexMapX = (((unsigned short)(nXRes-1) / 512) + 1) * 512;
	g_nTexMapY = (((unsigned short)(nYRes-1) / 512) + 1) * 512;
	g_pTexMap = (XnRGB24Pixel*)malloc(g_nTexMapX * g_nTexMapY * sizeof(XnRGB24Pixel));
	printf("x-res:%d, y-res:%d\n", nXRes, nYRes);
	printf("texture map x:%d, y%d, addr:%#x\n", g_nTexMapX, g_nTexMapY, g_pTexMap);

	// OpenGL init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	glutCreateWindow ("OpenNI Simple Viewer");
	//glutFullScreen();
	glutSetCursor(GLUT_CURSOR_NONE);

	glutKeyboardFunc(glutKeyboard);
	glutDisplayFunc(glutDisplay); //
	glutIdleFunc(glutIdle);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	// Per frame code is in glutDisplay
	glutMainLoop();

	return 0;
}
