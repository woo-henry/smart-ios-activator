#include "pch.h"
#include <winsock2.h>
#include "hook_ws2_32.h"

typedef int (WSAAPI* pfn_connect)(SOCKET s, const sockaddr* name, int namelen);
typedef int (WSAAPI *pfn_send)(SOCKET s, const char* buf, int len, int flags);
typedef int (WSAAPI *pfn_recv)(SOCKET s, char* buf, int len, int flags);

static pfn_connect trust_connect = nullptr;
static pfn_send trust_send = nullptr;
static pfn_recv trust_recv = nullptr;

int WSAAPI hook_connect(SOCKET s, const sockaddr* name, int namelen)
{
    int result = trust_connect(s, name, namelen);

    SmartLogInfo("hook_connect, thread = %d, socket = %d, name = %p, namelen = %d, result = %d", 
        GetCurrentThreadId(), s, name, namelen, result);

    return result;
}

int WSAAPI hook_send(SOCKET s, const char* buf, int len, int flags)
{
    int result = trust_send(s, buf, len, flags);

    SmartLogInfo("hook_send, thread = %d, socket = %d, buf = %s, len = %d, flags = %d, result = %d", 
        GetCurrentThreadId(), s, buf, len, flags, result);

    return result;
}

int WSAAPI hook_recv(SOCKET s, char* buf, int len, int flags)
{
    int result = trust_recv(s, buf, len, flags);

    SmartLogInfo("hook_recv, thread = %d, socket = %d, buf = %s, len = %d, flags = %d, result = %d",
        GetCurrentThreadId(), s, buf, len, flags, result);

    return result;
}

LONG StartHookWS2_32_API()
{
	LONG result = ERROR_SUCCESS;
	HMODULE ws2_32_module = nullptr;

	do
	{
        ws2_32_module = GetModuleHandle(TEXT("ws2_32.dll"));
        if (ws2_32_module == nullptr)
        {
            MessageBox(nullptr, TEXT("LoadLibrary ws2_32 Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_connect = (pfn_connect)GetProcAddress(ws2_32_module, "connect");
        if (trust_connect == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress connect Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_send = (pfn_send)GetProcAddress(ws2_32_module, "send");
        if (trust_send == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress send Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_recv = (pfn_recv)GetProcAddress(ws2_32_module, "recv");
        if (trust_recv == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress recv Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        result = DetourAttach(&(PVOID&)trust_connect, hook_connect);
        SmartLogInfo("DetourAttach hook_connect : %d", result);

        result = DetourAttach(&(PVOID&)trust_send, hook_send);
        SmartLogInfo("DetourAttach hook_send : %d", result);

        result = DetourAttach(&(PVOID&)trust_recv, hook_recv);
        SmartLogInfo("DetourAttach hook_recv : %d", result);

	} while (false);

	return result;
}

LONG FinishHookWS2_32_API()
{
    LONG result = ERROR_SUCCESS;

    do
    {
        result = DetourDetach(&(PVOID&)trust_connect, hook_connect);
        SmartLogInfo("DetourDetach hook_connect : %d", result);

        result = DetourDetach(&(PVOID&)trust_send, hook_send);
        SmartLogInfo("DetourDetach hook_send : %d", result);

        result = DetourDetach(&(PVOID&)trust_recv, hook_recv);
        SmartLogInfo("DetourDetach hook_recv : %d", result);

    } while (false);
    
    return result;
}