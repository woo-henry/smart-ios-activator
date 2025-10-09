#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

class SmartiOSActivatorApp
	: public CWinAppEx
{
public:
	SmartiOSActivatorApp();
	virtual ~SmartiOSActivatorApp();
public:
	const CString GetAppPath();
	const CString GetLogPath();
protected:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
protected:
	DECLARE_MESSAGE_MAP()
private:
	int InitApp();
private:
	CString _app_path;
	CString	_log_path;
};

extern SmartiOSActivatorApp this_app;
