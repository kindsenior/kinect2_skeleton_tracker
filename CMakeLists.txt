cmake_minimum_required(VERSION 2.8.3)
project(kinect2_skeleton_tracker)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  kinect2_skeleton_tracker_win
)

find_package(catkin REQUIRED COMPONENTS roscpp rospy std_msgs tf kinect2_skeleton_tracker_win)

catkin_package()

add_executable(kinect2_skeleton_tracker kinect2_skeleton_tracker.cpp)
target_link_libraries(kinect2_skeleton_tracker zmq ${catkin_LIBRARIES})

