// Taken from WinFsp winposix.c
#ifdef _WIN32

#include "nxcompat/nxcompat.h"

#include <windows.h>

int
map_error_to_errno(DWORD dwError)
{
    switch (dwError) {
        case ERROR_INVALID_FUNCTION:
            return EINVAL;
        case ERROR_FILE_NOT_FOUND:
            return ENOENT;
        case ERROR_PATH_NOT_FOUND:
            return ENOENT;
        case ERROR_TOO_MANY_OPEN_FILES:
            return EMFILE;
        case ERROR_ACCESS_DENIED:
            return EACCES;
        case ERROR_INVALID_HANDLE:
            return EBADF;
        case ERROR_ARENA_TRASHED:
            return ENOMEM;
        case ERROR_NOT_ENOUGH_MEMORY:
            return ENOMEM;
        case ERROR_INVALID_BLOCK:
            return ENOMEM;
        case ERROR_BAD_ENVIRONMENT:
            return E2BIG;
        case ERROR_BAD_FORMAT:
            return ENOEXEC;
        case ERROR_INVALID_ACCESS:
            return EINVAL;
        case ERROR_INVALID_DATA:
            return EINVAL;
        case ERROR_INVALID_DRIVE:
            return ENOENT;
        case ERROR_CURRENT_DIRECTORY:
            return EACCES;
        case ERROR_NOT_SAME_DEVICE:
            return EXDEV;
        case ERROR_NO_MORE_FILES:
            return ENOENT;
        case ERROR_LOCK_VIOLATION:
            return EACCES;
        case ERROR_BAD_NETPATH:
            return ENOENT;
        case ERROR_NETWORK_ACCESS_DENIED:
            return EACCES;
        case ERROR_BAD_NET_NAME:
            return ENOENT;
        case ERROR_FILE_EXISTS:
            return EEXIST;
        case ERROR_CANNOT_MAKE:
            return EACCES;
        case ERROR_FAIL_I24:
            return EACCES;
        case ERROR_INVALID_PARAMETER:
            return EINVAL;
        case ERROR_NO_PROC_SLOTS:
            return EAGAIN;
        case ERROR_DRIVE_LOCKED:
            return EACCES;
        case ERROR_BROKEN_PIPE:
            return EPIPE;
        case ERROR_DISK_FULL:
            return ENOSPC;
        case ERROR_INVALID_TARGET_HANDLE:
            return EBADF;
        case ERROR_WAIT_NO_CHILDREN:
            return ECHILD;
        case ERROR_CHILD_NOT_COMPLETE:
            return ECHILD;
        case ERROR_DIRECT_ACCESS_HANDLE:
            return EBADF;
        case ERROR_NEGATIVE_SEEK:
            return EINVAL;
        case ERROR_SEEK_ON_DEVICE:
            return EACCES;
        case ERROR_DIR_NOT_EMPTY:
            return ENOTEMPTY;
        case ERROR_NOT_LOCKED:
            return EACCES;
        case ERROR_BAD_PATHNAME:
            return ENOENT;
        case ERROR_MAX_THRDS_REACHED:
            return EAGAIN;
        case ERROR_LOCK_FAILED:
            return EACCES;
        case ERROR_ALREADY_EXISTS:
            return EEXIST;
        case ERROR_FILENAME_EXCED_RANGE:
            return ENOENT;
        case ERROR_NESTING_NOT_ALLOWED:
            return EAGAIN;
        case ERROR_NOT_ENOUGH_QUOTA:
            return ENOMEM;
        default:
            if (ERROR_WRITE_PROTECT <= dwError &&
                    dwError <= ERROR_SHARING_BUFFER_EXCEEDED)
                return EACCES;
            else if (ERROR_INVALID_STARTING_CODESEG <= dwError &&
                    dwError <= ERROR_INFLOOP_IN_RELOC_CHAIN)
                return ENOEXEC;
            else
                return EINVAL;
    }
}
#endif
