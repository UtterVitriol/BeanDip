#include <iostream>

#include "connection.h"
#include "common.h"
#include "get.h"
#include "put.h"

// TODO: Set buf size?
// TODO: Get function pointers during init

int main()
{
	int iResult = 0;
	SOCKET sock = INVALID_SOCKET;


	iResult = init_client((char*)"127.0.0.1", (char*)"6969", &sock);
	if (iResult != 0)
	{
		return iResult;
	}

	DWORD dwRecvd = 0;
	MSG_HEADER msgHeader = { 0 };

	int ree = 0;
	for (;;)
	{
		ZeroMemory(&msgHeader, sizeof(msgHeader));

		dwRecvd = recv(sock, (char*)&msgHeader, sizeof(msgHeader), 0);
		if (dwRecvd <= 0)
		{
			DERR("recv", GetLastError());
			return dwRecvd;
		}
		
		switch (msgHeader.Type)
		{
		case GET:
			my_get(sock, &msgHeader);
			break;

		case PUT:
			my_put(sock, &msgHeader);
			break;

		case EXEC:
			
			break;

		case DIR:

			break;

		default:
			DMSG("Wtf");
		}

		break;
	}

	closesocket(sock);
}