/*
#include <iostream>
#include <zmq.hpp>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_CAM 7 

#define within(num) (int) ((float) num * random () / (RAND_MAX + 1.0))

int main (int argc, char ** argv)
{
		int status, no_cam;
		for(no_cam=1;no_cam<NUM_CAM+1;no_cam++)
		{
				status = fork();
				if(status == 0 | status == -1)
					break;
		}

	  if (status == -1)
    {
				std::cout<<"failed ..."<<std::endl;
      //error
    }
    else if (status == 0) //每个子进程都会执行的代码
    {
				sleep(1);
					zmq::context_t context(1);
		 	  zmq::socket_t subscriber_cmd (context, ZMQ_SUB);
        subscriber_cmd.connect("tcp://localhost:5557");

		 	  //  Subscribe to zipcode, default is NYC, 10001
				const char *filter = (argc > 1)? argv [1]: "msg";
			  subscriber_cmd.setsockopt(ZMQ_SUBSCRIBE, filter, strlen (filter));

				while(1)
			  {
						std::cout<<__LINE__<<std::endl;
								zmq::message_t update; 
								subscriber_cmd.recv(&update);
						
						std::cout<<__LINE__<<std::endl;
								std::string str(static_cast<char*>(update.data()), update.size());
			        	if(str != "msg_sys")
								{
										std::cout<<"error wait sys..."<<std::endl;
								}
								else
								{
										std::cout<<"sysing..."<<std::endl;
										break;
								}
			  }
				
				std::cout<<"ending............"<<std::endl;

		}
    else
    {	
			 //  Prepare our context and publisher
       zmq::context_t context (1);
       zmq::socket_t publisher_cmd (context, ZMQ_PUB);
       publisher_cmd.bind("tcp://*:5557");

			 do
			 {
					 zmq::message_t message(7);
					 memcpy(message.data(), "msg_sys", 7); 
					
					 std::cout<<__LINE__<<std::endl;
					 publisher_cmd.send(message); 
					 std::cout<<__LINE__<<std::endl;

			 }while(1);
		}

    return 0;
}
*/
#include <unistd.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core.hpp>
#include "opencv2/ccalib/omnidir.hpp"

#include <sstream>

int main(int argc, char* argv[])
{
		int no_cam = -1;
		int no_win = -1;
		int Npoints = -1;
		std::string str_dst;
		if(argc == 5)
		{
				no_cam = atoi(argv[1]);
				no_win = atoi(argv[2]);
				str_dst = argv[3];
				Npoints = atoi(argv[4]);
		}
		else
		{
				std::cout<<"Please input five parameters, \n" \
				<<	"	1th: name of exe \n" \
				<<	"	2th: NO of camera \n" \
				<<	"	3th: NO of window to show image \n"  \
				<<	"	4th: the destination where the images to be saved \n" \
				<<	"	5th: number of images to be grab"<<std::endl;
				return -1;
		}
		std::stringstream sttr;
		if(no_cam != 3)
			sttr<<"rtsp://admin:12345goccia@10.0.0.10"<<no_cam<<":554//Streaming/Channels/1";
		else
			sttr<<"rtsp://admin:123456goccia@10.0.0.10"<<no_cam<<":554//Streaming/Channels/1";
		
		std::string str;
		sttr>>str;
		std::cout<<str<<std::endl;
	
		cv::Mat frame;
		std::stringstream sttr2;
		sttr2<<no_win;
		std::string str2;
		sttr2>>str2;

		cv::VideoCapture *p_cap;
		int save_count = 0;

		float ratio = 0.5;
		while (1)
		{
				p_cap = new cv::VideoCapture(str);
				if(!p_cap->isOpened())  // check if we succeeded
				{
						std::cout<<"error:fail to load camera "<<no_cam<<std::endl;
						return -1;
				}

				double msec = p_cap->get(CV_CAP_PROP_POS_MSEC);

				std::cout<<msec<<std::endl;
				
				for(int i=0;i<30;i++)
				{
						p_cap->read(frame);
						if(frame.empty())
						{
								std::cout<<"fail to get "<<no_cam<<"th image..."<<std::endl;
								continue;
						}
						cv::Mat tmp;
						cv::resize(frame,tmp,cv::Size(),ratio,ratio,CV_INTER_LINEAR);
						
						cv::imshow(str2,tmp);
						char key = cv::waitKey(3);
						if(key == 'q' || save_count >= Npoints)
							break;
						else if(key == 'g')
						{
								std::stringstream dst; 
								dst<<str_dst<<save_count++<<".jpg";
								////dst<<"./test_img/cam_"<<no_cam<<"_img_"<<save_count++<<".jpg";
								std::string dst_str;
								dst>>dst_str;
								cv::imwrite(dst_str,frame);
								std::cout<<"grab "<<save_count<<"th image..."<<std::endl;
						}
				}
				p_cap->release();
				delete p_cap;
				sleep(1);
		}

		return 0;
}



