/* Windows version of plaintext_transport.c */

#include <assert.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

#include "plaintext_transport.h"

struct NetworkContext
{
    PlaintextParams_t * pParams;
};

static int initializeWinsock()
{
    static int initialized = 0;
    if (!initialized)
    {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0)
        {
            LogError(("WSAStartup failed with error: %d", result));
            return 0;
        }
        initialized = 1;
    }
    return 1;
}

static void logTransportError( int32_t errorNumber )
{
    char * msgBuf = NULL;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL, errorNumber, 0, (LPSTR)&msgBuf, 0, NULL);

    if (msgBuf)
    {
        LogError(("A transport error occurred: %s", msgBuf));
        LocalFree(msgBuf);
    }
    else
    {
        LogError(("A transport error occurred: Unknown error %d", errorNumber));
    }
}

SocketStatus_t Plaintext_Connect( NetworkContext_t * pNetworkContext,
                                  const ServerInfo_t * pServerInfo,
                                  uint32_t sendTimeoutMs,
                                  uint32_t recvTimeoutMs )
{
    if (!initializeWinsock())
    {
        return SOCKETS_API_ERROR;
    }

    if( ( pNetworkContext == NULL ) || ( pNetworkContext->pParams == NULL ) )
    {
        LogError(("Parameter check failed: pNetworkContext is NULL."));
        return SOCKETS_INVALID_PARAMETER;
    }

    PlaintextParams_t * pPlaintextParams = pNetworkContext->pParams;
    return Sockets_Connect( &pPlaintextParams->socketDescriptor,
                            pServerInfo,
                            sendTimeoutMs,
                            recvTimeoutMs );
}

SocketStatus_t Plaintext_Disconnect( const NetworkContext_t * pNetworkContext )
{
    if( ( pNetworkContext == NULL ) || ( pNetworkContext->pParams == NULL ) )
    {
        LogError(("Parameter check failed: pNetworkContext is NULL."));
        return SOCKETS_INVALID_PARAMETER;
    }

    PlaintextParams_t * pPlaintextParams = pNetworkContext->pParams;
    return Sockets_Disconnect( pPlaintextParams->socketDescriptor );
}

int32_t Plaintext_Recv( NetworkContext_t * pNetworkContext,
                        void * pBuffer,
                        size_t bytesToRecv )
{
    assert( pNetworkContext != NULL && pNetworkContext->pParams != NULL );
    assert( pBuffer != NULL );
    assert( bytesToRecv > 0 );

    SOCKET sock = pNetworkContext->pParams->socketDescriptor;
    fd_set readfds;
    struct timeval timeout = { 0, 0 }; // non-blocking
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    int pollStatus = select(0, &readfds, NULL, NULL, &timeout);
    int32_t bytesReceived = -1;

    if (pollStatus > 0 && FD_ISSET(sock, &readfds))
    {
        bytesReceived = recv(sock, pBuffer, (int)bytesToRecv, 0);
        if (bytesReceived == 0)
        {
            bytesReceived = -1; // Peer closed connection
        }
    }
    else if (pollStatus == SOCKET_ERROR)
    {
        logTransportError(WSAGetLastError());
        bytesReceived = -1;
    }
    else
    {
        bytesReceived = 0; // no data
    }

    return bytesReceived;
}

int32_t Plaintext_Send( NetworkContext_t * pNetworkContext,
                        const void * pBuffer,
                        size_t bytesToSend )
{
    assert( pNetworkContext != NULL && pNetworkContext->pParams != NULL );
    assert( pBuffer != NULL );
    assert( bytesToSend > 0 );

    SOCKET sock = pNetworkContext->pParams->socketDescriptor;
    fd_set writefds;
    struct timeval timeout = { 0, 0 }; // non-blocking
    FD_ZERO(&writefds);
    FD_SET(sock, &writefds);

    int pollStatus = select(0, NULL, &writefds, NULL, &timeout);
    int32_t bytesSent = -1;

    if (pollStatus > 0 && FD_ISSET(sock, &writefds))
    {
        bytesSent = send(sock, pBuffer, (int)bytesToSend, 0);
        if (bytesSent == 0)
        {
            bytesSent = -1; // Connection closed
        }
    }
    else if (pollStatus == SOCKET_ERROR)
    {
        logTransportError(WSAGetLastError());
        bytesSent = -1;
    }
    else
    {
        bytesSent = 0; // cannot write
    }

    return bytesSent;
}
