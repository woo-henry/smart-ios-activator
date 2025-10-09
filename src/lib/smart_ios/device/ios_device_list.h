#ifndef _IOS_DEVICE_LIST_H_
#define _IOS_DEVICE_LIST_H_
#pragma once

#include "device/ios_device_instance.h"

class iOSDeviceList
{
public:
	iOSDeviceList();
	virtual ~iOSDeviceList();
public:
	int Init();
	void Dispose();
	void AddDevice(iOSDeviceInstance* device);
	void RemoveDevices();
	std::vector<iOSDeviceInstance*>* GetDevices();
	UINT GetDeviceCount();
	iOSDeviceInstance* GetExistDevice(const char* device_id);
	void StopDevices();
private:
	std::vector<iOSDeviceInstance*>	_device_list;
	CRITICAL_SECTION				_device_list_lock;
};

#endif // _IOS_DEVICE_LIST_H_