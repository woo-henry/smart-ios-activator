#ifndef _IOS_DEVICE_QUERY_H_
#define _IOS_DEVICE_QUERY_H_
#pragma once

#include <vector>
#include <smart_adv.h>
#include "device/ios_device_command_info.h"

typedef struct tagQueryDeviceContext
{
	char*								device_id;
	HANDLE								device_query_event;
	HANDLE								device_query_shutdown_event;
	volatile bool						device_query_shutdown;
} QueryDeviceContext;

class iOSDeviceQuerier
	: public SmartThreadTask
{
public:
	iOSDeviceQuerier(SmartThreadPool* thread_pool, void* device_context, iOSDeviceCallbacks* device_callbacks);
	virtual ~iOSDeviceQuerier();
public:
	int Init();
	void Dispose();
	int QueryDevice(const char* device_id);
private:
	int CreateQueryContext(const char* device_id, QueryDeviceContext** context);
	void DestroyQueryContext(QueryDeviceContext* context);
	void DestroyQueryContexts();
	int QueryDeviceInternal(QueryDeviceContext* context);
	int GetDeviceInfo(QueryDeviceContext* context, std::map<std::string, std::string>* device_info);;
protected:
	virtual void OnThreadHandle(void* parameter) override;
private:
	SmartThreadPool*					_thread_pool;
	void*								_device_context;
	iOSDeviceCallbacks*					_device_callbacks;
	iOSDeviceCommandInfo*				_device_command_info;
	std::vector<QueryDeviceContext*>	_device_query_contexts;
	CRITICAL_SECTION					_device_query_contexts_lock;
};

#endif // _IOS_DEVICE_QUERY_H_