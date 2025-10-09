#ifndef _IOS_DEVICE_ENUMERATOR_H_
#define _IOS_DEVICE_ENUMERATOR_H_
#pragma once

#include <smart_adv.h>
#include <smart_ios.h>

class iOSDeviceEnumerator
	: public SmartThreadTask
{
public:
	iOSDeviceEnumerator(SmartThreadPool* thread_pool, void* device_context, iOSDeviceCallbacks* device_callbacks, iOSDeviceList* device_list);
	virtual ~iOSDeviceEnumerator();
public:
	int Init();
	void Dispose();
	int EnumDevices();
	int EnumDeviceIds(std::vector<std::string>* device_ids);
protected:
	virtual void OnThreadHandle(void* parameter) override;
private:
	SmartThreadPool*				_thread_pool;
	void*							_device_context;
	iOSDeviceCallbacks*				_device_callbacks;
	iOSDeviceList*					_device_list;
	HANDLE							_enum_devices_event;
	HANDLE							_enum_devices_shutdown_event;
	std::atomic<bool>				_enum_devices_shutdown;
};
#endif // _IOS_DEVICE_ENUMERATOR_H_