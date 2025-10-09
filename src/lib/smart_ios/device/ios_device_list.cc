#include <smart_base.h>
#include <smart_adv.h>
#include <smart_ios.h>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include "device/ios_device_list.h"

iOSDeviceList::iOSDeviceList()
{
    InitializeCriticalSection(&_device_list_lock);
}

iOSDeviceList::~iOSDeviceList()
{
    RemoveDevices();

    DeleteCriticalSection(&_device_list_lock);
}

int iOSDeviceList::Init()
{
    int result = IOS_ERROR_SUCCESS;

    return result;
}

void iOSDeviceList::Dispose()
{
    delete this;
}

void iOSDeviceList::AddDevice(iOSDeviceInstance* device)
{
    EnterCriticalSection(&_device_list_lock);
    _device_list.push_back(device);
    LeaveCriticalSection(&_device_list_lock);
}

void iOSDeviceList::RemoveDevices()
{
    EnterCriticalSection(&_device_list_lock);

    for (std::vector<iOSDeviceInstance*>::iterator it = _device_list.begin(); it != _device_list.end(); it++)
    {
        iOSDeviceInstance* device = *it;
        if (device)
        {
            delete device;
            device = nullptr;
        }
    }

    _device_list.erase(_device_list.begin(), _device_list.end());

    LeaveCriticalSection(&_device_list_lock);
}

std::vector<iOSDeviceInstance*>* iOSDeviceList::GetDevices()
{
    return &_device_list;
}

UINT iOSDeviceList::GetDeviceCount()
{
    return _device_list.size();
}

iOSDeviceInstance* iOSDeviceList::GetExistDevice(const char* device_id)
{
    iOSDeviceInstance* result = nullptr;

    EnterCriticalSection(&_device_list_lock);

    for (std::vector<iOSDeviceInstance*>::iterator it = _device_list.begin(); it != _device_list.end(); it++)
    {
        iOSDeviceInstance* device = *it;
        if (device == nullptr)
            continue;

        if (lstrcmpA(device->GetDeviceId().c_str(), device_id) == 0)
        {
            result = device;
            break;
        }
    }

    LeaveCriticalSection(&_device_list_lock);

    return result;
}

void iOSDeviceList::StopDevices()
{
    EnterCriticalSection(&_device_list_lock);

    for (std::vector<iOSDeviceInstance*>::iterator it = _device_list.begin(); it != _device_list.end(); it++)
    {
        iOSDeviceInstance* device_instance = *it;
        if (device_instance == nullptr)
            continue;
    }

    LeaveCriticalSection(&_device_list_lock);
}