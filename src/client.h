#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>

#define dwAddrLen ((sizeof(SOCKADDR_IN) + 16))

typedef struct
{
    WSAOVERLAPPED wsaOverlapped;
    SOCKET sock;
    HANDLE ioPort;
    WSABUF wsaBuf;
    DWORD dwFlags;
    DWORD dwBytes;
    CHAR                buff[dwAddrLen * 3];
} CLIENT_CTX, *PCLIENT_CTX, **PPCLIENT_CTX;

VOID CALLBACK handle_client(PTP_CALLBACK_INSTANCE Instance, PVOID Parameter, PTP_WORK WORK);
PCLIENT_CTX   create_client(HANDLE hCompPort);
void close_client(PPCLIENT_CTX ppCtx);
