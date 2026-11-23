# syntax=docker/dockerfile:1
# iMOVA 데모 통합 이미지
# 비공개 레포 인증 (둘 중 하나, 이미지 / 레이어에 남지 않음)
#   SSH agent : docker build --ssh default --build-arg REPOS=demo.release.repos -t imova-demo:0.1.0 .
#   토큰      : GH_TOKEN=... docker build --secret id=gh_token,env=GH_TOKEN ...
#               (snap 으로 설치한 docker 는 SSH agent 전달이 안 되므로 토큰 방식 사용)

ARG ROS_DISTRO=humble

# ---------------------------------------------------------------- build
FROM ros:${ROS_DISTRO}-ros-base AS build
ARG REPOS=demo.repos

RUN apt-get update && apt-get install -y --no-install-recommends \
      python3-vcstool openssh-client git \
      ros-${ROS_DISTRO}-turtlesim \
    && rm -rf /var/lib/apt/lists/*
RUN mkdir -p -m 0700 ~/.ssh && ssh-keyscan github.com >> ~/.ssh/known_hosts

WORKDIR /ws
COPY ${REPOS} /ws/build.repos
RUN --mount=type=ssh --mount=type=secret,id=gh_token \
    if [ -s /run/secrets/gh_token ]; then \
      export GIT_CONFIG_COUNT=1 \
             GIT_CONFIG_KEY_0="url.https://x-access-token:$(cat /run/secrets/gh_token)@github.com/.insteadOf" \
             GIT_CONFIG_VALUE_0="git@github.com:"; \
    fi \
    && mkdir src && vcs import --recursive src < build.repos \
    && vcs export --exact src > /ws/exact.repos

RUN . /opt/ros/${ROS_DISTRO}/setup.sh \
    && colcon build --merge-install --event-handlers console_cohesion- \
         --cmake-args -DCMAKE_BUILD_TYPE=Release

# ---------------------------------------------------------------- runtime
FROM ros:${ROS_DISTRO}-ros-base AS runtime
ARG ROS_DISTRO

RUN apt-get update && apt-get install -y --no-install-recommends \
      ros-${ROS_DISTRO}-turtlesim \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /ws/install /ws/install
# 이 이미지에 들어간 레포별 커밋 (추적용)
COPY --from=build /ws/exact.repos /ws/exact.repos

COPY docker/entrypoint.sh /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
CMD ["ros2", "launch", "demo_bringup", "demo.launch.py"]
