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

#include <dali-toolkit/dali-toolkit.h>
#include <dali/devel-api/adaptor-framework/component-application.h>
#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "DALI_WIDGET_TEST"

#define LOG_I(fmt, ...) dlog_print(DLOG_INFO, LOG_TAG, "[COMPONENT_PROVIDER] " fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) dlog_print(DLOG_ERROR, LOG_TAG, "[COMPONENT_PROVIDER] " fmt, ##__VA_ARGS__)

using namespace Dali;
using namespace Dali::Toolkit;

/**
 * ComponentAppController — Component-based widget provider scaffold.
 * 
 * Note: NUI provides NUIWidgetComponent to wrap Tizen's widget_component C-API.
 * In DALi C++, you would initialize the ComponentApplication and use the 
 * widget_component_create lifecycle callbacks from Tizen C-API to link 
 * Dali::Window with the widget component.
 */
class ComponentAppController : public Dali::ConnectionTracker
{
public:
  ComponentAppController(Dali::ComponentApplication& app) : mApp(app)
  {
    mApp.CreateSignal().Connect(this, &ComponentAppController::OnCreate);
  }

  Dali::Any OnCreate()
  {
    LOG_I("ComponentApplication Created. (Widget Component Initialization goes here)");

    // In a full implementation, you would register widget_component lifecycle
    // callbacks via Tizen C-API (widget_component_provider_add_class) here,
    // and create Dali::Window when a widget component instance is requested.
    return Dali::Any();
  }

private:
  Dali::ComponentApplication& mApp;
};

__attribute__((visibility("default"))) int main(int argc, char** argv)
{
  Dali::ComponentApplication app = Dali::ComponentApplication::New(&argc, &argv, "");
  ComponentAppController     controller(app);
  app.MainLoop();
  return 0;
}
