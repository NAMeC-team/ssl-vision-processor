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
#ifdef IDS_PEAK
#include "idspeakdriver.h"

#include "cameradriver.h"

#define CAMERA_FRAME_CAPTURE_TIMEOUT 1'000  // 1 second
static std::atomic<uint8_t> nbCameras(0);
using namespace peak::core;

IdsPeakDriver::IdsPeakDriver(const CameraConfig &config) : _deviceManager(peak::DeviceManager::Instance()) {
	if (nbCameras == 0) {
		peak::Library::Initialize();
	}
	_cameraId = nbCameras;
	nbCameras = nbCameras + 1;

	// open the next available camera
	_deviceDescriptor = _deviceManager.FirstAvailableDevice(DeviceAccessType::Control);
	if (_deviceDescriptor == nullptr) {
		FATAL("[IDS Peak driver] Cannot open camera : no camera left available !");
	}
	_device = _deviceDescriptor->OpenDevice(DeviceAccessType::Control);

	// force capture type to Bayer RG8 (same logic as spinnakerdriver.cpp)
	_nodeMapRemoteDevice = _device->NodeMaps().at(0);
	const auto pixelFormatNode =
		_nodeMapRemoteDevice->FindNode<nodes::EnumerationNode>("PixelFormat");
	pixelFormatNode->SetCurrentEntry("BayerRG8");

	_dataStream = _device->DataStreams().at(0)->OpenDataStream();

	// Get the required payload size. If the data stream does not
	// define it, fall back to the remote device node map.
	const auto payloadSizeNode = _nodeMapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("PayloadSize");
	const auto payloadSize = _dataStream->DefinesPayloadSize() ? _dataStream->PayloadSize() :
														  static_cast<size_t>(payloadSizeNode->Value());
	const auto bufferCount = _dataStream->NumBuffersAnnouncedMinRequired();

	// Allocate buffers and queue them
	_dataStream->AddAcquisitionBuffers(payloadSize, bufferCount);

	// Lock transport layer parameters (TLParamsLocked = 1) to
	// prevent irregular access to the remote device during acquisition
	_nodeMapRemoteDevice->FindNode<nodes::IntegerNode>("TLParamsLocked")->SetValue(1);

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
	BufferGuard guard(_dataStream->WaitForFinishedBuffer(CAMERA_FRAME_CAPTURE_TIMEOUT));
	auto buffer = guard.Buffer();
	// todo: convert buffer to RawImage
}

const PixelFormat IdsPeakDriver::format() {
	return PixelFormat::RGGB8;
}

double IdsPeakDriver::expectedFrametime() {return 0.;}
#endif