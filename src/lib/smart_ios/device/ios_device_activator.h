#ifndef _IOS_DEVICE_ACTIVATOR_H_
#define _IOS_DEVICE_ACTIVATOR_H_
#pragma once

#include <smart_adv.h>
#include <smart_ios.h>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/mobileactivation.h>
#include <libideviceactivation.h>

class iOSDeviceActivator
{
public:
	iOSDeviceActivator(SmartThreadPool* thread_pool, void* device_context, iOSDeviceCallbacks* device_callbacks);
	virtual ~iOSDeviceActivator();
public:
	int Init();
	void Dispose();
	int ActivateDevice(const char* device_id, bool skip_install_setup);
	int ActivateDeviceEx(const char* device_id, bool skip_install_setup);
	int ActivateDeviceByCommand(const char* device_id, bool skip_install_setup);
	int DeactivateDevice(const char* device_id);
	int QueryDeviceState(const char* device_id, bool* activated);
protected:
	int OpenAppleMobileServiceConnection(SOCKET* sock);
	int SendAppleMobileServiceMessage(SOCKET& sock);
	int RecvAppleMobileServiceMessage(SOCKET& sock);
protected:
	int GetMobileActivationClient(idevice_t device, lockdownd_client_t lockdown, mobileactivation_client_t* mobileactivation_client);
	uint32_t GetProductVersion(lockdownd_client_t lockdown);
	int GetActivationState(lockdownd_client_t lockdown, char** activation_state_string);
	int StartOrStopAppleMobileDeviceService(bool start);
};

#endif // _IOS_DEVICE_ACTIVATOR_H_