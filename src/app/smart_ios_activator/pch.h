// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

#define WM_IOS_DEVICE_CONNECT			WM_USER + 1000
#define WM_IOS_DEVICE_RECONNECT			WM_USER + 1001
#define WM_IOS_DEVICE_OBJECT_ADD		WM_USER + 1002
#define WM_IOS_DEVICE_OBJECT_REMOVE		WM_USER + 1003
#define	WM_IOS_DEVICE_OBJECT_UPDATE		WM_USER + 1004
#define WM_IOS_DEVICE_STATUS_UPDATE		WM_USER + 1005

#define WM_IOS_DEVICE_MESSAGE_SUCCESS	WM_USER + 9001
#define WM_IOS_DEVICE_MESSAGE_ERROR		WM_USER + 9002

typedef struct tagIosDevice
{
	char	device_id[MAX_PATH];
	char	device_name[MAX_PATH];
	char	serial_number[MAX_PATH];
	char	product_name[MAX_PATH];
	char	product_type[MAX_PATH];
	char	product_version[MAX_PATH];
	char	phone_number[MAX_PATH];
	bool	activated;
} IosDevice;

#endif //PCH_H
