#include "pch.h"
#include "hook_log.h"

int InitLog()
{
	int result = ERROR_SUCCESS;
	TCHAR* log_path = nullptr;

	do
	{
		log_path = (TCHAR*)SmartMemAlloc(MAX_PATH * sizeof(TCHAR));
		if (log_path == nullptr)
		{
			MessageBox(NULL, TEXT("SmartMemAlloc Error"), TEXT("Hook"), MB_OK);
			break;
		}

		if (!SmartFsGetAppPath(log_path, TEXT("log")))
		{
			MessageBox(NULL, TEXT("SmartFsGetAppPath Error"), TEXT("Hook"), MB_OK);
			break;
		}

		if (!SmartLogInitialize(log_path, TEXT("hook.log")))
		{
			MessageBox(NULL, TEXT("SmartLogInitialize Error"), TEXT("Hook"), MB_OK);
			result = ERROR_LOG_STATE_INVALID;
			break;
		}

	} while (false);

	if (log_path)
	{
		SmartMemFree(log_path);
		log_path = nullptr;
	}

	return result;
}

void ShutdownLog()
{
	SmartLogShutdown();
}