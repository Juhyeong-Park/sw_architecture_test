// Navigation 역할: waypoint 를 순서대로 추종하는 cmd_vel 을 생성 (demo_mathlib 사용)
#include <memory>
#include <vector>

#include "demo_mathlib/geometry.hpp"
#include "geometry_msgs/msg/pose2_d.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/rclcpp.hpp"

class NavigationNode : public rclcpp::Node
{
public:
  NavigationNode()
  : Node("demo_navigation")
  {
    // [x0, y0, x1, y1, ...]
    waypoints_ = declare_parameter<std::vector<double>>(
      "waypoints", {2.0, 2.0, 9.0, 2.0, 9.0, 9.0, 2.0, 9.0});
    max_lin_ = declare_parameter("max_linear", 2.0);
    max_ang_ = declare_parameter("max_angular", 2.0);
    tolerance_ = declare_parameter("tolerance", 0.2);

    pub_ = create_publisher<geometry_msgs::msg::Twist>("/demo/nav_cmd_vel", 10);
    sub_ = create_subscription<geometry_msgs::msg::Pose2D>(
      "/demo/pose", 10,
      [this](geometry_msgs::msg::Pose2D::SharedPtr msg) {on_pose(*msg);});
  }

private:
  void on_pose(const geometry_msgs::msg::Pose2D & p)
  {
    const size_t n = waypoints_.size() / 2;
    if (n == 0) {return;}
    const double gx = waypoints_[2 * idx_], gy = waypoints_[2 * idx_ + 1];

    const double dist = demo_mathlib::distance(p.x, p.y, gx, gy);
    if (dist < tolerance_) {
      idx_ = (idx_ + 1) % n;
      RCLCPP_INFO(get_logger(), "waypoint reached -> next #%zu", idx_);
      return;
    }
    const double heading_err =
      demo_mathlib::normalize_angle(std::atan2(gy - p.y, gx - p.x) - p.theta);

    geometry_msgs::msg::Twist cmd;
    // clamp 는 demo_mathlib v1.1.0 부터 제공 (버전 고정 테스트 포인트)
    cmd.angular.z = demo_mathlib::clamp(4.0 * heading_err, -max_ang_, max_ang_);
    cmd.linear.x = std::fabs(heading_err) > 0.5 ? 0.0 :
      demo_mathlib::clamp(1.5 * dist, 0.0, max_lin_);
    pub_->publish(cmd);
  }

  std::vector<double> waypoints_;
  double max_lin_, max_ang_, tolerance_;
  size_t idx_ {0};
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_;
  rclcpp::Subscription<geometry_msgs::msg::Pose2D>::SharedPtr sub_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<NavigationNode>());
  rclcpp::shutdown();
  return 0;
}
