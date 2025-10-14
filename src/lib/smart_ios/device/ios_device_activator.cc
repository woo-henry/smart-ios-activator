#include <winsock2.h>
#include <ws2tcpip.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <smart_base.h>
#include <plist/plist.h>
#include <libimobiledevice/service.h>
#include <libimobiledevice/property_list_service.h>
#include <mimalloc.h>
#include <curl/curl.h>
#include "device/ios_device_activator.h"
#include <usbmuxd.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/ssl.h>

iOSDeviceActivator::iOSDeviceActivator(SmartThreadPool* thread_pool, void* device_context, iOSDeviceCallbacks* device_callbacks)
{
    std::memset(_configuration_path, 0, MAX_PATH);
}

iOSDeviceActivator::~iOSDeviceActivator()
{
    curl_global_cleanup();
}

int iOSDeviceActivator::Init()
{
	int result = ERROR_SUCCESS;

    do
    {
        if (!SHGetSpecialFolderPathA(nullptr, _configuration_path, CSIDL_COMMON_APPDATA, false))
        {
            result = ERROR_PATH_NOT_FOUND;
            break;
        }

        result = usbmuxd_set_tcp_endpoint("127.0.0.1", 27015);
        if (result != IDEVICE_E_SUCCESS)
            break;

        curl_global_init(CURL_GLOBAL_ALL);

    } while (false);

	return result;
}

void iOSDeviceActivator::Dispose()
{
	delete this;
}

