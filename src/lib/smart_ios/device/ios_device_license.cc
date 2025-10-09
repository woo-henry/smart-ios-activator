#include <smart_base.h>
#include <smart_adv.h>
#include <smart_ios.h>
#include <smart_license.h>
#include <shlwapi.h>
#include "device/ios_device_license.h"

#ifndef SMART_LICENSE_TIMEOUT
#define SMART_LICENSE_TIMEOUT               1000 * 60 * 5               
#endif // !SMART_LICENSE_TIMEOUT

iOSDeviceLicense::iOSDeviceLicense(SmartThreadPool* thread_pool, iOSDeviceLicenseCallback* license_callback)
    : _thread_pool(thread_pool)
    , _license_callback(license_callback)
    , _license_file(nullptr)
    , _license_password(nullptr)
    , _validate_license_event(nullptr)
    , _validate_license_shutdown_event(nullptr)
    , _validate_license_shutdown(false)
{

}

iOSDeviceLicense::~iOSDeviceLicense()
{
    _validate_license_shutdown = true;

    if (_validate_license_event)
    {
        SetEvent(_validate_license_event);
    }

    if (_validate_license_shutdown_event)
    {
        WaitForSingleObject(_validate_license_shutdown_event, INFINITE);
        CloseHandle(_validate_license_shutdown_event);
        _validate_license_shutdown_event = nullptr;
    }

    if (_validate_license_event)
    {
        CloseHandle(_validate_license_event);
        _validate_license_event = nullptr;
    }

    if (_license_password)
    {
        SmartMemFree(_license_password);
        _license_password = nullptr;
    }

    if (_license_file)
    {
        SmartMemFree(_license_file);
        _license_file = nullptr;
    }

    _license_callback = nullptr;
}

int iOSDeviceLicense::Init(const char* license_file, const char* license_password)
{
    int result = IOS_ERROR_SUCCESS;

    do
    {
        result = ValidateLicenseFile(license_file, license_password);
        if (result != IOS_ERROR_SUCCESS)
        {
            std::string message;
            if (result == IOS_ERROR_LICENSE_FILE_NOT_FOUND)
            {
                message.append("License文件缺失，请联系客服解决问题！");
            }
            else if (result == IOS_ERROR_LICENSE_OBJECT_CORRUPT || result == IOS_ERROR_LICENSE_FILE_CORRUPT)
            {
                message.append("License已经损坏，请联系客服解决问题！");
            }
            else if (result == IOS_ERROR_LICENSE_PASSWORD_EMPTY)
            {
                message.append("License密码错误，请联系客服解决问题！");
            }
            else if (result == IOS_ERROR_LICENSE_PASSWORD_NOT_MATCHED)
            {
                message.append("License密码失效，请联系客服解决问题！");
            }
            else if (result == IOS_ERROR_LICENSE_EXPIRED)
            {
                message.append("License已经过期，请联系客服解决问题！");
            }
            else if (result != IOS_ERROR_LICENSE_INVALIDTED)
            {
                message.append("License已经无效，请联系客服解决问题！");
            }

            if (!message.empty())
            {
                SmartLogError(message.c_str());
            }
            break;
        }

        _validate_license_event = CreateEvent(nullptr, false, false, nullptr);
        if (_validate_license_event == nullptr)
        {
            result = GetLastError();
            break;
        }

        _validate_license_shutdown_event = CreateEvent(nullptr, false, false, nullptr);
        if (_validate_license_shutdown_event == nullptr)
        {
            result = GetLastError();
            break;
        }

        _validate_license_shutdown = false;

        _thread_pool->Execute(this, this);

    } while (false);

    return result;
}

void iOSDeviceLicense::Dispose()
{
    delete this;
}

ULONGLONG iOSDeviceLicense::GetLicenseExpireTime()
{
    return _license_expire_time;
}

int iOSDeviceLicense::GetLicenseCount()
{
    return _license_count;
}

