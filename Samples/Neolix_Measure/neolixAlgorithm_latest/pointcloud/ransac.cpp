#include"ransac.h"

#include"../driverdecorate/base.h"

namespace neolix
{


//////////////////////////ransac///////////////////////////

ransac::ransac(pointcloud<double,1,1>* pts)
{
    this->ptcld = pts;
    this->model = nullptr;
}
ransac::~ransac()
{
    if(this->model != nullptr)
    {
        delete[] model;
        model = nullptr;
    }
}
int  ransac::fittingPlane(double p, double w, double threshold)
{
    this->p = p ;
    this->w = w;
    this->threshold = threshold;
    int numPar = 3;//平面模型的参数有3个
    int index[3];
   // double pt[9]
    delete[] model;
    model = new double[numPar];

	this->iters = static_cast<unsigned int>((std::log(1-p))/(std::log(1 - (std::pow(w,numPar)) )));
   // std::random_device rd;
   // std::default_random_engine e(rd());
    std::default_random_engine e(time(NULL));
    std::uniform_int_distribution<> u(0,ptcld->getSize()-1);
	unsigned int count = 0;
    int temp = 0;
    cv::Mat L_Eq, R_Eq;
    cv::Mat Plane;
    best_error = 1.0;//最大
    best_points = 0;
    long long current_best_points = 0;
    long long totalDepth = 0;
    long      currentAvgDepth = 999999999999999;
    long      bestDepth =       9999;
    double current_error = DBL_MAX;
    LeastAdaptDataNum = static_cast<unsigned int>(ptcld->getSize()*(w*0.5));
    double diffDis, dis_sub,dis_up;
    while(count < iters)
    {
       std::cout<<std::endl<<std::endl<<std::endl<<std::endl<<"++++++++++++++++iters"<<count<<"++++++++++++++++++"<<std::endl<<std::endl;
        //找非三点共线的点
        int inner_iter = 50;
        while(inner_iter--)
        {
        int innner_iters = 50;
     //   std::cout<<"===============find three rands points============="<<<<std::endl<<std::endl;
        index[0] = u(e);
        while( index[0]== (index[1] = u(e)) &&  innner_iters--) ;
        temp = u(e);
        innner_iters = 50;
        while(( (temp == index[0]) || (temp == index[1]) )&&innner_iters-- ) temp = u(e);
        if(innner_iters <= 0) continue;
		index[2] = temp;

       std::cout<<"three rands poins: "<<std::endl;
		std::cout<<(ptcld->operator[](index[0])) <<std::endl
			<<(ptcld->operator[](index[1]))<<std::endl
			<<(ptcld->operator[](index[2]))<<std::endl<<std::endl
			<<"size"<<ptcld->getSize()<<std::endl;;

		Point3D<double> pt1((ptcld->operator[](index[0]))[0] -  (ptcld->operator[](index[1]))[0],
							(ptcld->operator[](index[0]))[1] -  (ptcld->operator[](index[1]))[1],
							(ptcld->operator[](index[0]))[2] -  (ptcld->operator[](index[1]))[3]);

        Point3D<double> pt2((ptcld->operator[](index[0]))[0] -  (ptcld->operator[](index[2]))[0],
							(ptcld->operator[](index[0]))[1] -  (ptcld->operator[](index[2]))[1],
							(ptcld->operator[](index[0]))[2] -  (ptcld->operator[](index[2]))[3]);
		//std::cout<<pt1<<std::endl<<pt2<<std::endl<<std::endl<<std::endl;
		//std::cout<<"dot = "<<pt1.dot(pt2)<<std::endl<<"model1= "<<pt1.model()<<"  model2="<<pt2.model()<<std::endl;
        double cos_val = pt1.dot(pt2)/(pt1.model()*pt2.model());
		//					std::cout<<"cosval= "<<cos_val<<std::endl;
        if(cos_val > 0 && std::abs(cos_val - 1.0) < 0.0000001 )continue;
        if(cos_val < 0 && std::abs(cos_val + 1.0) < 0.0000001) continue;
        break;

        }

       if( inner_iter <= 0)
       {
           count++;
           continue;
       }
        L_Eq = (cv::Mat_<double>(3,3)<<(double)(((*ptcld)[index[0]])[0]),
                                       ( (*ptcld) [index[0]] )[1],1,
                                       ( (*ptcld) [index[1]] )[0],
                                       ( (*ptcld) [index[1]] )[1],1,
                                       ( (*ptcld) [index[2]] )[0],
                                       ( (*ptcld) [index[2]] )[1],1);

        R_Eq = (cv::Mat_<double>(3,1)<<( ((*ptcld)[index[0]])[2]),
                                       ( ((*ptcld)[index[1]])[2]),
                                       ( ((*ptcld)[index[2]])[2]));

       cv::solve(L_Eq,R_Eq,Plane,CV_SVD);
       std::cout<<"Plane"<<Plane<<std::endl;
	   dis_sub  = std::sqrt((std::pow(Plane.at<double>(0,0),2) + std::pow(Plane.at<double>(1,0),2) + 1.0));
	  /// std::cout<<"cal depth= "<<-(( (*ptcld)[index[0]])[0]*Plane.at<double>(0,0)+
         /// ( (*ptcld)[index[0]])[1]*Plane.at<double>(1,0)+
        ///  Plane.at<double>(2,0))<<std::endl;
       current_best_points = 0;
     //  dis_up = Plane.at<double>(0,0)*((*ptcld)[index[0]])[0]+ Plane.at<double>(1,0)*((*ptcld)[index[0]])[1] + Plane.at<double>(2,0) +((*ptcld)[index[0]])[2] ;
    //   dis_up = Plane.at<double>(0,0)*((*ptcld)[index[0]])[0]+ Plane.at<double>(1,0)*((*ptcld)[index[0]])[1] + Plane.at<double>(2,0) +((*ptcld)[index[0]])[2]<<std::endl;

     //  diffDis = std::abs(dis_up)/dis_sub;
     //  std::cout<<"dis_up= "<<dis_up<<std::endl;
     //  std::cout<<"dis_sub= "<<dis_sub<<std::endl;
    //   std::cout<<"cal diffDis= "<<diffDis<<std::endl;
      // std::cout<<"ptcd nums = "<<ptcld->getSize()<<std::endl;
     /// std::ofstream ofile;
     ///ofile.open("watch.txt");
      totalDepth = 0;
       for(size_t i = 0; i < ptcld->getSize(); i++)
       {
		   /*
          diffDis = ( (*ptcld)[i])[0]*Plane.at<double>(0,0)+
          ( (*ptcld)[i])[1]*Plane.at<double>(1,0)+
          Plane.at<double>(2,0) - ( (*ptcld)[i])[2];
		  */
		  dis_up = Plane.at<double>(0,0)*((*ptcld)[i])[0] + Plane.at<double>(1,0)*((*ptcld)[i])[1] + Plane.at<double>(2,0) + ((*ptcld)[i])[2];
          //dis_up = Plane.at<double>(0,0)*((*ptcld)[i])[0] + Plane.at<double>(1,0)*((*ptcld)[i])[1] + Plane.at<double>(2,0) + ((*ptcld)[i])[2] ;
		  diffDis = std::abs(dis_up)/dis_sub;

		/// std::cout<<"diffDis"<<diffDis<<std::endl;
		/// ofile<<"calDis = "<<diffDis<<"  depth+ "<<(*ptcld)[i][2]<<std::endl;
          if(std::abs(diffDis) < threshold)
          {
              current_best_points++;
              totalDepth += (*ptcld)[i][2];
          }
       }
      /// ofile.close();
       current_error = 1 - (double)(current_best_points)/ptcld->getSize();
       currentAvgDepth = totalDepth/current_best_points;
       if( current_best_points > LeastAdaptDataNum && current_error> 0 && current_error < best_error && currentAvgDepth < bestDepth)
       {
           model[0] = Plane.at<double>(0,0);
           model[1] = Plane.at<double>(1,0);
           model[2] = Plane.at<double>(2,0);
           best_points = current_best_points;
           best_error  = current_error;
           bestDepth = currentAvgDepth;

       }

       std::cout<<"best_plane = ["<<model[0]<<", "<<model[1]<<", "<<model[2]<<"]"<<std::endl;
	   std::cout<<"current_plane = "<<std::endl<<Plane<<std::endl;
	   std::cout<<"best_error = "<<best_error<<std::endl;
	   std::cout<<"current_error = "<<current_error<<std::endl;
	   std::cout<<"best_depth = "<<bestDepth<<std::endl;
	   std::cout<<"current_depth = "<<currentAvgDepth<<std::endl;
	   std::cout<<"total points = "<<ptcld->getSize()<<std::endl;
	   std::cout<<"inner points = "<<current_best_points<<std::endl;


       if(best_error < 0.7)
        break;
       count++;

    }
    if(iters <= 0)
        return NEOLIX_FALSE;
	if(best_error < 0.7 )
		return NEOLIX_SUCCESS;
	else
	{
		return NEOLIX_FALSE;
	}
	return NEOLIX_SUCCESS;

}

 const double* ransac::getModel()
    {
        return this->model;
    }

 unsigned int ransac::getLeastAdaptDatanum()
    {
        return this->LeastAdaptDataNum;
    }

 double ransac::getBestError()
    {
        return best_error;
    }



}
