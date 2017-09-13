#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <opencv2/opencv.hpp>


class Mat_fat 
{
public:
		int id_cam;
		long long time_stamp;
		cv::Mat frame;
};

class Cams_Status
{

friend class boost::serialization::access;  

    template<class Archive>  
    void serialize(Archive& ar, const unsigned int version)  
    {  
        ar & id_cam;  
        ar & b_sys_ready;
        ar & b_preFor_sys;
    }

public:
		int get_camID(){ return id_cam; }
		bool Is_sys_ready(){ return b_sys_ready; }
		bool Is_preFor_sys(){ return b_preFor_sys; }

public:
		int id_cam;
		bool b_sys_ready = false;
		bool b_preFor_sys = false;


};
/*
public:
		int Get_CameraID()
		{
				return id_cam;
		}
		long long Get_frameStamp()
		{
				return time_stamp;
		}

		void Set_CameraID(int& no_cam)
		{
				id_cam = no_cam;
		}
		void Set_frameStamp(long long& time)
		{
				time_stamp = time;
		}

		void Set_ImageData()
		{
				frame = mat;
		}

}
*/

using namespace std;
namespace boost {
		namespace serialization {
				template<class Archive>
        void serialize(Archive &ar, Mat_fat& mat_f, const unsigned int)
        {
            int cols, rows, type;
            bool continuous;

						int id_cam;
						long long time_stamp;

            if (Archive::is_saving::value) {
                cols = mat_f.frame.cols; rows = mat_f.frame.rows; type = mat_f.frame.type();

								id_cam = mat_f.id_cam;
								time_stamp = mat_f.time_stamp;

                continuous = mat_f.frame.isContinuous();
            }

            ar & cols & rows & type & continuous & id_cam & time_stamp ;

            if (Archive::is_loading::value)
						{
								mat_f.id_cam = id_cam;
								mat_f.time_stamp = time_stamp;
                mat_f.frame.create(rows, cols, type);
						}
            if (continuous) {
                const unsigned int data_size = rows * cols * mat_f.frame.elemSize();
                ar & boost::serialization::make_array(mat_f.frame.ptr(), data_size);
            }
            else {
                const unsigned int row_size = cols*mat_f.frame.elemSize();
                for (int i = 0; i < rows; i++) {
                    ar & boost::serialization::make_array(mat_f.frame.ptr(i), row_size);
                }
            }

        }

    }
}
/*
void test()  
{  

		cv::Mat img = cv::imread("./img.jpg");  
      
		std::ofstream ofs("matrices.bin", std::ios::out | std::ios::binary);  
		{ // use scope to ensure archive goes out of scope before stream  
				boost::archive::binary_oarchive oa(ofs);  
				oa << img;  
		}
		
		ofs.close();  

		cv::Mat imgOut;  
		ifstream inf("matrices.bin",std::ios::binary);  
		{  
				boost::archive::binary_iarchive ia(inf);  
				ia >> imgOut;  
		}  

		cv::imshow("img",imgOut);  
		cv::waitKey(0);  
} 

int main()  
{
		test();    
}
*/
