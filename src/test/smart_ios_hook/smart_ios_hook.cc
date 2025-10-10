// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "hook.h"

BOOL APIENTRY DllMain(HMODULE module, DWORD ul_reason_for_call, LPVOID reserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        StartHooks();
        break;
    case DLL_PROCESS_DETACH:
        FinishHooks();
        break;
    }

    return TRUE;
}

