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
#include <dali/public-api/adaptor-framework/widget-application.h>
#include <dali/public-api/adaptor-framework/widget-impl.h>
#include <dali/public-api/adaptor-framework/widget.h>
#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "DALI_WIDGET_TEST"

#define LOG_I(fmt, ...) dlog_print(DLOG_INFO, LOG_TAG, "[PROVIDER] " fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) dlog_print(DLOG_ERROR, LOG_TAG, "[PROVIDER] " fmt, ##__VA_ARGS__)

#define APP_ID "com.samsung.dali.widget-app-template"
#define WIDGET_CLASS_ID_CLS1 "class1@" APP_ID
#define WIDGET_CLASS_ID_CLS2 "class2@" APP_ID

using namespace Dali;
using namespace Dali::Toolkit;

/**
 * TemplateWidget — widget provider.
 *
 * One provider app registers two classes (class1, class2) that differ only in
 * color and label, matching the NUI RedWidget/BlueWidget sample.
 */
class TemplateWidget : public Dali::Internal::Adaptor::Widget
{
public:
  TemplateWidget(const Vector4& bgColor, const char* labelText)
  : mBgColor(bgColor), mLabelText(labelText) {}

  void OnCreate(const Dali::String& contentInfo, Dali::Window window) override
  {
    const Dali::String countValue = DecodeCountFromBundle(contentInfo);
    LOG_I("OnCreate: label='%s' COUNT=%s", mLabelText, countValue.CStr());

    window.SetBackgroundColor(mBgColor);

    mRootView = Control::New();
    mRootView.SetBackgroundColor(mBgColor);
    mRootView.SetProperty(Actor::Property::PIVOT, Pivot::CENTER);
    mRootView.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER);
    mRootView.SetResizePolicy(ResizePolicy::FILL_TO_PARENT, Dimension::ALL_DIMENSIONS);
    window.Add(mRootView);

    TextLabel label = TextLabel::New(Dali::String(mLabelText));
    label.SetProperty(TextLabel::Property::HORIZONTAL_ALIGNMENT, "CENTER");
    label.SetProperty(TextLabel::Property::VERTICAL_ALIGNMENT, "CENTER");
    label.SetProperty(TextLabel::Property::TEXT_COLOR, Color::BLACK);
    label.SetProperty(TextLabel::Property::POINT_SIZE, 9.0f);
    label.SetProperty(TextLabel::Property::MULTI_LINE, true);
    label.SetProperty(TextLabel::Property::ELLIPSIS, false);
    label.SetResizePolicy(ResizePolicy::FILL_TO_PARENT, Dimension::ALL_DIMENSIONS);
    label.SetProperty(Actor::Property::PIVOT, Pivot::CENTER);
    label.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER);
    mRootView.Add(label);
    mLabel = label;

    // DALi is a lazy renderer — keep a looping animation so the viewer sees
    // continuous buffer updates.
    mAnimation = Animation::New(1.0f);
    KeyFrames scaleKeyFrames = KeyFrames::New();
    scaleKeyFrames.Add(0.0f, Vector3(1.0f, 1.0f, 1.0f));
    scaleKeyFrames.Add(0.5f, Vector3(1.5f, 1.5f, 1.0f));
    scaleKeyFrames.Add(1.0f, Vector3(1.0f, 1.0f, 1.0f));
    mAnimation.AnimateBetween(Property(mLabel, Actor::Property::SCALE), scaleKeyFrames);
    mAnimation.SetLooping(true);
    mAnimation.Play();
  }

  void OnTerminate(const Dali::String& contentInfo, Dali::Widget::Termination type) override
  {
    LOG_I("OnTerminate: label='%s' type=%d", mLabelText, static_cast<int>(type));
    if(mAnimation)
    {
      mAnimation.Stop();
      mAnimation.Reset();
    }
    if(mLabel)
    {
      mLabel.Unparent();
      mLabel.Reset();
    }
    if(mRootView)
    {
      mRootView.Unparent();
      mRootView.Reset();
    }
  }

  void OnPause() override {}
  void OnResume() override {}
  void OnResize(Dali::Window window) override {}
  void OnUpdate(const Dali::String& contentInfo, int force) override {}

private:
  static Dali::String DecodeCountFromBundle(const Dali::String& contentInfo)
  {
    if(contentInfo.Empty())
    {
      return Dali::String("(none)");
    }
    bundle* b = bundle_decode(reinterpret_cast<const bundle_raw*>(contentInfo.CStr()),
                              static_cast<int>(contentInfo.Size()));
    if(!b)
    {
      LOG_E("DecodeCountFromBundle: bundle_decode failed (len=%zu)", contentInfo.Size());
      return Dali::String("(decode-failed)");
    }
    char* count = nullptr;
    Dali::String result("(missing)");
    if(bundle_get_str(b, "COUNT", &count) == BUNDLE_ERROR_NONE && count)
    {
      result = count;
    }
    bundle_free(b);
    return result;
  }

  Vector4      mBgColor;
  const char*  mLabelText{nullptr};
  Control      mRootView;
  TextLabel    mLabel;
  Animation    mAnimation;
};

Dali::Widget CreateRedWidget(const Dali::String& /*widgetName*/)
{
  return Dali::Widget(new TemplateWidget(Color::RED, "Red Widget"));
}

Dali::Widget CreateBlueWidget(const Dali::String& /*widgetName*/)
{
  return Dali::Widget(new TemplateWidget(Color::BLUE, "Blue Widget"));
}

/**
 * Register widget creating functions on InitSignal rather than before MainLoop.
 * appcore-widget framework (widget_base_on_create) is only initialized after
 * MainLoop starts, matching the pattern used by NUI's NUIWidgetCoreBackend.
 */
class ProviderController : public Dali::ConnectionTracker
{
public:
  ProviderController(Dali::WidgetApplication& app) : mApp(app)
  {
    mApp.InitSignal().Connect(this, &ProviderController::OnInit);
  }

  void OnInit(Dali::Application&)
  {
    mApp.RegisterWidgetCreatingFunction(WIDGET_CLASS_ID_CLS1, &CreateRedWidget);
    mApp.RegisterWidgetCreatingFunction(WIDGET_CLASS_ID_CLS2, &CreateBlueWidget);
  }

private:
  Dali::WidgetApplication& mApp;
};

__attribute__((visibility("default"))) int main(int argc, char** argv)
{
  Dali::WidgetApplication app = Dali::WidgetApplication::New(&argc, &argv, "");
  ProviderController      controller(app);
  app.MainLoop();
  return 0;
}
