//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "plaintext_transport.h"

#include "stdint.h"
#include "stddef.h"

#include "tif/c/drivers/wiznet/dhcp.h"
#include "tif/c/drivers/wiznet/socket.h"
#include "tif/c/drivers/wiznet/wizchip_conf.h"

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
    
    int8_t status;
    uint8_t ip[4];
    uint16_t port = 8080;
    
    unsigned int a, b, c, d;

    PlaintextParams_t * pPlaintextParams = NULL;
    pPlaintextParams = pNetworkContext->pParams;
    
    sscanf(pServerInfo->pHostName, "%u.%u.%u.%u", &a, &b, &c, &d);
    ip[0] = a;
    ip[1] = b;
    ip[2] = c;
    ip[3] = d;

	/* Opens the socket on this side */
	//status = socket((uint8_t)pPlaintextParams->socketDescriptor, Sn_MR_TCP, port, 0);
	status = socket(0, Sn_MR_TCP, port, 0);
    //LogInfo( "Socket status: %d.", status );
    if( status != 0 ) return SOCKETS_API_ERROR;

	/* Connects to the socket on the server side*/
	//status = connect((uint8_t)pPlaintextParams->socketDescriptor, ip, port);

	status = connect(0, ip, pServerInfo->port);


    //if( status != SOCK_OK ) return SOCKETS_CONNECT_FAILURE;
    
    return SOCKETS_SUCCESS;
}
//-----------------------------------------------------------------------------
SocketStatus_t Plaintext_Disconnect( const NetworkContext_t * pNetworkContext ){

    PlaintextParams_t * pPlaintextParams = NULL;
    pPlaintextParams = pNetworkContext->pParams;

	/* Disconnects from the server */
	//disconnect((uint8_t)pPlaintextParams->socketDescriptor);
	disconnect(0);

	/* Closes this socket */
	//close((uint8_t)pPlaintextParams->socketDescriptor);
	close(0);

    return SOCKETS_SUCCESS;
}
//-----------------------------------------------------------------------------
int32_t Plaintext_Recv( NetworkContext_t * pNetworkContext,
                        void * pBuffer,
                        size_t bytesToRecv ){
    
    PlaintextParams_t * pPlaintextParams = NULL;
    pPlaintextParams = pNetworkContext->pParams;
    int32_t pollstatus;
    int32_t ret = -1;

    //ret = recv((uint8_t)pPlaintextParams->socketDescriptor, (uint8_t *)pBuffer, bytesToRecv);
    pollstatus = poll_rx(0);

    if(pollstatus == 0) return 0;

    ret = recv(0, (uint8_t *)pBuffer, (uint16_t)bytesToRecv);

    return ret; 
}
//-----------------------------------------------------------------------------
int32_t Plaintext_Send( NetworkContext_t * pNetworkContext,
                        const void * pBuffer,
                        size_t bytesToSend ){

    PlaintextParams_t * pPlaintextParams = NULL;
    pPlaintextParams = pNetworkContext->pParams;
    int32_t ret = -1;
   
    //ret = send((uint8_t)pPlaintextParams->socketDescriptor, (uint8_t *)pBuffer, bytesToSend);
    ret = send(0, (uint8_t *)pBuffer, (uint16_t)bytesToSend);

    return ret;
}
//-----------------------------------------------------------------------------
//=============================================================================



