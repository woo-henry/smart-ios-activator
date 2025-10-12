#ifndef _IOS_DEVICE_LICENSE_H_
#define _IOS_DEVICE_LICENSE_H_
#pragma once

#include <atomic>
#include <smart_adv.h>

class iOSDeviceLicenseCallback
{
public:
	virtual void OnLicenseError(const char* message) = 0;
};

class iOSDeviceLicense
	: public SmartThreadTask
{
public:
	iOSDeviceLicense(SmartThreadPool* thread_pool, iOSDeviceLicenseCallback* license_callback);
	virtual ~iOSDeviceLicense();
public:
	int Init(const char* license_file, const char* license_password);
	void Dispose();
	ULONGLONG GetLicenseExpireTime();
	int GetLicenseCount();
private:
	int ValidateLicenseFile(const char* license_file, const char* license_password);
	int ValidateLicenseData(const char* license_file, const char* license_password);
	const char* GetLicenseFile();
	const char* GetLicensePassword();
protected:
	virtual void OnThreadHandle(void* parameter) override;
private:
	SmartThreadPool*			_thread_pool;
	iOSDeviceLicenseCallback*	_license_callback;
	char*						_license_file;
	std::atomic<ULONGLONG>		_license_expire_time;
	char*						_license_password;
	std::atomic<int>			_license_count;
	HANDLE						_validate_license_event;
	HANDLE						_validate_license_shutdown_event;
	std::atomic<bool>			_validate_license_shutdown;
};
#endif // _IOS_DEVICE_LICENSE_H_