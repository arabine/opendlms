
#include "os_util.h"
#include "hdlc.h"

#include "catch.hpp"
#include <iostream>
#include <cstdlib>
#include <cstring>


// Quick simple tests to checks that the clock is working in nominal way
// Not full test
void StreamingDecoder()
{
    // The following packet contains three HDLC frames.
    // We are using it to test the streaming capability of the decoder
    static const char multi_frames[] = "7EA88C0300020023421CB9E6E700C401C10001010205020209060000600100FF0A08736E303030303030020509060100628601FF1224D502020F001604020211001101090C07E10A19030D1813FF8000FF01010206020309060101020800FF060000000002020F03161E020309060101010800FF060000000002020F03161E020309060101050800FF060096CC7E7EA88C0300020023442ADC00000002020F001620020309060101060800FF060000000002020F001620020309060101070800FF060000000002020F001620020309060101080800FF060000000002020F00162001010201020509060101010801FF060000000002020F00161E0000010102020906000062853DFF0105020312000002020F00161B090C07E1EED87E7EA07A030002002356990D0A19FF0D1813FF8000FF020312000002020F00161B090C07E10A19FF0D1503FF8000FF020312000002020F00161B090C07C80106FF161E00FF8000FF020312000002020F00161B090C07C80106FF160F00FF8000FF020312000002020F00161B090C07C80106FF160000FF8000FF05B47E";

    // transform the hexadecimal string into an array of integers
    int sz = sizeof(multi_frames);

    size_t packet_size = sz/2U;
    uint8_t *packet = (uint8_t *) malloc(packet_size);

    if (packet != NULL)
    {
        int ret;
        uint8_t *packet_ptr = packet;

        hex2bin(multi_frames, (char *)packet, sz);
        //print_hex(packet, packet_size);

        hdlc_t hdlc;
        hdlc_init(&hdlc);

        do
        {
            ret = hdlc_decode(&hdlc, packet_ptr, packet_size);

            hdlc_print_result(&hdlc, ret);

            REQUIRE(HDLC_OK == ret);

            debug_puts("HDLC data: ");
            print_hex((const char*)&packet[hdlc.data_index], hdlc.data_size);
            debug_puts("\r\n");

            packet_size -= hdlc.frame_size;
            packet_ptr += hdlc.frame_size;
        }
        while (packet_size);

        free(packet);
    }
    else
    {
       printf("Cannot allocate memory!\r\n");
    }

}


void SnrmEncoder()
{
    static const char snrm_expected[] = "7EA0210002002303939A74818012050180060180070400000001080400000007655E7E";

    // transform the hexadecimal string into an array of integers
    int sz = sizeof(snrm_expected);
    uint8_t hdlc_buffer[256];

    size_t snrm_size = sz/2U;
    uint8_t *snrm = (uint8_t *) malloc(snrm_size);

    if (snrm != NULL)
    {
        int ret;

        hex2bin(snrm_expected, (char *)snrm, sz);

        hdlc_t hdlc;
        hdlc_init(&hdlc);

        hdlc.sender = HDLC_CLIENT;
        hdlc.client_addr = 1U;
        hdlc.addr_len = 4U;
        hdlc.logical_device = 1U;
        hdlc.phy_address = 17U;

        ret = hdlc_encode_snrm(&hdlc, hdlc_buffer, sizeof(hdlc_buffer));

        debug_puts("HDLC data gene: ");
        print_hex((const char*)&hdlc_buffer[0], ret);
        debug_puts("\r\n");
        debug_puts("HDLC reference: ");
        print_hex((const char*)&snrm[0], ret);
        debug_puts("\r\n");

        REQUIRE(ret == snrm_size);

        int compare = memcmp(hdlc_buffer, snrm, snrm_size);

        REQUIRE(compare == 0);

        free(snrm);
    }
    else
    {
       printf("Cannot allocate memory!\r\n");
    }
}

