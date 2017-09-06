#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <opencv2/opencv.hpp>

using namespace std;
namespace boost {
		namespace serialization {
				template<class Archive>
        void serialize(Archive &ar, cv::Mat& mat, const unsigned int)
        {
            int cols, rows, type;
            bool continuous;

            if (Archive::is_saving::value) {
                cols = mat.cols; rows = mat.rows; type = mat.type();
                continuous = mat.isContinuous();
            }

            ar & cols & rows & type & continuous;

            if (Archive::is_loading::value)
                mat.create(rows, cols, type);

            if (continuous) {
                const unsigned int data_size = rows * cols * mat.elemSize();
                ar & boost::serialization::make_array(mat.ptr(), data_size);
            }
            else {
                const unsigned int row_size = cols*mat.elemSize();
                for (int i = 0; i < rows; i++) {
                    ar & boost::serialization::make_array(mat.ptr(i), row_size);
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
