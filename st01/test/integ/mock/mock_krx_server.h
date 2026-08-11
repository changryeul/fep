/*------------------------------------------------------------------------
#   Module  : Mock KRX TCP Server
#   File    : mock_krx_server.h
#   Purpose : Header for mock KRX server used in integration tests.
------------------------------------------------------------------------*/
#ifndef MOCK_KRX_SERVER_H
#define MOCK_KRX_SERVER_H

/* Server states */
#define MOCK_ST_INIT        0
#define MOCK_ST_CONNECTED   1
#define MOCK_ST_LOGON       2
#define MOCK_ST_LINKED      3
#define MOCK_ST_CLOSED      4

/* Default port */
#define MOCK_DEFAULT_PORT   19999

/* Max message buffer */
#define MOCK_BUF_SIZE       4096

/* Return codes */
#define MOCK_OK             0
#define MOCK_ERR            -1

#endif /* MOCK_KRX_SERVER_H */
