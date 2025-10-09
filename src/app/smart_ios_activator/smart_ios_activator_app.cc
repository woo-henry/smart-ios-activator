#include "pch.h"
#include "smart_ios_activator_app.h"
#include "smart_ios_activator_dlg.h"

BEGIN_MESSAGE_MAP(SmartiOSActivatorApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

SmartiOSActivatorApp this_app;

SmartiOSActivatorApp::SmartiOSActivatorApp()
	: _app_path(TEXT(""))
	, _log_path(TEXT(""))
{
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

SmartiOSActivatorApp::~SmartiOSActivatorApp()
{

}

const CString SmartiOSActivatorApp::GetAppPath()
{
	return _app_path;
}

const CString SmartiOSActivatorApp::GetLogPath()
{
	return _log_path;
}

BOOL SmartiOSActivatorApp::InitInstance()
{
	int result = ERROR_SUCCESS;

	do
	{
		result = InitApp();
		if (result != ERROR_SUCCESS)
			break;

		SmartLogInfo("--------------------------------------------------------------");
		SmartLogInfo("iOS Device Mirror Application Start");
		SmartLogInfo("--------------------------------------------------------------");

		if (!CWinApp::InitInstance())
		{
			result = ERROR_APP_INIT_FAILURE;
			break;
		}

		if (!AfxSocketInit())
		{
			result = ERROR_APP_DATA_CORRUPT;
			break;
		}

		INITCOMMONCONTROLSEX initCtrls;
		initCtrls.dwSize = sizeof(initCtrls);
		initCtrls.dwICC = ICC_WIN95_CLASSES;
		InitCommonControlsEx(&initCtrls);

		AfxEnableControlContainer();
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

		SetRegistryKey(_T("SaneSong"));

		SmartiOSActivatorDlg dlg;
		m_pMainWnd = &dlg;
		INT_PTR response = dlg.DoModal();
		if (response == IDOK)
		{
		}
		else if (response == IDCANCEL)
		{
		}
		else if (response == -1)
		{
			
		}
#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
		ControlBarCleanUp();
#endif
	} while (false);

	return FALSE;
}

int SmartiOSActivatorApp::ExitInstance()
{
	if (!SmartMemCheck())
	{
		SmartLogWarn("iOS Device Mirror Application Memory Leak!!!");
	}

	SmartLogInfo("iOS Device Mirror Application Stop");
	SmartLogInfo("--------------------------------------------------------------\r\n\r\n\r\n");

	SmartLogShutdown();

	return CWinApp::ExitInstance();
}

int SmartiOSActivatorApp::InitApp()
{
	int result = ERROR_SUCCESS;
	TCHAR* app_path = nullptr;
	TCHAR* log_path = nullptr;

	do
	{
		app_path = (TCHAR*)SmartMemAlloc(MAX_PATH * sizeof(TCHAR));
		if (app_path == nullptr)
		{
			result = ERROR_NOT_ENOUGH_MEMORY;
			break;
		}

		log_path = (TCHAR*)SmartMemAlloc(MAX_PATH * sizeof(TCHAR));
		if (log_path == nullptr)
		{
			result = ERROR_NOT_ENOUGH_MEMORY;
			break;
		}

		if (!SmartFsGetAppPath(app_path))
		{
			result = ERROR_PATH_NOT_FOUND;
			break;
		}

		if (!SmartFsGetAppPath(log_path, TEXT("log")))
		{
			result = ERROR_PATH_NOT_FOUND;
			break;
		}

		if (!SmartLogInitialize(log_path))
		{
			result = ERROR_LOG_STATE_INVALID;
			break;
		}

		_app_path.Format(TEXT("%s"), app_path);
		_log_path.Format(TEXT("%s"), log_path);

	} while (false);

	if (log_path)
	{
		SmartMemFree(log_path);
		log_path = nullptr;
	}

	if (app_path)
	{
		SmartMemFree(app_path);
		app_path = nullptr;
	}

	return result;
}