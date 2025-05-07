#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <stdlib.h>
#include "transports.h"

void tcp_server_send(int8_t channel_id, const char *buffer, size_t size);
int tcp_server_init(data_handler data_func, connection_handler conn_func, disconnection_handler discon_func, int tcp_port);

#endif // TCP_SERVER_H
