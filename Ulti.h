#pragma once

#include <fltKernel.h>
#include <dontuse.h>
#include <Ntstrsafe.h>
#include <Wdm.h>

#define MAX_SIZE 500

namespace ulti
{
	int MergeString(PWCHAR des, int size, PWCHAR src1, PWCHAR src2);
	
	int GetWstringSize(PWCHAR str);

    BOOLEAN CheckSubstring(PWCHAR string, PWCHAR sub_string);

    void SetUlongAt(long long addr, ULONG value);
}


struct _CUSTOM_EPROCESS
{
    UCHAR Pcb[0x438];                                                       //0x0
    UCHAR ProcessLock[0x8];                                                 //0x438
    VOID* UniqueProcesId;                                                   //0x440
    struct _LIST_ENTRY ActiveProcessLinks;                                  //0x448;
    UCHAR padding[0x160];
    UCHAR ImageFileName[15];                                                //0x5a8

};
typedef _CUSTOM_EPROCESS CUSTOM_EPROCESS, *P_CUSTOM_EPROCESS;