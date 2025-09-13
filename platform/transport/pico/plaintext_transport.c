//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "plaintext_transport.h"

#include "stdint.h"
#include "stddef.h"

#include "mdrivers/wiznet/dhcp.h"
#include "mdrivers/wiznet/socket.h"
#include "mdrivers/wiznet/wizchip_conf.h"

#ifndef MQTT_MNG_PICO_W5500_SN
#define MQTT_MNG_PICO_W5500_SN  ( 0U )
#endif

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
    pPlaintextParams->socketDescriptor = MQTT_MNG_PICO_W5500_SN;
    status = socket(MQTT_MNG_PICO_W5500_SN, Sn_MR_TCP, port, 0);
    if( status != 0 ) return SOCKETS_API_ERROR;

    status = connect(MQTT_MNG_PICO_W5500_SN, ip, pServerInfo->port);
    
    return SOCKETS_SUCCESS;
}
//-----------------------------------------------------------------------------
SocketStatus_t Plaintext_Disconnect( const NetworkContext_t * pNetworkContext ){

    (void)pNetworkContext;

    /* Disconnects from the server */
    disconnect(MQTT_MNG_PICO_W5500_SN);

    /* Closes this socket */
    close(MQTT_MNG_PICO_W5500_SN);

    return SOCKETS_SUCCESS;
}
//-----------------------------------------------------------------------------
int32_t Plaintext_Recv( NetworkContext_t * pNetworkContext,
                        void * pBuffer,
                        size_t bytesToRecv ){
    
    (void) pNetworkContext;
    int32_t pollstatus;
    int32_t ret = -1;

    pollstatus = poll_rx(0);

    if(pollstatus == 0) return 0;

    ret = recv(MQTT_MNG_PICO_W5500_SN, (uint8_t *)pBuffer, (uint16_t)bytesToRecv);

    return ret; 
}
//-----------------------------------------------------------------------------
int32_t Plaintext_Send( NetworkContext_t * pNetworkContext,
                        const void * pBuffer,
                        size_t bytesToSend ){

    (void) pNetworkContext;
    int32_t ret = -1;
   
    ret = send(MQTT_MNG_PICO_W5500_SN, (uint8_t *)pBuffer, (uint16_t)bytesToSend);

    return ret;
}
//-----------------------------------------------------------------------------
//=============================================================================



