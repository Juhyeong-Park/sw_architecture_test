#!/bin/bash
set -e
source /opt/ros/${ROS_DISTRO}/setup.bash
source /ws/install/setup.bash
# DISPLAY 가 없으면 turtlesim 을 창 없이 실행
[ -z "$DISPLAY" ] && export QT_QPA_PLATFORM=offscreen
exec "$@"
