#ifndef _GETFEATURES_HPP_
#define _GETFEATURES_HPP_


#include "../common/common.hpp"
#include"camdecorate.h"


namespace neolix
{
static char     buffer[1024*1024];
static int32_t  n;

void dumpFeature(TY_DEV_HANDLE handle, TY_COMPONENT_ID compID, TY_FEATURE_ID featID, const char* name)
{
	TY_FEATURE_INFO featInfo;
	ASSERT_OK(TYGetFeatureInfo(handle, compID, featID, &featInfo));

	if(featInfo.isValid && (featInfo.accessMode&TY_ACCESS_READABLE)){

		if(TYFeatureType(featID) == TY_FEATURE_INT){
			ASSERT_OK(TYGetInt(handle, compID, featID, &n));

		}
		if(TYFeatureType(featID) == TY_FEATURE_FLOAT){
			float v;
			ASSERT_OK(TYGetFloat(handle, compID, featID, &v));
		}
		if(TYFeatureType(featID) == TY_FEATURE_ENUM){
			ASSERT_OK(TYGetEnumEntryCount(handle, compID, featID, &n));
			if(n > 0){
				TY_ENUM_ENTRY* pEntry = new TY_ENUM_ENTRY[n];
				ASSERT_OK(TYGetEnumEntryInfo(handle, compID, featID, pEntry, n, &n));

				delete [] pEntry;
			}
		}
		if(TYFeatureType(featID) == TY_FEATURE_BOOL){
			bool v;
			ASSERT_OK(TYGetBool(handle, compID, featID, &v));

		}
		if(TYFeatureType(featID) == TY_FEATURE_STRING){
			ASSERT_OK(TYGetStringBufferSize(handle, compID, featID, &n));
;
			ASSERT_OK(TYGetString(handle, compID, featID, buffer, sizeof(buffer)));

		}
#if 0
		if(TYFeatureType(featID) == TY_FEATURE_BYTEARRAY){
			ASSERT_OK(TYGetByteArraySize(handle, compID, featID, &n));

			n = sizeof(buffer);
			ASSERT_OK(TYGetByteArray(handle, compID, featID, (uint8_t*)buffer, n, &n));

		}
#endif
		if(TYFeatureType(featID) == TY_FEATURE_STRUCT){
			switch(featID){
			case TY_STRUCT_CAM_INTRINSIC:{
				TY_CAMERA_INTRINSIC* p = (TY_CAMERA_INTRINSIC*)buffer;
				ASSERT_OK(TYGetStruct(handle, compID, featID, (void*)p
					, sizeof(TY_CAMERA_INTRINSIC)));
				return;
										 }
			case TY_STRUCT_EXTRINSIC_TO_LEFT_IR:
			case TY_STRUCT_EXTRINSIC_TO_LEFT_RGB: {
				TY_CAMERA_EXTRINSIC* p = (TY_CAMERA_EXTRINSIC*)buffer;
				ASSERT_OK(TYGetStruct(handle, compID, featID, (void*)p
					, sizeof(TY_CAMERA_EXTRINSIC)));
				return;
												  }
			case TY_STRUCT_CAM_DISTORTION:{
				TY_CAMERA_DISTORTION* p = (TY_CAMERA_DISTORTION*)buffer;
				ASSERT_OK(TYGetStruct(handle, compID, featID, (void*)p
					, sizeof(TY_CAMERA_DISTORTION)));
				return;
										  }
			default:
				LOGD("===         %s: Unknown struct", name);
				return;
			}
		}
	}
}

#define DUMP_FEATURE(handle, compid, feature)  dumpFeature( (handle), (compid), (feature) , #feature );


#define DUMP_COMPONENT(handle,compIds,id) do{\
	if(compIds & id){\
	LOGD("===  %s:",#id);\
	dumpComponentFeatures(handle, id);\
	}\
}while(0)


int getfeatures(cv::Mat &K, TY_DEV_HANDLE &evicehandle)
{
    //const char* ID = NULL;
	TY_DEV_HANDLE handle = evicehandle;
/*
	// Init lib
	ASSERT_OK(TYInitLib());
	TY_VERSION_INFO* pVer = (TY_VERSION_INFO*)buffer;
	ASSERT_OK( TYLibVersion(pVer) );
	LOGD("=== lib version: %d.%d.%d", pVer->major, pVer->minor, pVer->patch);

    TY_DEVICE_BASE_INFO* pBaseInfo = (TY_DEVICE_BASE_INFO*)buffer;

    // Get device info
    ASSERT_OK(TYGetDeviceNumber(&n));
    LOGD("=== device number %d", n);

    ASSERT_OK(TYGetDeviceList(pBaseInfo, 100, &n));

    if(n == 0){
        LOGD("=== No device got");
        TYDeinitLib();
        return -1;
    }
    ID = pBaseInfo[0].id;
    LOGD("=== Open device: %s", ID);
    ASSERT_OK(TYOpenDevice(ID, &handle));

*/
	// List all components
	int32_t compIDs;
	ASSERT_OK(TYGetComponentIDs(handle, &compIDs));
	DUMP_FEATURE(handle, TY_COMPONENT_DEPTH_CAM, TY_STRUCT_CAM_INTRINSIC );
	TY_CAMERA_INTRINSIC* p = (TY_CAMERA_INTRINSIC*)buffer;
	//cv::Mat M1=(cv::Mat_<double>(3,3) << p->data[0],p->data[1],p->data[2],p->data[3],p->data[4],p->data[5],p->data[6],p->data[7],p->data[8]);
	K = (cv::Mat_<double>(4,1)<<p->data[0],p->data[4],p->data[2],p->data[5]);
	return 0;
}
}
#endif // GETFEATURES_HPP_
