/*
 * @Description: IMU-lidar fusion for localization workflow
 * @Author: Ge Yao
 * @Date: 2020-11-12 15:14:07
 */
#ifndef LIDAR_LOCALIZATION_FILTERING_FILTERING_FLOW_HPP_
#define LIDAR_LOCALIZATION_FILTERING_FILTERING_FLOW_HPP_

#include <ros/ros.h>

// subscriber:
// a. IMU:
#include "lidar_localization/subscriber/imu_subscriber.hpp"
// b. lidar:
#include "lidar_localization/subscriber/cloud_subscriber.hpp"
// c. GNSS:
#include "lidar_localization/subscriber/odometry_subscriber.hpp"
// d. lidar to imu:
#include "lidar_localization/tf_listener/tf_listener.hpp"

// publisher:
#include "lidar_localization/publisher/cloud_publisher.hpp"
#include "lidar_localization/publisher/odometry_publisher.hpp"
#include "lidar_localization/publisher/tf_broadcaster.hpp"

// filtering instance:
#include "lidar_localization/filtering/filtering.hpp"

namespace lidar_localization {

class FilteringFlow {
  public:
    FilteringFlow(ros::NodeHandle& nh);
    bool Run();

  private:
    bool ReadData();
    bool HasInited();
    
    bool HasData();

    bool HasIMUData(void) const { return !imu_raw_data_buff_.empty(); }
    bool HasLidarData(void) const { 
      return (
        !cloud_data_buff_.empty() && 
        !gnss_data_buff_.empty() && 
        !imu_synced_data_buff_.empty()
       );
    }
    bool HasIMUComesFirst(void) const { return imu_raw_data_buff_.front().time < cloud_data_buff_.front().time; }

    bool ValidIMUData();
    bool ValidLidarData();

    bool InitCalibration();
    bool InitLocalization();
    
    bool UpdateLocalization();
    bool CorrectLocalization();

    bool PublishGlobalMap();
    bool PublishLocalMap();
    bool PublishData();

  private:
    // subscriber:
    // a. IMU raw:
    std::shared_ptr<IMUSubscriber> imu_raw_sub_ptr_;
    std::deque<IMUData> imu_raw_data_buff_;
    // b. lidar:
    std::shared_ptr<CloudSubscriber> cloud_sub_ptr_;
    std::deque<CloudData> cloud_data_buff_;
    // c. GNSS:
    std::shared_ptr<OdometrySubscriber> gnss_sub_ptr_;
    std::deque<PoseData> gnss_data_buff_;
    // d. IMU synced:
    std::shared_ptr<IMUSubscriber> imu_synced_sub_ptr_;
    std::deque<IMUData> imu_synced_data_buff_;
    // e. lidar to imu tf:
    std::shared_ptr<TFListener> lidar_to_imu_ptr_;
    Eigen::Matrix4f lidar_to_imu_ = Eigen::Matrix4f::Identity();

    // publisher:
    // a. global-local map and current scan:
    std::shared_ptr<CloudPublisher> global_map_pub_ptr_;
    std::shared_ptr<CloudPublisher> local_map_pub_ptr_;
    std::shared_ptr<CloudPublisher> current_scan_pub_ptr_;
    // b. odometry:
    bool has_new_fused_odom_ = false, has_new_lidar_odom_ = false;
    std::shared_ptr<OdometryPublisher> fused_odom_pub_ptr_;
    std::shared_ptr<OdometryPublisher> laser_odom_pub_ptr_;
    // c. tf:
    std::shared_ptr<TFBroadCaster> laser_tf_pub_ptr_;

    // filtering instance:
    std::shared_ptr<Filtering> filtering_ptr_;

    IMUData current_imu_raw_data_;

    CloudData current_cloud_data_;
    PoseData current_gnss_data_;
    IMUData current_imu_synced_data_;

    Eigen::Matrix4f fused_pose_ = Eigen::Matrix4f::Identity();
    Eigen::Vector3f fused_vel_ = Eigen::Vector3f::Zero();
    Eigen::Matrix4f laser_pose_ = Eigen::Matrix4f::Identity();
};

} // namespace lidar_localization

#endif // LIDAR_LOCALIZATION_FILTERING_FILTERING_FLOW_HPP_