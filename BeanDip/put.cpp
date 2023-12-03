#include "put.h"
#include "connection.h"
#include "common.h"

#include <winternl.h>
#pragma comment(lib, "ntdll.lib")

int my_put(_In_ SOCKET sock, _In_ PMSG_HEADER pMsgHeader)
{
	NTSTATUS ntStatus = 0;
	char buf[4096] = FULLY_QALIFIED_PATH_PREFIX;

	DWORD dwRecvd = 0;
	NTWRITEFILE pNtWriteFile = NULL;
	HMODULE hNtDll = NULL;

	HANDLE hFile = INVALID_HANDLE_VALUE;
	OBJECT_ATTRIBUTES FileAttrs = { 0 };
	UNICODE_STRING FileName = { 0 };
	IO_STATUS_BLOCK iosb = { 0 };

	MSG_HEADER msgHeader = { 0 };
	DWORD dwFileSz = 0;

	dwRecvd = recv(sock, buf + 4, pMsgHeader->Len, 0);
	if (dwRecvd != pMsgHeader->Len)
	{
		DERR("recv", GetLastError());
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
		FILE_WRITE_DATA | SYNCHRONIZE,
		&FileAttrs,
		&iosb,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_WRITE,
		FILE_SUPERSEDE,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0
	);
	if (!NT_SUCCESS(ntStatus))
	{
		DERR("NtCreateFile", RtlNtStatusToDosError(ntStatus));
		goto __end;
	}

	hNtDll = GetModuleHandleA("ntdll");
	if (NULL == hNtDll)
	{
		DERR("GetModuleHandleA", GetLastError());
		ntStatus = -1;
		goto __end;
	}

	pNtWriteFile = (NTWRITEFILE)GetProcAddress(hNtDll, "NtWriteFile");
	if (NULL == pNtWriteFile)
	{
		DERR("GetProcAddress", GetLastError());
		ntStatus = -1;
		goto __end;
	}

	dwRecvd = recv(sock, (char*)&msgHeader, sizeof(msgHeader), 0);
	if (dwRecvd != sizeof(msgHeader))
	{
		DERR("recv", GetLastError());
		ntStatus = -1;
		goto __end;
	}

	if (PUT != msgHeader.Type)
	{
		DMSG("Bad Message Type");
		ntStatus = -1;
		goto __end;
	}

	dwFileSz = msgHeader.Len;


	while (dwRecvd < dwFileSz)
	{
		int temp = 0;
		ZeroMemory(&msgHeader, sizeof(msgHeader));
		ZeroMemory(buf, sizeof(buf));

		temp = recv(sock, (char*)&msgHeader, sizeof(msgHeader), 0);
		if (temp != sizeof(msgHeader))
		{
			DERR("recv", GetLastError());
			ntStatus = -1;
			goto __end;
		}

		temp = recv_buf(sock, buf, msgHeader.Len);
		if (temp <= 0)
		{
			ntStatus = -1;
			goto __end;
		}
		
		ntStatus = pNtWriteFile(hFile,
			NULL,
			NULL,
			NULL,
			&iosb,
			buf,
			temp,
			NULL,
			NULL
		);

		// TODO: written vs recvd? (NtWriteFile -> iosb.information?)
		dwRecvd += temp;
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