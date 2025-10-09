#ifndef _IOS_DEVICE_COMMAND_INFO_H_
#define _IOS_DEVICE_COMMAND_INFO_H_
#pragma once

#include <atomic>
#include <plist/plist.h>

class iOSDeviceCommandInfo
{
public:
	iOSDeviceCommandInfo();
	virtual ~iOSDeviceCommandInfo();
public:
	int Init();
	void Dispose();
public:
	int GetDeviceInfo(const char* device_id, std::map<std::string, std::string>* device_info);
	void Interrupt();
protected:
	int GetDeviceInfoInternal(plist_t node, std::map<std::string, std::string>* device_info);
	int GetDeviceDictInfo(plist_t node, int* indent_level, std::map<std::string, std::string>* device_info);
	int GetDeviceArrayInfo(plist_t node, int* indent_level, std::map<std::string, std::string>* device_info);
	int GetDeviceNodeInfo(plist_t node, int* indent_level, std::map<std::string, std::string>* device_info);
	int GetDeviceArrayInfo(plist_t node, int* indent_level, const char* key, std::map<std::string, std::string>* device_info);
	int GetDeviceNodeInfo(plist_t node, int* indent_level, const char* key, std::map<std::string, std::string>* device_info);
	void GetDeviceNodeBooleanInfo(plist_t node, const char* key, std::map<std::string, std::string>* device_info);
	void GetDeviceNodeRealInfo(plist_t node, const char* key, std::map<std::string, std::string>* device_info);
	void GetDeviceNodeUintInfo(plist_t node, const char* key, std::map<std::string, std::string>* device_info);
	void GetDeviceNodeStringInfo(plist_t node, const char* key, std::map<std::string, std::string>* device_info);
	void GetDeviceNodeKeyInfo(plist_t node, const char* key, std::map<std::string, std::string>* device_info);
	void GetDeviceNodeDataInfo(plist_t node, const char* key, std::map<std::string, std::string>* device_info);
	void GetDeviceNodeDateInfo(plist_t node, const char* key, std::map<std::string, std::string>* device_info);
private:
	char* Base64Encode(const unsigned char* buf, size_t size);
private:
	std::atomic<bool> interrupt;
};

#endif // _IOS_DEVICE_COMMAND_INFO_H_