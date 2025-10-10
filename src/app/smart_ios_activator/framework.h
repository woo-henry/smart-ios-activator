#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit
#define _AFX_ALL_WARNINGS

#include <afxwin.h>				// MFC core and standard components
#include <afxext.h>				// MFC extensions
#include <afxdisp.h>			// MFC Automation classes
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#include <afxcmn.h>             // MFC support for Windows Common Controls
#include <afxdialogex.h>		// MFC support for dialogs
#include <afxcontrolbars.h>     // MFC support for ribbons and control bars
#include <afxsock.h>            // MFC socket extensions

#include <smart_base.h>
#include <smart_adv.h>
#include <smart_ios.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


