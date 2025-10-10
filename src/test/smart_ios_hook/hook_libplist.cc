#include "pch.h"
#include "hook_log.h"
#include "hook_utils.h"
#include "hook_libplist.h"

typedef plist_t (*pfn_plist_new_dict)(void);
typedef plist_t (*pfn_plist_new_array)(void);
typedef plist_t (*pfn_plist_new_string)(const char* val);
typedef plist_t (*pfn_plist_new_bool)(uint8_t val);
typedef plist_t (*pfn_plist_new_uint)(uint64_t val);
typedef plist_t (*pfn_plist_new_real)(double val);
typedef plist_t (*pfn_plist_new_data)(const char* val, uint64_t length);
typedef void (*pfn_plist_array_new_iter)(plist_t node, plist_array_iter* iter);
typedef void (*pfn_plist_array_next_item)(plist_t node, plist_array_iter iter, plist_t* item);
typedef void (*pfn_plist_array_append_item)(plist_t node, plist_t item);
typedef plist_t(*pfn_plist_dict_get_item)(plist_t node, const char* key);
typedef void (*pfn_plist_dict_set_item)(plist_t node, const char* key, plist_t item);
typedef void (*pfn_plist_get_string_val)(plist_t node, char** val);
typedef const char* (*pfn_plist_get_string_ptr)(plist_t node, uint64_t* length);
typedef void (*pfn_plist_get_bool_val)(plist_t node, uint8_t* val);
typedef void (*pfn_plist_get_uint_val)(plist_t node, uint64_t* val);
typedef void (*pfn_plist_get_real_val)(plist_t node, double* val);
typedef void (*pfn_plist_get_data_val)(plist_t node, char** val, uint64_t* length);
typedef plist_type(*pfn_plist_get_node_type)(plist_t node);
typedef void (*pfn_plist_to_xml)(plist_t plist, char** plist_xml, uint32_t* length);
typedef void (*pfn_plist_to_bin)(plist_t plist, char** plist_bin, uint32_t* length);
typedef void (*pfn_plist_from_xml)(const char* plist_xml, uint32_t length, plist_t* plist);
typedef void (*pfn_plist_from_bin)(const char* plist_bin, uint32_t length, plist_t* plist);
typedef plist_t(*pfn_plist_copy)(plist_t node);
typedef void (*pfn_plist_free)(plist_t plist);

static pfn_plist_new_dict trust_plist_new_dict = nullptr;
static pfn_plist_new_array trust_plist_new_array = nullptr;
static pfn_plist_new_string trust_plist_new_string = nullptr;
static pfn_plist_new_bool trust_plist_new_bool = nullptr;
static pfn_plist_new_uint trust_plist_new_uint = nullptr;
static pfn_plist_new_real trust_plist_new_real = nullptr;
static pfn_plist_new_data trust_plist_new_data = nullptr;
static pfn_plist_array_new_iter trust_plist_array_new_iter = nullptr;
static pfn_plist_array_next_item trust_plist_array_next_item = nullptr;
static pfn_plist_array_append_item trust_plist_array_append_item = nullptr;
static pfn_plist_dict_get_item trust_plist_dict_get_item = nullptr;
static pfn_plist_dict_set_item trust_plist_dict_set_item = nullptr;
static pfn_plist_get_string_val trust_plist_get_string_val = nullptr;
static pfn_plist_get_string_ptr trust_plist_get_string_ptr = nullptr;
static pfn_plist_get_bool_val trust_plist_get_bool_val = nullptr;
static pfn_plist_get_uint_val trust_plist_get_uint_val = nullptr;
static pfn_plist_get_real_val trust_plist_get_real_val = nullptr;
static pfn_plist_get_data_val trust_plist_get_data_val = nullptr;
static pfn_plist_get_node_type trust_plist_get_node_type = nullptr;
static pfn_plist_to_xml trust_plist_to_xml = nullptr;
static pfn_plist_to_bin trust_plist_to_bin = nullptr;
static pfn_plist_from_xml trust_plist_from_xml = nullptr;
static pfn_plist_from_bin trust_plist_from_bin = nullptr;
static pfn_plist_copy trust_plist_copy = nullptr;
static pfn_plist_free trust_plist_free = nullptr;

plist_t hook_plist_new_dict(void)
{
    plist_t result = trust_plist_new_dict();

    SmartLogInfo("hook_plist_new_dict, thread = %d, result = %p",
        GetCurrentThreadId(),  result);

    return result;
}

