#include <iostream>

#include "soft_meter.h"


#include "app_database.h"
#include "db_cosem_clock.h"
#include "db_cosem_associations.h"
#include "db_cosem_image_transfer.h"

const db_attr_descr clock_attributes[] = {
{ DB_ACCESS_GETSET, 2, DB_TYPE_DATE_TIME },
{ DB_ACCESS_GETSET, 3, DB_TYPE_SIGNED16 },
{ DB_ACCESS_GET,     4, DB_TYPE_UNSIGNED8 },
{ DB_ACCESS_GETSET, 5, DB_TYPE_DATE_TIME },
{ DB_ACCESS_GETSET, 6, DB_TYPE_DATE_TIME },
{ DB_ACCESS_GETSET, 7, DB_TYPE_SIGNED8 },
{ DB_ACCESS_GETSET, 8, DB_TYPE_BOOLEAN },
{ DB_ACCESS_GETSET, 9, DB_TYPE_ENUM },
};

const db_attr_descr current_association_attributes[] = {
{ DB_ACCESS_GET, 2, DB_TYPE_ARRAY },
{ DB_ACCESS_GET, 3, DB_TYPE_STRUCTURE },
{ DB_ACCESS_GET, 4, DB_TYPE_STRUCTURE },
{ DB_ACCESS_GET, 5, DB_TYPE_STRUCTURE },
{ DB_ACCESS_GET, 6, DB_TYPE_STRUCTURE },
{ DB_ACCESS_GETSET, 8, DB_TYPE_ENUM },
{ DB_ACCESS_GETSET, 9, DB_TYPE_OCTET_STRING },
{ DB_ACCESS_GETSET, 1, DB_TYPE_OCTET_STRING },
};

const db_attr_descr current_association_methods[] = {
{ DB_ACCESS_GETSET, 1, DB_TYPE_OCTET_STRING },
};

const db_object_descr clock_objects[] = {
    {&clock_attributes[0], NULL, 8U , { 0U, 0U, 1U, 0U, 0U, 255U } , 0U , 8U, 0U },
};

const db_object_descr associations_objects[] = {
    {&current_association_attributes[0], &current_association_methods[0], 15U , { 0U, 0U, 40U, 0U, 0U, 255U } , 0U , 7U, 1U },
};


const struct db_element gDataBaseList[] = {
    { &clock_objects[0], db_cosem_clock_func, 1U },
    { &associations_objects[0], db_cosem_associations_func, 1U },
};


#define COSEM_DATABASE_SIZE (sizeof(gDataBaseList)/sizeof(gDataBaseList[0]))


#define NUMBER_OF_CHANNELS 1U


void SoftMeter::Initialize()
{
    // Debug: fill buffer with pattern
    memset(&m_buffer[0], 0xAA, BUF_SIZE);

    m_memory.data = &m_buffer[0];
    m_memory.offset = BUF_WRAPPER_OFFSET;
    m_memory.max_size = BUF_SIZE;
    m_currentChannel = -1;

    m_channels.resize(NUMBER_OF_CHANNELS);
}

void SoftMeter::AddDefaultAssociations()
{
    m_assos_config.push_back(// Client public association
        { {16U, 1U},
          CSM_CBLOCK_GET | CSM_CBLOCK_BLOCK_TRANSFER_WITH_GET_OR_READ,
          0U, // No auto-connected
        });
    
    m_assos_config.push_back(
        // Client management association
        { {1U, 1U},
            CSM_CBLOCK_GET | CSM_CBLOCK_ACTION | CSM_CBLOCK_SET |CSM_CBLOCK_BLOCK_TRANSFER_WITH_GET_OR_READ | CSM_CBLOCK_SELECTIVE_ACCESS,
            0U, // No auto-connected
        }
    );

    m_assos.resize(m_assos_config.size());

}

