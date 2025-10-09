#include "pch.h"
#include "hook_kernelbase.h"

typedef void (*PfnSleep)(DWORD dwMilliseconds);
typedef ULONGLONG(*PfnGetTickCount64)();
typedef BOOL (*PfnDeviceIoControl)(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped);
typedef BOOL (*PfnGetOverlappedResult)(HANDLE hFile, LPOVERLAPPED lpOverlapped, LPDWORD lpNumberOfBytesTransferred, BOOL bWait);
typedef HANDLE (*PfnCreateFileW)(LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile);
typedef BOOL(*PfnReadFile)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);

typedef BOOL (*PfnCreateProcessA)(LPCSTR lpApplicationName,
    LPSTR                 lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCSTR                lpCurrentDirectory,
    LPSTARTUPINFOA        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation);

static PfnSleep TrustSleep = nullptr;
static PfnGetTickCount64 TrustGetTickCount64 = nullptr;
static PfnDeviceIoControl TrustDeviceIoControl = nullptr;
static PfnGetOverlappedResult TrustGetOverlappedResult = nullptr;
static PfnCreateFileW TrustCreateFileW = nullptr;
static PfnReadFile TrustReadFile = nullptr;
static PfnCreateProcessA TrustCreateProcessA = nullptr;

void HookSleep(DWORD dwMilliseconds)
{
    SmartLogInfo("HookSleep, thread = %d, dwMilliseconds = %d", GetCurrentThreadId(), dwMilliseconds);
    TrustSleep(dwMilliseconds);
}

ULONGLONG HookGetTickCount64()
{
    ULONGLONG result = TrustGetTickCount64();
    SmartLogInfo("HookGetTickCount64, thread = %d, result = %ld", GetCurrentThreadId(), result);

    return result;
}

BOOL HookDeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped)
{
    BOOL result = TrustDeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);

    return result;
}

BOOL HookGetOverlappedResult(HANDLE hFile, LPOVERLAPPED lpOverlapped, LPDWORD lpNumberOfBytesTransferred, BOOL bWait)
{
    BOOL result = TrustGetOverlappedResult(hFile, lpOverlapped, lpNumberOfBytesTransferred, bWait);
    SmartLogInfo("HookGetOverlappedResult, thread = %d, hFile = %p, lpOverlapped = %p, lpNumberOfBytesTransferred = %p, bWait = %d, result = %d",
        GetCurrentThreadId(), hFile, lpOverlapped, lpNumberOfBytesTransferred, bWait, result);

    return result;
}

HANDLE HookCreateFileW(LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile)
{
    HANDLE result = TrustCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

    char* filename = nullptr;
    do
    {
        filename = (char*)SmartMemAlloc(MAX_PATH);
        if (filename)
        {
            int filename_length = 0;
            if (SmartStrW2A(filename, &filename_length, lpFileName))
            {
                if (StrStrIA(filename, "userdefaults.conf"))
                {
                    SmartLogInfo("HookCreateFileW, thread = %d, lpFileName = %s, dwDesiredAccess = 0x%X, dwShareMode = 0x%X, lpSecurityAttributes = %p, dwCreationDisposition = 0x%X, dwFlagsAndAttributes = 0x%X, hTemplateFile = %p, result = %p",
                        GetCurrentThreadId(), filename, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile, result);
                }
            }
        }
    } while (false);

    if (filename)
    {
        SmartMemFree(filename);
        filename = nullptr;
    }

    return result;
}

BOOL HookReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
    BOOL result = TrustReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
    
    {
        SmartLogInfo("HookReadFile, thread = %d, hFile = %p, lpBuffer = %p, nNumberOfBytesToRead = %d, lpNumberOfBytesRead = %p, lpOverlapped = %p, result = %d",
            GetCurrentThreadId(), hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped, result);
    }
   
    return result;
}

BOOL HookCreateProcessA(LPCSTR lpApplicationName,
    LPSTR                 lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCSTR                lpCurrentDirectory,
    LPSTARTUPINFOA        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation)
{
    BOOL result = TrustCreateProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    if (lpApplicationName)
    {
        SmartLogInfo("HookCreateProcessA, thread = %d, lpApplicationName = %s", GetCurrentThreadId(), lpApplicationName);
    }
    
    if (lpCommandLine)
    {
        SmartLogInfo("HookCreateProcessA, thread = %d, lpCommandLine = %s", GetCurrentThreadId(), lpCommandLine);
    }
    
    SmartLogInfo("HookCreateProcessA, thread = %d, result = %ld", GetCurrentThreadId(), result);

    return result;
}

