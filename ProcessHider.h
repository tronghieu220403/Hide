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
    public:
        static NTSTATUS Register();

        static void HideOnInitializeOperation();

        static void HideOnCreateOperation(HANDLE ppid, HANDLE pid, BOOLEAN create);

        static void Unload();

        static void HideProcess(HANDLE pid);
    };
}
