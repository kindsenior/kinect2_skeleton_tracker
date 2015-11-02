#include <sstream>
#include <string>

#include <zmq.hpp>

#include "ros/ros.h"
#include "std_msgs/String.h"
#include <tf/transform_broadcaster.h>

typedef struct POINT3D{
	double x;
	double y;
	double z;
}point;

typedef struct ORIENT{
	double x;
	double y;
	double z;
	double w;
}orient;

typedef struct COORDINATE{
	int id;
	time_t timeStamp;
	point p;
	orient q;
	char name[10];
}coordinate;

using namespace std;

int main(int argc, char **argv)
{
	static time_t startTimeTable[] = {0,0,0,0,0}; // kinectは5個まで

  ros::init(argc, argv, "talker");

  ros::NodeHandle n;
  // ros::Publisher chatter_pub = n.advertise<std_msgs::String>("chatter", 1000);
  static tf::TransformBroadcaster br;

  // 0MQの初期化
  void *context = zmq_ctx_new();

  // // サーバ用ソケットの作成と初期化
  // void *responder = zmq_socket(context, ZMQ_REP);
  // zmq_bind(responder, "tcp://*:5555");// サーバがbind

  // subscriberのソケット作成とpublisherに接続
  void *subscriber = zmq_socket(context, ZMQ_SUB);
  zmq_bind(subscriber, "tcp://*:5556");// サーバがbind
  zmq_setsockopt( subscriber, ZMQ_SUBSCRIBE, "", 0 );

  bool flg = true;
  std::map<std::string, point> jointPosMap;

  ros::Rate loop_rate(30);
  int i = 0;
  while (ros::ok()){
      // // サービス
      // // 受信
      // zmq_recv(responder, buffer, 10, 0);
      // printf("%c%c%c%c\n",buffer[0],buffer[1],buffer[2],buffer[3]);
      // // 送信
      // zmq_send(responder, "aaaaa", 5, 0);

      // // bufer
      // zmq_recv(subscriber, buffer, 255, 0);
      // printf( "%c%c%c%c%c\n", buffer[0],buffer[1],buffer[2],buffer[3],buffer[4] );


      // msg

      // ros
      std_msgs::String msg;
      std::stringstream ss;
      while(1){
          zmq_msg_t zmq_msg;
          zmq_msg_init(&zmq_msg);

          zmq_msg_recv(&zmq_msg, subscriber, 0);

          // std::cout << zmq_msg_size(&zmq_msg) << std::endl;
          // coordinate *coordinate1 = new coordinate;
          // memcpy( coordinate1, zmq_msg_data(&zmq_msg), sizeof(*coordinate1)+10 );
          coordinate coordinate1;
          memcpy( &coordinate1, zmq_msg_data(&zmq_msg), sizeof(coordinate1)+10 );
          // ss << " id:" << coordinate1->id << " name:" << coordinate1->name;

					if ( startTimeTable[coordinate1.id] == 0 ){
						startTimeTable[coordinate1.id] = coordinate1.timeStamp;
					}else if( coordinate1.timeStamp < startTimeTable[coordinate1.id] ){
						ss << "old zmq message ( "  << coordinate1.timeStamp << " " << i << " )";
						break;
					}

          // ss << " id:" << coordinate1.id;
          // ss << " x:" << coordinate1.p.x << " y:" << coordinate1.p.y << " z:" << coordinate1.p.z << " " << i;
          // ss << coordinate1.name << " qx:" << coordinate1.q.x << " qy:" << coordinate1.q.y << " qz:" << coordinate1.q.z << " qw:" << coordinate1.q.w << std::endl;
					std::string linkName = coordinate1.name;
					// ss << str << std::endl;
          // free(coordinate1);

          zmq_msg_close(&zmq_msg);

          // tf
          tf::Transform transform;
					transform.setOrigin(tf::Vector3(0,0,0));
          transform.setRotation(tf::Quaternion(coordinate1.q.x, coordinate1.q.y, coordinate1.q.z, coordinate1.q.w));
					if( linkName.find("hip_") != string::npos || linkName.find("knee_") != string::npos || linkName.find("foot_") != string::npos ){
              tf::Transform tmp_tf(tf::Matrix3x3(0,0,-1, 0,-1,0, -1,0,0));
              transform = transform * tmp_tf;
					}else if( linkName.find("left_shoulder_") != string::npos || linkName.find("left_elbow_") != string::npos || linkName.find("left_hand_") != string::npos ){
              tf::Transform tmp_tf(tf::Matrix3x3(0,-1,0, 1,0,0, 0,0,1));
              transform = transform * tmp_tf;
					}else if( linkName.find("right_shoulder_") != string::npos || linkName.find("right_elbow_") != string::npos || linkName.find("right_hand_") != string::npos ){
              tf::Transform tmp_tf(tf::Matrix3x3(0,1,0, -1,0,0, 0,0,1));
              transform = transform * tmp_tf;
					}
					transform.setOrigin(tf::Vector3(coordinate1.p.x, coordinate1.p.y, coordinate1.p.z));

          // tf::Quaternion joint_rotation;
          // joint_rotation.setEulerZYX(1.5708, 0, 1.5708);
          // transform.setRotation(joint_rotation);

          // #4994
          tf::Transform change_frame;
          change_frame.setOrigin(tf::Vector3(0, 0, 0));
          tf::Quaternion frame_rotation;
          frame_rotation.setEulerZYX(1.5708, 0, 1.5708);
          change_frame.setRotation(frame_rotation);
          transform = change_frame * transform;

					// ros::Time _time = ros::Time::now();
          br.sendTransform(tf::StampedTransform(transform, ros::Time::now(), "/openni_depth_frame", linkName));
					// ss << " " << _time.toSec() << "." << _time.toNSec() << std::endl;

          if(!zmq_msg_more(&zmq_msg)) break;
      }

      msg.data = ss.str();

      ROS_INFO("%s", msg.data.c_str());
      // chatter_pub.publish(msg);

      ++i;
      ros::spinOnce();
      loop_rate.sleep();
  }


  return 0;
}
