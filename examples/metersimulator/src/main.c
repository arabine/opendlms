
#include "meter.h"
#include "tcp_server.h"
#include "server_config.h"

int main(int argc, const char * argv[])
{
    (void) argc;
    (void) argv;

    meter_initialize();
    
    printf("Starting DLMS/Cosem meter simulator\r\nCosem library version: %s\r\n\r\n", CSM_DEF_LIB_VERSION);

    int ret = tcp_server_init(meter_tcp_data_handler, meter_connect, meter_disconnect, TCP_PORT);
    
    printf("Exiting DLMS/Cosem meter simulator\r\n");

    return 0;
}

