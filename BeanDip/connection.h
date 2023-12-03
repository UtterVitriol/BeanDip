#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#pragma pack(push, 1)
typedef struct __MSGHEADER
{
	short Type;
	DWORD Len;
} MSG_HEADER, *PMSG_HEADER;
#pragma pack(pop)

enum __MSGTYPE {
	GET,
	PUT,
	EXEC,
	DIR
};

int init_client(_In_ char* ip, _In_ char* port, _Out_ SOCKET* pSock);
int init_server(_In_ char* port, _Out_ SOCKET* pSock);

/// <summary>
/// Send the buf
/// </summary>
/// <returns>Bytes sent or -1 if failure</returns>
int send_buf(_In_ SOCKET sock, _In_ char* buf, _In_ DWORD len);

int recv_buf(_In_ SOCKET sock, _Out_ char* buf, _In_ DWORD len);
