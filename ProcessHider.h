#pragma once

#include <fltKernel.h>
#include <dontuse.h>

#include "Debug.h"
#include "Ulti.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

namespace filter
{

    class ProcessHider
    {
    private:
        static inline KGUARDED_MUTEX process_lock_ = { 0 };
    public:
        static NTSTATUS Register();

        static void HideOnInitializeOperation();

        static void HideOnCreateOperation(P_CUSTOM_EPROCESS peprocess, HANDLE pid, PPS_CREATE_NOTIFY_INFO create_info);

        static void Unload();

        static bool IsProcessInHiddenList(PWCHAR process_name);

        static bool HideProcess(P_CUSTOM_EPROCESS peprocess);
    };
}
