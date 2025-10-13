#include <shlwapi.h>
#include <smart_base.h>
#include <plist/plist.h>
#include <mimalloc.h>
#include "device/ios_device_activator.h"

iOSDeviceActivator::iOSDeviceActivator(SmartThreadPool* thread_pool, void* device_context, iOSDeviceCallbacks* device_callbacks)
{

}

iOSDeviceActivator::~iOSDeviceActivator()
{

}

int iOSDeviceActivator::Init()
{
	int result = ERROR_SUCCESS;

	return result;
}

void iOSDeviceActivator::Dispose()
{
	delete this;
}

int iOSDeviceActivator::ActivateDevice(const char* device_id, bool skip_install_setup)
{
	int result = ERROR_SUCCESS;
    idevice_t device = nullptr;
    lockdownd_client_t lockdown = nullptr;
    lockdownd_service_descriptor_t svc = nullptr;
    mobileactivation_client_t mobileactivation_client = nullptr;
    idevice_activation_request_t request = nullptr;
    idevice_activation_response_t response = nullptr;
    plist_t fields = nullptr;
    plist_dict_iter iter = nullptr;
    plist_t record = nullptr;
    char* signing_service_url = nullptr;
    const char* response_title = nullptr;
    const char* response_description = nullptr;
    char* field_key = nullptr;
    char* field_label = nullptr;
    char input[1024];
    int session_mode = 0;
    int interactive = skip_install_setup ? 0 : 1;

    do
    {
        result = idevice_new_with_options(&device, device_id, IDEVICE_LOOKUP_USBMUX);
        if (result != IDEVICE_E_SUCCESS)
            break;

        result = lockdownd_client_new_with_handshake(device, &lockdown, "ideviceactivation");
        if (result != LOCKDOWN_E_SUCCESS)
            break;

        uint32_t product_version = GetProductVersion(lockdown);
        if (product_version >= 0x0A0200) 
        {
            char* state_string = nullptr;
            result = GetActivationState(lockdown, &state_string);
            if (state_string)
            {
                if (lstrcmpA(state_string, "Unactivated") != 0)
                {
                    mi_free(state_string);
                    state_string = nullptr;
                    break;
                }
                else
                {
                    mi_free(state_string);
                    state_string = nullptr;
                }
            }
        }

        bool use_mobileactivation = GetMobileActivationClient(device, lockdown, &mobileactivation_client) == ERROR_SUCCESS;
        if (use_mobileactivation) 
        {
            plist_t ainfo = nullptr;
            if ((product_version >= 0x0A0000) 
                || (mobileactivation_create_activation_info(mobileactivation_client, &ainfo) != MOBILEACTIVATION_E_SUCCESS)) 
            {
                session_mode = 1;
            }

            mobileactivation_client_free(mobileactivation_client);
            mobileactivation_client = nullptr;
            if (session_mode) 
            {
                plist_t blob = nullptr;
                result = mobileactivation_client_start_service(device, &mobileactivation_client, "ideviceactivation");
                if (result != MOBILEACTIVATION_E_SUCCESS)
                {
                    SmartLogError("Failed to connect to %s", MOBILEACTIVATION_SERVICE_NAME);
                    break;
                }

                result = mobileactivation_create_activation_session_info(mobileactivation_client, &blob);
                if (result != MOBILEACTIVATION_E_SUCCESS)
                {
                    SmartLogError("Failed to get ActivationSessionInfo from mobileactivation");
                    break;
                }

                if (mobileactivation_client)
                {
                    mobileactivation_client_free(mobileactivation_client);
                    mobileactivation_client = nullptr;
                }
                
                result = idevice_activation_drm_handshake_request_new(IDEVICE_ACTIVATION_CLIENT_MOBILE_ACTIVATION, &request);
                if (result != IDEVICE_ACTIVATION_E_SUCCESS) {
                    SmartLogError("Failed to create drmHandshake request");
                    break;
                }

                idevice_activation_request_set_fields(request, blob);
                if (blob)
                {
                    plist_free(blob);
                    blob = nullptr;
                }

                result = idevice_activation_send_request(request, &response);
                if (result != IDEVICE_ACTIVATION_E_SUCCESS)
                    break;

                plist_t handshake_response = nullptr;
                idevice_activation_response_get_fields(response, &handshake_response);
                if (response)
                {
                    idevice_activation_response_free(response);
                    response = nullptr;
                }
                
                result = mobileactivation_client_start_service(device, &mobileactivation_client, "ideviceactivation");
                if (result != MOBILEACTIVATION_E_SUCCESS) 
                {
                    SmartLogError("Failed to connect to %s", MOBILEACTIVATION_SERVICE_NAME);
                    break;
                }

                result = mobileactivation_create_activation_info_with_session(mobileactivation_client, handshake_response, &ainfo);
                if (result != MOBILEACTIVATION_E_SUCCESS || !ainfo || plist_get_node_type(ainfo) != PLIST_DICT) 
                {
                    SmartLogError("Failed to get ActivationInfo from mobileactivation");
                    break;
                }

                if (mobileactivation_client)
                {
                    mobileactivation_client_free(mobileactivation_client);
                    mobileactivation_client = nullptr;
                }
            }
            else if (!ainfo || plist_get_node_type(ainfo) != PLIST_DICT) 
            {
                SmartLogError("Failed to get ActivationInfo from mobileactivation");
                break;
            }

            result = idevice_activation_request_new(IDEVICE_ACTIVATION_CLIENT_MOBILE_ACTIVATION, &request);
            if (result != IDEVICE_ACTIVATION_E_SUCCESS) 
            {
                SmartLogError("Failed to create activation request");
                break;
            }

            plist_t request_fields = plist_new_dict();
            plist_dict_set_item(request_fields, "activation-info", ainfo);
            idevice_activation_request_set_fields(request, request_fields);
        }
        else 
        {
            result = idevice_activation_request_new_from_lockdownd(IDEVICE_ACTIVATION_CLIENT_MOBILE_ACTIVATION, lockdown, &request);
            if (result != IDEVICE_ACTIVATION_E_SUCCESS) 
            {
                SmartLogError("Failed to create activation request");
                break;
            }
        }

        if (lockdown)
        {
            lockdownd_client_free(lockdown);
            lockdown = nullptr;
        }
        

        if (request && signing_service_url) 
        {
            idevice_activation_request_set_url(request, signing_service_url);
        }

        while (true) 
        {
            result = idevice_activation_send_request(request, &response);
            if (result != IDEVICE_ACTIVATION_E_SUCCESS) {
                SmartLogError("Failed to send request or retrieve response");
                break;
            }

            result = idevice_activation_response_has_errors(response);
            if (result)
            {
                SmartLogError("Activation server reports errors");

                idevice_activation_response_get_title(response, &response_title);
                if (response_title) 
                {
                    SmartLogError("\t%s\n", response_title);
                }

                idevice_activation_response_get_description(response, &response_description);
                if (response_description)
                {
                    SmartLogError("\t%s\n", response_description);
                }
                break;
            }

            idevice_activation_response_get_activation_record(response, &record);
            if (record) 
            {
                result = lockdownd_client_new_with_handshake(device, &lockdown, "ideviceactivation");
                if (result != LOCKDOWN_E_SUCCESS)
                {
                    SmartLogError("Failed to connect to lockdownd");
                    break;
                }

                if (use_mobileactivation) 
                {
                    result = lockdownd_start_service(lockdown, MOBILEACTIVATION_SERVICE_NAME, &svc);
                    if (result != LOCKDOWN_E_SUCCESS)
                    {
                        SmartLogError("Failed to start service %s", MOBILEACTIVATION_SERVICE_NAME);
                        break;
                    }

                    result = mobileactivation_client_new(device, svc, &mobileactivation_client);
                    if (svc)
                    {
                        lockdownd_service_descriptor_free(svc);
                        svc = nullptr;
                    }
                    
                    if (result != MOBILEACTIVATION_E_SUCCESS)
                    {
                        SmartLogError("Failed to connect to %s", MOBILEACTIVATION_SERVICE_NAME);
                        break;
                    }

                    if (session_mode)
                    {
                        plist_t headers = nullptr;
                        idevice_activation_response_get_headers(response, &headers);
                        result = mobileactivation_activate_with_session(mobileactivation_client, record, headers);
                        if (result != MOBILEACTIVATION_E_SUCCESS)
                        {
                            if (headers)
                            {
                                plist_free(headers);
                                headers = nullptr;
                            }
                            SmartLogError("Failed to activate device with record");
                            break;
                        }

                        if (headers)
                        {
                            plist_free(headers);
                            headers = nullptr;
                        }
                    }
                    else 
                    {
                        result = mobileactivation_activate(mobileactivation_client, record);
                        if (result != MOBILEACTIVATION_E_SUCCESS)
                        {
                            SmartLogError("Failed to activate device with record");
                            break;
                        }
                    }
                }
                else 
                {
                    result = lockdownd_activate(lockdown, record);
                    if (result != LOCKDOWN_E_SUCCESS) 
                    {
                        char* state_string = nullptr;
                        result = GetActivationState(lockdown, &state_string);
                        if (result != ERROR_SUCCESS)
                            break;

                        bool success = false;
                        if (state_string)
                        {
                            success = lstrcmpA(state_string, "Unactivated");
                            mi_free(state_string);
                            state_string = nullptr;
                        }
       
                        if (!success) 
                        {
                            SmartLogError("Failed to activate device with record");
                            result = ERROR_ACTIVE_CONNECTIONS;
                            break;
                        }
                    }
                }

                result = lockdownd_set_value(lockdown, nullptr, "ActivationStateAcknowledged", plist_new_bool(1));
                if (result != LOCKDOWN_E_SUCCESS) 
                {
                    SmartLogError("Failed to set ActivationStateAcknowledged on device");
                    break;
                }
                break;
            }
            else 
            {
                if (idevice_activation_response_is_activation_acknowledged(response)) 
                {
                    SmartLogInfo("Activation server reports that device is already activated");
                    result = ERROR_SUCCESS;
                    break;
                }

                idevice_activation_response_get_title(response, &response_title);
                if (response_title) 
                {
                    SmartLogError("Server reports:\n%s", response_title);
                }

                idevice_activation_response_get_description(response, &response_description);
                if (response_description) 
                {
                    SmartLogError("Server reports:\n%s", response_description);
                }

                idevice_activation_response_get_fields(response, &fields);
                if (!fields || plist_dict_get_size(fields) == 0) 
                {
                    SmartLogError("Unknown error");
                    result = EXIT_FAILURE;
                    break;
                }

                plist_dict_new_iter(fields, &iter);
                if (!iter) 
                {
                    SmartLogError("Unknown error");
                    result = EXIT_FAILURE;
                    break;
                }

                if (request)
                {
                    idevice_activation_request_free(request);
                    request = nullptr;
                }
               
                result = idevice_activation_request_new(IDEVICE_ACTIVATION_CLIENT_MOBILE_ACTIVATION, &request);
                if (result != IDEVICE_ACTIVATION_E_SUCCESS) 
                {
                    SmartLogError("Could not create new request");
                    break;
                }

                idevice_activation_request_set_fields_from_response(request, response);

                int interactive_count = 0;
                do 
                {
                    field_key = nullptr;
                    plist_dict_next_item(fields, iter, &field_key, nullptr);
                    if (field_key) 
                    {
                        if (idevice_activation_response_field_requires_input(response, field_key)) 
                        {
                            idevice_activation_response_get_label(response, field_key, &field_label);
                            if (interactive) 
                            {
                                char* field_placeholder = nullptr;
                                int secure = idevice_activation_response_field_secure_input(response, field_key);
                                idevice_activation_response_get_placeholder(response, field_key, &field_placeholder);
                                SmartLogInfo("input %s", field_label ? field_label : field_key);
                                if (field_placeholder) 
                                {
                                    SmartLogInfo(" (%s)", field_placeholder);
                                    mi_free(field_placeholder);
                                    field_placeholder = nullptr;
                                }
                                printf(": ");
                                //get_user_input(input, 1023, secure);
                            }
                            else 
                            {
                                SmartLogError("Server requires input for '%s' but we're not running interactively", field_label ? field_label : field_key);
                                strcpy(input, "");
                                interactive_count++;
                            }

                            idevice_activation_request_set_field(request, field_key, input);

                            if (field_label) 
                            {
                                mi_free(field_label);
                                field_label = nullptr;
                            }
                        }
                    }
                } while (field_key);

                if (iter)
                {
                    mi_free(iter);
                    iter = nullptr;
                }
               
                if (response)
                {
                    idevice_activation_response_free(response);
                    response = nullptr;
                }
               
                if (interactive_count > 0 && !interactive) 
                {
                    SmartLogError("Failed to activate device");
                    result = EXIT_FAILURE;
                    break;
                }
            }
        }
    } while (false);

    if (request)
    {
        idevice_activation_request_free(request);
        request = nullptr;
    }

    if (response)
    {
        idevice_activation_response_free(response);
        response = nullptr;
    }

    if (fields)
    {
        plist_free(fields);
        fields = nullptr;
    }

    if (field_label)
    {
        mi_free(field_label);
        field_label = nullptr;
    }

    if (iter)
    {
        mi_free(iter);
        iter = nullptr;
    }

    if (record)
    {
        plist_free(record);
        record = nullptr;
    }

    if (mobileactivation_client)
    {
        mobileactivation_client_free(mobileactivation_client);
        mobileactivation_client = nullptr;
    }

    if (lockdown)
    {
        lockdownd_client_free(lockdown);
        lockdown = nullptr;
    }

    if (device)
    {
        idevice_free(device);
        device = nullptr;
    }

	return result;
}

