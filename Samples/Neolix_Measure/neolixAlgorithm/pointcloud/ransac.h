
#ifndef _RANSAC_HPP_
#define _RANSAC_HPP_

#include"pointcloud.hpp"
#include<opencv2/core/core.hpp>
#include<math.h>
#include<random>
#include<float.h>
namespace  neolix{

class ransac
{
public:

	ransac(pointcloud<double,1,1> *pts);
    ~ransac();
    int  fittingPlane(double p , double w, double threshold);
    const double* getModel();
    unsigned int getLeastAdaptDatanum();
    double getBestError();

private:
	pointcloud<double,1,1> *ptcld;//观测数据
    double p;//算法产出正确结果的概率
    double w;//数据组中内点的比率，一般设置一个；鲁棒值
    double *model;//模型
    unsigned int LeastAdaptDataNum;//适用于模型的最少数据个数
    unsigned int iters;//迭代次数
    double threshold;//阈值
    long long best_points;

    double best_error;//误差


};

}
#endif // _RANSAC_HPP_
