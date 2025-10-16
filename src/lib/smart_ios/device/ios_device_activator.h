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
	int ActivateDevice(const char* device_id, bool skip_install_setup, const char* wifi_ssid, const char* wifi_password);
	int DeactivateDevice(const char* device_id);
	int QueryDeviceState(const char* device_id, bool* activated, bool* setup_done);
	int SetupDoneDevice(idevice_t device, const char* wifi_ssid, const char* wifi_password);
protected:
	int ValidateDevice(const char* device_id);
	int SyncTimeIntervalSince1970(idevice_t device);
	int SetupWifiConnection(lockdownd_client_t client, const char* wifi_ssid, const char* wifi_password);
	int SetupLanguage(lockdownd_client_t client, bool chinese);
	int SetupCloudConfiguration(lockdownd_client_t client, plist_t configuration);
	int CreateDomainsDict(plist_t* dict);
	int CreateWifiDict(const char* ssid, const char* password, plist_t* dict);
	int CreateWifiSettingDict(plist_t* dict);
protected:
	int GetMobileActivationClient(idevice_t device, lockdownd_client_t lockdown, mobileactivation_client_t* mobileactivation_client);
	uint32_t GetProductVersion(lockdownd_client_t lockdown);
	int GetActivationState(lockdownd_client_t lockdown, char** activation_state_string);
	int StartOrStopAppleMobileDeviceService(bool start);
protected:
	char	_configuration_path[MAX_PATH];
};

#endif // _IOS_DEVICE_ACTIVATOR_H_