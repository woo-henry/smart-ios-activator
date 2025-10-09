#include "pch.h"
#include "hook_log.h"
#include "hook_ws2_32.h"
#include "hook_libplist.h"
#include "hook.h"

LONG StartHooks()
{
    LONG result = ERROR_SUCCESS;

    do
    {
        MessageBox(NULL, TEXT("StartHooks"), TEXT("Hook"), MB_OK);

        result = InitLog();
        if (result != ERROR_SUCCESS)
        {
            MessageBox(NULL, TEXT("InitLog Error"), TEXT("Hook"), MB_OK);
            break;
        }

        SmartLogInfo("--------------------------------------------------------------");
        SmartLogInfo("Hook Start ......");
        SmartLogInfo("--------------------------------------------------------------");

        result = DetourTransactionBegin();
        SmartLogInfo("DetourTransactionBegin : %d", result);

        result = DetourUpdateThread(GetCurrentThread());
        SmartLogInfo("DetourUpdateThread : %d", result);

        result = StartHookLibplist();
        SmartLogInfo("StartHookLibplist : %d", result);

        result = StartHookWS2_32_API();
        SmartLogInfo("StartHookWS2_32_API : %d", result);

        result = DetourTransactionCommit();
        SmartLogInfo("DetourTransactionCommit : %d", result);

        SmartLogInfo("Hook Started");

    } while (false);

    return result;
}

LONG FinishHooks()
{
    LONG result = ERROR_SUCCESS;

    SmartLogInfo("Hook Finish ......");

    do
    {
        result = DetourTransactionBegin();
        SmartLogInfo("DetourTransactionBegin : %d", result);

        result = DetourUpdateThread(GetCurrentThread());
        SmartLogInfo("DetourUpdateThread : %d", result);

        result = FinishHookLibplist();
        SmartLogInfo("FinishHookLibplist : %d", result);

        result = FinishHookWS2_32_API();
        SmartLogInfo("FinishHookWS2_32_API : %d", result);

        result = DetourTransactionCommit();
        SmartLogInfo("DetourTransactionCommit : %d", result);

    } while (false);

    SmartLogInfo("Hook Finished");
    SmartLogInfo("---------------------------------------------------------\r\n\r\n\r\n");

    ShutdownLog();

    return result;
}