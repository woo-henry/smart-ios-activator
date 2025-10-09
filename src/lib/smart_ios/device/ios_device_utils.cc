#include <smart_base.h>
#include <smart_adv.h>
#include <smart_ios.h>
#include <direct.h>
#include <shlwapi.h>
#include <boost/algorithm/string.hpp>
#include "device/ios_device_utils.h"

#define MAX_PROCESS_BUFFER		1024 * 1024

bool CompareProcessBufferA(const char* process_path, const char** compare_string_array, int array_size)
{
	bool result = false;
	char* process_commandline = nullptr;
	PVOID process_buffer = nullptr;
	HANDLE process_read_pipe = nullptr;
	HANDLE process_write_pipe = nullptr;

	do
	{
		process_commandline = (char*)SmartMemAlloc(MAX_PATH * sizeof(char));
		if (process_commandline == nullptr)
			break;

		process_buffer = (PVOID)SmartMemAlloc(MAX_PROCESS_BUFFER);
		if (process_buffer == nullptr)
			break;

		SECURITY_ATTRIBUTES security_attributes = { 0 };
		security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
		security_attributes.lpSecurityDescriptor = nullptr;
		security_attributes.bInheritHandle = TRUE;
		if (!CreatePipe(&process_read_pipe, &process_write_pipe, &security_attributes, 0))
			break;
		
		STARTUPINFOA si = { 0 };
		RtlZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.wShowWindow = SW_HIDE;
		si.hStdOutput = process_write_pipe;
		si.hStdError = process_write_pipe;

		PROCESS_INFORMATION pi = { 0 };
		RtlZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		lstrcpyA(process_commandline, process_path);
		if (!CreateProcessA(NULL, process_commandline, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
			break;

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);

		DWORD number_of_bytes_read = 0;
		while (!result)
		{
			if (!ReadFile(process_read_pipe, process_buffer, MAX_PROCESS_BUFFER, &number_of_bytes_read, nullptr) || number_of_bytes_read == 0)
				break;

			for (int i = 0; i < array_size; i++)
			{
				if (StrStrIA((char*)process_buffer, compare_string_array[i]))
				{
					result = true;
					break;
				}
			}
		}
	} while (false);

	if (process_read_pipe)
	{
		CloseHandle(process_read_pipe);
		process_read_pipe = nullptr;
	}

	if (process_write_pipe)
	{
		CloseHandle(process_write_pipe);
		process_write_pipe = nullptr;
	}

	if (process_buffer)
	{
		SmartMemFree(process_buffer);
		process_buffer = nullptr;
	}

	if (process_commandline)
	{
		SmartMemFree(process_commandline);
		process_commandline = nullptr;
	}

	return result;
}

bool CompareProcessBufferW(const wchar_t* process_path, const wchar_t** compare_string_array, int array_size)
{
	bool result = false;
	wchar_t* process_commandline = nullptr;
	PVOID process_buffer = nullptr;
	wchar_t* process_buffer_w = nullptr;
	int process_buffer_w_length = 0;
	HANDLE process_read_pipe = nullptr;
	HANDLE process_write_pipe = nullptr;

	do
	{
		process_commandline = (TCHAR*)SmartMemAlloc(MAX_PATH * sizeof(wchar_t));
		if (process_commandline == nullptr)
			break;

		process_buffer = (PVOID)SmartMemAlloc(MAX_PROCESS_BUFFER);
		if (process_buffer == nullptr)
			break;

		process_buffer_w = (wchar_t*)SmartMemAlloc(MAX_PROCESS_BUFFER * sizeof(wchar_t));
		if (process_buffer_w == nullptr)
			break;

		SECURITY_ATTRIBUTES security_attributes = { 0 };
		security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
		security_attributes.lpSecurityDescriptor = nullptr;
		security_attributes.bInheritHandle = TRUE;
		if (!CreatePipe(&process_read_pipe, &process_write_pipe, &security_attributes, 0))
			break;

		STARTUPINFOW si = { 0 };
		RtlZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.wShowWindow = SW_HIDE;
		si.hStdOutput = process_write_pipe;
		si.hStdError = process_write_pipe;

		PROCESS_INFORMATION pi = { 0 };
		RtlZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		lstrcpyW(process_commandline, process_path);
		if (!CreateProcessW(NULL, process_commandline, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
			break;

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);

		DWORD number_of_bytes_read = 0;
		while (!result)
		{
			if (!ReadFile(process_read_pipe, process_buffer, MAX_PROCESS_BUFFER, &number_of_bytes_read, nullptr) || number_of_bytes_read == 0)
				break;

			if (!SmartStrA2W((wchar_t*)process_buffer_w, &process_buffer_w_length, (const char*)process_buffer))
				break;

			for (int i = 0; i < array_size; i++)
			{
				if (StrStrIW((wchar_t*)process_buffer_w, compare_string_array[i]))
				{
					result = true;
					break;
				}
			}
		}
	} while (false);

	if (process_read_pipe)
	{
		CloseHandle(process_read_pipe);
		process_read_pipe = nullptr;
	}

	if (process_write_pipe)
	{
		CloseHandle(process_write_pipe);
		process_write_pipe = nullptr;
	}

	if (process_buffer_w)
	{
		SmartMemFree(process_buffer_w);
		process_buffer_w = nullptr;
	}

	if (process_buffer)
	{
		SmartMemFree(process_buffer);
		process_buffer = nullptr;
	}

	if (process_commandline)
	{
		SmartMemFree(process_commandline);
		process_commandline = nullptr;
	}

	return result;
}

bool ReadProcessBufferA(const char* process_path, bool sync_process, char* process_buffer, int* buffer_size)
{
	bool result = false;
	char* process_commandline = nullptr;
	HANDLE process_read_pipe = nullptr;
	HANDLE process_write_pipe = nullptr;

	do
	{
		process_commandline = (char*)SmartMemAlloc(MAX_PATH * sizeof(char));
		if (process_commandline == nullptr)
			break;

		SECURITY_ATTRIBUTES security_attributes = { 0 };
		security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
		security_attributes.lpSecurityDescriptor = nullptr;
		security_attributes.bInheritHandle = TRUE;
		if (!CreatePipe(&process_read_pipe, &process_write_pipe, &security_attributes, 0))
			break;

		STARTUPINFOA si = { 0 };
		RtlZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.wShowWindow = SW_HIDE;
		si.hStdOutput = process_write_pipe;
		si.hStdError = process_write_pipe;

		PROCESS_INFORMATION pi = { 0 };
		RtlZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		lstrcpyA(process_commandline, process_path);
		if (!CreateProcessA(NULL, process_commandline, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
			break;

		if (sync_process)
		{
			WaitForSingleObject(pi.hProcess, INFINITE);
		}

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		
		DWORD number_of_bytes_read = 0;
		if (!ReadFile(process_read_pipe, process_buffer, MAX_PROCESS_BUFFER, &number_of_bytes_read, nullptr) || number_of_bytes_read == 0)
			break;

		if (buffer_size)
		{
			*buffer_size = number_of_bytes_read;
		}

		result = true;

	} while (false);

	if (process_read_pipe)
	{
		CloseHandle(process_read_pipe);
		process_read_pipe = nullptr;
	}

	if (process_write_pipe)
	{
		CloseHandle(process_write_pipe);
		process_write_pipe = nullptr;
	}

	if (process_commandline)
	{
		SmartMemFree(process_commandline);
		process_commandline = nullptr;
	}

	return result;
}

bool ReadProcessBufferW(const wchar_t* process_path, bool sync_process, wchar_t* process_buffer, int* buffer_size)
{
	bool result = false;
	wchar_t* process_commandline = nullptr;
	char* process_buffer_a = nullptr;
	HANDLE process_read_pipe = nullptr;
	HANDLE process_write_pipe = nullptr;

	do
	{
		process_commandline = (wchar_t*)SmartMemAlloc(MAX_PATH * sizeof(wchar_t));
		if (process_commandline == nullptr)
			break;

		process_buffer_a = (char*)SmartMemAlloc(MAX_PROCESS_BUFFER);
		if (process_buffer_a == nullptr)
			break;

		SECURITY_ATTRIBUTES security_attributes = { 0 };
		security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
		security_attributes.lpSecurityDescriptor = nullptr;
		security_attributes.bInheritHandle = TRUE;
		if (!CreatePipe(&process_read_pipe, &process_write_pipe, &security_attributes, 0))
			break;

		STARTUPINFOW si = { 0 };
		RtlZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.wShowWindow = SW_HIDE;
		si.hStdOutput = process_write_pipe;
		si.hStdError = process_write_pipe;

		PROCESS_INFORMATION pi = { 0 };
		RtlZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		lstrcpyW(process_commandline, process_path);
		if (!CreateProcessW(NULL, process_commandline, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
			break;

		if (sync_process)
		{
			WaitForSingleObject(pi.hProcess, INFINITE);
		}
		
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);

		DWORD number_of_bytes_read = 0;
		if (!ReadFile(process_read_pipe, process_buffer_a, MAX_PROCESS_BUFFER, &number_of_bytes_read, nullptr) || number_of_bytes_read == 0)
			break;

		if (!SmartStrA2W((wchar_t*)process_buffer, buffer_size, (const char*)process_buffer_a))
			break;

		if (buffer_size)
		{
			*buffer_size = number_of_bytes_read;
		}

		result = true;

	} while (false);

	if (process_read_pipe)
	{
		CloseHandle(process_read_pipe);
		process_read_pipe = nullptr;
	}

	if (process_write_pipe)
	{
		CloseHandle(process_write_pipe);
		process_write_pipe = nullptr;
	}

	if (process_buffer_a)
	{
		SmartMemFree(process_buffer_a);
		process_buffer_a = nullptr;
	}

	if (process_commandline)
	{
		SmartMemFree(process_commandline);
		process_commandline = nullptr;
	}

	return result;
}

bool ReadAndCompareProcessBufferA(const char* process_path, bool sync_process, const char** compare_string_array, int array_size, char* process_buffer, int* buffer_size)
{
	bool result = false;
	char* process_commandline = nullptr;
	char* process_temp_buffer = nullptr;
	HANDLE process_read_pipe = nullptr;
	HANDLE process_write_pipe = nullptr;

	do
	{
		process_commandline = (char*)SmartMemAlloc(MAX_PATH * sizeof(char));
		if (process_commandline == nullptr)
			break;

		process_temp_buffer = (char*)SmartMemAlloc(MAX_PROCESS_BUFFER);
		if (process_temp_buffer == nullptr)
			break;

		SECURITY_ATTRIBUTES security_attributes = { 0 };
		security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
		security_attributes.lpSecurityDescriptor = nullptr;
		security_attributes.bInheritHandle = TRUE;
		if (!CreatePipe(&process_read_pipe, &process_write_pipe, &security_attributes, 0))
			break;

		STARTUPINFOA si = { 0 };
		RtlZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.wShowWindow = SW_HIDE;
		si.hStdOutput = process_write_pipe;
		si.hStdError = process_write_pipe;

		PROCESS_INFORMATION pi = { 0 };
		RtlZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		lstrcpyA(process_commandline, process_path);
		if (!CreateProcessA(NULL, process_commandline, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
			break;

		if (sync_process)
		{
			WaitForSingleObject(pi.hProcess, INFINITE);
		}
		else
		{
			WaitForSingleObject(pi.hProcess, 1000);
		}

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);

		DWORD read_count = 0;
		DWORD number_of_bytes_read = 0;
		while (read_count < 3)
		{
			if (result)
				break;

			RtlZeroMemory(process_temp_buffer, MAX_PROCESS_BUFFER);
			if (!ReadFile(process_read_pipe, process_temp_buffer, MAX_PROCESS_BUFFER, &number_of_bytes_read, nullptr) || number_of_bytes_read == 0)
				break;

			for (int i = 0; i < array_size; i++)
			{
				if (boost::ifind_first(process_temp_buffer, compare_string_array[i]))
				{
					result = true;
					break;
				}

				if (StrStrIA(process_temp_buffer, compare_string_array[i]))
				{
					result = true;
					break;
				}
			}

			read_count++;
		}
		
		if (result)
		{
			if (process_buffer)
			{
				lstrcpyA(process_buffer, process_temp_buffer);
			}

			if (buffer_size)
			{
				*buffer_size = number_of_bytes_read;
			}
		}
	} while (false);

	if (process_read_pipe)
	{
		CloseHandle(process_read_pipe);
		process_read_pipe = nullptr;
	}

	if (process_write_pipe)
	{
		CloseHandle(process_write_pipe);
		process_write_pipe = nullptr;
	}

	if (process_temp_buffer)
	{
		SmartMemFree(process_temp_buffer);
		process_temp_buffer = nullptr;
	}

	if (process_commandline)
	{
		SmartMemFree(process_commandline);
		process_commandline = nullptr;
	}

	return result;
}

bool ReadAndCompareProcessBufferW(const wchar_t* process_path, bool sync_process, const wchar_t** compare_string_array, int array_size, wchar_t* process_buffer, int* buffer_size)
{
	bool result = false;
	wchar_t* process_commandline = nullptr;
	char* process_buffer_a = nullptr;
	wchar_t* process_buffer_w = nullptr;
	HANDLE process_read_pipe = nullptr;
	HANDLE process_write_pipe = nullptr;

	do
	{
		process_commandline = (wchar_t*)SmartMemAlloc(MAX_PATH * sizeof(wchar_t));
		if (process_commandline == nullptr)
			break;

		process_buffer_a = (char*)SmartMemAlloc(MAX_PROCESS_BUFFER);
		if (process_buffer_a == nullptr)
			break;

		process_buffer_w = (wchar_t*)SmartMemAlloc(MAX_PROCESS_BUFFER * sizeof(wchar_t));
		if (process_buffer_w == nullptr)
			break;

		SECURITY_ATTRIBUTES security_attributes = { 0 };
		security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
		security_attributes.lpSecurityDescriptor = nullptr;
		security_attributes.bInheritHandle = TRUE;
		if (!CreatePipe(&process_read_pipe, &process_write_pipe, &security_attributes, 0))
			break;

		STARTUPINFOW si = { 0 };
		RtlZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.wShowWindow = SW_HIDE;
		si.hStdOutput = process_write_pipe;
		si.hStdError = process_write_pipe;

		PROCESS_INFORMATION pi = { 0 };
		RtlZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		lstrcpyW(process_commandline, process_path);
		if (!CreateProcessW(NULL, process_commandline, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
			break;

		if (sync_process)
		{
			WaitForSingleObject(pi.hProcess, INFINITE);
		}
		else
		{
			WaitForSingleObject(pi.hProcess, 1000);
		}

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);

		DWORD number_of_bytes_read = 0;
		RtlZeroMemory(process_buffer_a, MAX_PROCESS_BUFFER);
		if (!ReadFile(process_read_pipe, process_buffer_a, MAX_PROCESS_BUFFER, &number_of_bytes_read, nullptr) || number_of_bytes_read == 0)
			break;

		int process_buffer_size = 0;
		if (!SmartStrA2W(process_buffer_w, &process_buffer_size, (const char*)process_buffer_a))
			break;

		for (int i = 0; i < array_size; i++)
		{
			if (boost::ifind_first(process_buffer_w, compare_string_array[i]))
			{
				result = true;
				break;
			}

			if (StrStrIW(process_buffer_w, compare_string_array[i]))
			{
				result = true;
				break;
			}
		}

		if (result)
		{
			if (process_buffer)
			{
				lstrcpyW(process_buffer, process_buffer_w);
			}

			if (buffer_size)
			{
				*buffer_size = number_of_bytes_read;
			}
		}
	} while (false);

	if (process_read_pipe)
	{
		CloseHandle(process_read_pipe);
		process_read_pipe = nullptr;
	}

	if (process_write_pipe)
	{
		CloseHandle(process_write_pipe);
		process_write_pipe = nullptr;
	}

	if (process_buffer_w)
	{
		SmartMemFree(process_buffer_w);
		process_buffer_w = nullptr;
	}

	if (process_buffer_a)
	{
		SmartMemFree(process_buffer_a);
		process_buffer_a = nullptr;
	}

	if (process_commandline)
	{
		SmartMemFree(process_commandline);
		process_commandline = nullptr;
	}

	return result;
}

#ifndef WIN64
inline unsigned __int64 GetCycleCount()
{
	__asm   _emit   0x0F
	__asm   _emit   0x31
}
#endif // !WIN64

void PrintBytes(const char* prefix, char* bytes, int size)
{
	/*
	char* buffer = (char*)SmartMemAlloc(8);
	if (buffer)
	{
		std::string buffer_string;
		for (int i = 0; i < size; i++)
		{
			RtlZeroMemory(buffer, 8);
			if (i == 0)
			{
				SmartFormatString(buffer, "0x%02X", (BYTE)bytes[i]);
			}
			else
			{
				SmartFormatString(buffer, ", 0x%02X", (BYTE)bytes[i]);
			}

			buffer_string.append(buffer);
		}

		SmartLogInfo("%s, size = %d, bytes = %s", prefix, size, buffer_string.c_str());

		SmartMemFree(buffer);
		buffer = nullptr;
	}
	*/
}