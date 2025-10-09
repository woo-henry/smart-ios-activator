#include <time.h>
#include <smart_base.h>
#include <smart_ios.h>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <mimalloc.h>
#include "device/ios_device_command_info.h"

#ifndef IOS_DEVICE_MAC_EPOCH
#define IOS_DEVICE_MAC_EPOCH        978307200
#endif // !IOS_DEVICE_MAC_EPOCH

#ifndef IOS_DEVICE_BASE64_STRING
#define IOS_DEVICE_BASE64_STRING    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
#endif // !IOS_DEVICE_BASE64_STRING

#ifndef IOS_DEVICE_BASE64_PAD
#define IOS_DEVICE_BASE64_PAD       '='
#endif // !IOS_DEVICE_BASE64_PAD

iOSDeviceCommandInfo::iOSDeviceCommandInfo()
    : interrupt(false)
{

}

iOSDeviceCommandInfo::~iOSDeviceCommandInfo()
{
    interrupt = true;
}

int iOSDeviceCommandInfo::Init()
{
    int result = IOS_ERROR_SUCCESS;

    return result;
}

void iOSDeviceCommandInfo::Dispose()
{
    delete this;
}

void iOSDeviceCommandInfo::Interrupt()
{
    interrupt = true;
}

int iOSDeviceCommandInfo::GetDeviceInfo(const char* device_id, std::map<std::string, std::string>* device_info)
{
    int result = IOS_ERROR_SUCCESS;
    idevice_t device = nullptr;
    lockdownd_client_t client = nullptr;
    const char* domain = nullptr;
    const char* key = nullptr;
    plist_t node = nullptr;

    do
    {
        result = idevice_new_with_options(&device, device_id, IDEVICE_LOOKUP_USBMUX);
        if (result != IDEVICE_E_SUCCESS)
        {
            if (result == IDEVICE_E_NO_DEVICE)
            {
                result = IOS_ERROR_DEVICE_NEED_REPLUG;
            }
            break;
        }

        if (interrupt)
            break;

        result = lockdownd_client_new_with_handshake(device, &client, "ideviceinfo");
        if (result != LOCKDOWN_E_SUCCESS)
        {
            if (result == LOCKDOWN_E_INVALID_HOST_ID)
            {
                result = IOS_ERROR_DEVICE_NEED_PAIR;
            }
            break;
        }

        if (interrupt)
            break;

        result = lockdownd_get_value(client, domain, key, &node);
        if (result != LOCKDOWN_E_SUCCESS)
        {
            if (result == LOCKDOWN_E_INVALID_HOST_ID)
            {
                result = IOS_ERROR_DEVICE_NEED_PAIR;
            }
            result = IOS_ERROR_DEVICE_NEED_PAIR;
            break;
        }

        if (interrupt)
            break;

        if (node == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }

        if (interrupt)
            break;

        result = GetDeviceInfoInternal(node, device_info);
   
    } while (false);

    if (client)
    {
        lockdownd_client_free(client);
        client = nullptr;
    }

    if (device)
    {
        idevice_free(device);
        device = nullptr;
    }

    return result;
}

int iOSDeviceCommandInfo::GetDeviceInfoInternal(plist_t node, std::map<std::string, std::string>* device_info)
{
    int result = IOS_ERROR_SUCCESS;
    int indent = 0;

    do
    {
        if (interrupt)
            break;

        plist_type type = plist_get_node_type(node);
        switch (type)
        {
        case PLIST_DICT:
            result = GetDeviceDictInfo(node, &indent, device_info);
            break;
        case PLIST_ARRAY:
            result = GetDeviceArrayInfo(node, &indent, device_info);
            break;
        default:
            result = GetDeviceNodeInfo(node, &indent, device_info);
        }
    } while (false);

    return result;
}

int iOSDeviceCommandInfo::GetDeviceDictInfo(plist_t node, int* indent_level, std::map<std::string, std::string>* device_info)
{
    int result = IOS_ERROR_SUCCESS;

    plist_dict_iter it = nullptr;
    plist_dict_new_iter(node, &it);

    char* key = nullptr;
    plist_t subnode = nullptr;
    plist_dict_next_item(node, it, &key, &subnode);

    while (subnode)
    {
        if (interrupt)
            break;

        result = GetDeviceNodeInfo(subnode, indent_level, key, device_info);
        if (result != ERROR_SUCCESS)
            break;

        if (key)
        {
            mi_free(key);
            key = nullptr;
        }
        
        plist_dict_next_item(node, it, &key, &subnode);
    }

    if (it)
    {
        mi_free(it);
        it = nullptr;
    }

    return result;
}

