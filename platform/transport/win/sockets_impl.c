/* Windows version of sockets_impl.c */

#include <assert.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

#include "sockets_impl.h"

#define ONE_SEC_TO_MS    ( 1000 )
#define ONE_MS_TO_US     ( 1000 )

static SocketStatus_t retrieveError( int32_t errorNumber )
{
    if (errorNumber == WSAENOBUFS || errorNumber == WSA_NOT_ENOUGH_MEMORY)
    {
        return SOCKETS_INSUFFICIENT_MEMORY;
    }
    else if (errorNumber == WSAENOTSOCK || errorNumber == WSAEINVAL)
    {
        return SOCKETS_INVALID_PARAMETER;
    }

    return SOCKETS_API_ERROR;
}

SocketStatus_t Sockets_Connect( int32_t * pTcpSocket,
                                const ServerInfo_t * pServerInfo,
                                uint32_t sendTimeoutMs,
                                uint32_t recvTimeoutMs )
{
    if( pServerInfo == NULL || pServerInfo->pHostName == NULL ||
        pTcpSocket == NULL || pServerInfo->hostNameLength == 0UL )
    {
        LogError(("Invalid parameters to Sockets_Connect."));
        return SOCKETS_INVALID_PARAMETER;
    }

    struct addrinfo hints = {0}, *pResult = NULL;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    char portStr[6];
    snprintf(portStr, sizeof(portStr), "%u", pServerInfo->port);

    if (getaddrinfo(pServerInfo->pHostName, portStr, &hints, &pResult) != 0)
    {
        LogError(("getaddrinfo failed."));
        return SOCKETS_DNS_FAILURE;
    }

    SOCKET sock = INVALID_SOCKET;
    for (struct addrinfo *ptr = pResult; ptr != NULL; ptr = ptr->ai_next)
    {
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock == INVALID_SOCKET) continue;

        if (connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen) == 0)
        {
            break; // success
        }

        closesocket(sock);
        sock = INVALID_SOCKET;
    }

    freeaddrinfo(pResult);

    if (sock == INVALID_SOCKET)
    {
        LogError(("Could not connect to any resolved address."));
        return SOCKETS_CONNECT_FAILURE;
    }

    DWORD timeout = sendTimeoutMs;
    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout)) == SOCKET_ERROR)
    {
        LogError(("Setting send timeout failed."));
        closesocket(sock);
        return retrieveError(WSAGetLastError());
    }

    timeout = recvTimeoutMs;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) == SOCKET_ERROR)
    {
        LogError(("Setting receive timeout failed."));
        closesocket(sock);
        return retrieveError(WSAGetLastError());
    }

    *pTcpSocket = (int32_t)sock;
    return SOCKETS_SUCCESS;
}

SocketStatus_t Sockets_Disconnect( int32_t tcpSocket )
{
    if( tcpSocket > 0 )
    {
        shutdown((SOCKET)tcpSocket, SD_BOTH);
        closesocket((SOCKET)tcpSocket);
        return SOCKETS_SUCCESS;
    }
    else
    {
        LogError(("Invalid tcpSocket passed to Sockets_Disconnect."));
        return SOCKETS_INVALID_PARAMETER;
    }
}
