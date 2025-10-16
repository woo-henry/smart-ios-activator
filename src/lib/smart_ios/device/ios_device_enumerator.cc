#include <smart_base.h>
#include <smart_ios.h>
#include <cfgmgr32.h>
#include <boost\algorithm\string.hpp>
#include <libimobiledevice/libimobiledevice.h>
#include "device/ios_device_list.h"
#include "device/ios_device_enumerator.h"

#ifndef IOS_DEVICE_ENUM_TIMEOUT
#define IOS_DEVICE_ENUM_TIMEOUT					3000
#endif // !IOS_DEVICE_ENUM_TIMEOUT

iOSDeviceEnumerator::iOSDeviceEnumerator(SmartThreadPool* thread_pool, void* device_context, iOSDeviceCallbacks* device_callbacks, iOSDeviceList* device_list)
    : _thread_pool(thread_pool)
    , _device_context(device_context)
    , _device_callbacks(device_callbacks)
    , _device_list(device_list)
    , _enum_devices_event(nullptr)
    , _enum_devices_shutdown_event(nullptr)
    , _enum_devices_shutdown(true)
{

}

iOSDeviceEnumerator::~iOSDeviceEnumerator()
{
    _enum_devices_shutdown = true;

    if (_enum_devices_event)
    {
        SetEvent(_enum_devices_event);
    }

    if (_enum_devices_shutdown_event)
    {
        WaitForSingleObject(_enum_devices_shutdown_event, INFINITE);
        CloseHandle(_enum_devices_shutdown_event);
        _enum_devices_shutdown_event = nullptr;
    }

    if (_enum_devices_event)
    {
        CloseHandle(_enum_devices_event);
        _enum_devices_event = nullptr;
    }
}

int iOSDeviceEnumerator::Init()
{
    int result = IOS_ERROR_SUCCESS;

    do
    {
        _enum_devices_event = CreateEvent(nullptr, false, false, nullptr);
        if (_enum_devices_event == nullptr)
        {
            result = GetLastError();
            break;
        }

        _enum_devices_shutdown_event = CreateEvent(nullptr, false, false, nullptr);
        if (_enum_devices_shutdown_event == nullptr)
        {
            result = GetLastError();
            break;
        }

        _thread_pool->Execute(this, this);

    } while (false);

    return result;
}

void iOSDeviceEnumerator::Dispose()
{
    delete this;
}

