#ifndef __POINT_CLOUD_HPP_
#define __POINT_CLOUD_HPP_

#include<iostream>
#include<iomanip>
#include<math.h>
#include<opencv2/core/core.hpp>
#include<vector>
namespace neolix{
    template<typename Tp, int cnt> class Point_;
    template<typename Tp> class Point3D;
    template<typename Tp,unsigned int row, unsigned int col >class pointcloud;



    template<typename _Tp> std::ostream& operator<<(std::ostream& os,const Point3D<_Tp> &point)
    {
        os<<"["<<point.data[0]<<","<<point.data[1]<<","<<point.data[2]<<"]";
        return os;
    }


    template<typename Tp, int cnt>

    class Point_{

    public:
            typedef Tp value_type;
            enum{size = cnt};
            Point_();
            virtual  ~Point_();
			explicit Point_(const Point_<Tp,cnt> &pt);
            Point_(Tp v0);
            Point_(Tp v0, Tp v1);
            Point_(Tp v0, Tp v1, Tp v2);


            explicit Point_(const Tp *value);


            //点积
           Tp dot(const Point_<Tp,cnt> &pt) const;
           Tp model();

            value_type& operator[](int index);
            value_type* getPoint()
            {
                return data;
            }
            value_type *data;

    };

template<typename Tp>
class Point3D:public Point_<Tp,3>
{
    public:
          Point3D();
          Point3D(Tp value);
          Point3D(Tp x, Tp y, Tp z);
		  ~Point3D(){};
		  explicit Point3D(const Point3D<Tp> &pt3d);

          template<typename _Tp> friend std::ostream& operator<<(std::ostream& os,const Point3D<_Tp> &point);
          explicit Point3D(const Tp *value);


};

template<typename Tp,unsigned int row=1, unsigned int col = 1>
class pointcloud
{
    public:
        pointcloud();
        pointcloud(Point3D<Tp> pt0);
        pointcloud(Point3D<Tp> pt0, Point3D<Tp> pt1);
        pointcloud(Point3D<Tp> pt0, Point3D<Tp> pt1, Point3D<Tp> pt2);
        pointcloud(Point3D<Tp> pt0, Point3D<Tp> pt1, Point3D<Tp> pt2, Point3D<Tp> pt3);
        pointcloud(Point3D<Tp> pt0, Point3D<Tp> pt1, Point3D<Tp> Pt2, Point3D<Tp> pt3, Point3D<Tp> pt4);
        pointcloud(Point3D<Tp> pt0, Point3D<Tp> pt1, Point3D<Tp> Pt2, Point3D<Tp> pt3, Point3D<Tp> pt4, Point3D<Tp> pt5);

        virtual Point3D<Tp>& operator[](unsigned int i);
        virtual Point3D<Tp>* ptr();
		virtual Point3D<Tp>* ptr(unsigned int r);
        virtual size_t getSize()
        {
            return size;
        }
        virtual void resize(unsigned long size_, unsigned int row_);
		int converToPointCloud(cv::Mat depthImage ,double fu, double fv, double u0, double v0);
        template<typename TTp, unsigned int _Trow, unsigned int _Tcol> friend std::ostream& operator<<(std::ostream& os,const pointcloud<TTp,_Trow,_Tcol> &points);
        pointcloud(const pointcloud<Tp,row,col> &ptcld);
        ~pointcloud();

        unsigned rows;
        unsigned cols;
        Point3D<Tp> *data;
		std::vector<Point3D<double>> points;

    private:
        unsigned long size;
};


/////////////////////Point_/////////////////////////

    template<typename Tp, int cnt> inline Point_<Tp,cnt>::Point_()
    {
        data = new Tp[cnt];
        for(unsigned int i =0; i < cnt; i++)data[i] = 0;
    }


     template<typename Tp, int cnt> inline Point_<Tp, cnt>::~Point_()
    {
        if(data != nullptr)
        {
            delete[] data;
            data = nullptr;
        }
    }

    template<typename Tp, int cnt> inline Point_<Tp, cnt>::Point_(const Point_<Tp,cnt> &pt)
    {

        data = new Tp[pt.size];
        for(int i = 0;i < pt.size; i++ )data[i] = pt.data[i];
    }

