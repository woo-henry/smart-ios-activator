#include "pch.h"
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

    return result;
}

plist_t hook_plist_new_array(void)
{
    plist_t result = trust_plist_new_array();

    return result;
}

plist_t hook_plist_new_string(const char* val)
{
    plist_t result = trust_plist_new_string(val);

    return result;
}

plist_t hook_plist_new_bool(uint8_t val)
{
    plist_t result = trust_plist_new_bool(val);

    return result;
}

plist_t hook_plist_new_uint(uint64_t val)
{
    plist_t result = trust_plist_new_uint(val);

    return result;
}

plist_t hook_plist_new_real(double val)
{
    plist_t result = trust_plist_new_real(val);

    return result;
}

plist_t hook_plist_new_data(const char* val, uint64_t length)
{
    plist_t result = trust_plist_new_data(val, length);

    return result;
}

void  hook_plist_array_new_iter(plist_t node, plist_array_iter* iter)
{
    trust_plist_array_new_iter(node, iter);
}

void  hook_plist_array_next_item(plist_t node, plist_array_iter iter, plist_t* item)
{
    trust_plist_array_next_item(node, iter, item);
}

void  hook_plist_array_append_item(plist_t node, plist_t item)
{
    trust_plist_array_append_item(node, item);
}

plist_t hook_plist_dict_get_item(plist_t node, const char* key)
{
    plist_t result = trust_plist_dict_get_item(node, key);

    return result;
}

void  hook_plist_dict_set_item(plist_t node, const char* key, plist_t item)
{
    trust_plist_dict_set_item(node, key, item);
}

void  hook_plist_get_string_val(plist_t node, char** val)
{
    trust_plist_get_string_val(node, val);
}

const char* hook_plist_get_string_ptr(plist_t node, uint64_t* length)
{
    const char* result = trust_plist_get_string_ptr(node, length);

    return result;
}

void hook_plist_get_bool_val(plist_t node, uint8_t* val)
{
    trust_plist_get_bool_val(node, val);
}

void hook_plist_get_uint_val(plist_t node, uint64_t* val)
{
    trust_plist_get_uint_val(node, val);
}

void hook_plist_get_real_val(plist_t node, double* val)
{
    trust_plist_get_real_val(node, val);
}

void hook_plist_get_data_val(plist_t node, char** val, uint64_t* length)
{
    trust_plist_get_data_val(node, val, length);
}

plist_type hook_plist_get_node_type(plist_t node)
{
    plist_type result = trust_plist_get_node_type(node);

    return result;
}

void  hook_plist_to_xml(plist_t plist, char** plist_xml, uint32_t* length)
{
    trust_plist_to_xml(plist, plist_xml, length);
}

void  hook_plist_to_bin(plist_t plist, char** plist_bin, uint32_t* length)
{
    trust_plist_to_bin(plist, plist_bin, length);
}

void  hook_plist_from_xml(const char* plist_xml, uint32_t length, plist_t* plist)
{
    trust_plist_from_xml(plist_xml, length, plist);
}

void  hook_plist_from_bin(const char* plist_bin, uint32_t length, plist_t* plist)
{
    trust_plist_from_bin(plist_bin, length, plist);
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
            MessageBox(NULL, TEXT("LoadLibrary libplist2 Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_new_dict = (pfn_plist_new_dict)GetProcAddress(libplist_module, "plist_new_dict");
        if (trust_plist_new_dict == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_new_dict Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_new_array = (pfn_plist_new_array)GetProcAddress(libplist_module, "plist_new_array");
        if (trust_plist_new_array == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_new_array Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_new_string = (pfn_plist_new_string)GetProcAddress(libplist_module, "plist_new_string");
        if (trust_plist_new_string == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_new_string Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_new_bool = (pfn_plist_new_bool)GetProcAddress(libplist_module, "plist_new_bool");
        if (trust_plist_new_bool == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_new_bool Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_new_uint = (pfn_plist_new_uint)GetProcAddress(libplist_module, "plist_new_uint");
        if (trust_plist_new_uint == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_new_uint Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_new_real = (pfn_plist_new_real)GetProcAddress(libplist_module, "plist_new_real");
        if (trust_plist_new_real == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_new_real Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_new_data = (pfn_plist_new_data)GetProcAddress(libplist_module, "plist_new_data");
        if (trust_plist_new_data == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_new_data Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_array_new_iter = (pfn_plist_array_new_iter)GetProcAddress(libplist_module, "plist_array_new_iter");
        if (trust_plist_array_new_iter == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_array_new_iter Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_array_next_item = (pfn_plist_array_next_item)GetProcAddress(libplist_module, "plist_array_next_item");
        if (trust_plist_array_next_item == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_array_next_item Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_array_append_item = (pfn_plist_array_append_item)GetProcAddress(libplist_module, "plist_array_append_item");
        if (trust_plist_array_append_item == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_array_append_item Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_dict_get_item = (pfn_plist_dict_get_item)GetProcAddress(libplist_module, "plist_dict_get_item");
        if (trust_plist_dict_get_item == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_dict_get_item Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_dict_set_item = (pfn_plist_dict_set_item)GetProcAddress(libplist_module, "plist_dict_set_item");
        if (trust_plist_dict_set_item == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_dict_set_item Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_get_string_val = (pfn_plist_get_string_val)GetProcAddress(libplist_module, "plist_get_string_val");
        if (trust_plist_get_string_val == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_get_string_val Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_get_string_ptr = (pfn_plist_get_string_ptr)GetProcAddress(libplist_module, "plist_get_string_ptr");
        if (trust_plist_get_string_ptr == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_get_string_ptr Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_get_uint_val = (pfn_plist_get_uint_val)GetProcAddress(libplist_module, "plist_get_uint_val");
        if (trust_plist_get_uint_val == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_get_uint_val Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_get_real_val = (pfn_plist_get_real_val)GetProcAddress(libplist_module, "plist_get_real_val");
        if (trust_plist_get_real_val == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_get_real_val Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_get_data_val = (pfn_plist_get_data_val)GetProcAddress(libplist_module, "plist_get_data_val");
        if (trust_plist_get_data_val == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_get_data_val Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_get_node_type = (pfn_plist_get_node_type)GetProcAddress(libplist_module, "plist_get_node_type");
        if (trust_plist_get_node_type == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_get_node_type Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_to_xml = (pfn_plist_to_xml)GetProcAddress(libplist_module, "plist_to_xml");
        if (trust_plist_to_xml == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_to_xml Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_to_bin = (pfn_plist_to_bin)GetProcAddress(libplist_module, "plist_to_bin");
        if (trust_plist_to_bin == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_to_bin Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_from_xml = (pfn_plist_from_xml)GetProcAddress(libplist_module, "plist_from_xml");
        if (trust_plist_from_xml == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_from_xml Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_from_bin = (pfn_plist_from_bin)GetProcAddress(libplist_module, "plist_from_bin");
        if (trust_plist_from_bin == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_from_bin Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_copy = (pfn_plist_copy)GetProcAddress(libplist_module, "plist_copy");
        if (trust_plist_copy == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_copy Error"), TEXT("Hook"), MB_OK);
            result = GetLastError();
            break;
        }

        trust_plist_free = (pfn_plist_free)GetProcAddress(libplist_module, "plist_free");
        if (trust_plist_free == nullptr)
        {
            MessageBox(NULL, TEXT("GetProcAddress plist_free Error"), TEXT("Hook"), MB_OK);
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