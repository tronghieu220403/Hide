#include "FileFilter.h"

namespace filter
{
    NTSTATUS FileFilter::Register()
    {
        // DebugMessage("FileFilter registering hihi\n");

        KeInitializeGuardedMutex(&file_name_lock_);

        NTSTATUS status = FltRegisterFilter(p_driver_object_,
            &filter_registration_,
            &g_filter_handle_);

        if (!NT_SUCCESS(status))
        {
            switch (status)
            {
            case STATUS_INSUFFICIENT_RESOURCES:
                DebugMessage("STATUS_INSUFFICIENT_RESOURCES\n");
                break;
            case STATUS_INVALID_PARAMETER:
                DebugMessage("STATUS_INVALID_PARAMETER\n");
                break;
            case STATUS_FLT_NOT_INITIALIZED:
                DebugMessage("STATUS_FLT_NOT_INITIALIZED\n");
                break;
            case STATUS_OBJECT_NAME_NOT_FOUND:
                DebugMessage("STATUS_OBJECT_NAME_NOT_FOUND\n");
                break;
            default:
                break;
            }
        }
        else
        {
            DebugMessage("Reg Oke");
        }

        FLT_ASSERT(NT_SUCCESS(status));

        if (NT_SUCCESS(status)) {

            status = FltStartFiltering(g_filter_handle_);

            if (!NT_SUCCESS(status)) {

                FileFilter::Unload();
            }
        }

        return status;
    }

    FLT_PREOP_CALLBACK_STATUS FileFilter::PreDirControlOperation(PFLT_CALLBACK_DATA data, PCFLT_RELATED_OBJECTS flt_objects, PVOID* completion_context)
    {
        UNREFERENCED_PARAMETER(data);
        UNREFERENCED_PARAMETER(flt_objects);
        UNREFERENCED_PARAMETER(completion_context);
        return FLT_PREOP_SUCCESS_WITH_CALLBACK;
    }

    void PrintFileInfoClass(FILE_INFORMATION_CLASS FileInformationClass)
    {
        switch (FileInformationClass)
        {
        case FileBothDirectoryInformation:
            DebugMessage("FILE_BOTH_DIR_INFORMATION");
            break;
        case FileDirectoryInformation:
            DebugMessage("FILE_DIRECTORY_INFORMATION");
            break;
        case FileFullDirectoryInformation:
            DebugMessage("FILE_FULL_DIR_INFORMATION");
            break;
        case FileIdBothDirectoryInformation:
            DebugMessage("FILE_ID_BOTH_DIR_INFORMATION");
            break;
        case FileIdFullDirectoryInformation:
            DebugMessage("FILE_ID_FULL_DIR_INFORMATION");
            break;
        case FileNamesInformation:
            DebugMessage("FILE_NAMES_INFORMATION ");
            break;
        case FileObjectIdInformation:
            DebugMessage("FILE_OBJECTID_INFORMATION");
            break;
        case FileReparsePointInformation:
            DebugMessage("FILE_REPARSE_POINT_INFORMATION");
            break;

        case FileAllInformation:
            DebugMessage("FILE_ALL_INFORMATION ");
            break;
        case FileAttributeTagInformation:
            DebugMessage("FILE_ATTRIBUTE_TAG_INFORMATION");
            break;
        case FileBasicInformation:
            DebugMessage("FILE_BASIC_INFORMATION");
            break;
        case FileCompressionInformation:
            DebugMessage("FILE_COMPRESSION_INFORMATION  ");
            break;
        case FileEaInformation:
            DebugMessage("FILE_EA_INFORMATION ");
            break;
        case FileInternalInformation:
            DebugMessage("FILE_INTERNAL_INFORMATION");
            break;
        case FileMoveClusterInformation:
            DebugMessage("FILE_MOVE_CLUSTER_INFORMATION ");
            break;
        case FileNameInformation:
            DebugMessage("FILE_NAME_INFORMATION");
            break;
        case FileNetworkOpenInformation:
            DebugMessage("FILE_NETWORK_OPEN_INFORMATION");
            break;
        case FilePositionInformation:
            DebugMessage("FILE_POSITION_INFORMATION ");
            break;
        case FileStandardInformation:
            DebugMessage("FILE_STANDARD_INFORMATION");
            break;
        case FileStreamInformation:
            DebugMessage("FILE_STREAM_INFORMATION");
            break;
        case FileRemoteProtocolInformation:
            DebugMessage("FILE_REMOTE_PROTOCOL_INFORMATION");
            break;
        default:
            DebugMessage("File class number: %d", FileInformationClass);
            break;
        }
    }

