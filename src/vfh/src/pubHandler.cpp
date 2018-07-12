//============================================================================
// Name        : pubHandler.cpp
// Author      : Ryan Fite - ryanfite@live.com
// Version     : 1.0
// Date Created: Jul 6, 2018
// Description :
//============================================================================

#include "pubHandler.h"
//constructor for pubHandler class
pubHandler::pubHandler(ros::NodeHandle n, const std::string& s, int num){
	_pub = n.advertise<sensor_msgs::PointCloud2>(s,num);
	_queueSize = 7;
	_count = 2;
}

//used to publish data from the publisher and check for errors
void pubHandler::publish(pcl::PointCloud<pcl::PointXYZ> msg){
	try{
		_pub.publish(msg);
		ROS_INFO("Success");
	}catch(...){
		ROS_INFO("An error has occurred");
	}
}

//callback function for the subscriber. performs all of the enqueuing and dequeuing;
void pubHandler::messageReceivedCloud(const pcl::PointCloud<pcl::PointXYZ>::ConstPtr& msg){
	ROS_INFO("Recieving Cloud...");
	pcl::PointCloud<pcl::PointXYZ>::Ptr temp(new pcl::PointCloud<pcl::PointXYZ>(*msg));
	_data = temp;
	//ROS_INFO("Check 2");

	//ROS_INFO("Check 3");
	//ROS_INFO("Check 4");
}

//this recieves the odometry for the program to create the sliding window
void pubHandler::messageReceivedPose(const nav_msgs::Odometry::ConstPtr& msg){
	if(_count == 2){
	ROS_INFO("Receiving Odometry...");
	nav_msgs::Odometry odomData = *msg;
	if(_window.size() == _queueSize){
		_window.pop_back();
		_odomWindow.pop_back();
	}
	_window.push_front(_data);
	_odomWindow.push_front(odomData);
	//ROS_INFO("Check 2");
	pcl::PointCloud<pcl::PointXYZ> ptCloudScene = pcl::PointCloud<pcl::PointXYZ>(_preprocessing(_window, _odomWindow));
	//ROS_INFO("Check 3");
	this->publish(ptCloudScene);
	_count = 0;
	}else{
		ROS_INFO("SKIPPING...");
		_count++;
	}
	//ROS_INFO("Check 4");
}

//takes in two quaternions and gives back the quaternion that rotates from the starting quaternion to the next quaternion
Eigen::Quaterniond pubHandler::differenceOfQuat(Eigen::Quaterniond start, Eigen::Quaterniond end){
	Eigen::Quaterniond difference =  end * start.inverse();
	return difference;
}
Eigen::Vector3d pubHandler::differenceOfVec(Eigen::Vector3d start, Eigen::Vector3d end){
	Eigen::Vector3d difference = start-end;
	return difference;
}


//this is the pre-processing step that transforms and filters the point cloud queue
pcl::PointCloud<pcl::PointXYZ> pubHandler::_preprocessing(std::deque<pcl::PointCloud<pcl::PointXYZ>::Ptr > window, std::deque<nav_msgs::Odometry> odomWindow){
	pcl::PointCloud<pcl::PointXYZ>::Ptr ptCloudScene(new pcl::PointCloud<pcl::PointXYZ>(*window[0]));
	pcl::PointCloud<pcl::PointXYZ>::Ptr ptCloudSceneFiltered(new pcl::PointCloud<pcl::PointXYZ>);

	Eigen::Vector3d endV;
	Eigen::Vector3d startV;
	Eigen::Quaterniond endQ;
	Eigen::Quaterniond startQ;

	tf2::fromMsg(odomWindow[0].pose.pose.position, endV);
	tf2::fromMsg(odomWindow[0].pose.pose.orientation,endQ);
	//endV = endV.transpose() * endQ.toRotationMatrix();
	for(int i = 1; i<window.size();i++){
		tf2::fromMsg(odomWindow[i].pose.pose.position, startV);
		tf2::fromMsg(odomWindow[i].pose.pose.orientation,startQ);
		Eigen::Quaterniond differenceQ = differenceOfQuat(startQ,endQ);
		//startV = startV.transpose() * endQ.toRotationMatrix();
		Eigen::Vector3d differenceV = differenceOfVec(startV,endV);
		differenceV = differenceV.transpose() * endQ.toRotationMatrix();
		Eigen::Affine3d affine = Eigen::Affine3d::Identity();;
		affine.translation() << differenceV[0],differenceV[1],differenceV[2];
		affine.rotate(differenceQ.toRotationMatrix());
		pcl::PointCloud<pcl::PointXYZ>::Ptr transformedCloud(new pcl::PointCloud<pcl::PointXYZ>);
		pcl::transformPointCloud(*window[i],*transformedCloud,affine);
		*ptCloudScene += *transformedCloud;
	}
	pcl::VoxelGrid<pcl::PointXYZ> sor;
	sor.setInputCloud(ptCloudScene);
	sor.setLeafSize (0.075f, 0.075f, 0.075f);
	sor.filter(*ptCloudSceneFiltered);
	ROS_INFO("SUCCESS");
	return *ptCloudSceneFiltered;
}

//this is the primary algorithm used to determine the best vectors for travel
std::map<std::string,std::vector<trajectory> > pubHandler::_vfh3D(){
	std::map<std::string,std::vector<trajectory> > trajVec;
	return trajVec;

}
