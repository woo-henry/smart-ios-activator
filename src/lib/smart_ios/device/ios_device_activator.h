#ifndef _IOS_DEVICE_ACTIVATOR_H_
#define _IOS_DEVICE_ACTIVATOR_H_
#pragma once

#include <smart_adv.h>
#include <smart_ios.h>

class iOSDeviceActivator
{
public:
	iOSDeviceActivator(SmartThreadPool* thread_pool, void* device_context, iOSDeviceCallbacks* device_callbacks);
	virtual ~iOSDeviceActivator();
public:
	int Init();
	void Dispose();
	int ActivateDevice(const char* device_id, bool skip_install_setup);
	int DeactivateDevice(const char* device_id);
};

#endif // _IOS_DEVICE_ACTIVATOR_H_