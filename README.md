# dali-widget-sample

DALi native C++ widget samples for Tizen.

## Samples

| Sample | Type | Description |
|--------|------|-------------|
| [widget-app-template](widget-app-template/) | widget-application (provider) | Bare minimum widget provider using `Dali::WidgetApplication`. |
| [widget-viewer-template](widget-viewer-template/) | ui-application (viewer) | Bare minimum widget viewer using `Dali::WidgetView::WidgetViewManager`. |
| [widget-app-interactive](widget-app-interactive/) | widget-application (provider) | Advanced provider demonstrating MessagePort, Resize handling, and Content updates. |
| [widget-viewer-interactive](widget-viewer-interactive/) | ui-application (viewer) | Advanced viewer demonstrating key-triggered MessagePort IPC and Resize testing. |
| [widget-app-component](widget-app-component/) | component-application (provider) | **[TODO]** CBA widget provider scaffold — widget rendering not yet supported in DALi C++ (see note below). |
| [widget-viewer-component](widget-viewer-component/) | ui-application (viewer) | **[TODO]** Viewer for CBA widget components — not yet functional. |

The viewer loads widget id `class1@com.samsung.dali.widget-app-template`, which is provided by the app template.

## Build

### Build all samples at once

Run the wrapper script from the repository root:

```bash
./build-all.sh                  # default: -A armv7l
./build-all.sh -A aarch64       # override architecture
./build-all.sh --clean-once     # extra gbs options are forwarded
```

The script runs `gbs build --include-all --packaging-dir <sample>/packaging` for every sample in sequence and stops on the first failure.

### Build a single sample

```bash
gbs build -A armv7l --include-all --packaging-dir widget-app-template/packaging
gbs build -A armv7l --include-all --packaging-dir widget-viewer-template/packaging
```

## Install & Run

Install the provider first, then the viewer, then launch the viewer:

```bash
tpk-backend --preload -y com.samsung.dali.widget-app-template
tpk-backend --preload -y com.samsung.dali.widget-viewer-template
app_launcher -s com.samsung.dali.widget-viewer-template
```

The provider process is spawned automatically when the viewer calls `AddWidget`.

## Test Checklist

### 1. Template (`widget-app-template` + `widget-viewer-template`)

**Setup**
```bash
tpk-backend --preload -y com.samsung.dali.widget-app-template
tpk-backend --preload -y com.samsung.dali.widget-viewer-template
app_launcher -s com.samsung.dali.widget-viewer-template
```

| # | Test | Expected |
|---|------|----------|
| T-1 | 앱 실행 | Red(class1) / Blue(class2) 위젯이 화면에 나란히 표시됨 |
| T-2 | Blue 위젯 자동 toggle | 2초 간격으로 Blue 위젯이 사라졌다 나타남을 반복 |
| T-3 | WidgetAdded/WidgetDeleted 로그 | `dlogutil DALI_WIDGET_TEST` 에서 `[VIEWER] WidgetAdded/WidgetDeleted` 출력 확인 |
| T-4 | 초기 ContentInfo 전달 | provider `OnCreate` 로그에 `COUNT=1` 출력 확인 |
| T-5 | Back/Escape 키 | 앱 정상 종료, `OnTerminate` 로그 출력 |
| T-6 | 위젯 강제 종료 후 Re-load | `app_launcher -k com.samsung.dali.widget-app-template` 으로 provider 강제 종료 → `dlogutil DALI_WIDGET_TEST` 에서 `WidgetFaulted` 로그 확인 → 위젯이 자동으로 다시 로드되어 화면에 표시됨. 2~3회 빠르게 반복해도 크래시 없이 정상 복구되는지 확인 |
| T-7 | 위젯 정상 종료 후 Re-load | `app_launcher -t com.samsung.dali.widget-app-template` 으로 provider 정상 종료 후 위젯이 자동으로 다시 로드되어 화면에 표시됨 |
| T-8 | 메모리 측정 | `memps -v \| grep widget` 으로 viewer/provider 메모리 사용량 확인. 위젯 add/remove 반복 후 메모리가 비정상적으로 증가하지 않는지 확인 |

---

### 2. Interactive (`widget-app-interactive` + `widget-viewer-interactive`)

**Setup**
```bash
tpk-backend --preload -y com.samsung.dali.widget-app-interactive
tpk-backend --preload -y com.samsung.dali.widget-viewer-interactive
app_launcher -s com.samsung.dali.widget-viewer-interactive
```

| # | Test | Expected |
|---|------|----------|
| I-1 | 앱 실행 | Red 위젯(pulse 애니메이션) / Blue 위젯 표시 |
| I-2 | ContentInfo 주기적 업데이트 | 5초마다 viewer 로그에 `WidgetContentUpdated → COUNT: N` 출력 (N 증가) |
| I-3 | `1` 또는 Return 키 — 위젯 리사이즈 | Red 위젯 크기 200px씩 증가 (200→400→…→1000→wrap) |
| I-4 | OnResize 콜백 | provider 로그에 `OnResize: widget was resized to width=N, height=N` 출력, 위젯 label에 크기 표시 |
| I-5 | MessagePort IPC | 리사이즈 후 provider label이 `Red Widget\nMsg: Viewer resized width:N` 으로 업데이트 |
| I-6 | Blue 위젯 자동 toggle | 4초 간격으로 Blue 위젯 add/remove 반복 |
| I-7 | Back/Escape 키 | 앱 정상 종료 |

---

### 3. Component (`widget-app-component` + `widget-viewer-component`) — **미지원 (TODO)**

> **현재 미지원**: DALi C++ public API에는 CBA 위젯 렌더링에 필요한 `WidgetComponent` 헬퍼가 없습니다.
>
> CBA 위젯은 Tizen C-API(`component_based_app_add_widget_component`, `base_widget_create_window`)를 통해
> DALi Window를 EFL `Evas_Object*`에 브릿징해야 동작합니다. 이 브릿징 레이어는 NUI(TizenFX C#)에만
> 구현되어 있으며(`NUIWidgetComponent`, `WidgetComponentStateManager`), DALi C++ 레이어에는 해당
> public API가 제공되지 않습니다.
>
> dali-adaptor에 C++ `WidgetComponent` 클래스 추가가 필요합니다.

현재 `widget-app-component`는 `ComponentApplication` MainLoop 진입까지만 동작하며, 실제 위젯 렌더링 시 크래시가 발생합니다.
