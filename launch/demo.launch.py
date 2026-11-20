from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    params = PathJoinSubstitution([FindPackageShare('demo_bringup'), 'config', 'demo.yaml'])
    use_sim = LaunchConfiguration('use_turtlesim')
    return LaunchDescription([
        DeclareLaunchArgument('use_turtlesim', default_value='true'),
        Node(package='turtlesim', executable='turtlesim_node', name='sim',
             condition=IfCondition(use_sim)),
        Node(package='demo_localization', executable='localization_node', parameters=[params]),
        Node(package='demo_navigation', executable='navigation_node', parameters=[params]),
        Node(package='demo_safety', executable='safety_node', parameters=[params]),
        Node(package='demo_control', executable='control_node', parameters=[params]),
    ])
