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


#include <XnModuleCppInterface.h>

class ExportedDepthLetv : public xn::ModuleExportedProductionNode
{
public:
	ExportedDepthLetv();
	virtual void GetDescription(XnProductionNodeDescription* pDescription);
	virtual XnStatus EnumerateProductionTrees(xn::Context& context, xn::NodeInfoList& TreesList, xn::EnumerationErrors* pErrors);
	virtual XnStatus Create(xn::Context& context, const XnChar* strInstanceName, const XnChar* strCreationInfo, xn::NodeInfoList* pNeededTrees, const XnChar* strConfigurationDir, xn::ModuleProductionNode** ppInstance);
	virtual void Destroy(xn::ModuleProductionNode* pInstance);
};