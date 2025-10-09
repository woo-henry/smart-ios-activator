#include <smart_base.h>
#include <smart_ios.h>
#include <shlwapi.h>
#include <boost\algorithm\string.hpp>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include "device/ios_device_utils.h"
#include "device/ios_device_list.h"
#include "device/ios_device_querier.h"

#ifndef IOS_DEVICE_GETINFO_RETRY_MAX
#define IOS_DEVICE_GETINFO_RETRY_MAX			3
#endif // !IOS_DEVICE_GETINFO_RETRY_MAX

#ifndef IOS_DEVICE_QUERY_TIMEOUT
#define IOS_DEVICE_QUERY_TIMEOUT				300
#endif // !IOS_DEVICE_QUERY_TIMEOUT

#ifndef IOS_DEVICE_QUERY_RETRY_MAX
#define IOS_DEVICE_QUERY_RETRY_MAX				5
#endif // !IOS_DEVICE_QUERY_RETRY_MAX

iOSDeviceQuerier::iOSDeviceQuerier(SmartThreadPool* thread_pool, void* device_context, iOSDeviceCallbacks* device_callbacks)
    : _thread_pool(thread_pool)
    , _device_context(device_context)
    , _device_callbacks(device_callbacks)
    , _device_command_info(nullptr)
{
    InitializeCriticalSection(&_device_query_contexts_lock);
}

iOSDeviceQuerier::~iOSDeviceQuerier()
{
    DestroyQueryContexts();

    DeleteCriticalSection(&_device_query_contexts_lock);

    if (_device_command_info)
    {
        _device_command_info->Dispose();
        _device_command_info = nullptr;
    }
}