int iOSDeviceActivator::ActivateDeviceEx(const char* device_id, bool skip_install_setup)
{
    int result = IOS_ERROR_SUCCESS;
    SOCKET sock = INVALID_SOCKET;

    do
    {
        result = OpenAppleMobileServiceConnection(&sock);
        if (result != IOS_ERROR_SUCCESS)
            break;

        while (true)
        {
            result = SendAppleMobileServiceMessage(sock);
            if (result != IOS_ERROR_SUCCESS)
                break;

            result = RecvAppleMobileServiceMessage(sock);
            if (result != IOS_ERROR_SUCCESS)
                break;
        }
    } while (false);

    if (sock != INVALID_SOCKET)
    {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }

    return result;
}

int iOSDeviceActivator::ActivateDeviceByCommand(const char* device_id, bool skip_install_setup)
{
    int result = IOS_ERROR_SUCCESS;
    char* application_name = nullptr;

    do
    {
        application_name = (char*)SmartMemAlloc(MAX_PATH);
        if (application_name == nullptr)
        {
            result = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if (!SmartFsGetAppPathA(application_name, "activation.exe"))
        {
            result = ERROR_FILE_NOT_FOUND;
            break;
        }

        std::string command_line;
        command_line.append(application_name);
        command_line.append(" ");
        command_line.append("activate");
        command_line.append(" ");
        command_line.append(" -u ");
        command_line.append(device_id);
        if (skip_install_setup)
        {
            command_line.append(" -b");
        }

        PROCESS_INFORMATION pi = { 0 };
        RtlZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
        result = SmartProcessManager::CreateUserProcessA(command_line.c_str(), &pi);
        if (result != ERROR_SUCCESS)
            break;

        WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

    } while (false);

    if (application_name)
    {
        SmartMemFree(application_name);
        application_name = nullptr;
    }

    return result;
}

int iOSDeviceActivator::DeactivateDevice(const char* device_id)
{
    int result = ERROR_SUCCESS;
    idevice_t device = nullptr;
    lockdownd_client_t lockdown = nullptr;
    mobileactivation_client_t mobileactivation_client = nullptr;

    do
    {
        result = idevice_new_with_options(&device, device_id, IDEVICE_LOOKUP_USBMUX);
        if (result != IDEVICE_E_SUCCESS)
            break;

        result = lockdownd_client_new_with_handshake(device, &lockdown, "ideviceactivation");
        if (result != LOCKDOWN_E_SUCCESS)
            break;

        result = GetMobileActivationClient(device, lockdown, &mobileactivation_client);
        if (result == ERROR_SUCCESS)
        {
            result = mobileactivation_deactivate(mobileactivation_client);
            if (result != MOBILEACTIVATION_E_SUCCESS)
                break;
        }
        else 
        {
            result = lockdownd_deactivate(lockdown);
            if (result != LOCKDOWN_E_SUCCESS)
                break;
        }
    } while (false);

    if (mobileactivation_client)
    {
        mobileactivation_client_free(mobileactivation_client);
        mobileactivation_client = nullptr;
    }

    if (lockdown)
    {
        lockdownd_client_free(lockdown);
        lockdown = nullptr;
    }

    if (device)
    {
        idevice_free(device);
        device = nullptr;
    }

    return result;
}

int iOSDeviceActivator::QueryDeviceState(const char* device_id, bool* activated)
{
    int result = ERROR_SUCCESS;
    idevice_t device = nullptr;
    lockdownd_client_t lockdown = nullptr;
    mobileactivation_client_t mobileactivation_client = nullptr;
    plist_t state = nullptr;
    char* state_string = nullptr;

    do
    {
        result = idevice_new_with_options(&device, device_id, IDEVICE_LOOKUP_USBMUX);
        if (result != IDEVICE_E_SUCCESS)
            break;

        result = lockdownd_client_new_with_handshake(device, &lockdown, "ideviceactivation");
        if (result != LOCKDOWN_E_SUCCESS)
            break;

        result = GetMobileActivationClient(device, lockdown, &mobileactivation_client);
        if (result == ERROR_SUCCESS)
        {
            mobileactivation_get_activation_state(mobileactivation_client, &state);
        }
        else 
        {
            lockdownd_get_value(lockdown, nullptr, "ActivationState", &state);
        }

        if (plist_get_node_type(state) != PLIST_STRING)
        {
            result = ERROR_DATATYPE_MISMATCH;
            break;
        }

        plist_get_string_val(state, &state_string);
        if (state_string && strcmpi(state_string, "Activated") == 0)
        {
            if (activated)
            {
                *activated = true;
            }
        }
        else
        {
            if (activated)
            {
                *activated = false;
            }
        }
    } while (false);

    if (state_string)
    {
        mi_free(state_string);
        state_string = nullptr;
    }

    if (mobileactivation_client)
    {
        mobileactivation_client_free(mobileactivation_client);
        mobileactivation_client = nullptr;
    }

    if (lockdown)
    {
        lockdownd_client_free(lockdown);
        lockdown = nullptr;
    }

    if (device)
    {
        idevice_free(device);
        device = nullptr;
    }

    return result;
}

int iOSDeviceActivator::OpenAppleMobileServiceConnection(SOCKET* sock)
{
    int result = ERROR_SUCCESS;
    SOCKET s = INVALID_SOCKET;

    do
    {
        s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s != INVALID_SOCKET)
        {
            result = WSAGetLastError();
            break;
        }

        BOOL reuseaddr = TRUE;
        result = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseaddr, sizeof(reuseaddr));
        if (result == SOCKET_ERROR)
        {
            result = WSAGetLastError();
            break;
        }

        sockaddr_in service;
        service.sin_family = AF_INET;
        service.sin_addr.s_addr = inet_addr("127.0.0.1");
        service.sin_port = htons(27015);

        result = connect(s, (const sockaddr*)&service, sizeof(service));
        if (result == SOCKET_ERROR)
        {
            result = WSAGetLastError();
            break;
        }

        if (sock)
        {
            *sock = s;
        }

    } while (false);

    if (result != ERROR_SUCCESS)
    {
        if (s != INVALID_SOCKET)
        {
            closesocket(s);
            s = INVALID_SOCKET;
        }
    }

    return result;
}