void SoftMeter::SendMessage(const std::string &message)
{
    if (message.size() > BUF_SIZE)
    {
        std::cerr << "Message too long!" << std::endl;
        return;
    }

    hex2bin(message.c_str(), reinterpret_cast<char *>(&m_buffer[BUF_WRAPPER_OFFSET]), message.size());

    // channel to index
    int ret = HandleMessage(m_currentChannel - 1, &m_memory, message.size() / 2);

    if (ret > 0)
    {
        std::cout << "Reply: ";
        print_hex((const char *)&m_buffer[BUF_APDU_OFFSET], ret);
    }
    else
    {
        std::cout << "No reply" << std::endl;
    }
}

void SoftMeter::Connect()
{
    m_currentChannel = csm_server_connect(&m_context);
    std::cout << "Connected!" << std::endl;
}

void SoftMeter::Disconnect()
{
    if (m_currentChannel > 0)
    {
        csm_server_disconnect(&m_context, m_currentChannel);
    }
    m_currentChannel = -1;
}


int SoftMeter::HandleMessage(uint8_t channel, memory_t *b, uint32_t payload_size)
{
    int ret = -1;
    uint16_t version;
    uint16_t apdu_size;
    csm_array packet;
    uint8_t *buffer = b->data + b->offset;

    // The TCP/IP Cosem packet is sent with a header. See GreenBook 8 7.3.3.2 The wrapper protocol data unit (WPDU)

    // Version, 2 bytes
    // Source wPort, 2 bytes
    // Destination wPort: 2 bytes
    // Length: 2 bytes
    CSM_LOG("[LLC] TCP Packet received");

    print_hex(reinterpret_cast<const char *>(buffer), payload_size);

    if ((payload_size > COSEM_WRAPPER_SIZE) && (channel < NUMBER_OF_CHANNELS))
    {
        version = GET_BE16(&buffer[0U]);
        m_channels[channel].request.llc.ssap = GET_BE16(&buffer[2]);
        m_channels[channel].request.llc.dsap = GET_BE16(&buffer[4]);
        apdu_size = GET_BE16(&buffer[6]);

        // Sanity check of the packet
        if ((payload_size == (apdu_size + COSEM_WRAPPER_SIZE)) &&(version == COSEM_WRAPPER_VERSION))
        {
            print_hex(reinterpret_cast<const char *>(&b->data[BUF_APDU_OFFSET]), apdu_size);

            // Then decode the packet, the reply, if any is located in the buffer
            // The reply is valid if the return code is > 0
            csm_array_init(&packet, (uint8_t *)&b->data[0], b->max_size, apdu_size, BUF_APDU_OFFSET);
            ret = csm_server_execute(&m_context, channel, &packet);

            if (ret > 0)
            {
                print_hex(reinterpret_cast<const char *>(&b->data[BUF_APDU_OFFSET]), ret);

                // Set Version
                PUT_BE16(&buffer[0], version);

                // Swap SSAP and DSAP
                PUT_BE16(&buffer[2], m_channels[channel].request.llc.dsap);
                PUT_BE16(&buffer[4], m_channels[channel].request.llc.ssap);

                // Update Cosem Wrapper length
                PUT_BE16(&buffer[6], (uint16_t) ret);

                // Add wrapper size to the data packet size
                ret += COSEM_WRAPPER_SIZE;
            }
        }
        else
        {
            CSM_ERR("[LLC] Bad Packet received");
        }
    }
    else
    {
        CSM_ERR("[LLC] Bad Packet received");
    }

    return ret;

}

void SoftMeter::CosemStackInitialize()
{
    srand(time(NULL)); // seed init

    // DLMS/Cosem stack initialization
    m_context.asso_list = m_assos.data();
    m_context.asso_conf_list = m_assos_config.data();
    m_context.channel_list = m_channels.data();
    m_context.asso_list_size = m_assos.size();
    m_context.channel_list_size = m_channels.size();
    m_context.db_access_func = csm_db_access_func;

    csm_server_init(&m_context);
}



