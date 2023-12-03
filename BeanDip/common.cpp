#include "common.h"

int to_unicode(_In_ char* original, _In_ SIZE_T len, _Out_ PUNICODE_STRING puStr)
{
	if (!original || !puStr)
	{
		DMSG("NULL");
		return -1;
	}

	HANDLE hHeap = NULL;
	SIZE_T converted = 0;
	
	hHeap = GetProcessHeap();

	if (NULL == hHeap)
	{
		DERR("GetProcessHeap", GetLastError());
		return -1;
	}

	puStr->Buffer = NULL;

	puStr->Buffer = (PWSTR)HeapAlloc(hHeap, 0, (len * sizeof(WCHAR)) + sizeof(WCHAR));
	if (NULL == puStr->Buffer)
	{
		DERR("HeapAlloc", GetLastError());
		return -1;
	}

	puStr->Length = (USHORT)(len * sizeof(WCHAR));
	puStr->MaximumLength = puStr->Length;

	if (0 != mbstowcs_s(&converted, puStr->Buffer, len + 1, original, puStr->Length))
	{
		DERR("mbstowcs_s", GetLastError());
		return -1;
	}


	return 0;
}