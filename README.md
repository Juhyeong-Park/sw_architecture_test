# sw_architecture_test — 레포 분리 / 통합 빌드 데모

iMOVA1000 를 아키텍처 단위 레포로 나누고 `.repos`(vcstool) 로 모아 빌드하는 방식을 turtlesim 으로 미리 검증하는 데모.

이 레포(공개)는 **통합 레포** 이고, 기능 패키지는 각각 별도 **비공개** 레포다.

| 레포 | 태그 | 아키텍처 역할 | 언어 |
|---|---|---|---|
| `sw_architecture_test` (이 레포) | - | iMOVA1000 통합 레포: `.repos`, `build.sh`, CI | - |
| `Juhyeong-Park/demo_interfaces` | v0.1.0 | imova_interfaces (`SafetyState.msg`) | msg |
| `Juhyeong-Park/demo_localization` | v0.1.0 | Localization | Python |
| `Juhyeong-Park/demo_navigation` | v0.1.0 | Navigation (waypoint 추종) | C++ |
| `Juhyeong-Park/demo_safety` | v0.1.0 | Safety Supervisor (보호 영역 / E-stop latch) | Python |
| `Juhyeong-Park/demo_control` | v0.1.0 | Motion Controller (Safety 게이팅) | C++ |
| `Juhyeong-Park/demo_bringup` | v0.1.0 | 기동 구성 (launch / 파라미터) | launch |
| `Juhyeong-Park/demo_mathlib` | v1.0.0, v1.1.0 | third-party 역할 (header-only) | C++ |

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
# 기능 레포가 비공개이므로 GitHub 에 SSH 키가 등록된 계정이어야 한다 (ssh -T git@github.com)

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

## Docker

이미지 안에 7개 레포를 받아 빌드한 결과(`install/`)만 담는다. 빌드 단계(소스 / 컴파일)는 최종 이미지에 남지 않는다.

```bash
# 빌드 — 비공개 레포 인증 (둘 중 하나, 키 / 토큰은 이미지에 남지 않음)
docker build --ssh default --build-arg REPOS=demo.release.repos -t imova-demo:0.1.0 .
GH_TOKEN=<토큰> docker build --secret id=gh_token,env=GH_TOKEN \
  --build-arg REPOS=demo.release.repos -t imova-demo:0.1.0 .

# 실행
docker/run.sh imova-demo:0.1.0            # DISPLAY 있으면 turtlesim 창, 없으면 창 없이
docker/run.sh imova-demo:0.1.0 headless

# 이미지에 들어간 레포별 커밋
docker run --rm imova-demo:0.1.0 cat /ws/exact.repos
```

- 호스트에서 `ros2 topic echo /turtle1/pose`, `ros2 service call /demo/estop ...` 로 컨테이너에 바로 접근한다 (`--net=host`).
- 컨테이너는 **호스트 사용자로** 실행한다 (`run.sh` 의 `--user`). root 로 실행하면 Fast DDS 공유메모리 파일을 호스트가 못 읽어 호스트 쪽 `ros2` 명령이 데이터를 받지 못한다.
- snap 으로 설치한 docker 는 SSH agent 전달과 `/tmp` 아래 빌드 컨텍스트를 지원하지 않는다 → 토큰 방식, 홈 디렉터리에서 빌드.
- GUI 실행 시 `libGL error: failed to load driver` 경고는 소프트웨어 렌더링으로 넘어간다는 뜻이며 동작에는 영향 없음.

## 확인 시나리오

| # | 확인 항목 | 방법 | 기대 결과 |
|---|---|---|---|
| 1 | 통합 import / 빌드 | `./build.sh` | 7개 패키지 빌드, `src/core`, `src/third_party` 로 배치 |
| 2 | 레포 간 msg 의존 | `colcon graph` | `demo_control`, `demo_safety` 가 `demo_interfaces` 뒤에 빌드 |
| 3 | 버전 고정 | `demo.release.repos` 의 `demo_mathlib` 를 `v1.1.0` → `v1.0.0` 으로 바꿔 빌드 | `demo_navigation` 빌드 실패 (`find_package(demo_mathlib 1.1.0)` / `clamp` 없음) |
| 4 | 인터페이스 변경 전파 | `demo_interfaces` 에 필드 추가 → `./build.sh` | 의존 패키지만 재빌드, 미사용 쪽은 영향 없음 |
| 5 | 개발 / 릴리스 분리 | 기능 레포 main 에 커밋 후 두 `.repos` 로 각각 빌드 | `demo.repos` 만 변경 반영, release 는 태그 그대로 |
| 6 | 부분 빌드 | `colcon build --packages-up-to demo_control` | interfaces + control 만 빌드 |
| 7 | 빌드 기록 | `.last_build.repos` | 실제 빌드한 커밋 해시가 남음 (릴리스 `.repos` 작성에 그대로 사용) |
| 8 | Safety fail-safe | `demo_safety` 노드만 종료 | `safety_timeout`(0.5 s) 후 `demo_control` 이 정지 |
| 9 | CI | `main` 에 push / PR | GitHub Actions 가 두 `.repos` 로 각각 통합 빌드 (비공개 레포는 토큰으로 접근) |
| 10 | 접근 권한 | SSH 키 없는 환경에서 `./build.sh` | 비공개 레포 import 실패 — 레포별 권한 관리 확인 |
| 11 | Docker 이미지 | `docker build` → `docker/run.sh` | 7개 레포가 담긴 이미지로 주행, 호스트에서 E-stop 호출 가능 |

## 새 버전 릴리스 절차 (예: demo_mathlib)

```bash
cd src/third_party/demo_mathlib
# 수정 / package.xml version 올림 / 커밋
git tag v1.2.0 && git push origin main v1.2.0
# 이 레포의 demo.release.repos 에서 demo_mathlib version 을 v1.2.0 으로 변경 → PR
```

## CI 설정 (비공개 레포 접근)

GitHub Actions 는 SSH 키가 없으므로 토큰으로 기능 레포를 받는다.

1. 기능 레포 7개에 읽기 권한이 있는 토큰 준비 (fine-grained PAT 권장: 대상 레포 7개, `Contents: Read-only`)
2. 이 레포 → Settings → Secrets and variables → Actions → `DEMO_REPOS_TOKEN` 으로 등록
3. CI 가 `git@github.com:` 주소를 `https://x-access-token:<토큰>@github.com/` 으로 바꿔 import 한다

## 참고

- `build.sh` 는 PATH 에서 platformio venv 를 제외한다 (venv 의 python 에 `catkin_pkg` 가 없어 ament 빌드 실패).
- turtlesim 창 없이 돌리려면 `QT_QPA_PLATFORM=offscreen`.
