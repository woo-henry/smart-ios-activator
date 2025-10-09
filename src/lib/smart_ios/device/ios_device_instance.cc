#include <smart_base.h>
#include <smart_ios.h>
#include "device/ios_device_utils.h"
#include "device/ios_device_instance.h"

iOSDeviceInstance::iOSDeviceInstance(const std::string& device_id, const iOSDeviceInfo& device_info)
	: _device_id(device_id)
    , _device_info(device_info)
{

}

iOSDeviceInstance::~iOSDeviceInstance()
{

}

const std::string& iOSDeviceInstance::GetDeviceId()
{
    return _device_id;
}

const std::string& iOSDeviceInstance::GetDeviceName()
{
    return _device_info["DeviceName"];
}

const std::string& iOSDeviceInstance::GetProductName()
{
    return _device_info["ProductName"];
}

const std::string& iOSDeviceInstance::GetProductType()
{
    return _device_info["ProductType"];
}

const std::string& iOSDeviceInstance::GetProductVersion()
{
    return _device_info["ProductVersion"];
}

const std::string& iOSDeviceInstance::GetPhoneNumber()
{
    return _device_info["PhoneNumber"];
}

const std::string& iOSDeviceInstance::GetSerialNumber()
{
    return _device_info["SerialNumber"];
}