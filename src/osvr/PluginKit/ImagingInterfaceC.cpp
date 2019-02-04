/** @file
    @brief Implementation

    @date 2015

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2015 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Internal Includes
#include <osvr/PluginKit/ImagingInterfaceC.h>
#include <osvr/Connection/DeviceInitObject.h>
//#include <osvr/Connection/ImagingServerInterface.h>
#include <osvr/Connection/DeviceToken.h>
#include <osvr/Connection/DeviceInterfaceBase.h>
#include <osvr/PluginHost/PluginSpecificRegistrationContext.h>
#include <osvr/Common/ImagingComponent.h>
#include "HandleNullContext.h"
#include <osvr/Util/Verbosity.h>

// Library/third-party includes
// - none

// Standard includes
// - none

// @todo This is a hack. expect this to be moved to a separate osvrJniBridge
// library and encapsulated behind a proper API.

struct OSVR_ImagingDeviceInterfaceObject
    : public osvr::connection::DeviceInterfaceBase {
    osvr::common::ImagingComponent *imaging;
};

OSVR_ReturnCode
osvrDeviceImagingConfigure(OSVR_INOUT_PTR OSVR_DeviceInitOptions opts,
                           OSVR_OUT_PTR OSVR_ImagingDeviceInterface *iface,
                           OSVR_IN OSVR_ChannelCount /*numSensors*/) {

    OSVR_PLUGIN_HANDLE_NULL_CONTEXT("osvrDeviceImagingConfigure", opts);
    OSVR_PLUGIN_HANDLE_NULL_CONTEXT("osvrDeviceImagingConfigure", iface);
    OSVR_ImagingDeviceInterface ifaceObj =
        opts->makeInterfaceObject<OSVR_ImagingDeviceInterfaceObject>();
    *iface = ifaceObj;

    auto imaging = osvr::common::ImagingComponent::create();
    ifaceObj->imaging = imaging.get();
    opts->addComponent(imaging);
    return OSVR_RETURN_SUCCESS;
}

OSVR_ReturnCode
osvrDeviceImagingReportFrame(OSVR_IN_PTR OSVR_DeviceToken,
                             OSVR_IN_PTR OSVR_ImagingDeviceInterface iface,
                             OSVR_IN OSVR_ImagingMetadata metadata,
                             OSVR_IN_PTR OSVR_ImageBufferElement *imageData,
                             OSVR_IN OSVR_ChannelCount sensor,
                             OSVR_IN_PTR OSVR_TimeValue const *timestamp) {
    auto guard = iface->getSendGuard();
    if (guard->lock()) {
        iface->imaging->sendImageData(metadata, imageData, sensor, *timestamp);
        return OSVR_RETURN_SUCCESS;
    }

    return OSVR_RETURN_FAILURE;
}
