#ifndef __MEASURE_H__
#define __MEASURE_H__
#include<opencv2/core/core.hpp>
#include"../driverdecorate/base.h"
namespace neolix
{
#define IN_
#define OUT_
NEOLIX_STATUS_LIST measureBox(IN_ const cv::Mat depthImage,
				IN_  const double  planePar[3],
				OUT_ float &length,
				OUT_ float &width,
				OUT_ float &height,
				IN_  bool  visual = false,
				IN_  bool  useRect = true);
}
#endif
