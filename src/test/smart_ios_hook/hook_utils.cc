#include "pch.h"
#include "hook_utils.h"

void PrintBytes(const char* prefix, char* bytes, uint32_t length)
{
    char* buffer = (char*)SmartMemAlloc(8);
    if (buffer)
    {
        std::string buffer_string;
        for (uint32_t i = 0; i < length; i++)
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

        SmartLogInfo("%s, bytes = %s", prefix, buffer_string.c_str());

        SmartMemFree(buffer);
        buffer = nullptr;
    }
}