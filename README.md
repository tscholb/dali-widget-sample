# dali-widget-sample

DALi native C++ widget samples for Tizen.

## Samples

| Sample | Type | Description |
|--------|------|-------------|
| [widget-app-template](widget-app-template/) | widget-application (provider) | Creates a widget via `Dali::WidgetApplication` and `Dali::Internal::Adaptor::Widget`. |
| [widget-viewer-template](widget-viewer-template/) | ui-application (viewer) | Displays a widget via `Dali::WidgetView::WidgetViewManager` + `WidgetView` from widget-viewer-dali. |

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
