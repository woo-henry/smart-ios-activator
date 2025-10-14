#ifndef _IOS_DEVICE_CONTROL_H_
#define _IOS_DEVICE_CONTROL_H_
#pragma once

#include <smart_adv.h>
#include "device/ios_device_license.h"
#include "device/ios_device_list.h"
#include "device/ios_device_enumerator.h"
#include "device/ios_device_querier.h"
#include "device/ios_device_activator.h"

class iOSDeviceControl
	: public iOSDeviceLicenseCallback
{
public:
	iOSDeviceControl(void* device_context, iOSDeviceCallbacks* device_callbacks);
	virtual ~iOSDeviceControl();
public:
	int Init(const char* license_file, const char* license_password);
	void Dispose();
	int QueryDevice(const char* device_id);
	int ActivateDevice(const char* device_id, bool skip_install_setup, const char* wifi_ssid, const char* wifi_password);
	int DeactivateDevice(const char* device_id);
	int QueryDeviceState(const char* device_id, bool* activated, bool* setup_done);
protected:
	virtual void OnLicenseError(const char* message) override;
private:
	int StartAppleMobileService();
	int StopAppleMobileService();
private:
	SmartThreadPool*				_thread_pool;
	void*							_device_context;
	iOSDeviceCallbacks*				_device_callbacks;
	iOSDeviceLicense*				_device_license;
	iOSDeviceList*					_device_list;
	iOSDeviceEnumerator*			_device_enumerator;
	iOSDeviceQuerier*				_device_querier;
	iOSDeviceActivator*				_device_activator;
};

#endif // !_IOS_DEVICE_CONTROL_H_