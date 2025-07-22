//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "plaintext_transport.h"

#include "stdint.h"
#include "stddef.h"

#include "lwip/sockets.h"
#include "lwipopts.h"

#include "assert.h"
//============================================================================


/* Each compilation unit must define the NetworkContext struct. */
struct NetworkContext
{
    PlaintextParams_t * pParams;
};


//=============================================================================
/*-------------------------------- Functions --------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
SocketStatus_t Plaintext_Connect( NetworkContext_t * pNetworkContext,
                                  const ServerInfo_t * pServerInfo,
                                  uint32_t sendTimeoutMs,
                                  uint32_t recvTimeoutMs ){

    int sock;
    int status;
    struct sockaddr_in address;

    PlaintextParams_t * pPlaintextParams = NULL;
    pPlaintextParams = pNetworkContext->pParams;

    /* Opens the socket on this side */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if( sock < 0 ) return SOCKETS_API_ERROR;

    /* Connects to the socket on the server side*/
    address.sin_len = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_port = htons( pServerInfo->port );
    address.sin_addr.s_addr = inet_addr( pServerInfo->pHostName );
    status = connect( sock, (struct sockaddr *)&address, sizeof(address) );

    if( status < 0 ) return SOCKETS_API_ERROR;

    pPlaintextParams->socketDescriptor = sock;

    return SOCKETS_SUCCESS;
}
//-----------------------------------------------------------------------------
SocketStatus_t Plaintext_Disconnect( const NetworkContext_t * pNetworkContext ){

    PlaintextParams_t * pPlaintextParams = NULL;
    pPlaintextParams = pNetworkContext->pParams;

    /* Disconnects from the server */
    shutdown( pPlaintextParams->socketDescriptor, SHUT_RDWR );

    /* Closes this socket */
    close( pPlaintextParams->socketDescriptor );

    return SOCKETS_SUCCESS;
}
//-----------------------------------------------------------------------------
int32_t Plaintext_Recv( NetworkContext_t * pNetworkContext,
                        void * pBuffer,
                        size_t bytesToRecv ){

    PlaintextParams_t * pPlaintextParams = NULL;
    int32_t bytesReceived = -1, pollStatus = 1;
    struct pollfd pollFds;

    assert( pNetworkContext != NULL && pNetworkContext->pParams != NULL );
    assert( pBuffer != NULL );
    assert( bytesToRecv > 0 );

    /* Get receive timeout from the socket to use as the timeout for #select. */
    pPlaintextParams = pNetworkContext->pParams;

    /* Initialize the file descriptor.
     * #POLLPRI corresponds to high-priority data while #POLLIN corresponds
     * to any other data that may be read. */
    pollFds.events = POLLIN | POLLPRI;
    pollFds.revents = 0;
    /* Set the file descriptor for poll. */
    pollFds.fd = pPlaintextParams->socketDescriptor;

    /* Check if there is data to read (without blocking) from the socket. */
    pollStatus = poll( &pollFds, 1, 0 );

    if( pollStatus > 0 )
    {
        /* The socket is available for receiving data. */
        bytesReceived = ( int32_t ) recv( pPlaintextParams->socketDescriptor,
                                          pBuffer,
                                          bytesToRecv,
                                          0 );
    }
    else if( pollStatus < 0 )
    {
        /* An error occurred while polling. */
        bytesReceived = -1;
    }
    else
    {
        /* No data available to receive. */
        bytesReceived = 0;
    }

    /* Note: A zero value return from recv() represents
     * closure of TCP connection by the peer. */
    if( ( pollStatus > 0 ) && ( bytesReceived == 0 ) )
    {
        /* Peer has closed the connection. Treat as an error. */
        bytesReceived = -1;
    }
    else if( bytesReceived < 0 )
    {
        // logTransportError( errno );
    }
    else
    {
        /* Empty else MISRA 15.7 */
    }

    return bytesReceived;
}
//-----------------------------------------------------------------------------
int32_t Plaintext_Send( NetworkContext_t * pNetworkContext,
                        const void * pBuffer,
                        size_t bytesToSend ){

    PlaintextParams_t * pPlaintextParams = NULL;
    int32_t bytesSent = -1, pollStatus = -1;
    struct pollfd pollFds;

    assert( pNetworkContext != NULL && pNetworkContext->pParams != NULL );
    assert( pBuffer != NULL );
    assert( bytesToSend > 0 );

    /* Get send timeout from the socket to use as the timeout for #select. */
    pPlaintextParams = pNetworkContext->pParams;

    /* Initialize the file descriptor. */
    pollFds.events = POLLOUT;
    pollFds.revents = 0;
    /* Set the file descriptor for poll. */
    pollFds.fd = pPlaintextParams->socketDescriptor;

    /* Check if data can be written to the socket.
     * Note: This is done to avoid blocking on send() when
     * the socket is not ready to accept more data for network
     * transmission (possibly due to a full TX buffer). */
    pollStatus = poll( &pollFds, 1, 0 );

    if( pollStatus > 0 )
    {
        /* The socket is available for sending data. */
        bytesSent = ( int32_t ) send( pPlaintextParams->socketDescriptor,
                                      pBuffer,
                                      bytesToSend,
                                      0 );
    }
    else if( pollStatus < 0 )
    {
        /* An error occurred while polling. */
        bytesSent = -1;
    }
    else
    {
        /* Socket is not available for sending data. */
        bytesSent = 0;
    }

    if( ( pollStatus > 0 ) && ( bytesSent == 0 ) )
    {
        /* Peer has closed the connection. Treat as an error. */
        bytesSent = -1;
    }
    else if( bytesSent < 0 )
    {
        // logTransportError( errno );
    }
    else
    {
        /* Empty else MISRA 15.7 */
    }

    return bytesSent;
}
//-----------------------------------------------------------------------------
//=============================================================================