int iOSDeviceLicense::ValidateLicenseFile(const char* license_file, const char* license_password)
{
    int result = IOS_ERROR_SUCCESS;

    do
    {
        UINT license_password_length = lstrlenA(license_password);
        _license_password = (char*)SmartMemAlloc(license_password_length * sizeof(char));
        if (_license_password == nullptr)
        {
            result = IOS_ERROR_LICENSE_PASSWORD_EMPTY;
            break;
        }

        _license_file = (char*)SmartMemAlloc(MAX_PATH * sizeof(char));
        if (_license_file == nullptr)
        {
            result = IOS_ERROR_LICENSE_FILE_NOT_FOUND;
            break;
        }

        if (!SmartFsGetAppPathA(_license_file, license_file))
        {
            result = IOS_ERROR_LICENSE_FILE_NOT_FOUND;
            break;
        }

        if (!PathFileExistsA(_license_file))
        {
            result = IOS_ERROR_LICENSE_FILE_NOT_FOUND;
            break;
        }

        result = ValidateLicenseData(_license_file, license_password);
        if (result != IOS_ERROR_SUCCESS)
            break;

        if (_license_password)
        {
            lstrcpyA(_license_password, license_password);
        }

    } while (false);

    return result;
}

int iOSDeviceLicense::ValidateLicenseData(const char* license_file, const char* license_password)
{
    int result = IOS_ERROR_SUCCESS;
    char* license_data = nullptr;
    SmartLicense* license = nullptr;

    do
    {
        license_data = (char*)SmartMemAlloc(sizeof(SmartLicense));
        if (license_data == nullptr)
        {
            result = IOS_ERROR_LICENSE_OBJECT_CORRUPT;
            break;
        }

        size_t size = 0;
        if (!SmartFsReadFileA(license_file, (char*)license_data, &size))
        {
            result = IOS_ERROR_LICENSE_FILE_CORRUPT;
            break;
        }

        SmartCryptoBuffer(license_data, size, SMART_LICENSE_CRYPTO_KEY);

        license = (SmartLicense*)license_data;
        if (license->registry_status == 0)
        {
            result = IOS_ERROR_LICENSE_STATUS_NOT_MATCHED;
            break;
        }

        if(license_password == nullptr || license->password == nullptr)
        {
            result = IOS_ERROR_LICENSE_PASSWORD_EMPTY;
            break;
        }

        if (lstrcmpiA(license_password, license->password))
        {
            result = IOS_ERROR_LICENSE_PASSWORD_NOT_MATCHED;
            break;
        }

        ULONGLONG current_time = SmartSysGetCurrentTime();
        if(license->expire_time < current_time)
        {
           result = IOS_ERROR_LICENSE_EXPIRED;
           break;
        }

        _license_expire_time = license->expire_time;
        _license_count = license->license_count;

    } while (false);

    if (license_data)
    {
        SmartMemFree(license_data);
        license_data = nullptr;
    }

    return result;
}

const char* iOSDeviceLicense::GetLicenseFile()
{
    return _license_file;
}

const char* iOSDeviceLicense::GetLicensePassword()
{
    return _license_password;
}

void iOSDeviceLicense::OnThreadHandle(void* parameter)
{
    while (true)
    {
        if (_validate_license_shutdown)
            break;

        DWORD wait = WaitForSingleObject(_validate_license_event, SMART_LICENSE_TIMEOUT); // 5分钟轮询一次
        if (wait == WAIT_OBJECT_0)
            break;

        const char* license_file = GetLicenseFile();
        const char* license_password = GetLicensePassword();
        int validate_status = ValidateLicenseData(license_file, license_password);
        if (validate_status != IOS_ERROR_SUCCESS)
        {
            std::string message;
            if (validate_status == IOS_ERROR_LICENSE_FILE_NOT_FOUND)
            {
                message.append("License文件缺失，请联系客服解决问题！");
            }
            else if (validate_status == IOS_ERROR_LICENSE_OBJECT_CORRUPT || validate_status == IOS_ERROR_LICENSE_FILE_CORRUPT)
            {
                message.append("License已经损坏，请联系客服解决问题！");
            }
            else if (validate_status == IOS_ERROR_LICENSE_PASSWORD_EMPTY)
            {
                message.append("License密码错误，请联系客服解决问题！");
            }
            else if (validate_status == IOS_ERROR_LICENSE_PASSWORD_NOT_MATCHED)
            {
                message.append("License密码失效，请联系客服解决问题！");
            }
            else if (validate_status == IOS_ERROR_LICENSE_EXPIRED)
            {
                message.append("License已经过期，请联系客服解决问题！");
            }
            else if (validate_status != IOS_ERROR_LICENSE_INVALIDTED)
            {
                message.append("License已经无效，请联系客服解决问题！");
            }

            if (_license_callback)
            {
                _license_callback->OnLicenseError(message.c_str());
            }
        }
    }

    if (_validate_license_shutdown_event)
    {
        SetEvent(_validate_license_shutdown_event);
    }
}