plist_t hook_plist_new_array(void)
{
    plist_t result = trust_plist_new_array();

    SmartLogInfo("hook_plist_new_array, thread = %d, result = %p",
        GetCurrentThreadId(), result);

    return result;
}

plist_t hook_plist_new_string(const char* val)
{
    plist_t result = trust_plist_new_string(val);

    SmartLogInfo("hook_plist_new_string, thread = %d, val = %s, result = %p",
        GetCurrentThreadId(), val, result);

    return result;
}

plist_t hook_plist_new_bool(uint8_t val)
{
    plist_t result = trust_plist_new_bool(val);

    SmartLogInfo("hook_plist_new_bool, thread = %d, val = %d, result = %p",
        GetCurrentThreadId(), val, result);

    return result;
}

plist_t hook_plist_new_uint(uint64_t val)
{
    plist_t result = trust_plist_new_uint(val);

    SmartLogInfo("hook_plist_new_uint, thread = %d, val = %ld, result = %p",
        GetCurrentThreadId(), val, result);

    return result;
}

plist_t hook_plist_new_real(double val)
{
    plist_t result = trust_plist_new_real(val);

    SmartLogInfo("hook_plist_new_real, thread = %d, val = %f, result = %p",
        GetCurrentThreadId(), val, result);

    return result;
}

plist_t hook_plist_new_data(const char* val, uint64_t length)
{
    plist_t result = trust_plist_new_data(val, length);

    SmartLogInfo("hook_plist_new_data, thread = %d, val = %s, length = %ld, result = %p",
        GetCurrentThreadId(), val, result);

    return result;
}

void  hook_plist_array_new_iter(plist_t node, plist_array_iter* iter)
{
    trust_plist_array_new_iter(node, iter);

    SmartLogInfo("hook_plist_array_new_iter, thread = %d, node = %p, iter = %p",
        GetCurrentThreadId(), node, iter);
}

void  hook_plist_array_next_item(plist_t node, plist_array_iter iter, plist_t* item)
{
    trust_plist_array_next_item(node, iter, item);

    SmartLogInfo("trust_plist_array_next_item, thread = %d, node = %p, iter = %p, item = %p",
        GetCurrentThreadId(), node, iter, item);
}

void  hook_plist_array_append_item(plist_t node, plist_t item)
{
    trust_plist_array_append_item(node, item);

    SmartLogInfo("trust_plist_array_append_item, thread = %d, node = %p, item = %p",
        GetCurrentThreadId(), node, item);
}

plist_t hook_plist_dict_get_item(plist_t node, const char* key)
{
    plist_t result = trust_plist_dict_get_item(node, key);

    SmartLogInfo("hook_plist_dict_get_item, thread = %d, node = %p, key = %s, result = %p",
        GetCurrentThreadId(), node, key, result);

    return result;
}

void  hook_plist_dict_set_item(plist_t node, const char* key, plist_t item)
{
    trust_plist_dict_set_item(node, key, item);

    SmartLogInfo("hook_plist_dict_set_item, thread = %d, node = %p, key = %s, item = %p",
        GetCurrentThreadId(), node, key, item);
}

void  hook_plist_get_string_val(plist_t node, char** val)
{
    trust_plist_get_string_val(node, val);

    SmartLogInfo("hook_plist_get_string_val, thread = %d, node = %p, val = %s",
        GetCurrentThreadId(), node, val);
}

const char* hook_plist_get_string_ptr(plist_t node, uint64_t* length)
{
    const char* result = trust_plist_get_string_ptr(node, length);

    SmartLogInfo("hook_plist_get_string_ptr, thread = %d, node = %p, length = %p, result = %s",
        GetCurrentThreadId(), node, length, result);

    return result;
}

void hook_plist_get_bool_val(plist_t node, uint8_t* val)
{
    trust_plist_get_bool_val(node, val);

    SmartLogInfo("hook_plist_get_bool_val, thread = %d, node = %p, val = %d",
        GetCurrentThreadId(), node, *val);
}

void hook_plist_get_uint_val(plist_t node, uint64_t* val)
{
    trust_plist_get_uint_val(node, val);

    SmartLogInfo("hook_plist_get_uint_val, thread = %d, node = %p, val = %d",
        GetCurrentThreadId(), node, *val);
}

