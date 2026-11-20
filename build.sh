#!/usr/bin/env bash
# 사용: ./build.sh [repos 파일]   (기본 demo.repos)
#   ./build.sh demo.release.repos
set -euo pipefail
cd "$(dirname "$0")"
REPOS=${1:-demo.repos}

# platformio 등 다른 python venv 가 PATH 에 있으면 ament 가 catkin_pkg 를 못 찾음
export PATH=$(echo "$PATH" | tr ':' '\n' | grep -v platformio | paste -sd:)
set +u; source /opt/ros/humble/setup.bash; set -u   # ROS setup 스크립트는 nounset 미지원

mkdir -p src
vcs import --force --recursive src < "$REPOS"
vcs export --exact src > .last_build.repos   # 실제로 빌드한 커밋 기록
colcon build --symlink-install --base-paths src --event-handlers console_cohesion-
echo "OK: source install/setup.bash && ros2 launch demo_bringup demo.launch.py"
