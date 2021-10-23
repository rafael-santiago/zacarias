/*
 *                          Copyright (C) 2021 by Rafael Santiago
 *
 * Use of this source code is governed by GPL-v2 license that can
 * be found in the COPYING file.
 *
 */
#include <windows/kio_impl.h>
#include <ntifs.h>

int kwrite_impl(const char *filepath, void *buf, const size_t buf_size) {
    if (KeGetCurrentIrql() != PASSIVE_LEVEL) {
        return -1;
    }

    OBJECT_ATTRIBUTES file_attr;
    HANDLE file_handle;
    UNICODE_STRING filepath_ustr;
    ANSI_STRING temp_astr;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    IO_STATUS_BLOCK io_sts;
    int retval = -1;

    RtlInitAnsiString(&temp_astr, filepath);
    status = RtlAnsiStringToUnicodeString(&filepath_ustr, &temp_astr, FALSE);
    if (!NT_SUCCESS(status)) {
        return -1;
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
            retval = (int)io_sts.Information;
        }
        ZwFlushBuffersFile(file_handle, &io_sts);
    }

    ZwClose(file_handle);

    return retval;
}

int kread_impl(const char *filepath, void **buf, size_t *buf_size) {
    UNREFERENCED_PARAMETER(filepath);
    UNREFERENCED_PARAMETER(buf);
    UNREFERENCED_PARAMETER(buf_size);
    return 1;
}
