#pragma once

#include <fltKernel.h>
#include <dontuse.h>

#include "Ulti.h"
#include "Debug.h"

#pragma warning( disable : 4083 4024 4047 4702)

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

EXTERN_C_START

extern NTSTATUS FilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags);

EXTERN_C_END

namespace filter
{

    class FileFilter
    {
    private:
        // inline static WCHAR file_to_hide_[500] = { 0 };
        inline static PWCHAR file_to_hide_ = (PWCHAR)L"ggez.txt";
        inline static PDRIVER_OBJECT p_driver_object_ = { 0 };
        inline static PFLT_FILTER g_filter_handle_ = { 0 };
        static const FLT_OPERATION_REGISTRATION callbacks_[]; //  operation registration
        static const FLT_CONTEXT_REGISTRATION contexts_[]; //  operation registration
        static const FLT_REGISTRATION filter_registration_; //  This defines what we want to filter with FltMgr

    public:

        static NTSTATUS Register();

        static FLT_PREOP_CALLBACK_STATUS PreDirControlOperation(
            PFLT_CALLBACK_DATA data,
            PCFLT_RELATED_OBJECTS flt_objects,
            PVOID* completion_context
        );

        static FLT_POSTOP_CALLBACK_STATUS PostDirControlOperation(
            PFLT_CALLBACK_DATA data,
            PCFLT_RELATED_OBJECTS flt_objects,
            PVOID completion_context,
            FLT_POST_OPERATION_FLAGS flags
        );

        static NTSTATUS HideFile(PUCHAR info, PUCHAR nextEntryOffset, PUCHAR fileNameOffset, PUCHAR fileNameLengthOffset, PUCHAR info_address);

        static NTSTATUS Unload();

        static bool IsMyFolder(PFLT_CALLBACK_DATA data, PWCHAR my_folder);
        static void GetFileName(PFLT_CALLBACK_DATA data, PWCHAR name, int size);
        static void PrintCurFileName(PFLT_CALLBACK_DATA data);

        static void SetDriverObjectPtr(const PDRIVER_OBJECT p_driver_object);
        static PDRIVER_OBJECT GetDriverObjectPtr();

        static void SetFilterHandle(const PFLT_FILTER g_filter_handle);
        static PFLT_FILTER GetFilterHandle();

    };


}

