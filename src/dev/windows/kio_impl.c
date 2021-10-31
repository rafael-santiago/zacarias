/*
 *                          Copyright (C) 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <windows/kio_impl.h>
#include <ntifs.h>
#include <kryptos.h>

int kwrite_impl(const char *filepath, void *buf, const size_t buf_size) {
    OBJECT_ATTRIBUTES file_attr;
    HANDLE file_handle;
    UNICODE_STRING filepath_ustr;
    ANSI_STRING temp_astr;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    IO_STATUS_BLOCK io_sts;
    int retval = 1;

    if (KeGetCurrentIrql() != PASSIVE_LEVEL) {
        return 1;
    }

    RtlInitAnsiString(&temp_astr, filepath);
    status = RtlAnsiStringToUnicodeString(&filepath_ustr, &temp_astr, TRUE);
    if (!NT_SUCCESS(status)) {
        return 1;
    }

    InitializeObjectAttributes(&file_attr, &filepath_ustr, OBJ_KERNEL_HANDLE, NULL, NULL);
    status = ZwCreateFile(&file_handle,
                          GENERIC_WRITE,
                          &file_attr,
                          &io_sts,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          0,
                          FILE_OVERWRITE_IF,
                          FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0);
    if (NT_SUCCESS(status)) {
        status = ZwWriteFile(file_handle, NULL, NULL, NULL, &io_sts, buf, (ULONG)buf_size, NULL, NULL);
        if (NT_SUCCESS(status)) {
            retval = 0;
        }
        ZwFlushBuffersFile(file_handle, &io_sts);
        ZwClose(file_handle);
    }

    RtlFreeUnicodeString(&filepath_ustr);

    return retval;
}

int kread_impl(const char *filepath, void **buf, size_t *buf_size) {
    OBJECT_ATTRIBUTES file_attr;
    HANDLE file_handle = 0;
    UNICODE_STRING filepath_ustr = { 0 };
    ANSI_STRING temp_astr;
    NTSTATUS status = STATUS_UNSUCCESSFUL, create_file_status = STATUS_UNSUCCESSFUL, unicode_conv_status = STATUS_UNSUCCESSFUL;
    IO_STATUS_BLOCK io_sts;
    int retval = 1;
    FILE_STANDARD_INFORMATION file_info = { 0 };

    if (KeGetCurrentIrql() != PASSIVE_LEVEL || filepath == NULL || buf == NULL || buf_size == NULL) {
        return 1;
    }

    *buf_size = 0;

    RtlInitAnsiString(&temp_astr, filepath);
    unicode_conv_status = RtlAnsiStringToUnicodeString(&filepath_ustr, &temp_astr, TRUE);
    if (!NT_SUCCESS(unicode_conv_status)) {
        goto kread_impl_epilogue;
    }

    InitializeObjectAttributes(&file_attr, &filepath_ustr, OBJ_KERNEL_HANDLE, NULL, NULL);
    create_file_status = ZwCreateFile(&file_handle,
                                      GENERIC_READ,
                                      &file_attr,
                                      &io_sts,
                                      NULL,
                                      FILE_ATTRIBUTE_NORMAL,
                                      0,
                                      FILE_OPEN,
                                      FILE_SYNCHRONOUS_IO_NONALERT,
                                      NULL,
                                      0);
    if (!NT_SUCCESS(status)) {
        status = ZwQueryInformationFile(file_handle,
                                        &io_sts,
                                        &file_info,
                                        sizeof(file_info),
                                        FileStandardInformation);

        if (!NT_SUCCESS(status)) {
            goto kread_impl_epilogue;
        }

        *buf_size = (size_t) file_info.EndOfFile.QuadPart;

        *buf = kryptos_newseg(*buf_size + 1);
        if (*buf == NULL) {
            *buf_size = 0;
            goto kread_impl_epilogue;
        }

        status = ZwReadFile(file_handle, NULL, NULL, NULL, &io_sts, *buf, (ULONG)*buf_size, NULL, NULL);
        if (NT_SUCCESS(status)) {
            retval = 0;
        } else {
            kryptos_freeseg(*buf, *buf_size);
            *buf_size = 0;
        }
    }

kread_impl_epilogue:
    if (NT_SUCCESS(create_file_status)) {
        ZwClose(file_handle);
    }

    if (NT_SUCCESS(unicode_conv_status)) {
        RtlFreeUnicodeString(&filepath_ustr);
    }

    return retval;
}