int iOSDeviceActivator::ActivateDevice(const char* device_id, bool skip_install_setup, const char* wifi_ssid, const char* wifi_password)
{
	int result = ERROR_SUCCESS;
    idevice_t device = nullptr;
    lockdownd_client_t client = nullptr;
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
        SmartLogDebug("ActivateDevice[%s] 11 = %d", device_id, result);
        if (result != IDEVICE_E_SUCCESS)
            break;

        result = lockdownd_client_new_with_handshake(device, &client, "ideviceactivation");
        SmartLogDebug("ActivateDevice[%s] 22 = %d", device_id, result);
        if (result != LOCKDOWN_E_SUCCESS)
            break;

        uint32_t product_version = GetProductVersion(client);
        if (product_version >= 0x0A0200) 
        {
            char* state_string = nullptr;
            result = GetActivationState(client, &state_string);
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

        result = SyncTimeIntervalSince1970(device);
        SmartLogDebug("ActivateDevice[%s] 33 = %d", device_id, result);
        if (result != ERROR_SUCCESS)
            break;

        //SmartLogDebug("socket_connect[%s] aaa = %d", device_id, result);
        //int ret = socket_connect("127.0.0.1", 27015);
        //SmartLogDebug("socket_connect[%s] zzz = %d", device_id, ret);


        SmartLogDebug("ActivateDevice[%s] 333 = %d", device_id, result);
        bool use_mobileactivation = GetMobileActivationClient(device, client, &mobileactivation_client) == ERROR_SUCCESS;
        SmartLogDebug("ActivateDevice[%s] 334 = %d", device_id, result);

        if (use_mobileactivation) 
        {
            plist_t ainfo = nullptr;
            if ((product_version >= 0x0A0000) 
                || (mobileactivation_create_activation_info(mobileactivation_client, &ainfo) != MOBILEACTIVATION_E_SUCCESS)) 
            {
                session_mode = 1;
            }

            SmartLogDebug("ActivateDevice[%s] 44-1 = %d", device_id, session_mode);
            if (mobileactivation_client)
            {
                mobileactivation_client_free(mobileactivation_client);
                mobileactivation_client = nullptr;
            }
            SmartLogDebug("ActivateDevice[%s] 44-2 = %d", device_id, session_mode);

            if (session_mode) 
            {
                SmartLogDebug("ActivateDevice[%s] 55-1 = %d", device_id, result);
                plist_t blob = nullptr;
                result = mobileactivation_client_start_service(device, &mobileactivation_client, "ideviceactivation1");
                SmartLogDebug("ActivateDevice[%s] 55-2 = %d", device_id, result);
                if (result != MOBILEACTIVATION_E_SUCCESS)
                {
                    SmartLogError("Failed to connect to %s", MOBILEACTIVATION_SERVICE_NAME);
                    break;
                }

                result = mobileactivation_create_activation_session_info(mobileactivation_client, &blob);
                SmartLogDebug("ActivateDevice[%s] 66 = %d", device_id, result);
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
                SmartLogDebug("ActivateDevice[%s] 77 = %d", device_id, result);
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
                SmartLogDebug("ActivateDevice[%s] 88 = %d", device_id, result);
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
                SmartLogDebug("ActivateDevice[%s] 99 = %d", device_id, result);
                if (result != MOBILEACTIVATION_E_SUCCESS) 
                {
                    SmartLogError("Failed to connect to %s", MOBILEACTIVATION_SERVICE_NAME);
                    break;
                }

                result = mobileactivation_create_activation_info_with_session(mobileactivation_client, handshake_response, &ainfo);
                SmartLogDebug("ActivateDevice[%s] 111 = %d", device_id, result);
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
            SmartLogDebug("ActivateDevice[%s] 112 = %d", device_id, result);
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
            result = idevice_activation_request_new_from_lockdownd(IDEVICE_ACTIVATION_CLIENT_MOBILE_ACTIVATION, client, &request);
            SmartLogDebug("ActivateDevice[%s] 113 = %d", device_id, result);
            if (result != IDEVICE_ACTIVATION_E_SUCCESS) 
            {
                SmartLogError("Failed to create activation request");
                break;
            }
        }

        if (client)
        {
            lockdownd_client_free(client);
            client = nullptr;
        }
        
        SmartLogDebug("ActivateDevice[%s] 114 = %d", device_id, result);

        if (request && signing_service_url) 
        {
            idevice_activation_request_set_url(request, signing_service_url);
        }

        while (true) 
        {
            result = idevice_activation_send_request(request, &response);
            SmartLogDebug("ActivateDevice[%s] 115 = %d", device_id, result);
            if (result != IDEVICE_ACTIVATION_E_SUCCESS) {
                SmartLogError("Failed to send request or retrieve response");
                break;
            }

            result = idevice_activation_response_has_errors(response);
            SmartLogDebug("ActivateDevice[%s] 116 = %d", device_id, result);
            if (result)
            {
                SmartLogError("Activation server reports errors");

                idevice_activation_response_get_title(response, &response_title);
                SmartLogDebug("ActivateDevice[%s] 117 = %d", device_id, result);
                if (response_title) 
                {
                    SmartLogError("\t%s\n", response_title);
                }

                idevice_activation_response_get_description(response, &response_description);
                SmartLogDebug("ActivateDevice[%s] 118 = %d", device_id, result);
                if (response_description)
                {
                    SmartLogError("\t%s\n", response_description);
                }
                break;
            }

            idevice_activation_response_get_activation_record(response, &record);
            SmartLogDebug("ActivateDevice[%s] 119 = %d", device_id, result);
            if (record) 
            {
                result = lockdownd_client_new_with_handshake(device, &client, "ideviceactivation");
                SmartLogDebug("ActivateDevice[%s] 120 = %d", device_id, result);
                if (result != LOCKDOWN_E_SUCCESS)
                {
                    SmartLogError("Failed to connect to lockdownd");
                    break;
                }

                if (use_mobileactivation) 
                {
                    result = lockdownd_start_service(client, MOBILEACTIVATION_SERVICE_NAME, &svc);
                    SmartLogDebug("ActivateDevice[%s] 121 = %d", device_id, result);
                    if (result != LOCKDOWN_E_SUCCESS)
                    {
                        SmartLogError("Failed to start service %s", MOBILEACTIVATION_SERVICE_NAME);
                        break;
                    }

                    result = mobileactivation_client_new(device, svc, &mobileactivation_client);
                    SmartLogDebug("ActivateDevice[%s] 122 = %d", device_id, result);
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
                        SmartLogDebug("ActivateDevice[%s] 123 = %d", device_id, result);
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
                        SmartLogDebug("ActivateDevice[%s] 124 = %d", device_id, result);
                        if (result != MOBILEACTIVATION_E_SUCCESS)
                        {
                            SmartLogError("Failed to activate device with record");
                            break;
                        }
                    }
                }
                else 
                {
                    result = lockdownd_activate(client, record);
                    SmartLogDebug("ActivateDevice[%s] 125 = %d", device_id, result);
                    if (result != LOCKDOWN_E_SUCCESS) 
                    {
                        char* state_string = nullptr;
                        result = GetActivationState(client, &state_string);
                        SmartLogDebug("ActivateDevice[%s] 126 = %d", device_id, result);
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

                result = lockdownd_set_value(client, nullptr, "ActivationStateAcknowledged", plist_new_bool(1));
                SmartLogDebug("ActivateDevice[%s] 127 = %d", device_id, result);
                if (result != LOCKDOWN_E_SUCCESS) 
                {
                    SmartLogError("Failed to set ActivationStateAcknowledged on device");
                    break;
                }
                break;
            }
            else 
            {
                SmartLogDebug("ActivateDevice[%s] 128 = %d", device_id, result);
                if (idevice_activation_response_is_activation_acknowledged(response)) 
                {
                    SmartLogInfo("Activation server reports that device is already activated");
                    result = ERROR_SUCCESS;
                    break;
                }

                SmartLogDebug("ActivateDevice[%s] 129 = %d", device_id, result);
                idevice_activation_response_get_title(response, &response_title);
                if (response_title) 
                {
                    SmartLogError("Server reports:\n%s", response_title);
                }

                SmartLogDebug("ActivateDevice[%s] 130 = %d", device_id, result);
                idevice_activation_response_get_description(response, &response_description);
                if (response_description) 
                {
                    SmartLogError("Server reports:\n%s", response_description);
                }

                SmartLogDebug("ActivateDevice[%s] 131 = %d", device_id, result);
                idevice_activation_response_get_fields(response, &fields);
                if (!fields || plist_dict_get_size(fields) == 0) 
                {
                    SmartLogError("Unknown error");
                    result = EXIT_FAILURE;
                    break;
                }

                SmartLogDebug("ActivateDevice[%s] 132 = %d", device_id, result);
                plist_dict_new_iter(fields, &iter);
                if (!iter) 
                {
                    SmartLogError("Unknown error");
                    result = EXIT_FAILURE;
                    break;
                }

                SmartLogDebug("ActivateDevice[%s] 133 = %d", device_id, result);
                if (request)
                {
                    idevice_activation_request_free(request);
                    request = nullptr;
                }
               
                result = idevice_activation_request_new(IDEVICE_ACTIVATION_CLIENT_MOBILE_ACTIVATION, &request);
                SmartLogDebug("ActivateDevice[%s] 134 = %d", device_id, result);
                if (result != IDEVICE_ACTIVATION_E_SUCCESS) 
                {
                    SmartLogError("Could not create new request");
                    break;
                }

                idevice_activation_request_set_fields_from_response(request, response);

                int interactive_count = 0;
                do 
                {
                    SmartLogDebug("ActivateDevice[%s] 135 = %d", device_id, result);

                    field_key = nullptr;
                    plist_dict_next_item(fields, iter, &field_key, nullptr);
                    if (field_key) 
                    {
                        SmartLogDebug("ActivateDevice[%s] 136 = %d", device_id, result);

                        if (idevice_activation_response_field_requires_input(response, field_key)) 
                        {
                            SmartLogDebug("ActivateDevice[%s] 137 = %d", device_id, result);

                            idevice_activation_response_get_label(response, field_key, &field_label);
                            if (interactive) 
                            {
                                SmartLogDebug("ActivateDevice[%s] 138 = %d", device_id, result);

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

                            SmartLogDebug("ActivateDevice[%s] 139 = %d", device_id, result);

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

        SmartLogDebug("ActivateDevice[%s] 140 = %d", device_id, result);

        if (skip_install_setup)
        {
            result = SetupDoneDevice(device, wifi_ssid, wifi_password);
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

int iOSDeviceActivator::DeactivateDevice(const char* device_id)
{
    int result = ERROR_SUCCESS;
    idevice_t device = nullptr;
    lockdownd_client_t client = nullptr;
    mobileactivation_client_t mobileactivation_client = nullptr;

    do
    {
        result = idevice_new_with_options(&device, device_id, IDEVICE_LOOKUP_USBMUX);
        if (result != IDEVICE_E_SUCCESS)
            break;

        result = lockdownd_client_new_with_handshake(device, &client, "ideviceactivation");
        if (result != LOCKDOWN_E_SUCCESS)
            break;

        result = GetMobileActivationClient(device, client, &mobileactivation_client);
        if (result == ERROR_SUCCESS)
        {
            result = mobileactivation_deactivate(mobileactivation_client);
            if (result != MOBILEACTIVATION_E_SUCCESS)
                break;
        }
        else 
        {
            result = lockdownd_deactivate(client);
            if (result != LOCKDOWN_E_SUCCESS)
                break;
        }
    } while (false);

    if (mobileactivation_client)
    {
        mobileactivation_client_free(mobileactivation_client);
        mobileactivation_client = nullptr;
    }

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

int iOSDeviceActivator::QueryDeviceState(const char* device_id, bool* activated, bool* setup_done)
{
    int result = ERROR_SUCCESS;
    idevice_t device = nullptr;
    lockdownd_client_t client = nullptr;
    mobileactivation_client_t mobileactivation_client = nullptr;
    plist_t plist_activation_state = nullptr;
    char* activation_state_string = nullptr;

    do
    {
        if (activated)
        {
            *activated = false;
        }

        if (setup_done)
        {
            *setup_done = false;
        }

        result = idevice_new_with_options(&device, device_id, IDEVICE_LOOKUP_USBMUX);
        if (result != IDEVICE_E_SUCCESS)
            break;

        result = lockdownd_client_new_with_handshake(device, &client, "ideviceactivationstate");
        if (result != LOCKDOWN_E_SUCCESS)
            break;

        result = GetMobileActivationClient(device, client, &mobileactivation_client);
        if (result == ERROR_SUCCESS)
        {
            result = mobileactivation_get_activation_state(mobileactivation_client, &plist_activation_state);
        }
        else 
        {
            result = lockdownd_get_value(client, nullptr, "ActivationState", &plist_activation_state);
        }

        if (result == LOCKDOWN_E_SUCCESS)
        {
            if (plist_get_node_type(plist_activation_state) == PLIST_STRING)
            {
                plist_get_string_val(plist_activation_state, &activation_state_string);
                if (activation_state_string && strcmpi(activation_state_string, "Activated") == 0)
                {
                    if (activated)
                    {
                        *activated = true;
                    }
                }
            }
        }

        plist_t plist_setup_done = nullptr;
        result = lockdownd_get_value(client, "com.apple.purplebuddy", "SetupDone", &plist_setup_done);
        if (result == LOCKDOWN_E_SUCCESS)
        {
            uint8_t setup_done_value = 0;
            plist_get_bool_val(plist_setup_done, &setup_done_value);

            if (setup_done)
            {
                *setup_done = setup_done_value != 0;
            }

            plist_free(plist_setup_done);
            plist_setup_done = nullptr;
        }
    } while (false);

    if (plist_activation_state)
    {
        plist_free(plist_activation_state);
        plist_activation_state = nullptr;
    }

    if (activation_state_string)
    {
        mi_free(activation_state_string);
        activation_state_string = nullptr;
    }

    if (mobileactivation_client)
    {
        mobileactivation_client_free(mobileactivation_client);
        mobileactivation_client = nullptr;
    }

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

int iOSDeviceActivator::SetupDoneDevice(idevice_t device, const char* wifi_ssid, const char* wifi_password)
{
    int result = ERROR_SUCCESS;
    lockdownd_client_t client = nullptr;
    lockdownd_service_descriptor_t service = nullptr;
    plist_t cloud_configuration = nullptr;

    do
    {
        SmartLogDebug("SetupDoneDevice111");

        result = lockdownd_client_new_with_handshake(device, &client, "ss");
        SmartLogDebug("SetupDoneDevice112 = %d", result);
        if (result != LOCKDOWN_E_SUCCESS)
            break;


        result = lockdownd_start_service(client, "com.apple.mobile.MCInstall", &service);
        SmartLogDebug("SetupDoneDevice113 = %d", result);
        if (result != LOCKDOWN_E_SUCCESS)
            break;

        if (service)
        {
            lockdownd_service_descriptor_free(service);
            service = nullptr;
        }

        int wifi_ssid_length = lstrlenA(wifi_ssid);
        int wifi_password_length = lstrlenA(wifi_password);
        if (wifi_ssid_length > 0 && wifi_password_length > 0)
        {
            SmartLogDebug("SetupDoneDevice114 = %d", result);
            SetupWifiConnection(client, wifi_ssid, wifi_password);
            Sleep(9000);
        }

        if (client)
        {
            lockdownd_client_free(client);
            client = nullptr;
        }

        result = lockdownd_client_new_with_handshake(device, &client, "sd");
        SmartLogDebug("SetupDoneDevice115 = %d", result);
        if (result != LOCKDOWN_E_SUCCESS)
            break;
        
        result = SetupLanguage(client, true);
        SmartLogDebug("SetupDoneDevice116 = %d", result);
        if (result != LOCKDOWN_E_SUCCESS)
            break;

        plist_t setup_done = plist_new_bool(1);
        lockdownd_set_value(client, "com.apple.purplebuddy", "SetupDone", setup_done);

        plist_t setup_finished_all_steps = plist_new_bool(1);
        lockdownd_set_value(client, "com.apple.purplebuddy", "SetupFinishedAllSteps", setup_finished_all_steps);

        plist_t force_no_buddy = plist_new_bool(1);
        result = lockdownd_set_value(client, "com.apple.purplebuddy", "ForceNoBuddy", force_no_buddy);
        SmartLogDebug("SetupDoneDevice117 = %d", result);
        if (result != LOCKDOWN_E_SUCCESS)
            break;

        plist_t setup_version = plist_new_uint(11);
        result = lockdownd_set_value(client, "com.apple.purplebuddy", "SetupVersion", setup_version);
        SmartLogDebug("SetupDoneDevice118 = %d", result);
        if (result != LOCKDOWN_E_SUCCESS)
            break;

        if(true)
        {
            SmartLogDebug("SetupDoneDevice119 = %d", result);

            cloud_configuration = plist_new_dict();
            plist_dict_set_item(cloud_configuration, "AllowPairing", plist_new_bool(1));
            plist_dict_set_item(cloud_configuration, "CloudConfigurationUIComplete", plist_new_bool(1));
            plist_dict_set_item(cloud_configuration, "ConfigurationSource", plist_new_uint(2));
            plist_dict_set_item(cloud_configuration, "ConfigurationWasApplied", plist_new_bool(1));
            plist_dict_set_item(cloud_configuration, "IsMDMUnremovable", plist_new_bool(0));
            plist_dict_set_item(cloud_configuration, "IsMandatory", plist_new_bool(0));
            plist_dict_set_item(cloud_configuration, "IsSupervised", plist_new_bool(0));
            plist_dict_set_item(cloud_configuration, "PostSetupProfileWasInstalled", plist_new_bool(1));

            plist_t skip_setup = plist_new_array();
            plist_array_append_item(skip_setup, plist_new_string("AccessibilityAppearance"));
            plist_array_append_item(skip_setup, plist_new_string("All"));
            plist_array_append_item(skip_setup, plist_new_string("Android"));
            plist_array_append_item(skip_setup, plist_new_string("AppStore"));
            plist_array_append_item(skip_setup, plist_new_string("Appearance"));
            plist_array_append_item(skip_setup, plist_new_string("AppleID"));
            plist_array_append_item(skip_setup, plist_new_string("Biometric"));
            plist_array_append_item(skip_setup, plist_new_string("CloudStorage"));
            plist_array_append_item(skip_setup, plist_new_string("DeviceToDeviceMigration"));
            plist_array_append_item(skip_setup, plist_new_string("Diagnostics"));
            plist_array_append_item(skip_setup, plist_new_string("Display"));
            plist_array_append_item(skip_setup, plist_new_string("DisplayTone"));
            plist_array_append_item(skip_setup, plist_new_string("ExpressLanguage"));
            plist_array_append_item(skip_setup, plist_new_string("FileVault"));
            plist_array_append_item(skip_setup, plist_new_string("HomeButtonSensitivity"));
            plist_array_append_item(skip_setup, plist_new_string("IntendedUser"));
            plist_array_append_item(skip_setup, plist_new_string("Keyboard"));
            plist_array_append_item(skip_setup, plist_new_string("Language"));
            plist_array_append_item(skip_setup, plist_new_string("LanguageAndLocale"));
            plist_array_append_item(skip_setup, plist_new_string("Location"));
            plist_array_append_item(skip_setup, plist_new_string("MessagingActivationUsingPhoneNumber"));
            plist_array_append_item(skip_setup, plist_new_string("N/A"));
            plist_array_append_item(skip_setup, plist_new_string("OnBoarding"));
            plist_array_append_item(skip_setup, plist_new_string("Passcode"));
            plist_array_append_item(skip_setup, plist_new_string("Payment"));
            plist_array_append_item(skip_setup, plist_new_string("PreferredLanguage"));
            plist_array_append_item(skip_setup, plist_new_string("Privacy"));
            plist_array_append_item(skip_setup, plist_new_string("Region"));
            plist_array_append_item(skip_setup, plist_new_string("Registration"));
            plist_array_append_item(skip_setup, plist_new_string("Restore"));
            plist_array_append_item(skip_setup, plist_new_string("RestoreCompleted"));
            plist_array_append_item(skip_setup, plist_new_string("SIMSetup"));
            plist_array_append_item(skip_setup, plist_new_string("Safety"));
            plist_array_append_item(skip_setup, plist_new_string("ScreenSaver"));
            plist_array_append_item(skip_setup, plist_new_string("ScreenTime"));
            plist_array_append_item(skip_setup, plist_new_string("Siri"));
            plist_array_append_item(skip_setup, plist_new_string("SoftwareUpdate"));
            plist_array_append_item(skip_setup, plist_new_string("SpokenLanguage"));
            plist_array_append_item(skip_setup, plist_new_string("TOS"));
            plist_array_append_item(skip_setup, plist_new_string("TVHomeScreenSync"));
            plist_array_append_item(skip_setup, plist_new_string("TVProviderSignIn"));
            plist_array_append_item(skip_setup, plist_new_string("TVRoom"));
            plist_array_append_item(skip_setup, plist_new_string("TapToSetup"));
            plist_array_append_item(skip_setup, plist_new_string("TermsOfAddress"));
            plist_array_append_item(skip_setup, plist_new_string("Tone"));
            plist_array_append_item(skip_setup, plist_new_string("TouchID"));
            plist_array_append_item(skip_setup, plist_new_string("TrueToneDisplay"));
            plist_array_append_item(skip_setup, plist_new_string("UpdateCompleted"));
            plist_array_append_item(skip_setup, plist_new_string("WatchMigration"));
            plist_array_append_item(skip_setup, plist_new_string("Welcome"));
            plist_array_append_item(skip_setup, plist_new_string("WiFi"));
            plist_array_append_item(skip_setup, plist_new_string("Zoom"));
            plist_array_append_item(skip_setup, plist_new_string("iCloudDiagnostics"));
            plist_array_append_item(skip_setup, plist_new_string("iCloudStorage"));
            plist_array_append_item(skip_setup, plist_new_string("iMessageAndFaceTime"));
            plist_array_append_item(skip_setup, plist_new_string("WebContentFiltering"));
            plist_array_append_item(skip_setup, plist_new_string("CameraButton"));
            plist_dict_set_item(cloud_configuration, "SkipSetup", skip_setup);

            SmartLogDebug("SetupDoneDevice120 = %d", result);

            SetupCloudConfiguration(client, cloud_configuration);

            SmartLogDebug("SetupDoneDevice121 = %d", result);

        }
    } while (false);

    if (cloud_configuration)
    {
        plist_free(cloud_configuration);
        cloud_configuration = nullptr;
    }

    if (service)
    {
        lockdownd_service_descriptor_free(service);
        service = nullptr;
    }

    if (client)
    {
        lockdownd_client_free(client);
        client = nullptr;
    }

    return result;
}

int iOSDeviceActivator::SyncTimeIntervalSince1970(idevice_t device)
{
    int result = ERROR_SUCCESS;
    lockdownd_client_t client = nullptr;
    plist_t since_time_value = nullptr;
    plist_t sync_time_value = nullptr;

    do
    {
        result = lockdownd_client_new_with_handshake(device, &client, "pp");
        if (result != LOCKDOWN_E_SUCCESS)
            break;

        result = lockdownd_get_value(client, nullptr, "TimeIntervalSince1970", &since_time_value);
        if (result != LOCKDOWN_E_SUCCESS)
            break;

        time_t now;
        _time64(&now);
        plist_type type = plist_get_node_type(since_time_value);
        if (type == PLIST_UINT)
        {
            uint64_t time;
            plist_get_uint_val(since_time_value, &time);
            if ((unsigned __int64)(now - time) <= 3600)               
                break;

            sync_time_value = plist_new_uint(time);
        }
        else
        {
            if (type != PLIST_REAL)
                break;

            double time;
            plist_get_real_val(since_time_value, &time);
            if ((double)(int)now - time <= 3600.0)
                break;

            sync_time_value = plist_new_real(time);
        }

        result = lockdownd_set_value(client, nullptr, "TimeIntervalSince1970", sync_time_value);
        if (result != LOCKDOWN_E_SUCCESS)
            break;

    } while (false);

    if (sync_time_value)
    {
        plist_free(sync_time_value);
        sync_time_value = nullptr;
    }

    if (since_time_value)
    {
        plist_free(since_time_value);
        since_time_value = nullptr;
    }

    if (client)
    {
        lockdownd_client_free(client);
        client = nullptr;
    }

    return result;
}

int iOSDeviceActivator::SetupWifiConnection(lockdownd_client_t client, const char* wifi_ssid, const char* wifi_password)
{
    int result = ERROR_SUCCESS;
    plist_t domains = nullptr;
    plist_t wifi = nullptr;
    plist_t wifi_setting = nullptr;
    plist_t wifi_content = nullptr;
    char* wifi_setting_xml = nullptr;
    plist_t wifi_setting_data = nullptr;
    plist_t wifi_request = nullptr;
    plist_t wifi_response = nullptr;
    plist_t wifi_status = nullptr;
    char* wifi_status_string = nullptr;

    do
    {
        result = CreateDomainsDict(&domains);
        if (result != ERROR_SUCCESS)
            break;

        result = CreateWifiDict(wifi_ssid, wifi_password, &wifi);
        if (result != ERROR_SUCCESS)
            break;

        result = CreateWifiSettingDict(&wifi_setting);
        if (result != ERROR_SUCCESS)
            break;

        wifi_content = plist_new_array();
        if (wifi_content == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }

        plist_array_append_item(wifi_content, domains);
        plist_array_append_item(wifi_content, wifi);
        plist_dict_set_item(wifi_setting, "PayloadContent", wifi_content);

        uint32_t wifi_setting_length = 0;
        plist_to_xml(wifi_setting, &wifi_setting_xml, &wifi_setting_length);
        if (wifi_setting)
        {
            plist_free(wifi_setting);
            wifi_setting = nullptr;
        }

        if (wifi_setting_length == 0)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }

        wifi_setting_data = plist_new_data(wifi_setting_xml, wifi_setting_length);
        if (wifi_setting_xml)
        {
            mi_free(wifi_setting_xml);
            wifi_setting_xml = nullptr;
        }

        if (wifi_setting_data == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }

        wifi_request = plist_new_dict();
        if (wifi_request == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }

        plist_t request_type = plist_new_string("InstallProfile");
        if (request_type == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }

        plist_dict_set_item(wifi_request, "RequestType", request_type);
        plist_t payload = plist_copy(wifi_setting_data);
        if (payload == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }

        plist_dict_set_item(wifi_request, "Payload", payload);

        result = lockdownd_send(client, wifi_request);
        if (result != ERROR_SUCCESS)
            break;

        int try_count = 4;
        while (try_count > 0)
        {            
            result = lockdownd_receive(client, &wifi_response);
            if (result == ERROR_SUCCESS && wifi_response)
                break;

            if (result == LOCKDOWN_E_RECEIVE_TIMEOUT)
            {
                Sleep(500);
            }

            try_count--;
        }

        if (result == ERROR_SUCCESS && wifi_response)
        {
            wifi_status = plist_dict_get_item(wifi_response, "Status");
            if (wifi_status == nullptr)
            {
                result = ERROR_DATA_NOT_ACCEPTED;
                break;
            }

            plist_get_string_val(wifi_status, &wifi_status_string);
            if (strcmp(wifi_status_string, "Acknowledged"))
            {
                result = ERROR_DATATYPE_MISMATCH;
                break;
            }
        }
    } while (false);

    if (wifi_status_string)
    {
        mi_free(wifi_status_string);
        wifi_status_string = nullptr;
    }

    if (wifi_status)
    {
        plist_free(wifi_status);
        wifi_status = nullptr;
    }

    if (wifi_response)
    {
        plist_free(wifi_response);
        wifi_response = nullptr;
    }

    if (wifi_request)
    {
        plist_free(wifi_request);
        wifi_request = nullptr;
    }

    if (wifi_setting_data)
    {
        plist_free(wifi_setting_data);
        wifi_setting_data = nullptr;
    }

    if (wifi_setting_xml)
    {
        mi_free(wifi_setting_xml);
        wifi_setting_xml = nullptr;
    }

    return result;
}

int iOSDeviceActivator::SetupLanguage(lockdownd_client_t client, bool chinese)
{
    int result = ERROR_SUCCESS;
    
    do
    {
        plist_t time_zone = chinese ? plist_new_string("Asia/Shanghai") : plist_new_string("US/Pacific");
        if (time_zone == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        result = lockdownd_set_value(client, nullptr, "TimeZone", time_zone);
        if (result != ERROR_SUCCESS)
            break;

        plist_t uses_24hour_clock = plist_new_bool(1);
        if (uses_24hour_clock == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        result = lockdownd_set_value(client, nullptr, "Uses24HourClock", uses_24hour_clock);
        if (result != ERROR_SUCCESS)
            break;

        plist_t locale = chinese ? plist_new_string("zh_CN") : plist_new_string("en_US");
        if (locale == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        result = lockdownd_set_value(client, "com.apple.international", "Locale", locale);
        if (result != ERROR_SUCCESS)
            break;

        plist_t language = nullptr;
        if (chinese)
        {
            language = plist_new_string("zh-Hans");
        }
        else
        {
            language = plist_new_string("en_US");
        }
        if (language == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        result = lockdownd_set_value(client, "com.apple.international", "Language", language);
        if (result != ERROR_SUCCESS)
            break;

    } while (false);

    return result;
}

int iOSDeviceActivator::SetupCloudConfiguration(lockdownd_client_t client, plist_t cloud_configuration)
{
    int result = ERROR_SUCCESS;
    plist_t request = nullptr;
    plist_t response = nullptr;
    plist_t status = nullptr;
    char* status_string = nullptr;

    do
    {
        request = plist_new_dict();
        if (request == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }

        plist_t request_type = plist_new_string("SetCloudConfiguration");
        plist_dict_set_item(request, "RequestType", request_type);

        plist_t copy_cloud_configuration = plist_copy(cloud_configuration);
        plist_dict_set_item(request, "CloudConfiguration", copy_cloud_configuration);

        result = lockdownd_send(client, request);
        if (result != ERROR_SUCCESS)
            break;

        int try_count = 4;
        while (try_count > 0)
        {
            result = lockdownd_receive(client, &response);
            if (result == ERROR_SUCCESS && response)
                break;

            if (result == LOCKDOWN_E_RECEIVE_TIMEOUT)
            {
                result = ERROR_SUCCESS;
                Sleep(500);
            }

            try_count--;
        }

        if (result == ERROR_SUCCESS && response)
        {
            status = plist_dict_get_item(response, "Status");

            plist_get_string_val(status, &status_string);
            if (strcmp(status_string, "Acknowledged"))
            {
                result = ERROR_DATATYPE_MISMATCH;
                break;
            }
        }
    } while (false);

    if (status_string)
    {
        mi_free(status_string);
        status_string = nullptr;
    }

    if (status)
    {
        plist_free(status);
        status = nullptr;
    }

    if (response)
    {
        plist_free(response);
        response = nullptr;
    }

    if (request)
    {
        plist_free(request);
        request = nullptr;
    }

    return result;
}

int iOSDeviceActivator::CreateDomainsDict(plist_t* dict)
{
    int result = ERROR_SUCCESS;

    do
    {
        plist_t new_dict = plist_new_dict();
        if (new_dict == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }

        plist_t payload_display_name = plist_new_string("Domains");
        if (payload_display_name == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(new_dict, "PayloadDisplayName", payload_display_name);

        plist_t payload_identifier = plist_new_string("com.apple.domains.c1dcb149-edf2-4d41-82a9-961dd6ca279c");
        if (payload_identifier == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(new_dict, "PayloadIdentifier", payload_identifier);

        plist_t payload_type = plist_new_string("com.apple.domains");
        if (payload_type == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(new_dict, "PayloadType", payload_type);

        plist_t payload_uuid = plist_new_string("15815e78-d422-44d8-a4e9-d2b85649728e");
        if (payload_uuid == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(new_dict, "PayloadUUID", payload_uuid);

        plist_t payload_version = plist_new_uint(1);
        if (payload_version == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(new_dict, "PayloadVersion", payload_version);

        if (dict)
        {
            *dict = new_dict;
        }

    } while (false);

    return result;
}

int iOSDeviceActivator::CreateWifiDict(const char* ssid, const char* password, plist_t* dict)
{
    int result = ERROR_SUCCESS;

    do
    {
        plist_t new_dict = plist_new_dict();
        if (new_dict == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }

        plist_t payload_display_name = plist_new_string("Wi-Fi #1");
        if (payload_display_name == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(new_dict, "PayloadDisplayName", payload_display_name);

        plist_t payload_identifier = plist_new_string("com.apple.wifi.managed.2adb113c-104d-48c9-bb08-43c0ba9ae05f");
        if (payload_identifier == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(new_dict, "PayloadIdentifier", payload_identifier);

        plist_t payload_type = plist_new_string("com.apple.wifi.managed");
        if (payload_type == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(new_dict, "PayloadType", payload_type);

        plist_t payload_uuid = plist_new_string("61f9deed-317f-43cc-9870-c78490518a06");
        if (payload_uuid == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(new_dict, "PayloadUUID", payload_uuid);

        plist_t payload_version = plist_new_uint(1);
        if (payload_version == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(new_dict, "PayloadVersion", payload_version);

        plist_t encryption_type = plist_new_string("WPA");
        if (encryption_type == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(new_dict, "EncryptionType", encryption_type);

        if (ssid)
        {
            plist_t wifi_ssid = plist_new_string(ssid);
            if (wifi_ssid == nullptr)
            {
                result = ERROR_DATA_NOT_ACCEPTED;
                break;
            }
            plist_dict_set_item(new_dict, "SSID_STR", wifi_ssid);
        }
        
        if (password)
        {
            plist_t wifi_password = plist_new_string(password);
            if (wifi_password == nullptr)
            {
                result = ERROR_DATA_NOT_ACCEPTED;
                break;
            }
            plist_dict_set_item(new_dict, "Password", wifi_password);
        }
        
        if (dict)
        {
            *dict = new_dict;
        }

    } while (false);

    return result;
}

int iOSDeviceActivator::CreateWifiSettingDict(plist_t* dict)
{
    int result = ERROR_SUCCESS;

    do
    {
        plist_t new_dict = plist_new_dict();
        if (new_dict == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }

        plist_t  payload_display_name = plist_new_string("Wi-Fi Settings");
        if (payload_display_name == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(new_dict, "PayloadDisplayName", payload_display_name);

        plist_t payload_description = plist_new_string("SaneSong");
        if (payload_description == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(new_dict, "PayloadDescription", payload_description);

        plist_t payload_organization = plist_new_string("SaneSong.272bfdbc-0c34-417a-b598-7180d443ea9e");
        if (payload_organization == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(new_dict, "PayloadOrganization", payload_organization);

        plist_t payload_identifier = plist_new_string("SaneSong.272bfdbc-0c34-417a-b598-7180d443ea9e");
        if (payload_display_name == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(new_dict, "PayloadIdentifier", payload_identifier);

        plist_t payload_type = plist_new_string("Configuration");
        if (payload_type == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(new_dict, "PayloadType", payload_type);

        plist_t payload_uuid = plist_new_string("327f06b7-1356-4084-958c-0c0b25f7fc3d");
        if (payload_uuid == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(new_dict, "PayloadUUID", payload_uuid);

        plist_t payload_version = plist_new_uint(1);
        if (payload_version == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(new_dict, "PayloadVersion", payload_version);

        plist_t target_device_type = plist_new_uint(1);
        if (target_device_type == nullptr)
        {
            result = ERROR_DATA_NOT_ACCEPTED;
            break;
        }
        plist_dict_set_item(new_dict, "TargetDeviceType", target_device_type);

        if (dict)
        {
            *dict = new_dict;
        }

    } while (false);

    return result;
}

int iOSDeviceActivator::GetMobileActivationClient(idevice_t device, lockdownd_client_t client, mobileactivation_client_t* mobileactivation_client)
{
    int result = ERROR_SUCCESS;
    lockdownd_service_descriptor_t service = nullptr;

    do
    {
        result = lockdownd_start_service(client, MOBILEACTIVATION_SERVICE_NAME, &service);
        SmartLogDebug("GetMobileActivationClient 111 = %d", result);
        if (result != LOCKDOWN_E_SUCCESS)
            break;

        result = mobileactivation_client_new(device, service, mobileactivation_client);
        SmartLogDebug("GetMobileActivationClient 222 = %d", result);
        if (result != MOBILEACTIVATION_E_SUCCESS)
            break;

    } while (false);
 
    if (service)
    {
        lockdownd_service_descriptor_free(service);
        service = nullptr;
    }

    return result;
}

uint32_t iOSDeviceActivator::GetProductVersion(lockdownd_client_t client)
{
    uint32_t result = 0;
    plist_t product_version_value = nullptr;
    char* product_version_string = nullptr;

    do
    {
        if (lockdownd_get_value(client, nullptr, "ProductVersion", &product_version_value) != LOCKDOWN_E_SUCCESS)
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

int iOSDeviceActivator::GetActivationState(lockdownd_client_t client, char** activation_state_string)
{
    int result = IOS_ERROR_SUCCESS;
    plist_t state_value = nullptr;

    do
    {
        lockdownd_get_value(client, nullptr, "ActivationState", &state_value);
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