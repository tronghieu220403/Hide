#pragma once

#define DebugMessage(x, ...) DbgPrintEx(0, 0, x, __VA_ARGS__)

#include <fltKernel.h>
#include <dontuse.h>

#include "Ulti.h"

#pragma warning( disable : 4083 4024 4047 4702 4189 4101)


namespace debug
{
	void PrintFileName(PFLT_CALLBACK_DATA data);

	void PrintWstring(PWCHAR str);

	void PrintWstring(PWCHAR str, int size);

	void PrintUnicode(PUNICODE_STRING uni_str);

}