void hook_plist_get_real_val(plist_t node, double* val)
{
    trust_plist_get_real_val(node, val);

    SmartLogInfo("hook_plist_get_real_val, thread = %d, node = %p, val = %f",
        GetCurrentThreadId(), node, *val);
}

void hook_plist_get_data_val(plist_t node, char** val, uint64_t* length)
{
    trust_plist_get_data_val(node, val, length);

    SmartLogInfo("hook_plist_get_data_val, thread = %d, node = %p, val = %s, length = %d",
        GetCurrentThreadId(), node, *val, *length);
}

plist_type hook_plist_get_node_type(plist_t node)
{
    plist_type result = trust_plist_get_node_type(node);

    SmartLogInfo("hook_plist_get_node_type, thread = %d, node = %p, result = %d",
        GetCurrentThreadId(), node, result);

    return result;
}

void  hook_plist_to_xml(plist_t plist, char** plist_xml, uint32_t* length)
{
    trust_plist_to_xml(plist, plist_xml, length);

    SmartLogInfo("hook_plist_to_xml, thread = %d, plist = %p, plist_xml = %s, length = %d",
        GetCurrentThreadId(), plist, *plist_xml, *length);
}

void  hook_plist_to_bin(plist_t plist, char** plist_bin, uint32_t* length)
{
    trust_plist_to_bin(plist, plist_bin, length);

    SmartLogInfo("hook_plist_to_bin, thread = %d, plist = %p, plist_bin = %s, length = %d",
        GetCurrentThreadId(), plist, *plist_bin, *length);
}

void  hook_plist_from_xml(const char* plist_xml, uint32_t length, plist_t* plist)
{
    trust_plist_from_xml(plist_xml, length, plist);

    SmartLogInfo("hook_plist_from_xml, thread = %d, plist_xml = %s, length = %d, plist = %p",
        GetCurrentThreadId(), plist_xml, length, plist);
}

void  hook_plist_from_bin(const char* plist_bin, uint32_t length, plist_t* plist)
{
    trust_plist_from_bin(plist_bin, length, plist);

    SmartLogInfo("hook_plist_from_bin, thread = %d, plist_bin = %s, length = %d, plist = %p",
        GetCurrentThreadId(), plist_bin, length, plist);
}

plist_t hook_plist_copy(plist_t node)
{
    plist_t result = trust_plist_copy(node);

    return result;
}

void  hook_plist_free(plist_t plist)
{
    trust_plist_free(plist);
}

