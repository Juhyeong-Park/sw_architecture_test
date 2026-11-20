# sw_architecture_test — 레포 분리 / 통합 빌드 데모

iMOVA1000 를 아키텍처 단위 레포로 나누고 `.repos`(vcstool) 로 모아 빌드하는 방식을 turtlesim 으로 미리 검증하는 데모.

> GitHub 레포를 여러 개 만들지 않고 **이 레포 하나의 브랜치 = 레포 하나** 로 흉내낸다.
> 실제 분리 시에는 `.repos` 의 `url` 만 레포별로 바꾸면 된다.

| 브랜치 (= 가상 레포) | 패키지 | 아키텍처 역할 | 언어 |
|---|---|---|---|
| `main` | (통합 레포) `.repos`, `build.sh`, CI | iMOVA1000 통합 레포 | - |
| `pkg/demo_interfaces` | `demo_interfaces` (`SafetyState.msg`) | imova_interfaces | msg |
| `pkg/demo_localization` | `demo_localization` | Localization | Python |
| `pkg/demo_navigation` | `demo_navigation` | Navigation (waypoint 추종) | C++ |
| `pkg/demo_safety` | `demo_safety` | Safety Supervisor (보호 영역 / E-stop latch) | Python |
| `pkg/demo_control` | `demo_control` | Motion Controller (Safety 게이팅) | C++ |
| `pkg/demo_bringup` | `demo_bringup` | 기동 구성 (launch / 파라미터) | launch |
| `pkg/demo_mathlib` | `demo_mathlib` (header-only) | third-party 역할 | C++ |

## 구조

```
turtlesim ──/turtle1/pose──▶ demo_localization ──/demo/pose──▶ demo_navigation
    ▲                 └────▶ demo_safety ──/demo/safety_state──┐        │ /demo/nav_cmd_vel
    │                                                          ▼        ▼
    └──────────────/turtle1/cmd_vel──────────────────────── demo_control
```

레포 간 빌드 의존:

```
demo_control    ──▶ demo_interfaces        (msg, 레포 경계를 넘는 의존)
demo_safety     ──▶ demo_interfaces
demo_navigation ──▶ demo_mathlib >= 1.1.0  (third-party 버전 의존)
demo_bringup    ──▶ 전부 (exec)
```

## 실행

```bash
sudo apt install python3-vcstool ros-humble-turtlesim

git clone https://github.com/Juhyeong-Park/sw_architecture_test.git demo_ws
cd demo_ws
./build.sh                       # 개발용: demo.repos (각 브랜치 최신)
# ./build.sh demo.release.repos  # 릴리스용: 태그 고정

source install/setup.bash
ros2 launch demo_bringup demo.launch.py
```

거북이가 (2,2) → (9,2) → (9,9) → (2,9) 사각형을 반복 주행한다.

```bash
ros2 service call /demo/estop std_srvs/srv/Trigger          # 비상정지 → 즉시 정지 (latch)
ros2 service call /demo/safety_reset std_srvs/srv/Trigger   # 해제 → 주행 재개
```

## 확인 시나리오

| # | 확인 항목 | 방법 | 기대 결과 |
|---|---|---|---|
| 1 | 통합 import / 빌드 | `./build.sh` | 7개 패키지 빌드, `src/core`, `src/third_party` 로 배치 |
| 2 | 레포 간 msg 의존 | `colcon graph` | `demo_control`, `demo_safety` 가 `demo_interfaces` 뒤에 빌드 |
| 3 | 버전 고정 | `demo.release.repos` 의 `demo_mathlib/v1.1.0` → `v1.0.0` 으로 바꿔 빌드 | `demo_navigation` 빌드 실패 (`find_package(demo_mathlib 1.1.0)` / `clamp` 없음) |
| 4 | 인터페이스 변경 전파 | `pkg/demo_interfaces` 에 필드 추가 → `./build.sh` | 의존 패키지만 재빌드, 미사용 쪽은 영향 없음 |
| 5 | 개발 / 릴리스 분리 | `pkg/*` 에 커밋 후 두 `.repos` 로 각각 빌드 | `demo.repos` 만 변경 반영, release 는 태그 그대로 |
| 6 | 부분 빌드 | `colcon build --packages-up-to demo_control` | interfaces + control 만 빌드 |
| 7 | 빌드 기록 | `.last_build.repos` | 실제 빌드한 커밋 해시가 남음 (릴리스 `.repos` 작성에 그대로 사용) |
| 8 | Safety fail-safe | `demo_safety` 노드만 종료 | `safety_timeout`(0.5 s) 후 `demo_control` 이 정지 |
| 9 | CI | `main` 에 push / PR | GitHub Actions 가 두 `.repos` 로 각각 통합 빌드 |

## 새 버전 릴리스 절차 (예: demo_mathlib)

```bash
git checkout pkg/demo_mathlib
# 수정 / package.xml version 올림 / 커밋
git tag demo_mathlib/v1.2.0 && git push origin pkg/demo_mathlib demo_mathlib/v1.2.0
# main 의 demo.release.repos 에서 version 을 demo_mathlib/v1.2.0 으로 변경 → PR
```

## 참고

- `build.sh` 는 PATH 에서 platformio venv 를 제외한다 (venv 의 python 에 `catkin_pkg` 가 없어 ament 빌드 실패).
- turtlesim 창 없이 돌리려면 `QT_QPA_PLATFORM=offscreen`.
