#include "ros/ros.h"
#include "tf2_ros/transform_listener.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.h"
#include "geometry_msgs/TransformStamped.h"
#include "geometry_msgs/Twist.h"
#include "nav_msgs/Path.h"
#include "dock/ApproachPath.h"
#include <string>
#include <cmath>
#include <algorithm>


std::string g_target_frame_id;
std::string g_base_frame_id;
double g_dt_ms;
double g_P_theta;
double g_I_theta;
double g_D_theta;
double g_dThetaMax;
double g_close_range;
double g_dx_approach;
double g_dx_close_approach;
tf2_ros::Buffer g_tfBuffer;
ros::Publisher g_pathPublisher;
ros::Publisher g_commandPublisher;



bool GetRobotTransform(geometry_msgs::TransformStamped * o_robotTransform)
{
    geometry_msgs::TransformStamped robotTransform;
    try {
        robotTransform = g_tfBuffer.lookupTransform(
                g_target_frame_id,
                g_base_frame_id,
                ros::Time(0));
    }
    catch (...) {
        return false;
    }

    *o_robotTransform = robotTransform;
    return true;
}

void FollowPath(const dock::ApproachPath & path)
{
    static double lastThetaError = 0;
    static double totalThetaError = 0;

    double currentThetaError = path.getAngleError();
    double derivativeThetaError = (lastThetaError - currentThetaError) / g_dt_ms;
    lastThetaError = currentThetaError;
    totalThetaError += currentThetaError * g_dt_ms;

    // Do a PID correction
    double thetaTwist =
        -g_P_theta * currentThetaError +
        -g_I_theta * totalThetaError +
        g_D_theta * derivativeThetaError;

    // Constrain Twist command
    thetaTwist = std::clamp(thetaTwist, -std::abs(g_dThetaMax), std::abs(g_dThetaMax));

    // Compute forward speed
    double pathLength = path.getLength(); // positive => past collision
    double forwardSpeed;
    if (std::abs(currentThetaError) > 0.39) // ~pi/8
        forwardSpeed = 0;
    else if (pathLength < g_close_range)
        forwardSpeed = g_dx_approach;
    else if (pathLength < 0)
        forwardSpeed = g_dx_close_approach;
    else
    {   // past collision
        forwardSpeed = 0;
        thetaTwist = 0;
    }


    // Publish twist
    geometry_msgs::Twist motorOutput;
    motorOutput.linear.x = forwardSpeed;
    motorOutput.linear.y = 0;
    motorOutput.linear.z = 0;
    motorOutput.angular.x = 0;
    motorOutput.angular.y = 0;
    motorOutput.angular.z = thetaTwist;

    g_commandPublisher.publish(motorOutput);
}


void DockRoutine()
{
    geometry_msgs::TransformStamped robotTransform;

    // Obtain robot transform
    if (! GetRobotTransform(&robotTransform))
    {
        ROS_INFO_THROTTLE(10, "No transform to target");
        return;
    }

    // Compute path
    dock::ApproachPath path(robotTransform);
    g_pathPublisher.publish(path.getPath());

    // Compute robot commands
    FollowPath(path);
}


void LoadParameters(ros::NodeHandle * nh)
{
    nh->param("planner/target_frame_id", g_target_frame_id, std::string("target"));
    nh->param("planner/base_frame_id", g_base_frame_id, std::string("base_footprint"));
    nh->param("planner/dt_ms", g_dt_ms, 100.0);
    nh->param("planner/P_theta", g_P_theta, 0.0);
    nh->param("planner/I_theta", g_I_theta, 0.0);
    nh->param("planner/D_theta", g_D_theta, 0.0);
    nh->param("planner/dThetaMax", g_dThetaMax, 100.0);
    nh->param("planner/close_range", g_close_range, -1.0);
    nh->param("planner/dx_approach", g_dx_approach, 0.4);
    nh->param("planner/dx_close_approach", g_dx_close_approach, 0.2);
}

int main (int argc, char **argv)
{
    ros::init(argc, argv, "planner");

    ros::NodeHandle nh;

    // Parameters
    LoadParameters(&nh);

    // Initializing the transform buffer
    tf2_ros::TransformListener tfListener(g_tfBuffer);


    // Init publisher
    g_pathPublisher = nh.advertise<nav_msgs::Path>("approach_plan", 10);
    g_commandPublisher = nh.advertise<geometry_msgs::Twist>("cmd_vel", 10);


    ros::Rate rate(1/(g_dt_ms/1000.0));
    while(nh.ok())
    {
        DockRoutine();

        rate.sleep();
    }

    return 0;
}
