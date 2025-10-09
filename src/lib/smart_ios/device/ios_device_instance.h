#ifndef _IOS_DEVICE_INSTANCE_H_
#define _IOS_DEVICE_INSTANCE_H_
#pragma once

#include <smart_adv.h>
#include <lusb0_usb.h>

typedef std::map<std::string, std::string>	iOSDeviceInfo;

class iOSDeviceInstance 
{
public:
	iOSDeviceInstance(const std::string& device_id, const iOSDeviceInfo& device_info);
	virtual ~iOSDeviceInstance();
public:
	const std::string& GetDeviceId();
	const std::string& GetDeviceName();
	const std::string& GetProductName();
	const std::string& GetProductType();
	const std::string& GetProductVersion();
	const std::string& GetPhoneNumber();
	const std::string& GetSerialNumber();
private:
	std::string					_device_id;
	iOSDeviceInfo				_device_info;
};

#endif // !_IOS_DEVICE_INSTANCE_H_