int iOSDeviceQuerier::Init()
{
    int result = IOS_ERROR_SUCCESS;

    do
    {
        _device_command_info = new iOSDeviceCommandInfo;
        if (_device_command_info == nullptr)
        {
            result = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        result = _device_command_info->Init();

    } while (false);

    return result;
}

void iOSDeviceQuerier::Dispose()
{
    delete this;
}

int iOSDeviceQuerier::QueryDevice(const char* device_id)
{
    int result = IOS_ERROR_SUCCESS;

    do
    {
        QueryDeviceContext* context = nullptr;
        result = CreateQueryContext(device_id, &context);
        if (result != ERROR_SUCCESS)
            break;

        EnterCriticalSection(&_device_query_contexts_lock);
        _device_query_contexts.push_back(context);
        LeaveCriticalSection(&_device_query_contexts_lock);

        _thread_pool->Execute(context, this);

    } while (false);

    return result;
}

int iOSDeviceQuerier::CreateQueryContext(const char* device_id, QueryDeviceContext** context)
{
    int result = 0;

    do
    {
        QueryDeviceContext* context_ = new QueryDeviceContext;
        if (context_ == nullptr)
        {
            result = ERROR_OUTOFMEMORY;
            break;
        }

        int device_id_length = lstrlenA(device_id) + 1;
        context_->device_id = (char*)SmartMemAlloc(device_id_length);
        if (context_->device_id == nullptr)
        {
            delete context;
            context = nullptr;
            result = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        context_->device_query_event = CreateEvent(nullptr, false, false, nullptr);
        if (context_->device_query_event == nullptr)
        {
            delete context_;
            context_ = nullptr;
            result = GetLastError();
            break;
        }

        context_->device_query_shutdown_event = CreateEvent(nullptr, false, false, nullptr);
        if (context_->device_query_shutdown_event == nullptr)
        {
            delete context_;
            context_ = nullptr;
            result = GetLastError();
            break;
        }

        lstrcpyA(context_->device_id, device_id);
        context_->device_query_shutdown = false;
        
        *context = context_;

    } while (false);

    return result;
}

void iOSDeviceQuerier::DestroyQueryContext(QueryDeviceContext* context)
{
    context->device_query_shutdown = true;

    if (context->device_query_event)
    {
        SetEvent(context->device_query_event);
    }

    if (context->device_query_shutdown_event)
    {
        WaitForSingleObject(context->device_query_shutdown_event, INFINITE);
        CloseHandle(context->device_query_shutdown_event);
        context->device_query_shutdown_event = nullptr;
    }

    if (context->device_query_event)
    {
        CloseHandle(context->device_query_event);
        context->device_query_event = nullptr;
    }

    if (context->device_id)
    {
        SmartMemFree(context->device_id);
        context->device_id = nullptr;
    }
}

void iOSDeviceQuerier::DestroyQueryContexts()
{
    EnterCriticalSection(&_device_query_contexts_lock);
    if (_device_query_contexts.empty())
    {
        LeaveCriticalSection(&_device_query_contexts_lock);
        return;
    }

    for (std::vector<QueryDeviceContext*>::iterator it = _device_query_contexts.begin(); it != _device_query_contexts.end(); it++)
    {
        QueryDeviceContext* context = *it;
        if (context)
        {
            DestroyQueryContext(context);
            delete context;
            context = nullptr;
        }
    }

    _device_query_contexts.clear();

    LeaveCriticalSection(&_device_query_contexts_lock);
}

int iOSDeviceQuerier::QueryDeviceInternal(QueryDeviceContext* context)
{
    int result = IOS_ERROR_SUCCESS;

    do
    {
        iOSDeviceInfo device_info;
        int retry_count = 0;
        while (retry_count < IOS_DEVICE_QUERY_RETRY_MAX)
        {
            if (context->device_query_shutdown)
            {
                _device_command_info->Interrupt();
                break;
            }

            result = WaitForSingleObject(context->device_query_event, IOS_DEVICE_QUERY_TIMEOUT);
            if (result == WAIT_OBJECT_0)
                break;

            if (context->device_query_shutdown)
            {
                _device_command_info->Interrupt();
                break;
            }

            device_info.erase(device_info.begin(), device_info.end());
            result = _device_command_info->GetDeviceInfo(context->device_id, &device_info);
            if (result == IOS_ERROR_SUCCESS && !device_info.empty())
                break;

            if (context->device_query_shutdown)
            {
                _device_command_info->Interrupt();
                break;
            }

            retry_count++;
        }

        if (context->device_query_shutdown)
        {
            _device_command_info->Interrupt();
            break;
        }

        if (result != IOS_ERROR_SUCCESS)
        {
            if (result == IOS_ERROR_DEVICE_NEED_REPLUG || result == IOS_ERROR_DEVICE_NEED_PAIR)
            {
                if (_device_callbacks)
                {
                    _device_callbacks->callback_error(_device_context, context->device_id, result);
                }
            }
            break;
        }

        if (_device_callbacks)
        {
            std::string device_name = device_info["DeviceName"];
            std::string serial_number = device_info["SerialNumber"];
            std::string product_name = device_info["ProductName"];
            std::string product_type = device_info["ProductType"];
            std::string product_version = device_info["ProductVersion"];
            std::string phone_number = device_info["PhoneNumber"];
            _device_callbacks->callback_query(_device_context, context->device_id,
                device_name.c_str(), serial_number.c_str(), product_name.c_str(), product_type.c_str(),
                product_version.c_str(), phone_number.c_str());
        }
    } while (false);

    return result;
}

int iOSDeviceQuerier::GetDeviceInfo(QueryDeviceContext* context, std::map<std::string, std::string>* device_info)
{
    int result = IOS_ERROR_SUCCESS;
    char* ideviceinfo_path = nullptr;
    char* ideviceinfo_buffer = nullptr;
    int ideviceinfo_buffer_size = 1024 * 1024;

    do
    {
        if (context->device_query_shutdown)
            break;

        ideviceinfo_path = (char*)SmartMemAlloc(MAX_PATH * sizeof(char));
        if (ideviceinfo_path == nullptr)
        {
            result = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if (context->device_query_shutdown)
            break;

        if (!SmartFsGetAppPathA(ideviceinfo_path, "usbmuxd\\ideviceinfo.exe"))
        {
            result = ERROR_FILE_NOT_FOUND;
            break;
        }

        if (context->device_query_shutdown)
            break;

        std::string ideviceinfo_path_string;
        ideviceinfo_path_string.append(ideviceinfo_path);
        ideviceinfo_path_string.append((" -u "));
        ideviceinfo_path_string.append(context->device_id);

        ideviceinfo_buffer = (char*)SmartMemAlloc(ideviceinfo_buffer_size);
        if (ideviceinfo_buffer == nullptr)
        {
            result = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if (context->device_query_shutdown)
            break;

        int retry_count = 0;
        while (true)
        {
            if (context->device_query_shutdown)
                break;

            if (retry_count >= IOS_DEVICE_GETINFO_RETRY_MAX)
            {
                result = ERROR_RETRY;
                break;
            }

            if (SmartProcessManager::IsProcessRunningA("ideviceinfo.exe"))
            {
                if (context->device_query_shutdown)
                    break;

                result = SmartProcessManager::TerminateProcessByNameA("ideviceinfo.exe");
                if (result != IOS_ERROR_SUCCESS)
                    continue;

                if (context->device_query_shutdown)
                    break;

                Sleep(500);

                if (context->device_query_shutdown)
                    break;
            }

            if (context->device_query_shutdown)
                break;

            if (ReadProcessBufferA(ideviceinfo_path_string.c_str(), true, ideviceinfo_buffer, &ideviceinfo_buffer_size))
            {
                result = IOS_ERROR_SUCCESS;
                break;
            }

            if (context->device_query_shutdown)
                break;

            retry_count++;
        }

        if (context->device_query_shutdown)
            break;

        if (result != IOS_ERROR_SUCCESS)
            break;

        std::vector<std::string> lines;
        boost::split(lines, ideviceinfo_buffer, boost::is_any_of("\r\n"), boost::token_compress_on);
        if (lines.size() <= 1)
        {
            result = ERROR_APP_DATA_NOT_FOUND;
            break;
        }

        if (context->device_query_shutdown)
            break;

        std::vector<std::string> line_key_value;
        for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); it++)
        {
            if (context->device_query_shutdown)
                break;

            std::string line = *it;
            if (line.empty())
                continue;

            boost::split(line_key_value, line, boost::is_any_of(":"), boost::token_compress_on);
            if (line_key_value.size() == 0)
                break;

            std::string key = line_key_value.at(0);
            if (StrCmpIA(key.c_str(), "ERROR") == 0)
                continue;

            std::string value = line_key_value.at(1);
            boost::trim(key);
            boost::trim(value);

            device_info->insert(std::pair<std::string, std::string>(key, value));
        }
    } while (false);

    if (ideviceinfo_buffer)
    {
        SmartMemFree(ideviceinfo_buffer);
        ideviceinfo_buffer = nullptr;
    }

    if (ideviceinfo_path)
    {
        SmartMemFree(ideviceinfo_path);
        ideviceinfo_path = nullptr;
    }

    return result;
}

void iOSDeviceQuerier::OnThreadHandle(void* parameter)
{
    int result = IOS_ERROR_SUCCESS;
    QueryDeviceContext* context = nullptr;

    do
    {
        context = (QueryDeviceContext*)parameter;
        if (context == nullptr)
        {
            result = ERROR_INVALID_PARAMETER;
            break;
        }

        result = QueryDeviceInternal(context);

    } while (false);

    if (context)
    {
        if (context->device_query_shutdown_event)
        {
            SetEvent(context->device_query_shutdown_event);
        }
    }
}