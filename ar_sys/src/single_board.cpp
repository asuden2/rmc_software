/**
* @file single_board.cpp
* @author Hamdi Sahloul
* @date September 2014
* @version 0.1
* @brief ROS version of the example named "simple_board" in the Aruco software package.
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <aruco/aruco.h>
#include <aruco/boarddetector.h>
#include <aruco/cvdrawingutils.h>

#include <opencv2/core/core.hpp>
#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <ar_sys/utils.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_listener.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>

using namespace aruco;

class ArSysSingleBoard
{
	private:
		cv::Mat inImage, resultImg;
		aruco::CameraParameters camParam;
		bool useRectifiedImages;
		bool draw_markers;
		bool draw_markers_cube;
		bool draw_markers_axis;
		MarkerDetector mDetector;
		vector<Marker> markers;
		BoardConfiguration the_board_config;
		BoardDetector the_board_detector;
		Board the_board_detected;
		ros::Subscriber cam_info_sub;
		bool cam_info_received;
		image_transport::Publisher image_pub;
		image_transport::Publisher debug_pub;
		ros::Publisher pose_cov_pub;
		ros::Publisher transform_pub; 
		ros::Publisher position_pub;
		std::string board_frame;

		double marker_size;
		std::string board_config;

		ros::NodeHandle nh;
		image_transport::ImageTransport it;
		image_transport::Subscriber image_sub;

		tf::TransformListener _tfListener;
		tf::TransformBroadcaster _tfBroadcaster;

		tf::StampedTransform imOffsetTransform;
		tf::StampedTransform arOffsetTransform;
		tf::StampedTransform _firstTf;

		bool gotInitialTf;
		bool rtabTfExists;

	public:
		ArSysSingleBoard()
			: cam_info_received(false),
			nh("~"),
			it(nh),
			gotInitialTf(false),
			rtabTfExists(false)
		{
			image_sub = it.subscribe("/image", 1, &ArSysSingleBoard::image_callback, this);
			cam_info_sub = nh.subscribe("/camera_info", 1, &ArSysSingleBoard::cam_info_callback, this);

			image_pub = it.advertise("result", 1);
			debug_pub = it.advertise("debug", 1);
			pose_cov_pub = nh.advertise<geometry_msgs::PoseWithCovarianceStamped>("pose", 100);
			transform_pub = nh.advertise<geometry_msgs::TransformStamped>("transform", 100);
			position_pub = nh.advertise<geometry_msgs::Vector3Stamped>("position", 100);

			nh.param<double>("marker_size", marker_size, 0.05);
			nh.param<std::string>("board_config", board_config, "boardConfiguration.yml");
			nh.param<std::string>("board_frame", board_frame, "");
			nh.param<bool>("image_is_rectified", useRectifiedImages, true);
			nh.param<bool>("draw_markers", draw_markers, false);
			nh.param<bool>("draw_markers_cube", draw_markers_cube, false);
			nh.param<bool>("draw_markers_axis", draw_markers_axis, false);

			the_board_config.readFromFile(board_config.c_str());

			ROS_INFO("ArSys node started with marker size of %f m and board configuration: %s",
					 marker_size, board_config.c_str());
		}

		void image_callback(const sensor_msgs::ImageConstPtr& msg)
		{
			if(!cam_info_received) return;

			cv_bridge::CvImagePtr cv_ptr;
			try
			{
				cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::RGB8);
				inImage = cv_ptr->image;
				resultImg = cv_ptr->image.clone();

				//detection results will go into "markers"
				markers.clear();
				//Ok, let's detect
				mDetector.detect(inImage, markers, camParam, marker_size, false);
				//Detection of the board
				float probDetect=the_board_detector.detect(markers, the_board_config, the_board_detected, camParam, marker_size);
				if (probDetect > 0.0)
				{
/*
					//tf::Transform transform = ar_sys::getTf(the_board_detected.Rvec, the_board_detected.Tvec);
					tf::Transform arucoTransform = ar_sys::getTf(the_board_detected.Rvec, the_board_detected.Tvec);
//mult then inv
					_tfListener.lookupTransform("arena", "board_marker", ros::Time(0), arOffsetTransform);
					arucoTransform.mult(arucoTransform, arOffsetTransform);

					arucoTransform = arucoTransform.inverse(); 
					////imOffsetTransform.transform = imOffsetTransform.transform.inverse();


					tf::Transform transform;

					_tfListener.lookupTransform("base_link", "blackfly_mount_link", ros::Time(0), imOffsetTransform);
					//transform.mult(arucoTransform, imOffsetTransform);
					transform.mult(transform, imOffsetTransform);

					//transform *= imOffsetTransform.inverse();

					//tf::StampedTransform stampedTransform(transform, msg->header.stamp, "aruco_mapframe_test", "robot_base_link");
					tf::StampedTransform stampedTransform(transform, msg->header.stamp, "arena", "odom");
*/





					//tf::Transform transform = ar_sys::getTf(the_board_detected.Rvec, the_board_detected.Tvec);
					tf::Transform arucoTransform = ar_sys::getTf(the_board_detected.Rvec, the_board_detected.Tvec);

					//arucoTransform = arucoTransform.inverse();

					
					//arucoTransform.mult(arucoTransform, arOffsetTransform);


					tf::Transform transform = arucoTransform;

					//_tfListener.lookupTransform("base_link", "blackfly_mount_link", ros::Time(0), imOffsetTransform);
					//transform.mult(arucoTransform, imOffsetTransform);
					//transform.mult(transform, imOffsetTransform);

					//transform *= imOffsetTransform.inverse();

					tf::StampedTransform stampedTransform(transform, msg->header.stamp, "blackfly_optical_link", "ar_board_marker");
					//tf::StampedTransform stampedTransform(transform, ros::Time::now(), "blackfly_optical_link", "ar_board_marker");

					ros::spinOnce();

					// if(!gotInitialTf)
					// {
					// 	gotInitialTf = true;
					// }
					
					// else
					// {
					// 	_firstTf.stamp_ = ros::Time::now();
					// }

					geometry_msgs::PoseStamped newPoseMsg;

					geometry_msgs::PoseStamped rawPoseMsg;
					rawPoseMsg.header.frame_id = "board_marker";
					rawPoseMsg.header.stamp = msg->header.stamp;
					

					geometry_msgs::PoseWithCovarianceStamped poseMsg;

					tf::poseTFToMsg(transform, rawPoseMsg.pose);

					//_tfListener.transformPose("board_marker", rawPoseMsg, newPoseMsg);

					//poseMsg.pose.pose = newPoseMsg.pose;
					poseMsg.pose.pose = rawPoseMsg.pose;


					rawPoseMsg.header.frame_id = msg->header.frame_id;
					rawPoseMsg.header.stamp = msg->header.stamp;
					
					
					
					poseMsg.pose.covariance[0] = 0.08;
					poseMsg.pose.covariance[7] = 0.08;
					poseMsg.pose.covariance[14] = 0.005;
					poseMsg.pose.covariance[21] = 0.006;
					poseMsg.pose.covariance[28] = 0.006;
					poseMsg.pose.covariance[35] = 0.01;
					poseMsg.header.frame_id = "map";
					poseMsg.header.stamp = msg->header.stamp;

					pose_cov_pub.publish(poseMsg);
					geometry_msgs::TransformStamped transformMsg;
					tf::transformStampedTFToMsg(stampedTransform, transformMsg);
					transform_pub.publish(transformMsg);

					_tfBroadcaster.sendTransform(stampedTransform);

