#ifndef TRANSPORTS_H
#define TRANSPORTS_H

#include <stdint.h>


/**
 * @brief Cosem handler called from the transport layer upon reception of data
 */
typedef int (*data_handler)(int8_t channel_id, uint8_t *data, uint32_t payload_size, uint32_t buffer_size);

/**
 * @brief Connection handler called from the transport
 * @param channel: channel number, 0 if new connection
 * @param event: transport layer event
 * @return channel id
 */
typedef int8_t (*connection_handler)();

/**
 * @brief Disconnection handler called from the transport
 * @param channel: channel number
 * 
 */
typedef void (*disconnection_handler)(int8_t channel_id);

#endif // TRANSPORTS_H

