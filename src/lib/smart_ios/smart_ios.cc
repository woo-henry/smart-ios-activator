#include <smart_base.h>
#include <smart_ios.h>
#include <mimalloc-new-delete.h>
#include <device/ios_device_control.h>

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(module);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}
//////////////////////////////////////////////////////////////////////////
iOSDeviceControl* ios_device_control = nullptr;

int InitiOSDeviceEnviroment(void* context, iOSDeviceCallbacks* callbacks)
{
    int result = IOS_ERROR_SUCCESS;

    do
    {
        ios_device_control = new iOSDeviceControl(context, callbacks);
        if (ios_device_control == nullptr)
        {
            result = IOS_ERROR_OBJECT_NOT_CREATED;
            break;
        }

        result = ios_device_control->Init("license.dat", "SmartiOSActivator");

    } while (false);

    return result;
}

int IOS_API ShutdowniOSDeviceEnviroment()
{
    int result = IOS_ERROR_SUCCESS;

    do
    {
        if (ios_device_control == nullptr)
        {
            result = IOS_ERROR_OBJECT_IS_EMPTY;
            break;
        }

        ios_device_control->Dispose();
        ios_device_control = nullptr;

    } while (false);

    return result;
}

int IOS_API QueryiOSDevice(const char* device_id)
{
    int result = IOS_ERROR_SUCCESS;

    do
    {
        if (ios_device_control == nullptr)
        {
            result = IOS_ERROR_OBJECT_IS_EMPTY;
            break;
        }

        result = ios_device_control->QueryDevice(device_id);

    } while (false);

    return result;
}

int IOS_API ActivateiOSDevice(const char* device_id, bool skip_install_setup)
{
    int result = IOS_ERROR_SUCCESS;

    do
    {
        if (ios_device_control == nullptr)
        {
            result = IOS_ERROR_OBJECT_IS_EMPTY;
            break;
        }

        result = ios_device_control->ActivateDevice(device_id, skip_install_setup);

    } while (false);

    return result;
}

int IOS_API DeactivateiOSDevice(const char* device_id)
{
    int result = IOS_ERROR_SUCCESS;

    do
    {
        if (ios_device_control == nullptr)
        {
            result = IOS_ERROR_OBJECT_IS_EMPTY;
            break;
        }

        result = ios_device_control->DeactivateDevice(device_id);

    } while (false);

    return result;
}

int IOS_API QueryiOSDeviceState(const char* device_id, bool* activated)
{
    int result = IOS_ERROR_SUCCESS;

    do
    {
        if (ios_device_control == nullptr)
        {
            result = IOS_ERROR_OBJECT_IS_EMPTY;
            break;
        }

        result = ios_device_control->QueryDeviceState(device_id, activated);

    } while (false);

    return result;
}