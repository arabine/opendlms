#pragma once

#include <stdlib.h>
#include <string.h>
#include <time.h> // to initialize the seed
#include <vector>

// Cosem stack
#include "csm_array.h"
#include "csm_ber.h"
#include "csm_channel.h"
#include "csm_config.h"

// Meter environment

#include "app_database.h"
#include "os_util.h"
#include "bitfield.h"
#include "transports.h"

// Simple software only meter to allow testing, or more!
// Can inject frames, replay a scenario...
// It does not integrate the transport layer (HDLC, TCP/IP ...)
class SoftMeter
{

public:
    // Buffer has the following format:
    //   | SC + AK + CHALLENGE | TCP wrapper | APDU
    // With
    // SC+AK+CHALLENGE: security AAD header, room booked to optimize buffer management with cyphering
    // wrapper: Cosem standard TCP wrapper (4 words)
    // APDU: Cosem APDU

    static const uint16_t COSEM_WRAPPER_VERSION = 0x0001U;
    #define COSEM_WRAPPER_SIZE 8U
    #define BUF_SIZE (CSM_DEF_PDU_SIZE + CSM_DEF_MAX_HLS_SIZE + COSEM_WRAPPER_SIZE)


    #define BUF_WRAPPER_OFFSET  (CSM_DEF_MAX_HLS_SIZE)
    #define BUF_APDU_OFFSET     (COSEM_WRAPPER_SIZE + CSM_DEF_MAX_HLS_SIZE)


    void Initialize();
    void CosemStackInitialize();
    void AddDefaultAssociations();

    void Connect();
    void Disconnect();

    void SendMessage(const std::string &message);

private:

    
    std::vector<csm_asso_state> m_assos;
    std::vector<csm_asso_config> m_assos_config;
    std::vector<csm_channel> m_channels;

    csm_server_context_t m_context;

    int m_currentChannel{-1};

    uint8_t m_buffer[BUF_SIZE]; // working buffer, keep it private
    memory_t m_memory;

    int HandleMessage(uint8_t channel, memory_t *b, uint32_t payload_size);
};