int iOSDeviceActivator::SendAppleMobileServiceMessage(SOCKET& sock)
{
    int result = ERROR_SUCCESS;
    char* buffer = nullptr;
    plist_t dict = nullptr;

    do
    {
        dict = plist_new_dict();
        if (dict == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }

        plist_t bundle_id = plist_new_string("com.apple.iTunes");
        if (bundle_id == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(dict, "BundleID", bundle_id);

        plist_t usbmuxd = plist_new_string("usbmuxd-???");
        if (usbmuxd == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(dict, "ClientVersionString", usbmuxd);

        plist_t conn_type = plist_new_uint(1);
        if (conn_type == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(dict, "ConnType", conn_type);

        plist_t listen = plist_new_string("Listen");
        if (listen == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(dict, "MessageType", listen);

        plist_t usbmux_version = plist_new_uint(3);
        if (usbmux_version == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(dict, "kLibUSBMuxVersion", usbmux_version);

     
        uint32_t len = 0;
        plist_to_xml(dict, &buffer, &len);
        if (buffer == nullptr || len == 0)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }

        char buf[16] = { 0 };

        DWORD v = 16;
        memcpy(buf, &v, sizeof(v));

        v = len + 16;
        memcpy(buf + sizeof(v), &v, sizeof(v));
        int ret = send(sock, buf, 16, 0);
        if (ret != 16)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }

        ret = send(sock, buffer, len, 0);
        if (ret != v)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }

    } while (false);

    if (dict)
    {
        plist_free(dict);
        dict = nullptr;
    }

    if (buffer)
    {
        mi_free(buffer);
        buffer = nullptr;
    }

    return result;
}

int iOSDeviceActivator::RecvAppleMobileServiceMessage(SOCKET& sock)
{
    int result = ERROR_SUCCESS;
    return result;
}

int iOSDeviceActivator::GetMobileActivationClient(idevice_t device, lockdownd_client_t lockdown, mobileactivation_client_t* mobileactivation_client)
{
    int result = ERROR_SUCCESS;
    lockdownd_service_descriptor_t svc = nullptr;

    do
    {
        result = lockdownd_start_service(lockdown, MOBILEACTIVATION_SERVICE_NAME, &svc);
        if (result != LOCKDOWN_E_SUCCESS)
            break;

        result = mobileactivation_client_new(device, svc, mobileactivation_client);
        if (result != MOBILEACTIVATION_E_SUCCESS)
            break;

    } while (false);
 
    if (svc)
    {
        lockdownd_service_descriptor_free(svc);
        svc = nullptr;
    }

    return result;
}

uint32_t iOSDeviceActivator::GetProductVersion(lockdownd_client_t lockdown)
{
    uint32_t result = 0;
    plist_t product_version_value = nullptr;
    char* product_version_string = nullptr;

    do
    {
        if (lockdownd_get_value(lockdown, nullptr, "ProductVersion", &product_version_value) != LOCKDOWN_E_SUCCESS)
            break;

        int product_version[3] = { 0, 0, 0 };
        plist_get_string_val(product_version_value, &product_version_string);
        if (product_version_string == nullptr)
            break;

        if (sscanf(product_version_string, "%d.%d.%d", &product_version[0], &product_version[1], &product_version[2]) >= 2)
        {
            result = ((product_version[0] & 0xFF) << 16) | ((product_version[1] & 0xFF) << 8) | (product_version[2] & 0xFF);
        }

    } while (false);
    
    if (product_version_string)
    {
        mi_free(product_version_string);
        product_version_string = nullptr;
    }

    if (product_version_value)
    {
        plist_free(product_version_value);
        product_version_value = nullptr;
    }

    return result;
}

int iOSDeviceActivator::GetActivationState(lockdownd_client_t lockdown, char** activation_state_string)
{
    int result = IOS_ERROR_SUCCESS;
    plist_t state_value = nullptr;

    do
    {
        lockdownd_get_value(lockdown, nullptr, "ActivationState", &state_value);
        if (state_value == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }

        char* state_string = nullptr;
        plist_get_string_val(state_value, &state_string);
        if (state_string == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }

        if (activation_state_string)
        {
            *activation_state_string = state_string;
        }

    } while (false);

    if (state_value)
    {
        plist_free(state_value);
        state_value = nullptr;
    }

    return result;
}

int iOSDeviceActivator::StartOrStopAppleMobileDeviceService(bool start)
{
    int result = IOS_ERROR_SUCCESS;
    char* application_name = nullptr;
    char* command_line = nullptr;
    HANDLE process_read_pipe = nullptr;
    HANDLE process_write_pipe = nullptr;

    do
    {
        application_name = (char*)SmartMemAlloc(MAX_PATH);
        if (application_name == nullptr)
        {
            result = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if (GetSystemDirectoryA(application_name, MAX_PATH) <= 0)
        {
            result = ERROR_PATH_NOT_FOUND;
            break;
        }

        if (PathCombineA(application_name, application_name, "sc.exe") == nullptr)
        {
            result = ERROR_FILE_NOT_FOUND;
            break;
        }

        command_line = (char*)SmartMemAlloc(MAX_PATH);
        if (command_line == nullptr)
        {
            result = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if (start)
        {
            lstrcpyA(command_line, " start \"Apple Mobile Device Service\"");
        }
        else
        {
            lstrcpyA(command_line, " stop \"Apple Mobile Device Service\"");
        }

        SECURITY_ATTRIBUTES pipe_attributes;
        RtlZeroMemory(&pipe_attributes, sizeof(SECURITY_ATTRIBUTES));
        pipe_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        pipe_attributes.lpSecurityDescriptor = nullptr;
        pipe_attributes.bInheritHandle = TRUE;
        if (!CreatePipe(&process_read_pipe, &process_write_pipe, &pipe_attributes, 0))
        {
            result = GetLastError();
            break;
        }

        STARTUPINFOA si = { 0 };
        RtlZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
        si.wShowWindow = SW_HIDE;
        si.hStdOutput = process_write_pipe;
        si.hStdError = process_write_pipe;

        PROCESS_INFORMATION pi = { 0 };
        RtlZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
        if (!CreateProcessA(application_name,
            command_line,
            &pipe_attributes,
            &pipe_attributes,
            TRUE,
            0x8000020,
            nullptr,
            nullptr,
            &si,
            &pi))
        {
            result = GetLastError();
            break;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

    } while (false);

    if (process_read_pipe)
    {
        CloseHandle(process_read_pipe);
        process_read_pipe = nullptr;
    }

    if (process_write_pipe)
    {
        CloseHandle(process_write_pipe);
        process_write_pipe = nullptr;
    }

    if (command_line)
    {
        SmartMemFree(command_line);
        command_line = nullptr;
    }

    if (application_name)
    {
        SmartMemFree(application_name);
        application_name = nullptr;
    }

    return result;
}