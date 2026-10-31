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
// #include <peak/node_map/peak_string_node.hpp>
#include <peak/data_stream/peak_data_stream.hpp>
#include <peak/device/peak_device.hpp>
#include <peak/device/peak_device_descriptor.hpp>
#ifdef IDS_PEAK
#include <peak/peak.hpp>
#include <peak_icv/peak_icv.hpp>
// #include <ueye.h>
#include "idspeakdriver.h"

#include "cameradriver.h"

#define CAMERA_FRAME_CAPTURE_TIMEOUT 5'000  // 1 second
static std::atomic<uint8_t> nbCameras(0);
using namespace peak::core;

IdsPeakDriver::IdsPeakDriver(const CameraConfig &config) {
	peak::Library::Initialize();
	_cameraId = nbCameras;
	nbCameras = nbCameras + 1;
	_deviceManager = &peak::DeviceManager::Instance();
	_deviceManager->Update();

	// open the next available camera
 	_deviceDescriptor = _deviceManager->FirstAvailableDevice(DeviceAccessType::Control);
 	if (_deviceDescriptor == nullptr) {
 		FATAL("[IDS Peak driver] Cannot open camera : no camera left available !");
 	}

    uint64_t i = 0;
    for (const auto& deviceDescriptor : _deviceManager->Devices())
    {
        std::cout << i << ": " << deviceDescriptor->ModelName() << " ("
                << deviceDescriptor->ParentInterface()->DisplayName() << "; "
                << deviceDescriptor->ParentInterface()->ParentSystem()->DisplayName() << " v."
                << deviceDescriptor->ParentInterface()->ParentSystem()->Version() << ")" << std::endl;
        ++i;
    }
    std::cout << "Opened camera " << _deviceDescriptor->ModelName() << " ("
                    << _deviceDescriptor->ParentInterface()->DisplayName() << "; "
                    << _deviceDescriptor->ParentInterface()->ParentSystem()->DisplayName() << " v."
                    << _deviceDescriptor->ParentInterface()->ParentSystem()->Version() << ")" << std::endl;
 	_device = _deviceDescriptor->OpenDevice(DeviceAccessType::Control);

 	// force capture type to Bayer RG8 (same logic as spinnakerdriver.cpp)
 	_nodeMapRemoteDevice = _device->RemoteDevice()->NodeMaps().at(0);
 	// const auto pixelFormatNode =
 	// 	_nodeMapRemoteDevice->FindNode<nodes::EnumerationNode>("PixelFormat");
 	// pixelFormatNode->SetCurrentEntry("BayerRG8");

 	_dataStream = _device->DataStreams().at(0)->OpenDataStream();
    _nodeMapRemoteDevice->FindNode<peak::core::nodes::EnumerationNode>("UserSetSelector")->SetCurrentEntry("Default");
    _nodeMapRemoteDevice->FindNode<peak::core::nodes::CommandNode>("UserSetLoad")->Execute();
      // wait until the UserSetLoad command has been finished
    _nodeMapRemoteDevice->FindNode<peak::core::nodes::CommandNode>("UserSetLoad")->WaitUntilDone();

 	// Get the required payload size. If the data stream does not
 	// define it, fall back to the remote device node map.
 	const auto payloadSizeNode = _nodeMapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("PayloadSize");
 	const auto payloadSize = _dataStream->DefinesPayloadSize() ? _dataStream->PayloadSize() :
 														  static_cast<size_t>(payloadSizeNode->Value());
 	const auto bufferCountMax = _dataStream->NumBuffersAnnouncedMinRequired();
 	LOG("[IDS Peak driver] Allocating " << bufferCountMax << " buffer of size " << payloadSize);
    for (uint64_t bufferCount = 0; bufferCount < bufferCountMax; ++bufferCount)
    {
        auto buffer = _dataStream->AllocAndAnnounceBuffer(static_cast<size_t>(payloadSize), nullptr);
        _dataStream->QueueBuffer(buffer);
    }

 	_nodeMapRemoteDevice->FindNode<nodes::IntegerNode>("TLParamsLocked")->SetValue(1);
 	// Allocate buffers and queue them
 	// _dataStream->AddAcquisitionBuffers(payloadSize, bufferCount);
 	_dataStream->StartAcquisition(AcquisitionStartMode::Default);
    _nodeMapRemoteDevice->FindNode<peak::core::nodes::CommandNode>("AcquisitionStart")->Execute();

 	// Lock transport layer parameters (TLParamsLocked = 1) to
 	// prevent irregular access to the remote device during acquisition

 	LOG("[IDS Peak driver] Successfully initialized camera ID=" << _cameraId);
}

IdsPeakDriver::~IdsPeakDriver() {
	// Stop acquisition
	_nodeMapRemoteDevice->FindNode<nodes::CommandNode>("AcquisitionStop")->ExecuteAndWait();
	_dataStream->StopAcquisition(AcquisitionStopMode::Default);

	// Unlock transport layer parameters
	_nodeMapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")->SetValue(0);

	// Release acquisition buffers
	_dataStream->FlushAndRevokeAllBuffers();

	nbCameras = nbCameras - 1;
	if (nbCameras == 0) {
		peak::Library::Close();
		LOG("[IDS Peak driver] Latest camera capture closed, library closed");
	}
}

std::shared_ptr<RawImage> IdsPeakDriver::readImage() {
    auto buff = _dataStream->WaitForFinishedBuffer(CAMERA_FRAME_CAPTURE_TIMEOUT);
    auto img = std::make_shared<RawImage>(
        &PixelFormat::RGGB8,
        buff->ImageSize().GetWidth(),
        buff->ImageSize().GetHeight(),
        buff->Timestamp_ns(),
        static_cast<unsigned char*>( buff->BasePtr())
    );
    _dataStream->QueueBuffer(buff);
    return img;
}

const PixelFormat IdsPeakDriver::format() {
	return PixelFormat::RGGB8;
}

double IdsPeakDriver::expectedFrametime() {return 0.;}
#endif
