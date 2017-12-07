/*****************************************************************************
*                                                                            *
*  Copyright (C) 2017 Letv.                                                  *
*                                                                            *
*  This is afree software: you can redistribute it and/or modify             *
*  it under the terms of the GNU Lesser General Public License as published  *
*  by the Free Software Foundation, either version 3 of the License, or      *
*  (at your option) any later version.                                       *
*                                                                            *
*  This is distributed in the hope that it will be useful,                   *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
*  GNU Lesser General Public License for more details.                       *
*                                                                            *
*  You should see a copy of the GNU Lesser General Public License			 *
*  in <http://www.gnu.org/licenses/>.										 *
*                                                                            *
*****************************************************************************/


#include "ExportedDepthLetv.h"
#include "DepthLetv.h"

ExportedDepthLetv::ExportedDepthLetv()
{}

void ExportedDepthLetv::GetDescription( XnProductionNodeDescription* pDescription )
{
	pDescription->Type = XN_NODE_TYPE_DEPTH;
	strcpy(pDescription->strVendor, "Letv");
	strcpy(pDescription->strName, "DepthCamera");
	pDescription->Version.nMajor = 5;
	pDescription->Version.nMinor = XN_MINOR_VERSION;
	pDescription->Version.nMaintenance = XN_MAINTENANCE_VERSION;
	pDescription->Version.nBuild = XN_BUILD_VERSION;
}

XnStatus ExportedDepthLetv::EnumerateProductionTrees( xn::Context& context, xn::NodeInfoList& TreesList, xn::EnumerationErrors* pErrors )
{
	XnStatus nRetVal = XN_STATUS_OK;

	// return one option
	XnProductionNodeDescription desc;
	GetDescription(&desc);

	// Talvez devesse ser maior
	nRetVal = TreesList.Add(desc, NULL, NULL);
	XN_IS_STATUS_OK(nRetVal);


	return (XN_STATUS_OK);
}

XnStatus ExportedDepthLetv::Create( xn::Context& context, const XnChar* strInstanceName, const XnChar* strCreationInfo, xn::NodeInfoList* pNeededTrees, const XnChar* strConfigurationDir, xn::ModuleProductionNode** ppInstance )
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	DepthLetv* pDepth = new DepthLetv();

	nRetVal = pDepth->Init();
	if (nRetVal != XN_STATUS_OK)
	{
		delete pDepth;
		return (nRetVal);
	}

	*ppInstance = pDepth;

	
	return (XN_STATUS_OK);
}

void ExportedDepthLetv::Destroy( xn::ModuleProductionNode* pInstance )
{
	delete pInstance;
}