void SnrmDecoder()
{
    static const char snrm_no_parameters[] = "7EA00A00020023219318717E";
    static const char snrm_normal[] = "7EA023210002002373F6C58180140502008006020080070400000001080400000001CE6A7E";

    // transform the hexadecimal string into an array of integers
    int sz1 = sizeof(snrm_no_parameters);
    int sz2 = sizeof(snrm_normal);
    uint8_t hdlc_buffer[256];

    size_t snrm1_size = sz1/2U;
    size_t snrm2_size = sz2/2U;

    uint8_t *snrm1 = (uint8_t *) malloc(snrm1_size);
    uint8_t *snrm2 = (uint8_t *) malloc(snrm2_size);

    if ((snrm1 != NULL) &&
        (snrm2 != NULL))
    {
        int ret;

        hex2bin(snrm_no_parameters, (char *)snrm1, sz1);
        hex2bin(snrm_normal, (char *)snrm2, sz2);

        hdlc_t hdlc;

        hdlc_init(&hdlc);
        hdlc.sender = HDLC_CLIENT;

        ret = hdlc_decode(&hdlc, snrm1, snrm1_size);
        hdlc_print_result(&hdlc, ret);

        REQUIRE(ret == HDLC_OK);

        hdlc_init(&hdlc);
        ret = hdlc_decode(&hdlc, snrm2, snrm2_size);
        hdlc_print_result(&hdlc, ret);

        REQUIRE(ret == HDLC_OK);

        free(snrm1);
        free(snrm2);
    }
    else
    {
       printf("Cannot allocate memory!\r\n");
    }
}


void AckEncoder()
{
    static const char rr_expected[] = "7EA00A0002002507B132D27E";
    // transform the hexadecimal string into an array of integers
    int sz = sizeof(rr_expected);
    uint8_t hdlc_buffer[256];

    size_t rr_size = sz/2U;
    uint8_t *rr = (uint8_t *) malloc(rr_size);

    if (rr != NULL)
    {
        int ret;

        hex2bin(rr_expected, (char *)rr, sz);

        hdlc_t hdlc;
        hdlc_init(&hdlc);

        hdlc.sender = HDLC_CLIENT;
        hdlc.client_addr = 3U;
        hdlc.addr_len = 4U;
        hdlc.logical_device = 1U;
        hdlc.phy_address = 18U;
        hdlc.rrr = 5U;

        ret = hdlc_encode_rr(&hdlc, hdlc_buffer, sizeof(hdlc_buffer));

        debug_puts("HDLC data gene: ");
        print_hex((const char*)&hdlc_buffer[0], ret);
        debug_puts("\r\n");
        debug_puts("HDLC reference: ");
        print_hex((const char*)&rr[0], ret);
        debug_puts("\r\n");

        REQUIRE(ret == rr_size);

        int compare = memcmp(hdlc_buffer, rr, rr_size);

        REQUIRE(compare == 0);

        free(rr);
    }
    else
    {
       printf("Cannot allocate memory!\r\n");
    }
}

void InformationDecoder()
{
    static const char info[] = "7EA03A070002002530D388E6E7006129A109060760857405080101A203020100A305A103020100BE10040E0800065F1F040000181D0200000780F57E";

    // transform the hexadecimal string into an array of integers
    int sz1 = sizeof(info);
    uint8_t hdlc_buffer[256];

    size_t info_size = sz1/2U;

    uint8_t *info_frame = (uint8_t *) malloc(info_size);

    if (info_frame != NULL)
    {
        int ret;

        hex2bin(info, (char *)info_frame, sz1);

        hdlc_t hdlc;

        hdlc_init(&hdlc);
        ret = hdlc_decode(&hdlc, info_frame, info_size);
        hdlc_print_result(&hdlc, ret);

        REQUIRE(ret == HDLC_OK);


        free(info_frame);
    }
    else
    {
       printf("Cannot allocate memory!\r\n");
    }
}



TEST_CASE( "HDLC1", "[Streaming]" )
{
    puts("\r\n--------------------------  HDLC TEST 1  --------------------------\r\n");
    StreamingDecoder();
}

TEST_CASE( "HDLC2", "[SNRM encoder]" )
{
    puts("\r\n--------------------------  HDLC TEST 2  --------------------------\r\n");
    SnrmEncoder();
}

TEST_CASE( "HDLC3", "[SNRM decoder]" )
{
    puts("\r\n--------------------------  HDLC TEST 3  --------------------------\r\n");
    SnrmDecoder();
}

TEST_CASE( "HDLC4", "[RR]" )
{
    puts("\r\n--------------------------  HDLC TEST 4 --------------------------\r\n");
    AckEncoder();
}

TEST_CASE( "HDLC5", "[INFO]" )
{
    puts("\r\n--------------------------  HDLC TEST 5  --------------------------\r\n");
    InformationDecoder();
}