int iOSDeviceCommandInfo::GetDeviceArrayInfo(plist_t node, int* indent_level, std::map<std::string, std::string>* device_info)
{
    int result = IOS_ERROR_SUCCESS;

    int count = plist_array_get_size(node);
    for (int i = 0; i < count; i++) 
    {
        if (interrupt)
            break;

        plist_t subnode = plist_array_get_item(node, i);
        result = GetDeviceNodeInfo(subnode, indent_level, device_info);
        if (result != IOS_ERROR_SUCCESS)
            break;
    }

    return result;
}

int iOSDeviceCommandInfo::GetDeviceNodeInfo(plist_t node, int* indent_level, std::map<std::string, std::string>* device_info)
{
    int result = IOS_ERROR_SUCCESS;

    do
    {
        if (interrupt)
            break;

        plist_type type = plist_get_node_type(node);
        switch (type)
        {
        case PLIST_DICT:
            result = GetDeviceDictInfo(node, indent_level, device_info);
            break;
        case PLIST_ARRAY:
            result = GetDeviceArrayInfo(node, indent_level, device_info);
            break;
        default:
            result = GetDeviceNodeInfo(node, indent_level, device_info);
        }
    } while (false);

    return result;
}

int iOSDeviceCommandInfo::GetDeviceArrayInfo(plist_t node, int* indent_level, const char* key, std::map<std::string, std::string>* device_info)
{
    int result = IOS_ERROR_SUCCESS;

    int count = plist_array_get_size(node);
    for (int i = 0; i < count; i++)
    {
        if (interrupt)
            break;

        plist_t subnode = plist_array_get_item(node, i);
        result = GetDeviceNodeInfo(subnode, indent_level, key, device_info);
        if (result != IOS_ERROR_SUCCESS)
            break;
    }

    return result;
}

int iOSDeviceCommandInfo::GetDeviceNodeInfo(plist_t node, int* indent_level, const char* key, std::map<std::string, std::string>* device_info)
{
    int result = IOS_ERROR_SUCCESS;

    plist_type node_type = plist_get_node_type(node);
    switch (node_type) 
    {
    case PLIST_BOOLEAN:
        GetDeviceNodeBooleanInfo(node, key, device_info);
        break;
    case PLIST_UINT:
        GetDeviceNodeUintInfo(node, key, device_info);
        break;
    case PLIST_REAL:
        GetDeviceNodeRealInfo(node, key, device_info);
        break;
    case PLIST_STRING:
        GetDeviceNodeStringInfo(node, key, device_info);
        break;
    case PLIST_KEY:
        GetDeviceNodeKeyInfo(node, key, device_info);
        break;
    case PLIST_DATA:
        GetDeviceNodeDataInfo(node, key, device_info);
        break;
    case PLIST_DATE:
        GetDeviceNodeDateInfo(node, key, device_info);
        break;
    case PLIST_ARRAY:
        (*indent_level)++;
        result = GetDeviceArrayInfo(node, indent_level, key, device_info);
        (*indent_level)--;
        break;
    case PLIST_DICT:
        (*indent_level)++;
        result = GetDeviceDictInfo(node, indent_level, device_info);
        (*indent_level)--;
        break;
    default:
        break;
    }

    return result;
}

void iOSDeviceCommandInfo::GetDeviceNodeBooleanInfo(plist_t node, const char* key, std::map<std::string, std::string>* device_info)
{
    uint8_t value = 0;
    plist_get_bool_val(node, &value);
    device_info->insert(std::pair<std::string, std::string>(key, value ? "true" : "false"));
}

void iOSDeviceCommandInfo::GetDeviceNodeRealInfo(plist_t node, const char* key, std::map<std::string, std::string>* device_info)
{
    double value_double;
    plist_get_real_val(node, &value_double);
    device_info->insert(std::pair<std::string, std::string>(key, std::to_string(value_double)));
}