    int ggez = 0;

    FLT_POSTOP_CALLBACK_STATUS FileFilter::PostDirControlOperation(PFLT_CALLBACK_DATA data, PCFLT_RELATED_OBJECTS flt_objects, PVOID completion_context, FLT_POST_OPERATION_FLAGS flags)
    {
        UNREFERENCED_PARAMETER(flt_objects);
        UNREFERENCED_PARAMETER(completion_context);

        NTSTATUS status = STATUS_SUCCESS;
        PUNICODE_STRING name;
        PVOID directory_buffer;
        PVOID directory_buffer_addr;
        PMDL mdl_address;
        FILE_INFORMATION_CLASS info_class;
        PFILE_BOTH_DIR_INFORMATION p_file_both_dir_info = NULL;
        PFILE_FULL_DIR_INFORMATION p_file_full_dir_info = NULL;
        PFILE_ID_BOTH_DIR_INFORMATION p_file_id_both_dir_info = NULL;

        PFLT_FILE_NAME_INFORMATION flt_fame;

        // assert we're not draining.
        ASSERT(!FlagOn(flags, FLTFL_POST_OPERATION_DRAINING));
        if (flags & FLTFL_POST_OPERATION_DRAINING)
        {
            return FLT_POSTOP_FINISHED_PROCESSING;
        }

        if (!NT_SUCCESS(data->IoStatus.Status))
            return FLT_POSTOP_FINISHED_PROCESSING;


        status = FltGetFileNameInformation(data, FLT_FILE_NAME_NORMALIZED, &flt_fame);
        if (!NT_SUCCESS(status))
        {
            return FLT_POSTOP_FINISHED_PROCESSING;
        }

        __try
        {
            if (data->Iopb->MinorFunction == IRP_MN_QUERY_DIRECTORY)
            {
                name = data->Iopb->Parameters.DirectoryControl.QueryDirectory.FileName;

                info_class = data->Iopb->Parameters.DirectoryControl.QueryDirectory.FileInformationClass;
                // PrintFileInfoClass(info_class);

                directory_buffer = data->Iopb->Parameters.DirectoryControl.QueryDirectory.MdlAddress;
                directory_buffer_addr = &data->Iopb->Parameters.DirectoryControl.QueryDirectory.MdlAddress;
                if (directory_buffer == NULL)
                {
                    directory_buffer = data->Iopb->Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer;
                    directory_buffer_addr = &data->Iopb->Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer;
                }
                ULONG next_entry_cur_val;
                switch (info_class)
                {
                case FileFullDirectoryInformation:
                    status = HideFile((PUCHAR)directory_buffer,
                        (PUCHAR) & (((PFILE_FULL_DIR_INFORMATION)directory_buffer)->NextEntryOffset),
                        (PUCHAR) & (((PFILE_FULL_DIR_INFORMATION)directory_buffer)->FileName),
                        (PUCHAR) & (((PFILE_FULL_DIR_INFORMATION)directory_buffer)->FileNameLength),
                        (PUCHAR)directory_buffer_addr
                    );
                    break;
                case FileBothDirectoryInformation:
                    status = HideFile((PUCHAR)directory_buffer,
                        (PUCHAR) & (((PFILE_BOTH_DIR_INFORMATION)directory_buffer)->NextEntryOffset),
                        (PUCHAR) & (((PFILE_BOTH_DIR_INFORMATION)directory_buffer)->FileName),
                        (PUCHAR) & (((PFILE_BOTH_DIR_INFORMATION)directory_buffer)->FileNameLength),
                        (PUCHAR)directory_buffer_addr
                    );
                    break;
                case FileDirectoryInformation:
                    status = HideFile((PUCHAR)directory_buffer,
                        (PUCHAR) & (((PFILE_DIRECTORY_INFORMATION)directory_buffer)->NextEntryOffset),
                        (PUCHAR) & (((PFILE_DIRECTORY_INFORMATION)directory_buffer)->FileName),
                        (PUCHAR) & (((PFILE_DIRECTORY_INFORMATION)directory_buffer)->FileNameLength),
                        (PUCHAR)directory_buffer_addr
                    );
                    break;
                case FileIdFullDirectoryInformation:
                    status = HideFile((PUCHAR)directory_buffer,
                        (PUCHAR) & (((PFILE_ID_FULL_DIR_INFORMATION)directory_buffer)->NextEntryOffset),
                        (PUCHAR) & (((PFILE_ID_FULL_DIR_INFORMATION)directory_buffer)->FileName),
                        (PUCHAR) & (((PFILE_ID_FULL_DIR_INFORMATION)directory_buffer)->FileNameLength),
                        (PUCHAR)directory_buffer_addr
                    );
                    break;
                case FileIdBothDirectoryInformation:
                    status = HideFile((PUCHAR)directory_buffer,
                        (PUCHAR) & (((PFILE_ID_BOTH_DIR_INFORMATION)directory_buffer)->NextEntryOffset),
                        (PUCHAR) & (((PFILE_ID_BOTH_DIR_INFORMATION)directory_buffer)->FileName),
                        (PUCHAR) & (((PFILE_ID_BOTH_DIR_INFORMATION)directory_buffer)->FileNameLength),
                        (PUCHAR)directory_buffer_addr
                    );
                    break;
                case FileNamesInformation:
                    status = HideFile((PUCHAR)directory_buffer,
                        (PUCHAR) & (((PFILE_NAMES_INFORMATION)directory_buffer)->NextEntryOffset),
                        (PUCHAR) & (((PFILE_NAMES_INFORMATION)directory_buffer)->FileName),
                        (PUCHAR) & (((PFILE_NAMES_INFORMATION)directory_buffer)->FileNameLength),
                        (PUCHAR)directory_buffer_addr
                    );
                    break;
                default:
                    break;
                }
                data->IoStatus.Status = status;
            }
        }
        __finally
        {
            FltReleaseFileNameInformation(flt_fame);
        }

        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    NTSTATUS FileFilter::HideFile(PUCHAR info, PUCHAR nextEntryOffset, PUCHAR fileNameOffset, PUCHAR fileNameLengthOffset, PUCHAR info_addr)
    {

        PUCHAR prev_info = NULL;
        PUCHAR next_info = NULL;

        long long next_entry_rva = nextEntryOffset - info;
        long long file_name_rva = fileNameOffset - info;
        long long file_name_length_rva = fileNameLengthOffset - info;

        WCHAR file_name_str[MAX_SIZE + 1];
        bool set_prev;

        if (info != NULL)
        {
            while (true)
            {
                set_prev = true;
                ULONG cur_next_entry_val = ulti::GetUlongAt((long long)info + next_entry_rva);
                PWCHAR file_name = (PWCHAR)((PUCHAR)info + file_name_rva);
                ULONG file_name_length = ulti::GetUlongAt((long long)info + file_name_length_rva);

                UNICODE_STRING long_file_name = { (USHORT)file_name_length , (USHORT)file_name_length , file_name };

                if (file_name_length <= MAX_SIZE && ulti::GetNameWithoutDirectory(file_name_str, MAX_SIZE + 1, &long_file_name) == true)
                {
                    if (IsFileInHiddenList(file_name_str) == true)
                    {
                        DebugMessage("File is hidden: %S", file_name_str);
                        if (prev_info != NULL)
                        {
                            if (cur_next_entry_val != 0)
                            {
                                ulti::SetUlongAt((long long)prev_info + next_entry_rva,
                                    ulti::GetUlongAt((long long)prev_info + next_entry_rva) + cur_next_entry_val);
                            }
                            else
                            {
                                ulti::SetUlongAt((long long)prev_info + next_entry_rva, 0);
                            }
                        }
                        else
                        {
                            if (cur_next_entry_val != 0)
                            {
                                *(PVOID*)info_addr = ((PUCHAR)info + cur_next_entry_val);
                                set_prev = false;
                            }
                            else
                            {
                                return STATUS_NO_MORE_ENTRIES;
                            }
                        }
                    }
                }

                if (cur_next_entry_val == NULL)
                {
                    break;
                }
                else
                {
                    if (set_prev == true)
                    {
                        prev_info = info;
                    }
                    info = ((PUCHAR)info + cur_next_entry_val);
                }
            }
        }
        
        return STATUS_SUCCESS;
    }

    NTSTATUS FileFilter::Unload()
    {
        DebugMessage("FileFilter Unload: Entered");
        if (g_filter_handle_ != NULL)
        {
            FltUnregisterFilter(g_filter_handle_);
            g_filter_handle_ = NULL;
        }
        return STATUS_SUCCESS;
    }

    bool FileFilter::IsFileInHiddenList(PWCHAR file_name)
    {
        bool ret_val = false;
        KeAcquireGuardedMutex(&file_name_lock_);

        ret_val = ulti::CheckEqualString(file_name, file_to_hide_);

        KeReleaseGuardedMutex(&file_name_lock_);

        return ret_val;
    }


    void FileFilter::GetFileName(PFLT_CALLBACK_DATA data, PWCHAR name, int size)
    {
        PFLT_FILE_NAME_INFORMATION file_name_info;
        NTSTATUS status;

        RtlZeroMemory(name, size);

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
                }
            }
            FltReleaseFileNameInformation(file_name_info);
        }

        return;
    }

    void FileFilter::PrintCurFileName(PFLT_CALLBACK_DATA data)
    {
        WCHAR name[MAX_SIZE] = { 0 };
        int size = MAX_SIZE;

        GetFileName(data, name, size);
        DebugMessage("%ws", name);

    }

    void FileFilter::SetDriverObjectPtr(const PDRIVER_OBJECT p_driver_object)
    {
        p_driver_object_ = p_driver_object;
    }

    PDRIVER_OBJECT FileFilter::GetDriverObjectPtr()
    {
        return p_driver_object_;
    }

    void FileFilter::SetFilterHandle(const PFLT_FILTER g_filter_handle)
    {
        g_filter_handle_ = g_filter_handle;
    }

    PFLT_FILTER FileFilter::GetFilterHandle()
    {
        return g_filter_handle_;
    }

    const FLT_OPERATION_REGISTRATION FileFilter::callbacks_[] =
    {
        { IRP_MJ_DIRECTORY_CONTROL,
          0,
          FileFilter::PreDirControlOperation,
          FileFilter::PostDirControlOperation },
        { IRP_MJ_OPERATION_END }
    };

    const FLT_REGISTRATION FileFilter::filter_registration_ =
    {
        sizeof(FLT_REGISTRATION),
        FLT_REGISTRATION_VERSION,
        0,                                  //  flags

        NULL,                          //  Context
        FileFilter::callbacks_,                         //  Operation callbacks

        (PFLT_FILTER_UNLOAD_CALLBACK)::FilterUnload,    //  MiniFilterUnload

        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    };

}