LONG StartHookKernelBase()
{
	LONG result = ERROR_SUCCESS;
	HMODULE kernel_base_module = nullptr;

	do
	{
        kernel_base_module = GetModuleHandle(TEXT("KernelBase.dll"));
        if (kernel_base_module == nullptr)
        {
            MessageBox(NULL, TEXT("LoadLibrary KernelBase Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        TrustSleep = (PfnSleep)GetProcAddress(kernel_base_module, "Sleep");
        if (TrustSleep == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress Sleep Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        TrustGetTickCount64 = (PfnGetTickCount64)GetProcAddress(kernel_base_module, "GetTickCount64");
        if (TrustGetTickCount64 == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress GetTickCount64 Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        TrustDeviceIoControl = (PfnDeviceIoControl)GetProcAddress(kernel_base_module, "DeviceIoControl");
        if (TrustDeviceIoControl == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress DeviceIoControl Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        TrustGetOverlappedResult = (PfnGetOverlappedResult)GetProcAddress(kernel_base_module, "GetOverlappedResult");
        if (TrustGetOverlappedResult == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress GetOverlappedResult Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        TrustCreateFileW = (PfnCreateFileW)GetProcAddress(kernel_base_module, "CreateFileW");
        if (TrustCreateFileW == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress CreateFileW Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        TrustReadFile = (PfnReadFile)GetProcAddress(kernel_base_module, "ReadFile");
        if (TrustReadFile == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress ReadFile Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        TrustCreateProcessA = (PfnCreateProcessA)GetProcAddress(kernel_base_module, "CreateProcessA");
        if (TrustCreateProcessA == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress CreateProcessA Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        //result = DetourAttach(&(PVOID&)TrustSleep, HookSleep);
        //SmartLogInfo("DetourAttach Sleep : %d", result);

        //result = DetourAttach(&(PVOID&)TrustGetTickCount64, HookGetTickCount64);
        //SmartLogInfo("DetourAttach GetTickCount64 : %d", result);

        //result = DetourAttach(&(PVOID&)TrustDeviceIoControl, HookDeviceIoControl);
        //SmartLogInfo("DetourAttach DeviceIoControl : %d", result);

        //result = DetourAttach(&(PVOID&)TrustGetOverlappedResult, HookGetOverlappedResult);
        //SmartLogInfo("DetourAttach GetOverlappedResult : %d", result);

        //result = DetourAttach(&(PVOID&)TrustCreateFileW, HookCreateFileW);
        //SmartLogInfo("DetourAttach CreateFileW : %d", result);

        //result = DetourAttach(&(PVOID&)TrustReadFile, HookReadFile);
        //SmartLogInfo("DetourAttach ReadFile : %d", result);

        //result = DetourAttach(&(PVOID&)TrustCreateProcessA, HookCreateProcessA);
        //SmartLogInfo("DetourAttach CreateProcessA : %d", result);

	} while (false);

	return result;
}

LONG FinishHookKernelBase()
{
    LONG result = ERROR_SUCCESS;

    do
    {
        //result = DetourDetach(&(PVOID&)TrustSleep, HookSleep);
        //SmartLogInfo("DetourDetach Sleep : %d", result);

        //result = DetourDetach(&(PVOID&)TrustGetTickCount64, HookGetTickCount64);
        //SmartLogInfo("DetourDetach GetTickCount64 : %d", result);

        //result = DetourDetach(&(PVOID&)TrustDeviceIoControl, HookDeviceIoControl);
        //SmartLogInfo("DetourDetach DeviceIoControl : %d", result);

        //result = DetourDetach(&(PVOID&)TrustGetOverlappedResult, HookGetOverlappedResult);
        //SmartLogInfo("DetourDetach GetOverlappedResult : %d", result);

        //result = DetourDetach(&(PVOID&)TrustCreateFileW, HookCreateFileW);
        //SmartLogInfo("DetourDetach CreateFileW : %d", result);

        //result = DetourDetach(&(PVOID&)TrustReadFile, HookReadFile);
        //SmartLogInfo("DetourDetach ReadFile : %d", result);

        //result = DetourDetach(&(PVOID&)TrustCreateProcessA, HookCreateProcessA);
        //SmartLogInfo("DetourDetach HookCreateProcessA : %d", result);

    } while (false);
    
    return result;
}