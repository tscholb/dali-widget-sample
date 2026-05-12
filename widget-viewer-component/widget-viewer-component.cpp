/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <bundle.h>
#include <dali-toolkit/dali-toolkit.h>
#include <dali/dali.h>
#include <dlog.h>
#include <string>
#include <widget_viewer_dali.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "DALI_WIDGET_TEST"

#define LOG_I(fmt, ...) dlog_print(DLOG_INFO, LOG_TAG, "[VIEWER] " fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) dlog_print(DLOG_ERROR, LOG_TAG, "[VIEWER] " fmt, ##__VA_ARGS__)

#define VIEWER_APP_ID    "com.samsung.dali.widget-viewer-component"
#define PROVIDER_APP_ID  "com.samsung.dali.widget-app-component"
#define WIDGET_ID_CLASS1 "class1@" PROVIDER_APP_ID
#define WIDGET_ID_CLASS2 "class2@" PROVIDER_APP_ID

namespace
{
constexpr int WIDGET_WIDTH  = 400;
constexpr int WIDGET_HEIGHT = 400;
} // namespace

using namespace Dali;
using namespace Dali::Toolkit;

/**
 * WidgetViewerTemplateApp — widget viewer.
 *
 * Follows NUI SimpleWidgetViewApp: a white root view with a "Widget Viewer"
 * label on top, and two WidgetView instances (class1, class2) stacked below.
 */
class WidgetViewerTemplateApp : public ConnectionTracker
{
public:
  WidgetViewerTemplateApp(Application& app) : mApp(app)
  {
    app.InitSignal().Connect(this, &WidgetViewerTemplateApp::OnInit);
    app.TerminateSignal().Connect(this, &WidgetViewerTemplateApp::OnTerminate);
  }

  void OnInit(Application app)
  {
    mWindow = app.GetWindow();
    mWindow.SetBackgroundColor(Color::WHITE);
    mWindow.KeyEventSignal().Connect(this, &WidgetViewerTemplateApp::OnKeyEvent);

    const Vector2 windowSize(static_cast<float>(mWindow.GetSize().GetWidth()),
                             static_cast<float>(mWindow.GetSize().GetHeight()));

    mRootView = Control::New();
    mRootView.SetProperty(Control::Property::BACKGROUND, Color::WHITE);
    mRootView.SetProperty(Actor::Property::SIZE, windowSize);
    mRootView.SetProperty(Actor::Property::PIVOT, Pivot::CENTER);
    mRootView.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER);
    mWindow.Add(mRootView);

    TextLabel text = TextLabel::New("Widget Viewer");
    text.SetProperty(TextLabel::Property::TEXT_COLOR, Color::BLACK);
    text.SetProperty(TextLabel::Property::POINT_SIZE, 8);
    text.SetProperty(TextLabel::Property::HORIZONTAL_ALIGNMENT, "CENTER");
    text.SetProperty(Actor::Property::PIVOT, Pivot::TOP_CENTER);
    text.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::TOP_CENTER);
    text.SetProperty(Actor::Property::POSITION, Vector2(0.0f, 20.0f));
    text.SetProperty(Actor::Property::SIZE, Vector2(windowSize.x, 60.0f));
    mRootView.Add(text);

    mWidgetViewManager = Dali::WidgetView::WidgetViewManager::New(app, VIEWER_APP_ID);

    const std::string encodedBundle = EncodeBundle();

    // Widgets are added directly to the window (not rootView), matching NUI
    // SimpleWidgetViewApp. TOP_LEFT pivot/origin so POSITION acts as absolute
    // top-left offset in window coordinates.
    mWidgetView1 = mWidgetViewManager.AddWidget(
      WIDGET_ID_CLASS1, encodedBundle, WIDGET_WIDTH, WIDGET_HEIGHT, 0.0f);
    mWidgetView1.SetProperty(Actor::Property::PIVOT, Pivot::TOP_LEFT);
    mWidgetView1.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::TOP_LEFT);
    mWidgetView1.SetProperty(Actor::Property::POSITION, Vector2(100.0f, 150.0f));
    ConnectWidgetSignals(mWidgetView1);
    mWindow.Add(mWidgetView1);

    AddBlueWidget();
    mBlueCreated = true;

    // Toggle Blue Widget (class2) every 2 seconds to exercise add/remove
    // lifecycle from the viewer side. Mirrors NUI SimpleWidgetViewApp.onTick().
    mBlueToggleTimer = Dali::Timer::New(2000);
    mBlueToggleTimer.TickSignal().Connect(this, &WidgetViewerTemplateApp::OnBlueToggleTick);
    mBlueToggleTimer.Start();
  }

  void OnTerminate(Application)
  {
    if(mBlueToggleTimer)
    {
      mBlueToggleTimer.Stop();
      mBlueToggleTimer.Reset();
    }
    RemoveWidgetView(mWidgetView1);
    RemoveWidgetView(mWidgetView2);
  }

  void OnKeyEvent(Dali::Window, KeyEvent event)
  {
    if(event.GetState() == KeyEvent::DOWN)
    {
      if(IsKey(event, DALI_KEY_ESCAPE) || IsKey(event, DALI_KEY_BACK))
      {
        mApp.Quit();
      }
    }
  }

