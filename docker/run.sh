#!/usr/bin/env bash
# 사용: docker/run.sh [이미지] [gui|headless]
#   기본 이미지 imova-demo:latest, DISPLAY 가 있으면 gui
#   호스트에서 ros2 topic / service 로 바로 접근 가능 (--net=host)
set -euo pipefail
IMAGE=${1:-imova-demo:latest}
MODE=${2:-$([ -n "${DISPLAY:-}" ] && echo gui || echo headless)}

ARGS=(--rm -i --name "${CONTAINER_NAME:-imova-demo}" --init --net=host --ipc=host
      # 호스트 사용자로 실행: root 로 돌면 Fast DDS 공유메모리(/dev/shm)를 호스트가 못 읽어
      # 호스트의 ros2 명령이 컨테이너 토픽을 받지 못한다
      --user "$(id -u):$(id -g)" -e HOME=/tmp
      -e ROS_DOMAIN_ID="${ROS_DOMAIN_ID:-0}")

[ -t 0 ] && ARGS+=(-t)
if [ "$MODE" = gui ]; then
  xhost +local: >/dev/null
  ARGS+=(-e DISPLAY="$DISPLAY" -v /tmp/.X11-unix:/tmp/.X11-unix:ro)
fi
exec docker run "${ARGS[@]}" "$IMAGE"