/*
				    try
				    {
						_tfListener.lookupTransform("base_link","ar_board_marker", ros::Time(0), arOffsetTransform);
				    }
				    catch (tf::TransformException ex)
				    {
						ROS_ERROR("%s",ex.what());
						ros::Duration(1.0).sleep();
				    }

					

					tf::Transform blToArucoTF(arOffsetTransform.getRotation(), arOffsetTransform.getOrigin());
					tf::StampedTransform stampedBlToArucoTF(blToArucoTF, ros::Time(0), "arena", "odom");
					_tfBroadcaster.sendTransform(stampedBlToArucoTF);
*/
					geometry_msgs::Vector3Stamped positionMsg;
					positionMsg.header = transformMsg.header;
					positionMsg.vector = transformMsg.transform.translation;
					position_pub.publish(positionMsg);
				}
				//for each marker, draw info and its boundaries in the image
				for(size_t i=0; draw_markers && i < markers.size(); ++i)
				{
					markers[i].draw(resultImg,cv::Scalar(0,0,255),2);
				}


				if(camParam.isValid() && marker_size != -1)
				{
					//draw a 3d cube in each marker if there is 3d info
					for(size_t i=0; i<markers.size(); ++i)
					{
						if (draw_markers_cube) CvDrawingUtils::draw3dCube(resultImg, markers[i], camParam);
						if (draw_markers_axis) CvDrawingUtils::draw3dAxis(resultImg, markers[i], camParam);
					}
					//draw board axis
					if (probDetect > 0.0) CvDrawingUtils::draw3dAxis(resultImg, the_board_detected, camParam);
				}

				if(image_pub.getNumSubscribers() > 0)
				{
					//show input with augmented information
					cv_bridge::CvImage out_msg;
					out_msg.header.frame_id = msg->header.frame_id;
					out_msg.header.stamp = msg->header.stamp;
					out_msg.encoding = sensor_msgs::image_encodings::RGB8;
					out_msg.image = resultImg;
					image_pub.publish(out_msg.toImageMsg());
				}

				if(debug_pub.getNumSubscribers() > 0)
				{
					//show also the internal image resulting from the threshold operation
					cv_bridge::CvImage debug_msg;
					debug_msg.header.frame_id = msg->header.frame_id;
					debug_msg.header.stamp = msg->header.stamp;
					debug_msg.encoding = sensor_msgs::image_encodings::MONO8;
					debug_msg.image = mDetector.getThresholdedImage();
					debug_pub.publish(debug_msg.toImageMsg());
				}
			}
			catch (cv_bridge::Exception& e)
			{
				ROS_ERROR("cv_bridge exception: %s", e.what());
				return;
			}

			if(!rtabTfExists)
			{
				tf::StampedTransform rtabTf;

			    try
			    {
					_tfListener.lookupTransform("odom","base_link", ros::Time(0), rtabTf);
					rtabTfExists = true;
					ROS_INFO("Ar_sys: odom->base_link TF Exists from RTAB!");
					
			    }
			    catch (tf::TransformException ex)
			    {
					ROS_WARN("%s",ex.what());
					//ros::Duration(1.0).sleep();
					//return;
			    }								
			}
			else
			{
				if(!gotInitialTf)
				{
					tf::StampedTransform arOffsetTransform;

				    try
				    {
						_tfListener.lookupTransform("base_link","ar_board_marker", ros::Time(0), arOffsetTransform);
						gotInitialTf = true;

						tf::Transform blToArucoTF(arOffsetTransform.getRotation(), arOffsetTransform.getOrigin());

						tf::StampedTransform stampedBlToArucoTF(blToArucoTF.inverse(), ros::Time(0), "arena", "odom");
						_firstTf = stampedBlToArucoTF;
						_tfBroadcaster.sendTransform(stampedBlToArucoTF);
				    }
				    catch (tf::TransformException ex)
				    {
						ROS_WARN("%s",ex.what());
						//ros::Duration(1.0).sleep();
				    }				
				}
				else
				{
					_firstTf.stamp_ = ros::Time::now();
					_tfBroadcaster.sendTransform(_firstTf);
			    }
			}
			


		}

		// wait for one camerainfo, then shut down that subscriber
		void cam_info_callback(const sensor_msgs::CameraInfo &msg)
		{
			camParam = ar_sys::getCamParams(msg, useRectifiedImages);
			cam_info_received = true;
			cam_info_sub.shutdown();
		}
};


int main(int argc,char **argv)
{
	ros::init(argc, argv, "ar_single_board");

	ArSysSingleBoard node;

	ros::spin();
}