void iOSDeviceCommandInfo::GetDeviceNodeUintInfo(plist_t node, const char* key, std::map<std::string, std::string>* device_info)
{
    uint64_t value = 0;
    plist_get_uint_val(node, &value);
    device_info->insert(std::pair<std::string, std::string>(key, std::to_string(value)));
}

void iOSDeviceCommandInfo::GetDeviceNodeStringInfo(plist_t node, const char* key, std::map<std::string, std::string>* device_info)
{
    char* value = nullptr;
    plist_get_string_val(node, &value);
    if (value)
    {
        device_info->insert(std::pair<std::string, std::string>(key, value));
        mi_free(value);
        value = nullptr;
    }
}

void iOSDeviceCommandInfo::GetDeviceNodeKeyInfo(plist_t node, const char* key, std::map<std::string, std::string>* device_info)
{
    char* value = nullptr;
    plist_get_key_val(node, &value);
    if (value)
    {
        device_info->insert(std::pair<std::string, std::string>(key, value));
        mi_free(value);
        value = nullptr;
    }
}

void iOSDeviceCommandInfo::GetDeviceNodeDataInfo(plist_t node, const char* key, std::map<std::string, std::string>* device_info)
{
    char* value = nullptr;
    char* value_string = nullptr;

    do
    {
        uint64_t value_length = 0;
        plist_get_data_val(node, &value, &value_length);
        if (value_length <= 0)
            break;

        value_string = Base64Encode((unsigned char*)value, value_length);
        if (value_string == nullptr)
            break;

        device_info->insert(std::pair<std::string, std::string>(key, value_string));

    } while (false);
    
    if (value_string)
    {
        mi_free(value_string);
        value_string = nullptr;
    }

    if (value)
    {
        mi_free(value);
        value = nullptr;
    }
}

void iOSDeviceCommandInfo::GetDeviceNodeDateInfo(plist_t node, const char* key, std::map<std::string, std::string>* device_info)
{
    char* value_string = nullptr;

    do
    {
        struct timeval value_date = { 0, 0 };
        plist_get_date_val(node, (int32_t*)&value_date.tv_sec, (int32_t*)&value_date.tv_usec);
        time_t ti = (time_t)value_date.tv_sec + IOS_DEVICE_MAC_EPOCH;
        struct tm* btime = localtime(&ti);
        if (btime == nullptr)
            break;

        value_string = (char*)mi_malloc(24);
        if (value_string == nullptr)
            break;

        memset(value_string, 0, 24);

        if (strftime(value_string, 24, "%Y-%m-%dT%H:%M:%SZ", btime) <= 0)
            break;

        device_info->insert(std::pair<std::string, std::string>(key, value_string));

    } while (false);

    if (value_string)
    {
        mi_free(value_string);
        value_string = nullptr;
    }
}

char* iOSDeviceCommandInfo::Base64Encode(const unsigned char* buf, size_t size)
{
    if (!buf || !(size > 0))
        return nullptr;

    size_t length = (size / 3) * 4;
    char* result = (char*)mi_malloc(length + 5); // 4 spare bytes + 1 for '\0'
    if (result == nullptr)
        return nullptr;

    size_t n = 0;
    size_t m = 0;
    unsigned char input[3];
    unsigned int output[4];
    while (n < size) 
    {
        input[0] = buf[n];
        input[1] = (n + 1 < size) ? buf[n + 1] : 0;
        input[2] = (n + 2 < size) ? buf[n + 2] : 0;
        output[0] = input[0] >> 2;
        output[1] = ((input[0] & 3) << 4) + (input[1] >> 4);
        output[2] = ((input[1] & 15) << 2) + (input[2] >> 6);
        output[3] = input[2] & 63;
        result[m++] = IOS_DEVICE_BASE64_STRING[(int)output[0]];
        result[m++] = IOS_DEVICE_BASE64_STRING[(int)output[1]];
        result[m++] = (n + 1 < size) ? IOS_DEVICE_BASE64_STRING[(int)output[2]] : IOS_DEVICE_BASE64_PAD;
        result[m++] = (n + 2 < size) ? IOS_DEVICE_BASE64_STRING[(int)output[3]] : IOS_DEVICE_BASE64_PAD;
        n += 3;
    }

    result[m] = 0; // 0-termination!

    return result;
}