#include "Debug.h"

void debug::PrintFileName(PFLT_CALLBACK_DATA data)
{
    WCHAR name[MAX_SIZE] = { 0 };
    int size = MAX_SIZE;

    PFLT_FILE_NAME_INFORMATION file_name_info;
    NTSTATUS status;

    if (data == nullptr || name == nullptr)
    {
        return;
    }
    if (data->Iopb != NULL && data->Iopb->TargetFileObject != NULL)
    {
        PWCH buffer = data->Iopb->TargetFileObject->FileName.Buffer;
        USHORT max_length = data->Iopb->TargetFileObject->FileName.MaximumLength;
        USHORT length = data->Iopb->TargetFileObject->FileName.Length;
        if (buffer != NULL && max_length < size && length > 0)
        {
            RtlCopyMemory(name, buffer, max_length);
            DebugMessage("%ws", name);
            return;
        }
    }

    status = FltGetFileNameInformation(data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &file_name_info);

    if (NT_SUCCESS(status))
    {
        status = FltParseFileNameInformation(file_name_info);

        if (NT_SUCCESS(status))
        {
            if (file_name_info->Name.MaximumLength < size)
            {
                RtlCopyMemory(name, file_name_info->Name.Buffer, file_name_info->Name.MaximumLength);
                DebugMessage("%ws", name);
            }
        }
        FltReleaseFileNameInformation(file_name_info);
    }

    return;
}

void debug::PrintWstring(PWCHAR str)
{
    if (str == NULL)
    {
        return;
    }
    DebugMessage("%ws", str);
}

void debug::PrintWstring(PWCHAR str, int size)
{
    WCHAR s[MAX_SIZE];
    if (str == NULL)
    {
        return;
    }
    if (size > MAX_SIZE)
    {
        return;
    }
    RtlZeroMemory(s, MAX_SIZE * sizeof(WCHAR));
    RtlCopyMemory(s, str, size);
    PrintWstring(s);
    
}

void debug::PrintUnicode(PUNICODE_STRING uni_str)
{
    if (uni_str == NULL || uni_str->Buffer == NULL)
    {
        return;
    }
    WCHAR str[MAX_SIZE] = { 0 };
    if (uni_str->MaximumLength < uni_str->Length)
    {
        return;
    }
    int size = min(uni_str->Length, MAX_SIZE);
    if (size > MAX_SIZE)
    {
        return;
    }
    RtlCopyMemory(str, uni_str->Buffer, uni_str->Length);
    PrintWstring(str);
}
