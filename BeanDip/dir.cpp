#include "dir.h"
#include "connection.h"
#include "common.h"

#define TIME_STR_MAX_LEN 20

void set_wildcard(char* path, int len)
{
	if (path[len - 1] == '*')
	{
		return;
	}

	if (path[len - 1] == '\\')
	{
		path[len] = '*';

		return;
	}

	if (path[len - 1] != '\\')
	{
		path[len] = '\\';
		path[len + 1] = '*';
	}
}

int time_to_str(_Out_ PCHAR* ppWriteTime, _In_ PWIN32_FIND_DATAA pffData)
{
	SYSTEMTIME sysTime = { 0 };
	SYSTEMTIME locTime = { 0 };
	if (!FileTimeToSystemTime(&pffData->ftLastWriteTime, &sysTime))
	{
		DERR("FileTimeToSystemTime", GetLastError());
		return -1;
	}

	if (FALSE == SystemTimeToTzSpecificLocalTime(NULL, &sysTime, &locTime))
	{
		DERR("SystemTimeToTzSpecificLocalTime", GetLastError());
		return -1;
	}

	HANDLE hHeap = GetProcessHeap();
	if (NULL == hHeap)
	{
		DERR("GetProcessHeap", GetLastError());
		return -1;
	}

	*ppWriteTime = (char*)HeapAlloc(hHeap, 0, TIME_STR_MAX_LEN);
	if (NULL == ppWriteTime)
	{
		DERR("HeapAlloc", GetLastError());
		return -1;
	}

	if (-1 == sprintf_s(*ppWriteTime, TIME_STR_MAX_LEN, "%02d/%02d/%04d %02d:%02d:%02d",
		locTime.wMonth, locTime.wDay, locTime.wYear,
		locTime.wHour, locTime.wMinute, locTime.wSecond))
	{
		DERR("spritnf_s", GetLastError());
	}

	return 0;
}

int my_dir(_In_ SOCKET sock, _In_ PMSG_HEADER pMsgHeader)
{
	int iResult = 0;
	char buf[4096] = { 0 };

	WIN32_FIND_DATAA ffData = { 0 };
	HANDLE hFind = INVALID_HANDLE_VALUE;
	

	iResult = recv(sock, buf, pMsgHeader->Len, 0);
	if (iResult != pMsgHeader->Len)
	{
		DMSG("Invalid Recv");
		return -1;
	}

	set_wildcard(buf, pMsgHeader->Len);

	printf("Path: %s\n", buf);

	hFind = FindFirstFileA(buf, &ffData);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		DERR("FindFirstFileA", GetLastError());
		return -1;
	}

	do
	{
		char* writeTime = NULL;
		iResult = time_to_str(&writeTime, &ffData);
		if (iResult != 0)
		{
			goto __end;
		}

		if (ffData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			printf("<DIR> ");
		}
		else
		{
			printf("<FIL> ");
		}
		printf("%s ", writeTime);
		printf("%s\n", ffData.cFileName);

		if (writeTime)
		{
			HeapFree(GetProcessHeap(), 0, writeTime);
		}
	} while (FindNextFileA(hFind, &ffData));



__end:
	if (INVALID_HANDLE_VALUE != hFind)
	{
		FindClose(hFind);
	}
	return iResult;
}