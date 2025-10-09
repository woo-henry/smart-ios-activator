#ifndef _SMART_IOS_CONFIG_H_
#define _SMART_IOS_CONFIG_H_
#pragma once
//////////////////////////////////////////////////////////////////////////
#if defined(SMART_IOS_STATIC)
#define SMART_IOS_API
#elif defined(SMART_IOS_DYNAMIC)
//  definitions used when building DLL
#define SMART_IOS_API __declspec(dllexport)
#else
//  definitions used when using DLL
#define SMART_IOS_API __declspec(dllimport)
#endif

#ifndef IOS_API
#define IOS_API SMART_IOS_API
#endif

#ifdef IOS_API
typedef unsigned long long						CFTypeID;
typedef unsigned short							uint16;
typedef unsigned int							uint32;
typedef unsigned long long						uint64;
typedef long long								int64;
typedef double									float64;
#endif // SMART_IOS_API

//////////////////////////////////////////////////////////////////////////
#include <wtypes.h>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <windows.h>

#endif // _SMART_IOS_CONFIG_H_
