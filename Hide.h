#pragma once

#include "Ulti.h"
#include "Debug.h"
#include "FileFilter.h"
#include "ProcessHider.h"

#pragma warning( disable : 4083 4024 4047 4702)

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

UNICODE_STRING DEVICE_NAME = RTL_CONSTANT_STRING(L"\\Device\\mydevice123");
UNICODE_STRING DEVICE_SYMBOLIC_NAME = RTL_CONSTANT_STRING(L"\\??\\mydevice123");

#define IOCTL_HIEU CTL_CODE(FILE_DEVICE_UNKNOWN, 0x2204, METHOD_BUFFERED, FILE_ANY_ACCESS)

EXTERN_C_START

NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path);

NTSTATUS FilterUnload(FLT_FILTER_UNLOAD_FLAGS flags);

NTSTATUS DriverUnload(PDRIVER_OBJECT driver_object);

NTSTATUS HandleCustomIOCTL(PDEVICE_OBJECT device_object, PIRP irp);

NTSTATUS MajorFunctions(PDEVICE_OBJECT device_object, PIRP irp);

EXTERN_C_END

//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, FilterUnload)
#endif