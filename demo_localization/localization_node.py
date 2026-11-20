"""Localization 역할: turtlesim 의 실제 위치에 노이즈를 더해 추정 위치로 발행."""
import random

import rclpy
from geometry_msgs.msg import Pose2D
from rclpy.node import Node
from turtlesim.msg import Pose


class LocalizationNode(Node):

    def __init__(self):
        super().__init__('demo_localization')
        self.declare_parameter('noise_std', 0.02)
        self.noise = self.get_parameter('noise_std').value
        self.pub = self.create_publisher(Pose2D, '/demo/pose', 10)
        self.create_subscription(Pose, '/turtle1/pose', self.on_pose, 10)

    def on_pose(self, msg: Pose):
        out = Pose2D()
        out.x = msg.x + random.gauss(0.0, self.noise)
        out.y = msg.y + random.gauss(0.0, self.noise)
        out.theta = msg.theta
        self.pub.publish(out)


def main():
    rclpy.init()
    node = LocalizationNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.try_shutdown()
