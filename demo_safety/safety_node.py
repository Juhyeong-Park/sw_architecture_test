"""Safety Supervisor 역할: 보호 영역(벽 근접) 침범 또는 비상정지 요청 시 EMERGENCY_STOP 발행.

- /demo/estop        (std_srvs/Trigger): 비상정지 버튼 시뮬레이션
- /demo/safety_reset (std_srvs/Trigger): 해제. 보호 영역 안이면 거부 (latch)
"""
import rclpy
from demo_interfaces.msg import SafetyState
from rclpy.node import Node
from std_srvs.srv import Trigger
from turtlesim.msg import Pose

WORLD_MIN = 0.0
WORLD_MAX = 11.0


class SafetyNode(Node):

    def __init__(self):
        super().__init__('demo_safety')
        self.declare_parameter('stop_margin', 0.8)
        self.margin = self.get_parameter('stop_margin').value
        self.latched = False
        self.in_field = False
        self.reason = ''
        self.pub = self.create_publisher(SafetyState, '/demo/safety_state', 10)
        self.create_subscription(Pose, '/turtle1/pose', self.on_pose, 10)
        self.create_service(Trigger, '/demo/estop', self.on_estop)
        self.create_service(Trigger, '/demo/safety_reset', self.on_reset)
        self.create_timer(0.05, self.publish_state)

    def on_pose(self, p: Pose):
        lo, hi = WORLD_MIN + self.margin, WORLD_MAX - self.margin
        self.in_field = not (lo < p.x < hi and lo < p.y < hi)
        if self.in_field and not self.latched:
            self.latched = True
            self.reason = f'FIELD_STOP at ({p.x:.2f}, {p.y:.2f})'
            self.get_logger().warn(self.reason)

    def on_estop(self, _req, res):
        self.latched, self.reason = True, 'ESTOP requested'
        self.get_logger().warn(self.reason)
        res.success, res.message = True, 'latched'
        return res

    def on_reset(self, _req, res):
        if self.in_field:
            res.success, res.message = False, 'still inside protective field'
        else:
            self.latched, self.reason = False, ''
            res.success, res.message = True, 'reset'
            self.get_logger().info('safety reset')
        return res

    def publish_state(self):
        msg = SafetyState()
        msg.state = SafetyState.EMERGENCY_STOP if self.latched else SafetyState.SAFE
        msg.reason = self.reason
        self.pub.publish(msg)


def main():
    rclpy.init()
    node = SafetyNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.try_shutdown()
