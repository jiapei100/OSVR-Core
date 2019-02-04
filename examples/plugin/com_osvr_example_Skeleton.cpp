/** @date 2015
    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2015 Sensics Inc.
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
#include <osvr/PluginKit/PluginKit.h>
#include <osvr/PluginKit/SkeletonInterfaceC.h>
#include <osvr/PluginKit/TrackerInterfaceC.h>

// Generated JSON header file
#include "com_osvr_example_Skeleton_json.h"

// Library/third-party includes

// Standard includes
#include <chrono>
#include <cmath>
#include <ctime>
#include <iostream>
#include <random>
#include <thread>

// Anonymous namespace to avoid symbol collision
namespace {

class SkeletonDevice {
  public:
    SkeletonDevice(OSVR_PluginRegContext ctx) {
        /// Create the initialization options
        OSVR_DeviceInitOptions opts = osvrDeviceCreateInitOptions(ctx);

        osvrDeviceSkeletonConfigure(opts, &m_skeleton,
                                    com_osvr_example_Skeleton_json);
        osvrDeviceTrackerConfigure(opts, &m_tracker);

        /// Create the device token with the options
        m_dev.initSync(ctx, "Skeleton", opts);

        /// Send JSON descriptor
        m_dev.sendJsonDescriptor(com_osvr_example_Skeleton_json);

        /// Register update callback
        m_dev.registerUpdateCallback(this);
    }

    OSVR_ReturnCode update() {

        std::this_thread::sleep_for(std::chrono::milliseconds(
            500)); // Simulate waiting half a second for data.

        // add reporting of pre-recorded finger positions to send
        // sensible tracker reports

        double newVal = std::sin(mVal) * 0.25;
        OSVR_Pose3 samplePose;
        samplePose.rotation.data[0] = newVal;
        samplePose.rotation.data[1] = newVal;
        samplePose.rotation.data[2] = newVal;
        samplePose.rotation.data[3] = newVal;

        samplePose.translation.data[0] = 0.0;
        samplePose.translation.data[1] = newVal;
        samplePose.translation.data[2] = 0.0;

        OSVR_TimeValue timestamp;
        osvrTimeValueGetNow(&timestamp);
        for (OSVR_ChannelCount channel = 0; channel < 6; channel++) {
            OSVR_Pose3 channelPose = GetChannelPose(samplePose, channel);
            osvrDeviceTrackerSendPoseTimestamped(m_dev, m_tracker, &channelPose,
                                                 channel, &timestamp);
        }
        osvrDeviceSkeletonComplete(m_skeleton, 0, &timestamp);

        for (OSVR_ChannelCount channel = 6; channel < 12; channel++) {
            OSVR_Pose3 channelPose = GetChannelPose(samplePose, channel);
            osvrDeviceTrackerSendPoseTimestamped(m_dev, m_tracker, &channelPose,
                                                 channel, &timestamp);
        }
        osvrDeviceSkeletonComplete(m_skeleton, 1, &timestamp);

        mVal += mIncr;
        return OSVR_RETURN_SUCCESS;
    }

  private:
    double mIncr = 0.01;
    double mVal = 1.0;
    osvr::pluginkit::DeviceToken m_dev;
    OSVR_SkeletonDeviceInterface m_skeleton;
    OSVR_TrackerDeviceInterface m_tracker;

    inline OSVR_Pose3 GetChannelPose(OSVR_Pose3 basePose,
                                     OSVR_ChannelCount channel) {
        double offset = channel * 0.05;
        OSVR_Pose3 ret = basePose;
        ret.translation.data[0] += offset;
        return ret;
    }
};

class HardwareDetection {
  public:
    HardwareDetection() : m_found(false) {}
    OSVR_ReturnCode operator()(OSVR_PluginRegContext ctx) {

        if (m_found) {
            return OSVR_RETURN_SUCCESS;
        }

        std::cout << "PLUGIN: Got a hardware detection request" << std::endl;

        /// we always detect device in sample plugin
        m_found = true;

        std::cout << "PLUGIN: We have detected Skeleton device! " << std::endl;
        /// Create our device object
        osvr::pluginkit::registerObjectForDeletion(ctx,
                                                   new SkeletonDevice(ctx));

        return OSVR_RETURN_SUCCESS;
    }

  private:
    bool m_found;
};
} // namespace

OSVR_PLUGIN(com_osvr_example_Skeleton) {

    osvr::pluginkit::PluginContext context(ctx);

    /// Register a detection callback function object.
    context.registerHardwareDetectCallback(new HardwareDetection());

    return OSVR_RETURN_SUCCESS;
}