int iOSDeviceEnumerator::EnumDevices()
{
    int result = IOS_ERROR_SUCCESS;
    char* device_id_buffer = nullptr;
    ULONG device_id_length = 1024;
    std::queue<DEVINST> dev_queue;
    std::vector<std::string> device_ids;

    do
    {
        device_id_buffer = (char*)SmartMemAlloc(device_id_length);
        if (device_id_buffer == nullptr)
        {
            result = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        while (true)
        {
            if (_enum_devices_shutdown)
                break;

            result = WaitForSingleObject(_enum_devices_event, IOS_DEVICE_ENUM_TIMEOUT);
            if (result == WAIT_OBJECT_0)
                break;

            if (_enum_devices_shutdown)
                break;

            /*
            DEVINST dev_root;
            result = CM_Locate_DevNodeA(&dev_root, nullptr, CM_LOCATE_DEVNODE_NORMAL);
            if (result != CR_SUCCESS)
                break;

            if (_enum_devices_shutdown)
                break;

            device_ids.erase(device_ids.begin(), device_ids.end());

            if (_enum_devices_shutdown)
                break;

            dev_queue.push(dev_root);

            if (_enum_devices_shutdown)
                break;

            while (!dev_queue.empty())
            {
                if (_enum_devices_shutdown)
                    break;

                DEVINST dev_inst = dev_queue.front();
                dev_queue.pop();

                if (_enum_devices_shutdown)
                    break;

                RtlZeroMemory(device_id_buffer, device_id_length);
                result = CM_Get_Device_IDA(dev_inst, device_id_buffer, device_id_length, 0);

                if (_enum_devices_shutdown)
                    break;

                if (result == CR_SUCCESS)
                {
                    if (boost::ifind_first(device_id_buffer, "USB\\VID_05AC&PID_12A8\\"))
                    {
                        std::string device_id(device_id_buffer);
                        boost::replace_all(device_id, "USB\\VID_05AC&PID_12A8\\", "");
                        boost::replace_all(device_id, "-", "");
                        boost::to_lower(device_id);
                        boost::trim(device_id);
                        std::vector<std::string>::iterator it = std::find(device_ids.begin(), device_ids.end(), device_id);
                        if (it == device_ids.end())
                        {
                            device_ids.push_back(device_id);
                        }
                    }
                }

                if (_enum_devices_shutdown)
                    break;

                DEVINST dev_sibling;
                result = CM_Get_Sibling(&dev_sibling, dev_inst, 0);
                if (result == CR_SUCCESS)
                {
                    dev_queue.push(dev_sibling);
                }

                if (_enum_devices_shutdown)
                    break;

                DEVINST dev_child;
                result = CM_Get_Child(&dev_child, dev_inst, 0);
                if (result == CR_SUCCESS)
                {
                    dev_queue.push(dev_child);
                }
            }
            */

            result = EnumDeviceIds(&device_ids);

            for (std::vector<std::string>::const_iterator it = device_ids.begin(); it != device_ids.end(); it++)
            {
                if (_enum_devices_shutdown)
                    break;

                std::string device_id = *it;
                iOSDeviceInstance* device = _device_list->GetExistDevice(device_id.c_str());
                if (device == nullptr)
                {
                    iOSDeviceInfo device_info;
                    iOSDeviceInstance* device = new iOSDeviceInstance(device_id.c_str(), device_info);
                    if (device)
                    {
                        _device_list->AddDevice(device);
                    }

                    if (_device_callbacks)
                    {
                        _device_callbacks->callback_connected(_device_context, device_id.c_str());
                    }
                }
            }

            /*
            if (device_ids.empty())
                continue;
            */

            if (_enum_devices_shutdown)
                break;

            std::vector<iOSDeviceInstance*>* devices = _device_list->GetDevices();
            if (!_enum_devices_shutdown)
            {
                for (std::vector<iOSDeviceInstance*>::const_iterator it = devices->begin(); it != devices->end();)
                {
                    if (_enum_devices_shutdown)
                        break;

                    iOSDeviceInstance* device_instance = *it;
                    if (device_instance == nullptr)
                        continue;

                    std::vector<std::string>::iterator find = std::find(device_ids.begin(), device_ids.end(), device_instance->GetDeviceId());
                    if (find == device_ids.end())
                    {
                        it = devices->erase(it);

                        if (_device_callbacks)
                        {
                            _device_callbacks->callback_disconnected(_device_context, device_instance->GetDeviceId().c_str());
                        }

                        if (device_instance)
                        {
                            delete device_instance;
                            device_instance = nullptr;
                        }
                    }
                    else
                    {
                        it++;
                    }
                }
            }
        }
    } while (false);

    if (device_id_buffer)
    {
        SmartMemFree(device_id_buffer);
        device_id_buffer = nullptr;
    }

    return result;
}

int iOSDeviceEnumerator::EnumDeviceIds(std::vector<std::string>* device_ids)
{
    int result = IOS_ERROR_SUCCESS;
    idevice_info_t* dev_list = nullptr;

    do
    {
        int dev_count;
        if (idevice_get_device_list_extended(&dev_list, &dev_count) < 0)
        {
            result = ERROR_NOT_FOUND;
            break;
        }

        for (int i = 0; i < dev_count; i++)
        {
            if (_enum_devices_shutdown)
                break;

            if (dev_list[i] == nullptr)
                continue;

            if (dev_list[i]->conn_type != CONNECTION_USBMUXD)
                continue;

            device_ids->push_back(dev_list[i]->udid);
        }
    } while (false);

    if (dev_list)
    {
        idevice_device_list_extended_free(dev_list);
        dev_list = nullptr;
    }

    return result;
}

void iOSDeviceEnumerator::OnThreadHandle(void* parameter)
{
    _enum_devices_shutdown = false;

    EnumDevices();

    if (_enum_devices_shutdown_event)
    {
        SetEvent(_enum_devices_shutdown_event);
    }
}