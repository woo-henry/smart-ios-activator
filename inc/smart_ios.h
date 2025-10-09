#ifndef _SMART_IOS_H_
#define _SMART_IOS_H_
#pragma once
//////////////////////////////////////////////////////////////////////////
#include <smart_ios_config.h>
#include <smart_ios_error.h>
#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*iOSDeviceConnectedCallback)(void* context, const char* device_id);
typedef void (*iOSDeviceDisconnectedCallback)(void* context, const char* device_id);
typedef void (*iOSDeviceQueryCallback)(void* context,
	const char* device_id,
	const char* device_name,
	const char* serial_number,
	const char* product_name,
	const char* product_type,
	const char* product_version,
	const char* phone_number);
typedef void (*iOSDeviceActivateStartCallback)(void* context, const char* device_id);
typedef void (*iOSDeviceActivateProgressCallback)(void* context, const char* device_id, unsigned int progress);
typedef void (*iOSDeviceActivateCompleteCallback)(void* context, const char* device_id);
typedef void (*iOSDeviceDeactivateStartCallback)(void* context, const char* device_id);
typedef void (*iOSDeviceDeactivateProgressCallback)(void* context, const char* device_id, unsigned int progress);
typedef void (*iOSDeviceDeactivateCompleteCallback)(void* context, const char* device_id);
typedef void (*iOSDeviceErrorCallback)(void* context, const char* device_id, int error_code);
typedef struct tagiOSDeviceCallbacks
{
	iOSDeviceConnectedCallback			callback_connected;
	iOSDeviceDisconnectedCallback		callback_disconnected;
	iOSDeviceQueryCallback				callback_query;
	iOSDeviceActivateStartCallback		callback_activate_start;
	iOSDeviceActivateProgressCallback	callback_activate_progress;
	iOSDeviceActivateCompleteCallback	callback_activate_complete;
	iOSDeviceDeactivateStartCallback	callback_deactivate_start;
	iOSDeviceDeactivateProgressCallback	callback_deactivate_progress;
	iOSDeviceDeactivateCompleteCallback	callback_deactivate_complete;
	iOSDeviceErrorCallback				callback_error;
} iOSDeviceCallbacks;

int IOS_API InitiOSDeviceEnviroment(void* context, iOSDeviceCallbacks* callbacks);
int IOS_API ShutdowniOSDeviceEnviroment();
int IOS_API QueryiOSDevice(const char* device_id);
int IOS_API ActivateiOSDevice(const char* device_id, bool skip_install_setup = true);
int IOS_API DeactivateiOSDevice(const char* device_id);

#ifdef __cplusplus
};
#endif

#endif // _SMART_IOS_H_