private:
  void AddBlueWidget()
  {
    const std::string encodedBundle = EncodeBundle();
    mWidgetView2                    = mWidgetViewManager.AddWidget(
      WIDGET_ID_CLASS2, encodedBundle, WIDGET_WIDTH, WIDGET_HEIGHT, 0.0f);
    mWidgetView2.SetProperty(Actor::Property::PIVOT, Pivot::TOP_LEFT);
    mWidgetView2.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::TOP_LEFT);
    mWidgetView2.SetProperty(Actor::Property::POSITION, Vector2(WIDGET_WIDTH + 110.0f, 150.0f));
    ConnectWidgetSignals(mWidgetView2);
    mWindow.Add(mWidgetView2);
  }

  bool OnBlueToggleTick()
  {
    if(mBlueCreated)
    {
      RemoveWidgetView(mWidgetView2);
      mBlueCreated = false;
    }
    else
    {
      AddBlueWidget();
      mBlueCreated = true;
    }
    return true; // repeat
  }

  void ConnectWidgetSignals(Dali::WidgetView::WidgetView widgetView)
  {
    widgetView.WidgetAddedSignal().Connect(this, &WidgetViewerTemplateApp::OnWidgetAdded);
    widgetView.WidgetDeletedSignal().Connect(this, &WidgetViewerTemplateApp::OnWidgetDeleted);
    widgetView.WidgetCreationAbortedSignal().Connect(this, &WidgetViewerTemplateApp::OnWidgetCreationAborted);
    widgetView.WidgetContentUpdatedSignal().Connect(this, &WidgetViewerTemplateApp::OnWidgetContentUpdated);
    widgetView.WidgetFaultedSignal().Connect(this, &WidgetViewerTemplateApp::OnWidgetFaulted);
  }

  void RemoveWidgetView(Dali::WidgetView::WidgetView& view)
  {
    if(view)
    {
      mWidgetViewManager.RemoveWidget(view);
      // RemoveWidget only tears down the widget process/remote-surface link;
      // the Actor itself stays in the scene showing the last frame. Detach
      // it from the window so the buffer disappears immediately.
      mWindow.Remove(view);
      view.Reset();
    }
  }

  static Dali::String ExtractWidgetId(Dali::WidgetView::WidgetView widgetView)
  {
    Dali::Property::Value value = widgetView.GetProperty(Dali::WidgetView::WidgetView::Property::WIDGET_ID);
    Dali::String widgetId;
    value.Get(widgetId);
    return widgetId;
  }

  void OnWidgetAdded(Dali::WidgetView::WidgetView widgetView)
  {
    LOG_I("WidgetAdded: widgetId=%s", ExtractWidgetId(widgetView).CStr());
  }

  void OnWidgetDeleted(Dali::WidgetView::WidgetView widgetView)
  {
    LOG_I("WidgetDeleted: widgetId=%s", ExtractWidgetId(widgetView).CStr());
  }

  void OnWidgetCreationAborted(Dali::WidgetView::WidgetView widgetView)
  {
    LOG_E("WidgetCreationAborted: widgetId=%s", ExtractWidgetId(widgetView).CStr());
  }

  void OnWidgetContentUpdated(Dali::WidgetView::WidgetView widgetView)
  {
    LOG_I("WidgetContentUpdated: widgetId=%s", ExtractWidgetId(widgetView).CStr());
  }

  void OnWidgetFaulted(Dali::WidgetView::WidgetView widgetView)
  {
    LOG_E("WidgetFaulted: widgetId=%s -> ActivateFaultedWidget()",
          ExtractWidgetId(widgetView).CStr());
    widgetView.ActivateFaultedWidget();
  }

  std::string EncodeBundle()
  {
    std::string encoded;
    bundle*     b = bundle_create();
    if(!b)
    {
      LOG_E("EncodeBundle: bundle_create failed");
      return encoded;
    }
    bundle_add_str(b, "COUNT", "1");

    bundle_raw* raw = nullptr;
    int         len = 0;
    if(bundle_encode(b, &raw, &len) == BUNDLE_ERROR_NONE && raw)
    {
      encoded.assign(reinterpret_cast<const char*>(raw), static_cast<size_t>(len));
      free(raw);
    }
    else
    {
      LOG_E("EncodeBundle: bundle_encode failed");
    }
    bundle_free(b);
    return encoded;
  }

  Application&                        mApp;
  Window                              mWindow;
  Control                             mRootView;
  Dali::WidgetView::WidgetViewManager mWidgetViewManager;
  Dali::WidgetView::WidgetView        mWidgetView1;
  Dali::WidgetView::WidgetView        mWidgetView2;
  Dali::Timer                         mBlueToggleTimer;
  bool                                mBlueCreated{false};
};

__attribute__((visibility("default"))) int main(int argc, char** argv)
{
  Application             application = Application::New(&argc, &argv);
  WidgetViewerTemplateApp viewer(application);
  application.MainLoop();
  return 0;
}
