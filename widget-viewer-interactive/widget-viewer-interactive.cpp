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
#include <message_port.h>
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

#define VIEWER_APP_ID    "com.samsung.dali.widget-viewer-interactive"
#define PROVIDER_APP_ID  "com.samsung.dali.widget-app-interactive"
#define WIDGET_ID_CLASS1 "class1@" PROVIDER_APP_ID
#define WIDGET_ID_CLASS2 "class2@" PROVIDER_APP_ID

using namespace Dali;
using namespace Dali::Toolkit;

/**
 * WidgetViewerInteractiveApp — widget viewer.
 */
class WidgetViewerInteractiveApp : public ConnectionTracker
{
public:
  WidgetViewerInteractiveApp(Application& app) : mApp(app)
  {
    app.InitSignal().Connect(this, &WidgetViewerInteractiveApp::OnInit);
    app.TerminateSignal().Connect(this, &WidgetViewerInteractiveApp::OnTerminate);
  }

  void OnInit(Application app)
  {
    mWindow = app.GetWindow();
    mWindow.SetBackgroundColor(Color::WHITE);
    mWindow.KeyEventSignal().Connect(this, &WidgetViewerInteractiveApp::OnKeyEvent);

    const Vector2 windowSize(static_cast<float>(mWindow.GetSize().GetWidth()),
                             static_cast<float>(mWindow.GetSize().GetHeight()));

    mRootView = Control::New();
    mRootView.SetProperty(Control::Property::BACKGROUND, Color::WHITE);
    mRootView.SetProperty(Actor::Property::SIZE, windowSize);
    mRootView.SetProperty(Actor::Property::PIVOT, Pivot::CENTER);
    mRootView.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER);
    mWindow.Add(mRootView);

    TextLabel text = TextLabel::New("Widget Viewer (Interactive)\nPress '1' to Resize and Send Message");
    text.SetProperty(TextLabel::Property::TEXT_COLOR, Color::BLACK);
    text.SetProperty(TextLabel::Property::POINT_SIZE, 8);
    text.SetProperty(TextLabel::Property::HORIZONTAL_ALIGNMENT, "CENTER");
    text.SetProperty(TextLabel::Property::MULTI_LINE, true);
    text.SetProperty(Actor::Property::PIVOT, Pivot::TOP_CENTER);
    text.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::TOP_CENTER);
    text.SetProperty(Actor::Property::POSITION, Vector2(0.0f, 20.0f));
    text.SetProperty(Actor::Property::SIZE, Vector2(windowSize.x, 60.0f));
    mRootView.Add(text);

    mWidgetViewManager = Dali::WidgetView::WidgetViewManager::New(app, VIEWER_APP_ID);

    const std::string encodedBundle = EncodeBundle();

    mWidgetView1 = mWidgetViewManager.AddWidget(WIDGET_ID_CLASS1, encodedBundle, mWidgetWidth, mWidgetHeight, 0.0f);
    mWidgetView1.SetProperty(Actor::Property::PIVOT, Pivot::TOP_LEFT);
    mWidgetView1.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::TOP_LEFT);
    mWidgetView1.SetProperty(Actor::Property::POSITION, Vector2(100.0f, 150.0f));
    ConnectWidgetSignals(mWidgetView1);
    mWindow.Add(mWidgetView1);

    AddBlueWidget();
    mBlueCreated = true;

    mBlueToggleTimer = Dali::Timer::New(4000);
    mBlueToggleTimer.TickSignal().Connect(this, &WidgetViewerInteractiveApp::OnBlueToggleTick);
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
      else if(event.GetKeyName() == "1" || event.GetKeyName() == "Return")
      {
        mWidgetWidth += 200;
        mWidgetHeight += 200;
        if(mWidgetWidth > 1000 || mWidgetHeight > 1000)
        {
          mWidgetWidth = 200;
          mWidgetHeight = 200;
        }

        LOG_I("Resizing WidgetView1 to %d x %d", mWidgetWidth, mWidgetHeight);
        mWidgetView1.SetProperty(Actor::Property::SIZE, Vector2(static_cast<float>(mWidgetWidth), static_cast<float>(mWidgetHeight)));

        bundle* b = bundle_create();
        if(b)
        {
          std::string msg = "Viewer resized width:" + std::to_string(mWidgetWidth);
          bundle_add_str(b, "message", msg.c_str());
          message_port_send_message(PROVIDER_APP_ID, "my_widget_port", b);
          LOG_I("Sent message via MessagePort");
          bundle_free(b);
        }
      }
    }
  }

private:
  void AddBlueWidget()
  {
    const std::string encodedBundle = EncodeBundle();
    mWidgetView2                    = mWidgetViewManager.AddWidget(WIDGET_ID_CLASS2, encodedBundle, 400, 400, 0.0f);
    mWidgetView2.SetProperty(Actor::Property::PIVOT, Pivot::TOP_LEFT);
    mWidgetView2.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::TOP_LEFT);
    mWidgetView2.SetProperty(Actor::Property::POSITION, Vector2(510.0f, 150.0f));
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
    widgetView.WidgetAddedSignal().Connect(this, &WidgetViewerInteractiveApp::OnWidgetAdded);
    widgetView.WidgetDeletedSignal().Connect(this, &WidgetViewerInteractiveApp::OnWidgetDeleted);
    widgetView.WidgetCreationAbortedSignal().Connect(this, &WidgetViewerInteractiveApp::OnWidgetCreationAborted);
    widgetView.WidgetContentUpdatedSignal().Connect(this, &WidgetViewerInteractiveApp::OnWidgetContentUpdated);
    widgetView.WidgetFaultedSignal().Connect(this, &WidgetViewerInteractiveApp::OnWidgetFaulted);
  }

  void RemoveWidgetView(Dali::WidgetView::WidgetView& view)
  {
    if(view)
    {
      mWidgetViewManager.RemoveWidget(view);
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
    
    Dali::Property::Value val = widgetView.GetProperty(Dali::WidgetView::WidgetView::Property::CONTENT_INFO);
    Dali::String contentInfo;
    if(val.Get(contentInfo) && !contentInfo.Empty())
    {
      bundle* b = bundle_decode(reinterpret_cast<const bundle_raw*>(contentInfo.CStr()), static_cast<int>(contentInfo.Size()));
      if(b)
      {
        char* count = nullptr;
        if(bundle_get_str(b, "COUNT", &count) == BUNDLE_ERROR_NONE && count)
        {
          LOG_I("WidgetContentUpdated -> COUNT: %s", count);
        }
        bundle_free(b);
      }
    }
  }

  void OnWidgetFaulted(Dali::WidgetView::WidgetView widgetView)
  {
    LOG_E("WidgetFaulted: widgetId=%s -> ActivateFaultedWidget()", ExtractWidgetId(widgetView).CStr());
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
  int                                 mWidgetWidth{400};
  int                                 mWidgetHeight{400};
};

__attribute__((visibility("default"))) int main(int argc, char** argv)
{
  Application                application = Application::New(&argc, &argv);
  WidgetViewerInteractiveApp viewer(application);
  application.MainLoop();
  return 0;
}