LONG StartHookLibplist()
{
    LONG result = ERROR_SUCCESS;
    HMODULE libplist_module = nullptr;

    do
    {
        libplist_module = GetModuleHandle(TEXT("libplist2.dll"));
        if (libplist_module == nullptr)
        {
            MessageBox(nullptr, TEXT("LoadLibrary libplist2 Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_new_dict = (pfn_plist_new_dict)GetProcAddress(libplist_module, "plist_new_dict");
        if (trust_plist_new_dict == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_new_dict Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_new_array = (pfn_plist_new_array)GetProcAddress(libplist_module, "plist_new_array");
        if (trust_plist_new_array == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_new_array Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_new_string = (pfn_plist_new_string)GetProcAddress(libplist_module, "plist_new_string");
        if (trust_plist_new_string == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_new_string Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_new_bool = (pfn_plist_new_bool)GetProcAddress(libplist_module, "plist_new_bool");
        if (trust_plist_new_bool == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_new_bool Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_new_uint = (pfn_plist_new_uint)GetProcAddress(libplist_module, "plist_new_uint");
        if (trust_plist_new_uint == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_new_uint Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_new_real = (pfn_plist_new_real)GetProcAddress(libplist_module, "plist_new_real");
        if (trust_plist_new_real == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_new_real Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_new_data = (pfn_plist_new_data)GetProcAddress(libplist_module, "plist_new_data");
        if (trust_plist_new_data == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_new_data Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_array_new_iter = (pfn_plist_array_new_iter)GetProcAddress(libplist_module, "plist_array_new_iter");
        if (trust_plist_array_new_iter == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_array_new_iter Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_array_next_item = (pfn_plist_array_next_item)GetProcAddress(libplist_module, "plist_array_next_item");
        if (trust_plist_array_next_item == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_array_next_item Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_array_append_item = (pfn_plist_array_append_item)GetProcAddress(libplist_module, "plist_array_append_item");
        if (trust_plist_array_append_item == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_array_append_item Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_dict_get_item = (pfn_plist_dict_get_item)GetProcAddress(libplist_module, "plist_dict_get_item");
        if (trust_plist_dict_get_item == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_dict_get_item Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_dict_set_item = (pfn_plist_dict_set_item)GetProcAddress(libplist_module, "plist_dict_set_item");
        if (trust_plist_dict_set_item == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_dict_set_item Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_get_string_val = (pfn_plist_get_string_val)GetProcAddress(libplist_module, "plist_get_string_val");
        if (trust_plist_get_string_val == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_get_string_val Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_get_string_ptr = (pfn_plist_get_string_ptr)GetProcAddress(libplist_module, "plist_get_string_ptr");
        if (trust_plist_get_string_ptr == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_get_string_ptr Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_get_bool_val = (pfn_plist_get_bool_val)GetProcAddress(libplist_module, "plist_get_bool_val");
        if (trust_plist_get_bool_val == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_get_bool_val Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_get_uint_val = (pfn_plist_get_uint_val)GetProcAddress(libplist_module, "plist_get_uint_val");
        if (trust_plist_get_uint_val == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_get_uint_val Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_get_real_val = (pfn_plist_get_real_val)GetProcAddress(libplist_module, "plist_get_real_val");
        if (trust_plist_get_real_val == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_get_real_val Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_get_data_val = (pfn_plist_get_data_val)GetProcAddress(libplist_module, "plist_get_data_val");
        if (trust_plist_get_data_val == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_get_data_val Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_get_node_type = (pfn_plist_get_node_type)GetProcAddress(libplist_module, "plist_get_node_type");
        if (trust_plist_get_node_type == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_get_node_type Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_to_xml = (pfn_plist_to_xml)GetProcAddress(libplist_module, "plist_to_xml");
        if (trust_plist_to_xml == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_to_xml Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_to_bin = (pfn_plist_to_bin)GetProcAddress(libplist_module, "plist_to_bin");
        if (trust_plist_to_bin == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_to_bin Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_from_xml = (pfn_plist_from_xml)GetProcAddress(libplist_module, "plist_from_xml");
        if (trust_plist_from_xml == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_from_xml Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_from_bin = (pfn_plist_from_bin)GetProcAddress(libplist_module, "plist_from_bin");
        if (trust_plist_from_bin == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_from_bin Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_copy = (pfn_plist_copy)GetProcAddress(libplist_module, "plist_copy");
        if (trust_plist_copy == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_copy Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_free = (pfn_plist_free)GetProcAddress(libplist_module, "plist_free");
        if (trust_plist_free == nullptr)
        {
            MessageBox(nullptr, TEXT("GetProcAddress plist_free Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        result = DetourAttach(&(PVOID&)trust_plist_new_dict, hook_plist_new_dict);
        SmartLogInfo("DetourAttach hook_plist_new_dict : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_new_array, hook_plist_new_array);
        SmartLogInfo("DetourAttach hook_plist_new_array : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_new_string, hook_plist_new_string);
        SmartLogInfo("DetourAttach hook_plist_new_string : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_new_bool, hook_plist_new_bool);
        SmartLogInfo("DetourAttach hook_plist_new_bool : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_new_uint, hook_plist_new_uint);
        SmartLogInfo("DetourAttach hook_plist_new_uint : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_new_real, hook_plist_new_real);
        SmartLogInfo("DetourAttach hook_plist_new_real : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_new_data, hook_plist_new_data);
        SmartLogInfo("DetourAttach hook_plist_new_data : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_array_new_iter, hook_plist_array_new_iter);
        SmartLogInfo("DetourAttach hook_plist_array_new_iter : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_array_next_item, hook_plist_array_next_item);
        SmartLogInfo("DetourAttach hook_plist_array_next_item : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_array_append_item, hook_plist_array_append_item);
        SmartLogInfo("DetourAttach hook_plist_array_append_item : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_dict_get_item, hook_plist_dict_get_item);
        SmartLogInfo("DetourAttach hook_plist_dict_get_item : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_dict_set_item, hook_plist_dict_set_item);
        SmartLogInfo("DetourAttach hook_plist_dict_set_item : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_get_string_val, hook_plist_get_string_val);
        SmartLogInfo("DetourAttach hook_plist_get_string_val : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_get_string_ptr, hook_plist_get_string_ptr);
        SmartLogInfo("DetourAttach hook_plist_get_string_ptr : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_get_bool_val, hook_plist_get_bool_val);
        SmartLogInfo("DetourAttach hook_plist_get_bool_val : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_get_real_val, hook_plist_get_real_val);
        SmartLogInfo("DetourAttach hook_plist_get_real_val : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_get_data_val, hook_plist_get_data_val);
        SmartLogInfo("DetourAttach hook_plist_get_data_val : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_get_node_type, hook_plist_get_node_type);
        SmartLogInfo("DetourAttach hook_plist_get_node_type : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_to_xml, hook_plist_to_xml);
        SmartLogInfo("DetourAttach hook_plist_to_xml : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_to_bin, hook_plist_to_bin);
        SmartLogInfo("DetourAttach hook_plist_to_bin : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_from_xml, hook_plist_from_xml);
        SmartLogInfo("DetourAttach hook_plist_from_xml : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_from_bin, hook_plist_from_bin);
        SmartLogInfo("DetourAttach hook_plist_from_bin : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_copy, hook_plist_copy);
        SmartLogInfo("DetourAttach hook_plist_copy : %d", result);

        result = DetourAttach(&(PVOID&)trust_plist_free, hook_plist_free);
        SmartLogInfo("DetourAttach hook_plist_free : %d", result);

    } while (false);

    return result;
}

LONG FinishHookLibplist()
{
    LONG result = ERROR_SUCCESS;

    do
    {
        result = DetourDetach(&(PVOID&)trust_plist_new_dict, hook_plist_new_dict);
        SmartLogInfo("DetourDetach hook_plist_new_dict : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_new_array, hook_plist_new_array);
        SmartLogInfo("DetourDetach hook_plist_new_array : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_new_string, hook_plist_new_string);
        SmartLogInfo("DetourDetach hook_plist_new_string : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_new_bool, hook_plist_new_bool);
        SmartLogInfo("DetourDetach hook_plist_new_bool : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_new_uint, hook_plist_new_uint);
        SmartLogInfo("DetourDetach hook_plist_new_uint : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_new_real, hook_plist_new_real);
        SmartLogInfo("DetourDetach hook_plist_new_real : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_new_data, hook_plist_new_data);
        SmartLogInfo("DetourDetach hook_plist_new_data : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_array_new_iter, hook_plist_array_new_iter);
        SmartLogInfo("DetourDetach hook_plist_array_new_iter : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_array_next_item, hook_plist_array_next_item);
        SmartLogInfo("DetourDetach hook_plist_array_next_item : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_array_append_item, hook_plist_array_append_item);
        SmartLogInfo("DetourDetach hook_plist_array_append_item : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_dict_get_item, hook_plist_dict_get_item);
        SmartLogInfo("DetourDetach hook_plist_dict_get_item : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_dict_set_item, hook_plist_dict_set_item);
        SmartLogInfo("DetourDetach hook_plist_dict_set_item : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_get_string_val, hook_plist_get_string_val);
        SmartLogInfo("DetourDetach hook_plist_get_string_val : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_get_string_ptr, hook_plist_get_string_ptr);
        SmartLogInfo("DetourDetach hook_plist_get_string_ptr : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_get_bool_val, hook_plist_get_bool_val);
        SmartLogInfo("DetourDetach hook_plist_get_bool_val : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_get_real_val, hook_plist_get_real_val);
        SmartLogInfo("DetourDetach hook_plist_get_real_val : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_get_data_val, hook_plist_get_data_val);
        SmartLogInfo("DetourDetach hook_plist_get_data_val : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_get_node_type, hook_plist_get_node_type);
        SmartLogInfo("DetourDetach hook_plist_get_node_type : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_to_xml, hook_plist_to_xml);
        SmartLogInfo("DetourDetach hook_plist_to_xml : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_to_bin, hook_plist_to_bin);
        SmartLogInfo("DetourDetach hook_plist_to_bin : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_from_xml, hook_plist_from_xml);
        SmartLogInfo("DetourDetach hook_plist_from_xml : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_from_bin, hook_plist_from_bin);
        SmartLogInfo("DetourDetach hook_plist_from_bin : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_copy, hook_plist_copy);
        SmartLogInfo("DetourDetach hook_plist_copy : %d", result);

        result = DetourDetach(&(PVOID&)trust_plist_free, hook_plist_free);
        SmartLogInfo("DetourDetach hook_plist_free : %d", result);

    } while (false);

    return result;
}