    template<typename Tp, int cnt> inline Point_<Tp, cnt>::Point_(Tp v0)
    {
        if(cnt < 1) throw "neolix Point_ 指针越界";
        data = new Tp[cnt];
        data[0] = v0;
    }

    template<typename Tp, int cnt> inline Point_<Tp, cnt>::Point_(Tp v0, Tp v1)
    {
        if(cnt < 2) throw "neolix Point_ 指针越界";
        data = new Tp[cnt];
        data[0] = v0;
        data[1] = v1;
    }

    template<typename Tp, int cnt> inline Point_<Tp, cnt>::Point_(Tp v0, Tp v1, Tp v2)
    {
        if(cnt < 3) throw "neolix Point_ 指针越界";
        data = new Tp[cnt];
        data[0] = v0;
        data[1] = v1;
        data[2] = v2;
    }



    template<typename Tp, int cnt> inline Point_<Tp, cnt>::Point_(const Tp *value)
    {
        data = new Tp[cnt];
        for(int i = 0; i < cnt; i++)data[i] = value[i];
    }

    template<typename Tp, int cnt> inline Tp Point_<Tp, cnt>::dot(const Point_<Tp,cnt> &pt) const
    {
        Tp result = 0;
        for(int i = 0; i < cnt; i++)
        {
            result += data[i] * pt.data[i];
        }
        return result;
    }
    template<typename Tp, int cnt> inline Tp Point_<Tp, cnt>::model()
    {
        Tp result = 0;
        for(int i = 0; i < cnt; i++)
        {
            result += std::pow(data[i],2);
        }
        return std::sqrt(result);
    }

    template<typename Tp, int cnt> inline Tp& Point_<Tp, cnt>:: operator[](int index)
    {
        if(index > cnt) throw "neolix Point_ 指针越界";

        return this->data[index];
    }

//////////////////////////////Point3D////////////////////////////////

