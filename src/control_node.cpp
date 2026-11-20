// Motion Controller 역할: Navigation 명령을 Safety 상태로 게이팅해 turtle 에 전달
#include <memory>

#include "demo_interfaces/msg/safety_state.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/rclcpp.hpp"

using demo_interfaces::msg::SafetyState;
using geometry_msgs::msg::Twist;
using namespace std::chrono_literals;

class ControlNode : public rclcpp::Node
{
public:
  ControlNode()
  : Node("demo_control")
  {
    cmd_timeout_ = declare_parameter("cmd_timeout", 0.5);
    safety_timeout_ = declare_parameter("safety_timeout", 0.5);

    pub_ = create_publisher<Twist>("/turtle1/cmd_vel", 10);
    cmd_sub_ = create_subscription<Twist>(
      "/demo/nav_cmd_vel", 10,
      [this](Twist::SharedPtr m) {last_cmd_ = *m; last_cmd_time_ = now();});
    safety_sub_ = create_subscription<SafetyState>(
      "/demo/safety_state", 10,
      [this](SafetyState::SharedPtr m) {on_safety(*m);});
    timer_ = create_wall_timer(50ms, [this]() {tick();});
  }

private:
  void on_safety(const SafetyState & s)
  {
    if (s.state != safety_.state) {
      RCLCPP_WARN(get_logger(), "safety %s %s",
        s.state == SafetyState::SAFE ? "SAFE" : "EMERGENCY_STOP", s.reason.c_str());
    }
    safety_ = s;
    last_safety_time_ = now();
  }

  void tick()
  {
    Twist out;  // 기본 정지
    const bool safety_alive =
      last_safety_time_.nanoseconds() > 0 &&
      (now() - last_safety_time_).seconds() < safety_timeout_;
    const bool cmd_alive =
      last_cmd_time_.nanoseconds() > 0 &&
      (now() - last_cmd_time_).seconds() < cmd_timeout_;
    // Safety 상태가 끊기면(fail-safe) 정지
    if (safety_alive && safety_.state == SafetyState::SAFE && cmd_alive) {
      out = last_cmd_;
    }
    pub_->publish(out);
  }

  double cmd_timeout_, safety_timeout_;
  Twist last_cmd_;
  SafetyState safety_;
  rclcpp::Time last_cmd_time_ {0, 0, RCL_ROS_TIME};
  rclcpp::Time last_safety_time_ {0, 0, RCL_ROS_TIME};
  rclcpp::Publisher<Twist>::SharedPtr pub_;
  rclcpp::Subscription<Twist>::SharedPtr cmd_sub_;
  rclcpp::Subscription<SafetyState>::SharedPtr safety_sub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ControlNode>());
  rclcpp::shutdown();
  return 0;
}
