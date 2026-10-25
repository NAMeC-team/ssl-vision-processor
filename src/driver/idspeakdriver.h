/*
     Copyright 2024 Felix Weinmann

     Licensed under the Apache License, Version 2.0 (the "License");
     you may not use this file except in compliance with the License.
     You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

     Unless required by applicable law or agreed to in writing, software
     distributed under the License is distributed on an "AS IS" BASIS,
     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
     See the License for the specific language governing permissions and
     limitations under the License.
 */
#pragma once
#ifdef IDS_PEAK

#include <peak/peak.hpp>
#include "cameradriver.h"

class IdsPeakDriver : public CameraDriver {
public:
	explicit IdsPeakDriver(const CameraConfig& config);
	~IdsPeakDriver() override;

	std::shared_ptr<RawImage> readImage() override;

	const PixelFormat format() override;

	double expectedFrametime() override;

private:
	uint8_t _cameraId;
	peak::DeviceManager& _deviceManager;
	std::shared_ptr<peak::core::DeviceDescriptor> _deviceDescriptor;
	std::shared_ptr<peak::core::Device> _device;
	std::shared_ptr<peak::core::DataStream> _dataStream;
	std::shared_ptr<peak::core::NodeMap> _nodeMapRemoteDevice;
};

#endif
