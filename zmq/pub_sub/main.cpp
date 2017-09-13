#include <iostream>
#include <zmq.hpp>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "serialization.hpp"

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
    		std::vector<cv::Rect> vrc;
        vrc.push_back(cv::Rect(100,100,50,50));//#101
        vrc.push_back(cv::Rect(100,100,50,50));//#102
        vrc.push_back(cv::Rect(100,100,50,50));//#103
    		vrc.push_back(cv::Rect(500,130,20,20));//#104
        vrc.push_back(cv::Rect(320,100,20,20));//#105
        vrc.push_back(cv::Rect(100,100,50,50));//#106
        vrc.push_back(cv::Rect(100,100,50,50));//#107
    		cv::Rect ret = vrc[no_cam - 1];

				zmq::context_t context (1);
		 	  zmq::socket_t subscriber_cmd (context, ZMQ_SUB);
        zmq::socket_t sender_data (context, ZMQ_PUSH);
        zmq::socket_t sender_status (context, ZMQ_PUSH);
        subscriber_cmd.connect("tcp://localhost:5557");
        sender_data.connect("ipc://data.ipc");                
        sender_status.connect("ipc://status.ipc");          

				//  Subscribe to zipcode, default is NYC, 10001
		  	const char *filter = (argc > 1)? argv [1]: "msg";
			  subscriber_cmd.setsockopt(ZMQ_SUBSCRIBE, filter, strlen (filter));

				std::stringstream sttr;
    		if(no_cam != 3)
    			sttr<<"rtsp://admin:12345goccia@10.0.0.10"<< no_cam <<":554//Streaming/Channels/1";
    		else
    			sttr<<"rtsp://admin:123456goccia@10.0.0.10"<< no_cam <<":554//Streaming/Channels/1";
    		
    		std::string str;
    		sttr>>str;
    		std::cout<< str <<std::endl;
    		cv::VideoCapture *p_cap;
	
				int count = 0;
				int count_change_time = 0;
				int begin_stamp = 0;
				cv::Mat frame, gray, gray_clone, delt_Mat;
				Mat_fat mat_f;
				mat_f.id_cam = no_cam;
			
				bool b_resys = true;
				bool b_syned = false;
				while(1)
			  {

						if(b_resys)
						{
								zmq::message_t update; 
								subscriber_cmd.recv(&update);
						
								std::string str(static_cast<char*>(update.data()), update.size());
			        	if(str != "msg_syn")
								{
										std::cout<<"error wait sys..."<<std::endl;
										b_resys = true;
								}
								else
								{

										Cams_Status status;		

										status.id_cam = no_cam;
										status.b_preFor_sys = true;
										std::ostringstream os;  
										boost::archive::binary_oarchive oa(os);  
										oa << status;						
										std::string content = os.str();//content保存了序列化后的数据。  

					        	zmq::message_t message(content.size());
					        	memcpy(message.data(), content.c_str() , content.size());
					  
										sender_status.send(message);

										b_resys = false;

										//p_cap->release();
										p_cap = new cv::VideoCapture(str);
										if(!p_cap->isOpened())  // check if we succeeded
										{
												std::cout<<"error:fail to load camera "<<no_cam<<std::endl;
												return -1;
										}

										count = 0;
										count_change_time = 0;
										b_syned = false;

								}

						}
						else
						{
								p_cap.read(frame);
								if(frame.empty())
								{
										std::cout<<"fail to get "<< no_cam <<"th image..."<<std::endl;
										continue;   	
								}
		
								count++;
		
								double msec = p_cap->get(CV_CAP_PROP_POS_MSEC);
								//cv::resize(frame,tmp,cv::Size(),ratio,ratio,CV_INTER_LINEAR);
								//cv::rectangle(frame,ret,cv::Scalar(255,0,0),2);

								//cv::imshow(str,tmp_ret);
		            //cv::waitKey(3);

								if(!b_syned)
								{
		              	cv::Mat tmp_ret = frame(ret);
		              	
										cv::cvtColor(tmp_ret, gray, CV_RGB2GRAY);

		              	if(msec == 0)
		              		gray_last = gray.clone();
		              	delt_Mat = cv::abs(gray - gray_last);

		              	cv::Point minP,maxP;
		               	double MaxValue, MinValue;
		              	cv::minMaxLoc(delt_Mat, &MinValue, &MaxValue, &minP,&maxP);
		              	
		              	if(MaxValue > pixel_value_thresh)
		              	{

												count_change_time++;

												if(count_change_time == 2)
												{
														b_syned = true;
														begin_stamp = msec;
														std::cout<<"succeed to syn the cam "<<no_cam<<std::endl;
												}
												std::stringstream sttr, sttr2;
		              			std::string str, str2;
		              			sttr<<"cam_"<<no_cam<<"_newframe_"<<count<<".jpg";
		              			sttr2<<"cam_"<<no_cam<<"_oldframe_"<<count<<".jpg";
		              			sttr>>str;	
		              			sttr2>>str2;

		              			std::cout<<"Process No:"<<id_cam<<" Frame stamp:"<<msec<<" no: "<<count<<" MaxValue: "<<MaxValue<<std::endl;
		              			cv::imwrite(str,gray);
		              			cv::imwrite(str2,gray_last);	
		              	}
		              
		              	gray_last = gray.clone(); 

								}
								else
								{
									
					        	mat_f.time_stamp = msec - begin_stamp;
					        	mat_f.frame = frame;

					        	std::ostringstream os;  
					        	boost::archive::binary_oarchive oa(os);  
					        	oa << mat_f;//序列化到一个ostringstream里面  
					        	
					        	std::string content = os.str();//content保存了序列化后的数据。  

					        	zmq::message_t message(content.size());
					        	memcpy(message.data(), content.c_str() , content.size());
					        	//printf("%s",message.data());
					        	sender.send(message);
								}					
						}

			  }

		}
    else
    {	
			 //  Prepare our context and publisher
       zmq::context_t context (1);
       zmq::socket_t publisher_cmd (context, ZMQ_PUB);
       zmq::socket_t receiver_data (context, ZMQ_PULL);
       zmq::socket_t receiver_status (context, ZMQ_PULL);
       receiver_data.bind("ipc://data.ipc");             
       receiver_status.bind("ipc://status.ipc");       
       publisher_cmd.bind("tcp://*:5557");

			 int flag_preFor_sys = 0; 

			 //  Initialize poll set 
			 zmq::pollitem_t items [] = {
					 { receiver_status, 0, ZMQ_POLLIN, 0 },
					 { receiver_data, 0, ZMQ_POLLIN, 0 }
			 };
		
			 do
			 {
					 zmq::message_t message(7);
					 memcpy(message.data(), "msg_syn", 7); 
					
					 publisher_cmd.send(message); 
					 std::cout<<__LINE__<<std::endl;
					 
					 zmq::message_t message_poll;
					 zmq::poll (&items [0], 2, 1);
					 
					 if (items [0].revents & ZMQ_POLLIN)
					 {

							 receiver_status.recv(&message_poll);
							 //  Process task

							 std::cout<<__LINE__<<std::endl;

							 std::string smessage(static_cast<char*>(message_poll.data()), message_poll.size());

							 Cams_Status status;		
							 std::istringstream is(smessage); 
							 boost::archive::binary_iarchive ia(is);  
							 ia >> status;

							 if(status.b_preFor_sys)
							 {

									 std::cout<<__LINE__<<std::endl;

									 bool flag = (flag_preFor_sys & (1 << (status.id_cam-1)))>0 ? true:false;
									 if(!flag)
									 {
											 flag_preFor_sys |= (1 << (status.id_cam-1));

											 if( flag_preFor_sys == ((1 << NUM_CAM) - 1) )		
											 {
													 std::cout<<"succeed to prepare for sys..."<<std::endl;
											 }
									 }
									 else
									 {
											 std::cout<<"resys the cam for multi times..."<<std::endl;
									 }
							}

					 }

			 }while( flag_preFor_sys != ((1<<NUM_CAM)-1) );
					
			 Mat_fat mat_f;

			 while (1) {
					 zmq::message_t message;
					 zmq::poll (&items [0], 2, 1);
        
					 if (items [0].revents & ZMQ_POLLIN)
					 {
		
							 receiver_status.recv(&message);
							 //  Process task
							 
							 std::string smessage(static_cast<char*>(message.data()), message.size());

							 Cams_Status status;		
							 std::istringstream is(smessage); 
							 boost::archive::binary_iarchive ia(is);  
							 ia >> status;
		
					 }

					 if (items [1].revents & ZMQ_POLLIN) {

							 static int s = 0;

							 receiver_data.recv(&message);
							 //  Process weather update
							 
					     std::string smessage(static_cast<char*>(message.data()), message.size());
					     //std::cout<<smessage<<std::endl;
					     std::istringstream is(smessage); 
					     boost::archive::binary_iarchive ia(is);  
					     ia >> mat_f;
					     //log<<mat_f.id_cam<<":"<<mat_f.time_stamp;
					     std::cout<<mat_f.id_cam<<":"<<mat_f.time_stamp<<std::endl;

							 std::stringstream sttr;
		           std::string str;
		           sttr<<"cam_"<<no_cam<<"_frame_"<<s<<".jpg";
		           sttr>>str;	

		           cv::imwrite(str,gray);
							 
							 s++;
							 if(s>200)
								 break;
			     } 
					 
			 }
		}

    return 0;
}

