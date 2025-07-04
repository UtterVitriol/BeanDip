#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

#pragma comment(lib, "Ws2_32.lib")

typedef BOOL (*tAcceptEx)(SOCKET       sListenSocket,
                          SOCKET       sAcceptSocket,
                          PVOID        lpOutputBuffer,
                          DWORD        dwReceiveDataLength,
                          DWORD        dwLocalAddressLength,
                          DWORD        dwRemoteAddressLength,
                          LPDWORD      lpdwBytesReceived,
                          LPOVERLAPPED lpOverlapped);


SOCKET init_server(USHORT port);
VOID   server_run(SOCKET sServer);
VOID   cleanup_server(SOCKET sock);
