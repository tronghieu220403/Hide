#include "Hide.h"

NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path)
{
    NTSTATUS status = STATUS_SUCCESS;

    // ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

    UNREFERENCED_PARAMETER(registry_path);
    DebugMessage("PFMinifilter!DriverEntry: Entered\n");

    driver_object->DriverUnload = (PDRIVER_UNLOAD)DriverUnload;

    //
    //  Register with FltMgr to tell it our callback routines
    //
    filter::FileFilter::SetDriverObjectPtr(driver_object);
    status = filter::FileFilter::Register();


    if (!NT_SUCCESS(status))
    {
        DebugMessage("FileFilter: Register not successfull\n");
        return STATUS_UNSUCCESSFUL;
    }

    status = filter::ProcessHider::Register();

    if (!NT_SUCCESS(status))
    {
        DebugMessage("ProcessFilter: Register not successfull\n");
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;

}

NTSTATUS FilterUnload(FLT_FILTER_UNLOAD_FLAGS flags)
{
    UNREFERENCED_PARAMETER(flags);

    // File Filter Unload must always at the last of the Unload rountine
    filter::FileFilter::Unload();

    return STATUS_SUCCESS;
}

NTSTATUS DriverUnload(PDRIVER_OBJECT driver_object)
{
    UNREFERENCED_PARAMETER(driver_object);
    filter::ProcessHider::Unload();

    return STATUS_SUCCESS;
}