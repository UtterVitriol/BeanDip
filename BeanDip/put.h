#pragma once
#include "connection.h"

#include <winternl.h>

typedef NTSTATUS(WINAPI* NTWRITEFILE)(IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL);

int my_put(_In_ SOCKET sock, _In_ PMSG_HEADER pMsgHeader);