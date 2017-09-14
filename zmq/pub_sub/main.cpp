#include <iostream>
#include <zmq.hpp>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <signal.h>  
#include <time.h>
#include "serialization.hpp"

#define NUM_CAM 2 

#define within(num) (int) ((float) num * random () / (RAND_MAX + 1.0))

int main (int argc, char ** argv)
{
		int status, no_cam;
		for(no_cam=0;no_cam<NUM_CAM;no_cam++)
		{
				status = fork();
				if(status == 0 | status == -1)
					break;
		}

		std::vector<std::string> vstr;
		vstr.push_back(std::string("rtsp://admin:12345goccia@10.0.0.104:554//Streaming/Channels/1"));
		vstr.push_back(std::string("rtsp://admin:12345goccia@10.0.0.106:554//Streaming/Channels/1"));
		//vstr.push_back(std::string("rtsp://admin:12345goccia@10.0.0.104:554//Streaming/Channels/1"));
		//vstr.push_back(std::string("rtsp://admin:12345goccia@10.0.0.104:554//Streaming/Channels/1"));
		//vstr.push_back(std::string("rtsp://admin:12345goccia@10.0.0.104:554//Streaming/Channels/1"));
		//vstr.push_back(std::string("rtsp://admin:12345goccia@10.0.0.104:554//Streaming/Channels/1"));
		//vstr.push_back(std::string("rtsp://admin:12345goccia@10.0.0.104:554//Streaming/Channels/1"));

		std::vector<cv::Rect> vrc;
    vrc.push_back(cv::Rect(352,52,40,50));//#104
		vrc.push_back(cv::Rect(562,256,40,50));//#106
   //vrc.push_back(cv::Rect(302,150,40,40));//#101
	 //vrc.push_back(cv::Rect(512,242,40,50));//#102
   //vrc.push_back(cv::Rect(100,100,50,50));//#103
   //vrc.push_back(cv::Rect(100,100,50,50));//#105
   //vrc.push_back(cv::Rect(100,100,50,50));//#107

	 if (status == -1)
    {
				std::cout<<"failed ..."<<std::endl;
      //error
    }
    else if (status == 0) //每个子进程都会执行的代码
    {
    		    		
				prctl(PR_SET_PDEATHSIG, SIGKILL);

				cv::Rect ret = vrc[no_cam];

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

    		cv::VideoCapture *p_cap = NULL;
	
				int count = 0;
				int count_change_time = 0;
				int begin_stamp = 0;
				int pixel_value_thresh = 230;
				cv::Mat frame, gray, gray_last, delt_Mat;
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

										std::cout<<"begin sys..."<<std::endl;
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
										p_cap = new cv::VideoCapture(vstr[no_cam]);
										if(!p_cap->isOpened())  // check if we succeeded
										{
												std::cout<<"error:fail to load camera "<<no_cam<<std::endl;
												return -1;
										}

										std::cout<<p_cap->get(CV_CAP_PROP_FPS)<<std::endl;

										count = 0;
										count_change_time = 0;
										b_syned = false;

								}

						}
						else
						{
								p_cap->read(frame);
								if(frame.empty())
								{
										std::cout<<"fail to get "<< no_cam <<"th image..."<<std::endl;
										continue;   	
								}
		
								count++;
		
								double frame_stamp = p_cap->get(CV_CAP_PROP_POS_FRAMES);
								//cv::resize(frame,tmp,cv::Size(),ratio,ratio,CV_INTER_LINEAR);
								//cv::rectangle(frame,ret,cv::Scalar(255,0,0),2);

								//cv::imshow(str,tmp_ret);
		            //cv::waitKey(3);

								if(!b_syned)
								{
		              	cv::Mat tmp_ret = frame(ret);
		              	
										cv::cvtColor(tmp_ret, gray, CV_RGB2GRAY);

		              	if(frame_stamp == 0)
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
														begin_stamp = frame_stamp;
														std::cout<<"succeed to syn the cam "<<no_cam<<std::endl;
												}
												std::stringstream sttr, sttr2;
		              			std::string str, str2;
		              			sttr<<"cam_"<<no_cam<<"_newframe_"<<frame_stamp<<".jpg";
		              			sttr2<<"cam_"<<no_cam<<"_oldframe_"<<frame_stamp<<".jpg";
		              			sttr>>str;	
		              			sttr2>>str2;

		              			std::cout<<"Process No:"<<no_cam<<" Frame stamp:"<<frame_stamp<<" no: "<<count<<" MaxValue: "<<MaxValue<<std::endl;
		              			cv::imwrite(str,gray);
		              			cv::imwrite(str2,gray_last);	
		              	}
		              
		              	gray_last = gray.clone(); 

								}
								else
								{
									
					        	mat_f.time_stamp = frame_stamp - begin_stamp;
					        	mat_f.frame = frame;

					        	std::ostringstream os;  
					        	boost::archive::binary_oarchive oa(os);  
					        	oa << mat_f;//序列化到一个ostringstream里面  
					        	
									 std::cout<<__LINE__<<std::endl;
					        	std::string content = os.str();//content保存了序列化后的数据。  

					        	zmq::message_t message(content.size());
					        	memcpy(message.data(), content.c_str() , content.size());
					        	//printf("%s",message.data());
					        	sender_data.send(message);
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
		
			 zmq::message_t message_poll;

			 do
			 {
					 zmq::message_t message(7);
					 memcpy(message.data(), "msg_syn", 7); 
								 
					 publisher_cmd.send(message); 
					 					 
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

									 bool flag = (flag_preFor_sys & (1 << (status.id_cam)))>0 ? true:false;
									 if(!flag)
									 {
											 flag_preFor_sys |= (1 << (status.id_cam));

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

			 int s = 0;

			 while (1) {
					 zmq::message_t message;
        
					 zmq::poll (&items [0], 2, 1);
					 if (items [0].revents & ZMQ_POLLIN)
					 {
		
							 receiver_status.recv(&message);
							 //  Process task
							 
							 std::cout<<__LINE__<<std::endl;
							 
							 std::string smessage(static_cast<char*>(message.data()), message.size());

							 Cams_Status status;		
							 std::istringstream is(smessage); 
							 boost::archive::binary_iarchive ia(is);  
							 ia >> status;
		
					 }

					 if (items [1].revents & ZMQ_POLLIN) {
							
							 receiver_data.recv(&message);
							 //  Process weather update
							 
							 std::cout<<__LINE__<<std::endl;
					     
							 std::string smessage(static_cast<char*>(message.data()), message.size());
							 std::cout<<"message.size:"<<message.size()<<std::endl;
					     //std::cout<<smessage<<std::endl;
					     std::istringstream is(smessage); 
					     boost::archive::binary_iarchive ia(is);  
					     ia >> mat_f;
					     //log<<mat_f.id_cam<<":"<<mat_f.time_stamp;
					     std::cout<<mat_f.id_cam<<":"<<mat_f.time_stamp<<std::endl;

							 std::stringstream sttr;
		           std::string str;
		           sttr<<"cam_"<<mat_f.id_cam<<"_frame_"<<mat_f.time_stamp<<".jpg";
		           sttr>>str;	

		           cv::imwrite(str,mat_f.frame);

							 s++;
							 if(s>330)
								 break;
			     } 
					 
			 }
		}

    return 0;
}

