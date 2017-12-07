#ifndef __CONFIG_H_
#define __CONFIG_H_
#include <string>
#include<vector>
#include<opencv2/core/core.hpp>
#include"../imagepro/Utils.h"
namespace neolix{

    enum STORM_FLAG{
        NEOLIX_RECT = 0,
        NEOLIX_PLANE_FITTING_RACSCA,
        NEOLIX_PLANE_FITTING_LESRT,
        NEOLIX_CAMMER_INTRINSIC_TY,
        NEOLIX_CAMMER_INTRINSIC_ADI_TOF,
        NEOLIX_MEASURE_FLAT,
        NEOLIX_UNKONG = 10000,
    };
    const string configNames[] = {
        "rect",
        "panle_coeffic_racsca",
        "plane_coeffic_least",
        "ty_intrinsic",
        "adi_tof_intrinsic",
        "plane"};
const int flagNunber = 6;
void splitNareaFromPad(cv::Mat DepthImage, int n,  std::vector<neolix::index_value<int,cv::Point2i>> &centerPoints, std::vector<short> &distances);
void stormToXML(const std::vector<float> &distances, std::vector<neolix::index_value<int,cv::Point2i>> &centerPoints, std::string filename = "configInfo.xml");
void readFromXml(std::vector<float> &distances, std::vector<neolix::index_value<int, cv::Point2i>> &centerPoints, std::string filename);
void UptateComfigToXml(cv::Mat &data, STORM_FLAG flag);
void initEmptyMatConfigFile();
NEOLIX_STATUS_LIST config();
}
#endif // __CONFIG_H_
