#include <shlwapi.h>
#include <smart_base.h>
#include <boost\algorithm\string.hpp>
#include <libimobiledevice/libimobiledevice.h>
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

	return result;
}

int iOSDeviceActivator::DeactivateDevice(const char* device_id)
{
	int result = ERROR_SUCCESS;

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
            NULL,
            NULL,
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