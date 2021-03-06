
#include <ros/ros.h>
#include <nav_msgs/OccupancyGrid.h>
#include <nav_msgs/MapMetaData.h>
#include <nav_msgs/Odometry.h>
#include <nav_msgs/Path.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Pose.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include "tca/graph.hpp"
#include "tca/io.hpp"
#include "tca/voronoi.hpp"

using namespace std;

string tca_path_topic = "shortest_path";

int num_paths;
string fixed_frame, occupancy_topic, odom_topic, goal_topic;
ros::Subscriber og_sub, odom_sub, goal_sub;
ros::Publisher shortest_pub;
bool odom_set = false, goal_set = false, map_init = false;
nav_msgs::Odometry current_position;
geometry_msgs::PoseStamped goal;
DynamicVoronoi dv;
bool **mp;

Index pose_to_index(geometry_msgs::Pose pose, nav_msgs::MapMetaData info)
{
    double ui = pose.position.x - info.origin.position.x;
    double uj = pose.position.y - info.origin.position.y;
    int i = (int) (ui / info.resolution);
    int j = (int) (uj / info.resolution);
    return Index(j, i);
}

geometry_msgs::Pose index_to_pose(Index ind, nav_msgs::MapMetaData info)
{
    geometry_msgs::Pose pose;
    pose.position.x = ind.j * info.resolution + info.origin.position.x;
    pose.position.y = ind.i * info.resolution + info.origin.position.y;
    return pose;
}

void og_to_map(nav_msgs::OccupancyGrid og)
{
    const int width = og.info.width;
    const int height = og.info.height;

    if (!map_init)
    {
        mp = new bool*[height];
        for (int i = 0; i < height; i++)
        {
            mp[i] = new bool[width];
        }
        map_init = true;
    }

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            if (i == width - 1 or j == height - 1 or i == 0 or j == 0)
            {
                mp[j][i] = true;
            }
            else
            {
                mp[j][i] = og.data[j * width + i] > 0;
            }
        }
    }
}

void occupancy_grid_callback(nav_msgs::OccupancyGrid og)
{
    if (odom_set and goal_set)
    {
        Graph G;
        vector<Index> ind_path;
        nav_msgs::Path path;
        og_to_map(og);
        dv.initializeMap(og.info.width, og.info.height, mp);
        Index start = pose_to_index(current_position.pose.pose, og.info);
        Index end = pose_to_index(goal.pose, og.info);
        generate_graph(start, end, dv, G);
        G.shortest_path(start, end, ind_path);
        for (int i = 0; i < ind_path.size(); i++)
        {
            geometry_msgs::PoseStamped ps;
            ps.header.frame_id = fixed_frame;
            ps.pose = index_to_pose(ind_path[i], og.info);
            path.poses.push_back(ps);
        }
        path.header.frame_id = fixed_frame;
        shortest_pub.publish(path);
    }
}

void odom_callback(nav_msgs::Odometry odom)
{
    current_position = odom;
    odom_set = true;
}

void goal_callback(geometry_msgs::PoseStamped ps)
{
    goal = ps;
    goal_set = true;
}

int main(int argc, char *argv[])
{
    ros::init(argc, argv, "tca");
    ros::NodeHandle n;

    ros::param::get("~num_paths", num_paths);
    ros::param::get("~fixed_frame", fixed_frame);
    ros::param::get("~occupancy_topic", occupancy_topic);
    ros::param::get("~odom_topic", odom_topic);
    ros::param::get("~goal_topic", goal_topic);

    og_sub = n.subscribe(occupancy_topic, 1, occupancy_grid_callback);
    odom_sub = n.subscribe(odom_topic, 1, odom_callback);
    goal_sub = n.subscribe(goal_topic, 1, goal_callback);
    shortest_pub = n.advertise<nav_msgs::Path>(tca_path_topic, 1);
    ros::spin();

    return 0;
}
