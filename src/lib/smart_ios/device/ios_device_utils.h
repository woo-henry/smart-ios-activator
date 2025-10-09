#ifndef _IOS_UTILS_H_
#define _IOS_UTILS_H_
#pragma once

bool CompareProcessBufferA(const char* process_path, const char** compare_string_array, int array_size);
bool CompareProcessBufferW(const wchar_t* process_path, const wchar_t** compare_string_array, int array_size);

bool ReadProcessBufferA(const char* process_path, bool sync_process, char* process_buffer, int* buffer_size);
bool ReadProcessBufferW(const wchar_t* process_path, bool sync_process, wchar_t* process_buffer, int* buffer_size);

bool ReadAndCompareProcessBufferA(const char* process_path, bool sync_process, const char** compare_string_array, int array_size, char* process_buffer, int* buffer_size);
bool ReadAndCompareProcessBufferW(const wchar_t* process_path, bool sync_process, const wchar_t** compare_string_array, int array_size, wchar_t* process_buffer, int* buffer_size);

#ifndef WIN64
inline unsigned __int64 GetCycleCount();
#endif // !WIN64

void PrintBytes(const char* prefix, char* bytes, int size);

#endif // !_IOS_UTILS_H_