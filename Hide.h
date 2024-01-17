#pragma once

#include "Ulti.h"
#include "Debug.h"
#include "FileFilter.h"
#include "ProcessHider.h"

#pragma warning( disable : 4083 4024 4047 4702)

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

PDRIVER_OBJECT gDriverObject = { 0 };
PFLT_FILTER gFilterHandle = { 0 };

EXTERN_C_START

NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path);

NTSTATUS FilterUnload(FLT_FILTER_UNLOAD_FLAGS flags);

NTSTATUS DriverUnload(PDRIVER_OBJECT driver_object);

EXTERN_C_END

//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, FilterUnload)
#endif