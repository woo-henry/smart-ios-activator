#include <smart_base.h>
#include <boost\algorithm\string.hpp>
#include <libimobiledevice/libimobiledevice.h>
#include "device/ios_device_activator.h"

iOSDeviceActivator::iOSDeviceActivator(SmartThreadPool* thread_pool, void* device_context, iOSDeviceCallbacks* device_callbacks)
{

}

iOSDeviceActivator::~iOSDeviceActivator()
{

}

int iOSDeviceActivator::Init()
{
	int result = ERROR_SUCCESS;

	return result;
}

void iOSDeviceActivator::Dispose()
{
	delete this;
}

int iOSDeviceActivator::ActivateDevice(const char* device_id, bool skip_install_setup)
{
	int result = ERROR_SUCCESS;

	return result;
}

int iOSDeviceActivator::DeactivateDevice(const char* device_id)
{
	int result = ERROR_SUCCESS;

	return result;
}