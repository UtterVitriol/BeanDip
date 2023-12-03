#include "connection.h"
#include "common.h"

// TODO: Add cleanup
// TODO: Check NULL

int init_sock(_In_ char* ip, _In_ char* port, _Out_ SOCKET* pSock, _Out_ PADDRINFOA* ppResult);

int init_client(_In_ char* ip, _In_ char* port, _Out_ SOCKET* pSock)
{
	int iResult = 0;

	PADDRINFOA result = NULL;

	iResult = init_sock(ip, port, pSock, &result);
	if (0 != iResult || NULL == result)
	{
		if (!result)
		{
			DMSG("Result NULL");
		}
		else
		{
			DERR("init_sock", GetLastError());
		}
		goto __end;
	}

	iResult = connect(*pSock, result->ai_addr, (int)result->ai_addrlen);
	if (SOCKET_ERROR == iResult)
	{
		DERR("connect", GetLastError());
		goto __end;
	}

__end:
	if (result)
	{
		freeaddrinfo(result);
	}

	return iResult;
}

// TODO: not TODONE
int init_server(_In_ char* port, _Out_ SOCKET* pSock)
{
	struct addrinfo* result = NULL;

	return init_sock((char*)"localhost", port, pSock, &result);
}

// TODO: Fix gotos
int init_sock(_In_ char* ip, _In_ char* port, _Out_ SOCKET* pSock, _Out_ PADDRINFOA* ppResult)
{
	int iResult = 0;

	WSADATA wsaData = { 0 };

	struct addrinfo hints = { 0 };

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (0 != iResult)
	{
		DERR("WSAStartup", GetLastError());
		return iResult;
	}

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(ip, port, &hints, ppResult);
	if (0 != iResult)
	{
		DERR("getaddrinfo", GetLastError());
		goto __dead;
	}
	
	*pSock = socket((*ppResult)->ai_family, (*ppResult)->ai_socktype, (*ppResult)->ai_protocol);
	if (INVALID_SOCKET == *pSock)
	{
		DERR("socket", GetLastError());
		goto __dead;
	}

	goto __notdead;
__dead:
	WSACleanup();

__notdead:

	return iResult;
}

int send_buf(_In_ SOCKET sock, _In_ char* buf, _In_ DWORD len)
{
	DWORD dwSent = 0;
	DWORD dwTemp = 0;

	if (NULL == buf)
	{
		DMSG("NULL");
		return -1;
	}

	while (dwSent != len)
	{
		dwTemp = send(sock, buf + dwSent, len, 0);
		if (dwTemp <= 0)
		{
			DERR("send_buf", GetLastError());
			return -1;
		}
		dwSent += dwTemp;
	}

	return dwSent;
}

int recv_buf(_In_ SOCKET sock, _Out_ char* buf, _In_ DWORD len)
{
	DWORD dwRecvd = 0;
	DWORD dwTemp = 0;

	if (NULL == buf)
	{
		DMSG("NULL");
		return -1;
	}

	while (dwRecvd != len)
	{
		dwTemp = recv(sock, buf + dwRecvd, len, 0);
		if (dwTemp <= 0)
		{
			DERR("send_buf", GetLastError());
			return -1;
		}
		dwRecvd += dwTemp;
	}

	return dwRecvd;
}