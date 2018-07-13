//============================================================================
// Name        : VFH3D.cpp
// Author      : Ryan Fite - ryanfite@live.com
// Version     : 1.0
// Date Created: Jul 6, 2018
// Description : Primary Node for the VFH3D algorithm.
//============================================================================


#include "ros/ros.h"
#include "pubHandler.cpp"
#include "sensor_msgs/PointCloud2.h"
#include "nav_msgs/Odometry.h"
#include "pcl_ros/point_cloud.h"


int main(int argc, char** argv) {
	ros::init(argc,argv,"vfh3d");
	ros::NodeHandle n;
	//ros::NodeHandle nh;
	std::string topic = n.resolveName("velodyne_points");
	uint32_t queue_size = 5;
	pubHandler handler = pubHandler(n,"vfh", 100);
	ros::Subscriber sub = n.subscribe<pcl::PointCloud<pcl::PointXYZ> >(topic,queue_size,&pubHandler::messageReceivedCloud, &handler);
	ros::Subscriber subod = n.subscribe<nav_msgs::Odometry>("aft_mapped_to_init_CORRECTED",queue_size,&pubHandler::messageReceivedPose, &handler);
	ros::spin();
	return 0;
}