 template<typename Tp> inline Point3D<Tp>::Point3D() { }
 template<typename Tp> inline Point3D<Tp>::Point3D(Tp value):Point_<Tp,3>(value, value, value){}
 template<typename Tp> inline Point3D<Tp>::Point3D(Tp x, Tp y, Tp z):Point_<Tp,3>(x,y,z){};
 template<typename Tp> inline Point3D<Tp>::Point3D(const Point3D<Tp> &pt3d):Point_<Tp,3>(pt3d){}


///////////////////////////pointcloud/////////////////////////////

template<typename Tp, unsigned int row, unsigned col > pointcloud<Tp,row,col>::pointcloud():size(row*col),rows(row),cols(col)
{
    data = new Point3D<Tp>[size];

}

template<typename Tp, unsigned int row, unsigned col > void  pointcloud<Tp,row,col>::resize(unsigned long size_, unsigned int row_)
        {
            if(data) delete[] data;
            data = new Point3D<Tp>[size_];
            rows = row_;
            cols = size_/rows;
            this->size = size_;
        }
template<typename Tp, unsigned int row, unsigned col > pointcloud<Tp,row,col>::pointcloud(Point3D<Tp> pt0):size(row*col),rows(row),cols(col)
{
    if(size < 1) throw "neolix pointcloud 指针越界";
    data = new Point3D<Tp>[size];
    data[0] = pt0;
}

template<typename Tp, unsigned int row, unsigned col > pointcloud<Tp,row,col>::pointcloud(Point3D<Tp> pt0, Point3D<Tp> pt1):size(row*col),rows(row),cols(col)
{
    if(size < 2) throw "neolix pointcloud 指针越界";
    data = new Point3D<Tp>[size];
    data[0] = pt0;
    data[1] = pt1;
}

template<typename Tp, unsigned int row, unsigned col> pointcloud<Tp,row,col>::pointcloud(Point3D<Tp> pt0, Point3D<Tp> pt1, Point3D<Tp> pt3):size(row*col),rows(row),cols(col)
{
    if(size < 3) throw "neolix pointcloud 指针越界";
    data = new Point3D<Tp>[size];
    data[0] = pt0;
    data[1] = pt1;
    data[2] = pt3;
}

template<typename Tp, unsigned int row, unsigned col> pointcloud<Tp,row,col>::pointcloud(Point3D<Tp> pt0, Point3D<Tp> pt1, Point3D<Tp> pt3, Point3D<Tp> pt4):size(row*col),rows(row),cols(col)
{
    if(size < 4) throw "neolix pointcloud 指针越界";
    data = new Point3D<Tp>[size];
    data[0] = pt0;
    data[1] = pt1;
    data[2] = pt3;
    data[3] = pt4;
}

template<typename Tp, unsigned int row, unsigned col> pointcloud<Tp,row,col>::pointcloud(Point3D<Tp> pt0, Point3D<Tp> pt1, Point3D<Tp> pt3, Point3D<Tp> pt4, Point3D<Tp> pt5):size(row*col),rows(row),cols(col)
{
    if(size < 5) throw "neolix pointcloud 指针越界";
    data = new Point3D<Tp>[size];
    data[0] = pt0;
    data[1] = pt1;
    data[2] = pt3;
    data[3] = pt4;
    data[4] = pt5;
}
template<typename Tp, unsigned int row, unsigned col> pointcloud<Tp,row,col>::~pointcloud()
{
    if(data != nullptr)
    {
        delete[] data;
        data = nullptr;
    }
}
template<typename Tp, unsigned int row, unsigned int col> pointcloud<Tp,row,col>:: pointcloud(const pointcloud<Tp,row,col> &ptcld)
{
    this->~pointcloud();
    data = new Point3D<Tp>[size];
    for(int i = 0; i < size; i++)data[i] = ptcld.data[i];
}

template<typename Tp, unsigned int row, unsigned int col> Point3D<Tp>& pointcloud<Tp,row,col>::operator[](unsigned int i)
{
	if(i >  this->size-1 ) throw "neolix pointcloud指针越界";
  return data[i];

}

template<typename Tp, unsigned int row, unsigned int col> Point3D<Tp>* pointcloud<Tp,row,col>::ptr()
{
    return data;
}

template<typename Tp, unsigned int row, unsigned int col>  int  pointcloud<Tp,row,col>::converToPointCloud(cv::Mat depthImage, double fu, double fv,double u0, double v0)
 {

	cv::Mat shortImage;
	if (depthImage.type() != CV_16SC1)
	{
		depthImage.convertTo(shortImage, CV_16SC1);
	}else
	{
		shortImage = depthImage.clone();
	}
	 points.reserve(depthImage.rows * depthImage.cols +1);
	 points.clear();
     for(int i = 0; i < depthImage.rows; i++)
     {
         short *depthData  = shortImage.ptr<short>(i);
         for(int j = 0; j < depthImage.cols; j++)
         {
		//	 std::cout<<depthData[j]<<"  "<<i<<"  "<<j<<std::endl;

            if(depthData[j] > 0)
            {
			//转成点云
        //    Point3D<double> temp_pt(depthData[j]*(j-u0)/fu, depthData[j]*(i-v0)/fv, depthData[j]);
			Point3D<double> temp_pt(j, i, depthData[j]);
            points.push_back(temp_pt);

            }

         }
     }
	 if(points.size() == 0) return NEOLIX_FALSE;
     resize(points.size(),points.size());
	 for(size_t i = 0; i < points.size(); i++)
     {
        this->data[i][0] = this->points[i][0];
		this->data[i][1] = this->points[i][1];
		this->data[i][2] = this->points[i][2];

     }
	 return NEOLIX_SUCCESS;

 }

template<typename Tp, unsigned int row, unsigned int col> Point3D<Tp>* pointcloud<Tp,row,col>::ptr(unsigned int i)
{
    if(i > rows-1) throw "neolix pointcloud 指针越界";
    return &data[i];
}
   template<typename TTp, unsigned int _Trow, unsigned int _Tcol>  std::ostream& operator<<(std::ostream& os,const pointcloud<TTp,_Trow,_Tcol>& points)
        {
         os.flags(std::ios::internal);
         os.setf(std::ios::fixed);
         os<<"{"<<std::endl;
        for(int i = 0; i < points.rows; i++)
        {
            for(int j = 0; j < points.cols; j++)
            {
             os<<std::setw(5)<<points.data[i*points.rows+j]<<",";

            }
            os<<std::endl;
        }
        os<<"}"<<std::endl;

        return os;
    }

}
#endif // __POINTCLOUD_HPP
