#include "get.h"
#include "connection.h"
#include "common.h"

#include <winternl.h>

#pragma comment(lib, "ntdll.lib")

#define STATUS_END_OF_FILE 0xC0000011

// TODO: Add protocol
int my_get(_In_ SOCKET sock, _In_ PMSG_HEADER pMsgHeader)
{
	NTSTATUS ntStatus = 0;
	char buf[4096] = FULLY_QALIFIED_PATH_PREFIX;

	DWORD dwRecvd = 0;

	NTREADFILE pNtReadFile = NULL;
	HMODULE hNtDll = NULL;

	HANDLE hFile = INVALID_HANDLE_VALUE;
	OBJECT_ATTRIBUTES FileAttrs = { 0 };
	UNICODE_STRING FileName = { 0 };
	IO_STATUS_BLOCK iosb = { 0 };

	LARGE_INTEGER FileOffset = { 0 };
	DWORD dwFileSzLow = 0;
	DWORD dwFileSzHigh = 0;
	DWORD dwRead = 0;

	DWORD dwSent = 0;

	dwRecvd = recv(sock, buf + 4, pMsgHeader->Len, 0);
	if (dwRecvd != pMsgHeader->Len)
	{
		DMSG("INVALID RECV");
		return -1;
	}

	hNtDll = GetModuleHandleA("ntdll");
	if (NULL == hNtDll)
	{
		DERR("GetModuleHandleA", GetLastError());
		return -1;
	}

	pNtReadFile = (NTREADFILE)GetProcAddress(hNtDll, "NtReadFile");
	if (NULL == pNtReadFile)
	{
		DERR("GetProcAddress", GetLastError());
		return -1;
	}

	ntStatus = to_unicode(buf, pMsgHeader->Len + 4, &FileName);
	if (ntStatus != 0)
	{
		goto __end;
	}

	FileAttrs.ObjectName = &FileName;
	FileAttrs.Length = sizeof(FileAttrs);
	FileAttrs.RootDirectory = NULL;
	FileAttrs.Attributes = OBJ_OPENIF;
	FileAttrs.SecurityDescriptor = NULL;
	FileAttrs.SecurityQualityOfService = NULL;

	ntStatus = NtCreateFile(&hFile,
		FILE_READ_DATA | SYNCHRONIZE,
		&FileAttrs,
		&iosb,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0
	);
	if (!NT_SUCCESS(ntStatus))
	{
		DERR("NtCreateFile", RtlNtStatusToDosError(ntStatus));
		goto __end;
	}

	dwFileSzLow = GetFileSize(hFile, &dwFileSzHigh);
	if (INVALID_FILE_SIZE == dwFileSzLow)
	{
		DERR("GetFileSize", GetLastError());
		goto __end;
	}

	// TODO: Try big file?
	if (dwFileSzHigh)
	{
		DMSG("Big file");
	}

	ZeroMemory(buf, sizeof(buf));

	ntStatus = pNtReadFile(hFile,
		NULL,
		NULL,
		NULL,
		&iosb,
		buf,
		sizeof(buf) - 1,
		NULL,
		NULL
	);
	if (!NT_SUCCESS(ntStatus) && ntStatus != STATUS_END_OF_FILE)
	{
		DERR("NTReadFile", RtlNtStatusToDosError(ntStatus));
		goto __end;
	}

	dwRead += (DWORD)iosb.Information;

	ntStatus = send_buf(sock, buf, (DWORD)iosb.Information);
	if (ntStatus < 0)
	{
		goto __end;
	}

	FileOffset.QuadPart = dwRead;

	while (dwRead < dwFileSzLow)
	{
		ZeroMemory(buf, sizeof(buf));

		ntStatus = pNtReadFile(hFile,
			NULL,
			NULL,
			NULL,
			&iosb,
			buf,
			sizeof(buf) - 1,
			&FileOffset,
			NULL
		);
		if (!NT_SUCCESS(ntStatus) && ntStatus != STATUS_END_OF_FILE)
		{
			DERR("NTReadFile", RtlNtStatusToDosError(ntStatus));
			goto __end;
		}

		dwRead += (DWORD)iosb.Information;

		FileOffset.QuadPart = dwRead;

		ntStatus = send_buf(sock, buf, (DWORD)iosb.Information);
		if (ntStatus < 0)
		{
			goto __end;
		}
	}

__end:
	if (hFile)
	{
		NtClose(hFile);
	}
	if (NULL != FileName.Buffer)
	{
		HeapFree(GetProcessHeap(), 0, FileName.Buffer);
	}

	return ntStatus;
}