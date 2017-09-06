#include <iostream>
#include <zmq.hpp>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "serial_mat.hpp"

int main (int argc, char ** argv)
{
		int status, no_cam;
		for(no_cam=1;no_cam<8;no_cam++)
		{
				status = fork();
				if(status == 0 | status == 1)
					break;
		}

	  if (status == -1)
    {
				std::cout<<"failed ..."<<std::endl;
      //error
    }
    else if (status == 0) //每个子进程都会执行的代码
    {
				zmq::context_t context(1);
				zmq::socket_t sender(context,ZMQ_PUSH);
				sender.connect("ipc:///tmp/0");

				std::cout<<" sub process ..."<< no_cam <<std::endl;
        //sub process

				std::stringstream sttr;
    		if(no_cam != 3)
    			sttr<<"rtsp://admin:12345goccia@10.0.0.10"<< no_cam <<":554//Streaming/Channels/1";
    		else
    			sttr<<"rtsp://admin:123456goccia@10.0.0.10"<< no_cam <<":554//Streaming/Channels/1";
    		
    		std::string str;
    		sttr>>str;
    		std::cout<< str <<std::endl;
    		cv::VideoCapture vcap(str);
    		if(!vcap.isOpened())  // check if we succeeded
    		{
    				std::cout<<"error:fail to load camera "<<no_cam<<std::endl;
    				return -1;
    		}

				std::stringstream sttr2;
				sttr2<<no_cam;
				std::string str2;
				sttr2>>str2;

				int count = 0;
				cv::Mat frame;
				while (true)
				{	
						vcap>>frame;
		    		if(frame.empty())
		       	{
		       			std::cout<<"fail to get "<< no_cam <<"th image..."<<std::endl;
		       			continue;
		       	}
		       	//cv::Mat tmp;
		       	//cv::resize(frame,tmp,cv::Size(), 0.25, 0.25, CV_INTER_LINEAR);	
						//cv::imshow(str2,tmp);
						//cv::imshow(str2,frame);
		       	char key = cv::waitKey(3);
		       	if(key == 'q')
		       		break;  

						std::ostringstream os;  
						boost::archive::binary_oarchive oa(os);  
						oa << frame;//序列化到一个ostringstream里面  
						
						std::string content = os.str();//content保存了序列化后的数据。  

						zmq::message_t message(content.size());
						memcpy(message.data(), content.c_str() , content.size());
						//printf("%s",message.data());
						sender.send(message);
				}
				
				sleep (1);    

		}
    else
    {	
				zmq::context_t context(1);
				zmq::socket_t receiver(context,ZMQ_PULL);
				receiver.bind("ipc:///tmp/0");
				zmq::message_t message;
				
				//std::ofstream log("log.txt");
				bool b_beg = true;
				while (true)
				{
					
						receiver.recv(&message);
						std::string smessage(static_cast<char*>(message.data()), message.size());
						//std::cout<<smessage<<std::endl;
				
						if(b_beg)
						{
								cv::Mat tmp;
								std::istringstream is(smessage); 
								boost::archive::binary_iarchive ia(is);  
								ia >> tmp;

								cv::imwrite("111.jpg",tmp);

								b_beg = false;
						}
					//	log<<smessage;
				}

				//log.close();

				std::cout<<" parent process ..."<<std::endl;
      //parent process
    }

		sleep (1);  

    return 0;
}

