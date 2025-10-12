#include <smart_base.h>
#include <smart_ios.h>
#include <setupapi.h>
#include <boost/algorithm/string.hpp>
#include "device/ios_device_control.h"

#ifndef SMART_THREAD_MINIMUM
#define	SMART_THREAD_MINIMUM			        8
#endif // !SMART_THREAD_MINIMUM

#ifndef SMART_THREAD_MAXIMUM
#define	SMART_THREAD_MAXIMUM			        32
#endif // !SMART_THREAD_MAXIMUM

iOSDeviceControl::iOSDeviceControl(void* device_context, iOSDeviceCallbacks* device_callbacks)
    : _thread_pool(nullptr)
    , _device_context(device_context)
    , _device_callbacks(device_callbacks)
    , _device_license(nullptr)
    , _device_list(nullptr)
    , _device_enumerator(nullptr)
    , _device_querier(nullptr)
    , _device_activator(nullptr)
{
    
}

iOSDeviceControl::~iOSDeviceControl()
{
    if (_device_activator)
    {
        _device_activator->Dispose();
        _device_activator = nullptr;
    }

    if (_device_querier)
    {
        _device_querier->Dispose();
        _device_querier = nullptr;
    }

    if (_device_enumerator)
    {
        _device_enumerator->Dispose();
        _device_enumerator = nullptr;
    }

    if (_device_list)
    {
        _device_list->Dispose();
        _device_list = nullptr;
    }

    if (_device_license)
    {
        _device_license->Dispose();
        _device_license = nullptr;
    }

    StopAppleMobileService();

    if (_thread_pool)
    {
        _thread_pool->Dispose();
        _thread_pool = nullptr;
    }
}

int iOSDeviceControl::Init(const char* license_file, const char* license_password)
{
    int result = IOS_ERROR_SUCCESS;

    do
    {
        StopAppleMobileService();

        _thread_pool = SmartThreadPoolFactory::CreateThreadPool();
        if (_thread_pool == nullptr)
        {
            result = IOS_ERROR_THREADPOOL_NOT_CREATED;
            break;
        }

        result = _thread_pool->Init(SMART_THREAD_MINIMUM, SMART_THREAD_MAXIMUM);
        if (result != IOS_ERROR_SUCCESS)
        {
            result = IOS_ERROR_THREADPOOL_NOT_INITED;
            break;
        }

        _device_license = new iOSDeviceLicense(_thread_pool, this);
        if (_device_license == nullptr)
        {
            result = IOS_ERROR_DEVICE_LIST_NOT_CREATED;
            break;
        }

        result = _device_license->Init(license_file, license_password);
        if (result != IOS_ERROR_SUCCESS)
        {
            result = IOS_ERROR_OBJECT_NOT_INITED;
            break;
        }

        _device_list = new iOSDeviceList();
        if (_device_list == nullptr)
        {
            result = IOS_ERROR_DEVICE_LIST_NOT_CREATED;
            break;
        }
        
        result = _device_list->Init();
        if (result != IOS_ERROR_SUCCESS)
        {
            result = IOS_ERROR_OBJECT_NOT_INITED;
            break;
        }

        _device_enumerator = new iOSDeviceEnumerator(_thread_pool, _device_context, _device_callbacks, _device_list);
        if (_device_enumerator == nullptr)
        {
            result = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        result = _device_enumerator->Init();
        if (result != IOS_ERROR_SUCCESS)
            break;

        _device_querier = new iOSDeviceQuerier(_thread_pool, _device_context, _device_callbacks);
        if (_device_querier == nullptr)
        {
            result = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        result = _device_querier->Init();
        if (result != IOS_ERROR_SUCCESS)
            break;

        _device_activator = new iOSDeviceActivator(_thread_pool, _device_context, _device_callbacks);
        if (_device_activator == nullptr)
        {
            result = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        result = _device_activator->Init();
        if (result != IOS_ERROR_SUCCESS)
            break;

        result = StartAppleMobileService();
        if (result != IOS_ERROR_SUCCESS)
            break;

    } while (false);

    return result;
}

void iOSDeviceControl::Dispose()
{
    delete this;
}

int iOSDeviceControl::QueryDevice(const char* device_id)
{
    int result = IOS_ERROR_SUCCESS;

    do
    {
        result = _device_querier->QueryDevice(device_id);

    } while (false);

    return result;
}

int iOSDeviceControl::ActivateDevice(const char* device_id, bool skip_install_setup)
{
    int result = IOS_ERROR_SUCCESS;

    do
    {
        int license_count = _device_license->GetLicenseCount();
        int device_count = _device_list->GetDeviceCount();
        if (device_count > license_count)
        {
            result = IOS_ERROR_LICENSE_EXCEEDING_QUANTITY;
            break;
        }

        result = _device_activator->ActivateDeviceEx(device_id, skip_install_setup);
        if (result == ERROR_SUCCESS)
            break;

        result = _device_activator->ActivateDevice(device_id, skip_install_setup);
     
    } while (false);

    return result;
}

int iOSDeviceControl::DeactivateDevice(const char* device_id)
{
    int result = IOS_ERROR_SUCCESS;

    do
    {
        int license_count = _device_license->GetLicenseCount();
        int device_count = _device_list->GetDeviceCount();
        if (device_count > license_count)
        {
            result = IOS_ERROR_LICENSE_EXCEEDING_QUANTITY;
            break;
        }

        result = _device_activator->DeactivateDevice(device_id);

    } while (false);

    return result;
}

int iOSDeviceControl::QueryDeviceState(const char* device_id, bool* activated)
{
    int result = IOS_ERROR_SUCCESS;

    do
    {
        result = _device_activator->QueryDeviceState(device_id, activated);

    } while (false);

    return result;
}

void iOSDeviceControl::OnLicenseError(const char* message)
{
    SmartLogError("%s", message);
}

int iOSDeviceControl::StartAppleMobileService()
{
    int result = IOS_ERROR_SUCCESS;
    char* usbmuxd_path = nullptr;

    do
    {
        if (SmartProcessManager::IsProcessRunningA("apple-mobile-service.exe"))
            break;

        usbmuxd_path = (char*)SmartMemAlloc(MAX_PATH * sizeof(char));
        if (usbmuxd_path == nullptr)
        {
            result = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if (!SmartFsGetAppPathA(usbmuxd_path, "tools\\apple-mobile-service.exe"))
        {
            result = ERROR_FILE_NOT_FOUND;
            break;
        }

        STARTUPINFOA si = { 0 };
        RtlZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
        si.wShowWindow = SW_HIDE;

        PROCESS_INFORMATION pi = { 0 };
        RtlZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
        if (!CreateProcessA(nullptr, usbmuxd_path, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi))
        {
            result = GetLastError();
            break;
        }

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

    } while (false);

    if (usbmuxd_path)
    {
        SmartMemFree(usbmuxd_path);
        usbmuxd_path = nullptr;
    }

    return result;
}

int iOSDeviceControl::StopAppleMobileService()
{
    return SmartProcessManager::TerminateProcessByNameA("apple-mobile-service.exe");
}