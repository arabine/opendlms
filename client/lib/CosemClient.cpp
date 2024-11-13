/**
 * Cosem client engine
 *
 * Copyright (c) 2016, Anthony Rabine
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the BSD license.
 * See LICENSE.txt for more details.
 *
 */

#include <Util.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <fstream>


#include "CosemClient.h"
#include "serial.h"
#include "os_util.h"
#include "AxdrPrinter.h"

#include "hdlc.h"
#include "csm_array.h"
#include "csm_services.h"
#include "csm_axdr_codec.h"
#include "csm_definitions.h"
#include "clock.h"


CosemClient::CosemClient()
    : mModemState(DISCONNECTED)
    , mCosemState(CONNECT_HDLC)
    , mReadIndex(0U)
    , mMeterIndex(0U)
{

}


bool CosemClient::Initialize(const std::string &commFile, const std::string &objectsFile, const std::string &meterFile)
{
    bool ok = false;

    Transport::Params params;

    Result result;
    result.subject = "OPEN COM PORT";

    mConf.ParseComFile(commFile, params);
    mConf.ParseObjectsFile(objectsFile);
    mConf.ParseSessionFile(meterFile);

    if (mConf.modem.useModem)
    {
        std::cout << "** Using Modem device" << std::endl;
        mModemState = DISCONNECTED;
    }
    else
    {
        // Skip Modem state chart when no modem is in use
        mModemState = CONNECTED;
    }

    ok = mTransport.Open(params);
    if (ok)
    {
        mTransport.Start();
    }
    else
    {
        std::stringstream ss;
        ss << "** Cannot open serial port " << params.port << " at " << params.baudrate << " bauds";
        result.SetError(ss.str());
        mResults.push_back(result);
    }
    return ok;
}

void CosemClient::SetStartDate(const std::string &date)
{
    mConf.start_date = date;
}

void CosemClient::SetEndDate(const std::string &date)
{
    mConf.end_date = date;
}

void CosemClient::WaitForStop()
{
    mTransport.WaitForStop();
}


bool CosemClient::HdlcProcess(Meter &meter, const std::string &send, std::string &rcv, int timeout, bool enableRetries)
{
    bool retCode = false;

    csm_array_init(&mRcvArray, (uint8_t*)&mRcvBuffer[0], cBufferSize, 0U, 0U);

    bool loop = true;

    std::string dataToSend = send;
    std::string dataSent;
    uint32_t retries = 0U;

    if (!enableRetries)
    {
        // Disable retries by saturate the counter
        retries = mConf.retries;
    }

    do
    {
        if (dataToSend.size() > 0)
        {
            if (mTransport.Send(dataToSend, PRINT_HEX))
            {
                if (meter.hdlc.type == HDLC_PACKET_TYPE_I)
                {
                    if (meter.hdlc.sss == 7U)
                    {
                        meter.hdlc.sss = 0U;
                    }
                    else
                    {
                        meter.hdlc.sss++;
                    }
                }
                dataSent = dataToSend;
                dataToSend.clear();
            }
            else
            {
                loop = false;
                retCode = false;
            }
        }

        std::string data;
        hdlc_t hdlc;

        if (mTransport.WaitForData(data, timeout))
        {
            // We have something, add buffer
            if (!csm_array_write_buff(&mRcvArray, (const uint8_t*)data.c_str(), data.size()))
            {
                loop = false;
                retCode = false;
            }

            if (loop)
            {
                uint8_t *ptr = csm_array_rd_data(&mRcvArray);
                uint32_t size = csm_array_unread(&mRcvArray);


//                printf("Ptr: 0x%08X, Unread: %d\r\n", (unsigned long)ptr, size);
//
//                puts("Sent frame: ");
//                print_hex(mSendCopy.c_str(), mSendCopy.size());
//                puts("\r\n");

                // the frame seems correct, check echo
                if (dataSent.compare(0, dataSent.size(), (char*)ptr, size) == 0)
                {
                    // remove echo from the string
                    csm_array_reader_jump(&mRcvArray, size);
                    ptr = csm_array_rd_data(&mRcvArray);
                    size = csm_array_unread(&mRcvArray);
                    std::cout << "Echo canceled!" << std::endl;
                }

//                puts("Decoding: ");
//                print_hex((const char *)ptr, size);
//                puts("\r\n");

                do
                {
                    hdlc.sender = HDLC_SERVER;
                    int ret = hdlc_decode(&hdlc, ptr, size);
                    if (ret == HDLC_OK)
                    {


                        if (hdlc.type == HDLC_PACKET_TYPE_RR)
                        {
                           // Send again the request
                            puts("RR sync, send again\r\n");
                           dataToSend = send;
                           size = 0U;
                        }
                        else
                        {
                            std::cout << "Data packet" << std::endl;
                            // God packet! Copy to cosem data
                            rcv.append((const char*)&ptr[hdlc.data_index], hdlc.data_size);

                            // Continue with next one
                            csm_array_reader_jump(&mRcvArray, hdlc.frame_size);

                            if (hdlc.type == HDLC_PACKET_TYPE_I)
                            {
                                // ack last hdlc frame
                                if (hdlc.sss == 7U)
                                {
                                    meter.hdlc.rrr = 0U;
                                }
                                else
                                {
                                    meter.hdlc.rrr = hdlc.sss + 1;
                                }
                            }

                            // Test if it is a last HDLC packet
                            if ((hdlc.segmentation == 0U) &&
                                (hdlc.poll_final == 1U))
                            {
                                puts("Final packet\r\n");

                                retCode = true; // good Cosem packet
                                loop = false; // quit
                            }
                            else if (hdlc.segmentation == 1U)
                            {
                                puts("Segmentation packet: ");
                                hdlc_print_result(&hdlc, HDLC_OK);
                                // There are remaining frames to be received.
                                if (hdlc.poll_final == 1U)
                                {
                                    // Send RR
                                    hdlc.sender = HDLC_CLIENT;
                                    size = hdlc_encode_rr(&meter.hdlc, (uint8_t*)&mSndBuffer[0], cBufferSize);
                                    dataToSend.assign(&mSndBuffer[0], size);
                                }
                            }

                            // go to next frame, if any
                            ptr = csm_array_rd_data(&mRcvArray);
                            size = csm_array_unread(&mRcvArray);
                        }
                    }
                    else
                    {
                        // Maybe a partial packet, re-try later
                        size = 0U;
                    }
                }
                while (size);
            }
        }
        else
        {
            retries++;
            if (retries > mConf.retries)
            {
                // Timeout, we can't wait further for HDLC packets
                retCode = false;
                loop = false;
            }
            else
            {
                puts("Try to resync\r\n");
                csm_array_init(&mRcvArray, (uint8_t*)&mRcvBuffer[0], cBufferSize, 0U, 0U);
                // try to re-sync with server, send RR frame
                // Send RR
                hdlc.sender = HDLC_CLIENT;
                uint32_t size = hdlc_encode_rr(&meter.hdlc, (uint8_t*)&mSndBuffer[0], cBufferSize);
                dataToSend.assign(&mSndBuffer[0], size);
            }
        }
    }
    while (loop);

    return retCode;
}


bool CosemClient::SendModem(const std::string &command, const std::string &expected, std::string &modemReply, uint32_t timeout)
{
    bool retCode = false;

    if (mTransport.Send(command, PRINT_RAW))
    {
        bool loop = true;
        do {
            std::string data;
            if (mTransport.WaitForData(data, timeout))
            {
                Transport::Printer(data.c_str(), data.size(), PRINT_RAW);
                modemReply += data;

                // Wait again, if there is remaing data
                if (mTransport.WaitForData(data, 2U))
                {
                    modemReply += data;
                }

                if (modemReply.find(expected) != std::string::npos)
                {
                    retCode = true;
                }

                loop = false;
            }
            else
            {
                loop = false;
                retCode = false;
            }
        }
        while (loop);
    }

    std::this_thread::sleep_for(std::chrono::seconds(2U));

    return retCode;
}

int CosemClient::ConnectHdlc(Meter &meter)
{
    int ret = -1;

    int size = hdlc_encode_snrm(&meter.hdlc, (uint8_t *)&mSndBuffer[0], cBufferSize);

    std::string snrmData(&mSndBuffer[0], size);
    std::string data;

    if (HdlcProcess(meter, snrmData, data, mConf.timeout_connect, false))
    {
        ret = data.size();
        Transport::Printer(data.c_str(), data.size(), PRINT_HEX);

        // Decode UA
        ret = hdlc_decode_info_field(&meter.hdlc, (const uint8_t *)data.c_str(), data.size());
        if (ret == HDLC_OK)
        {
            hdlc_print_result(&meter.hdlc, ret);
            ret = 1U;
        }
    }

    return ret;
}

bool HasGoodLlc(csm_array *array)
{
    bool ret = false;
    uint8_t llc1, llc2, llc3;

    int valid = csm_array_read_u8(array, &llc1);
    valid = valid && csm_array_read_u8(array, &llc2);
    valid = valid && csm_array_read_u8(array, &llc3);

    if (valid && (llc1 == 0xE6U) &&
        (llc2 == 0xE7U) &&
        (llc3 == 0x00U))
    {
        ret = true;
    }

    return ret;
}


Result CosemClient::Pass3And4(Meter &meter)
{
    static const uint32_t cDigestBufferSize = 256U; // enough size to store max hash algorithm result: FIXME make it dependent of the supported algorithms
    Result result;
    result.subject = "CONNECT COSEM (AARQ Pass 3)";

    uint8_t digest_stoc[cDigestBufferSize];
    uint8_t digest_ctos[cDigestBufferSize];

    Object obj;

    // Access to Current Association object, method 1 (reply to HLS authentication)
    obj.class_id = 15U;
    obj.ln = "0.0.40.0.0.255";
    obj.attribute_id = 1U;
    obj.dump = false;

    // Prepare request
    csm_response response;
    csm_request request;
    request.db_request.service = SVC_ACTION;
    request.type = SVC_REQUEST_NORMAL;
    request.sender_invoke_id = 0x41U;

    // For reception
    csm_array app_array;
    csm_array_init(&app_array, &mAppBuffer[0], cAppBufferSize, 0, 0);


    uint32_t digest_size = 0U;

    // The following three levels are basically the same mechanism
    if ((mAssoState.auth_level == CSM_AUTH_HIGH_LEVEL) ||
        (mAssoState.auth_level == CSM_AUTH_HIGH_LEVEL_MD5) ||
        (mAssoState.auth_level == CSM_AUTH_HIGH_LEVEL_SHA1))
    {
        uint8_t input_stoc[64U + 16U]; // max size of challenge + size of secret
        uint32_t input_stoc_size = mAssoState.handshake.stoc.size + 16U;

        // Also compute our challenge
        uint8_t input_ctos[64U + 16U]; // max size of challenge + size of secret
        uint32_t input_ctos_size = mAssoState.handshake.ctos.size + 16U;

        // Prepare packet
        memcpy(&input_stoc[0], &mAssoState.handshake.stoc.value[0U], mAssoState.handshake.stoc.size);
        hex2bin(meter.cosem.auth_hls_secret.c_str(),  (char* )&input_stoc[mAssoState.handshake.stoc.size], 32U);

        // Prepare packet
        memcpy(&input_ctos[0], &mAssoState.handshake.ctos.value[0U], mAssoState.handshake.ctos.size);
        hex2bin(meter.cosem.auth_hls_secret.c_str(),  (char* )&input_ctos[mAssoState.handshake.ctos.size], 32U);

        // Compute digest
        if (mAssoState.auth_level == CSM_AUTH_HIGH_LEVEL_MD5)
        {
            // MD5(StoC || HLS Secret)
            csm_hal_md5(&input_stoc[0U], input_stoc_size, &digest_stoc[0U]);
            csm_hal_md5(&input_ctos[0U], input_ctos_size, &digest_ctos[0U]);
            digest_size = 16U;
            std::cout << "** Digest MD5: " << std::endl;
        }
        if (mAssoState.auth_level == CSM_AUTH_HIGH_LEVEL_SHA1)
        {
           // SHA1(StoC || HLS Secret)
            csm_hal_sha1(&input_stoc[0U], input_stoc_size, &digest_stoc[0U]);
            csm_hal_sha1(&input_ctos[0U], input_ctos_size, &digest_ctos[0U]);
           digest_size = 20U;
           std::cout << "** Digest SHA1: " << std::endl;
        }
        else
        {
            // Custom authentication: SHA256 (StoC || HLS Secret)
            csm_hal_sha256(&input_stoc[0U], input_stoc_size, &digest_stoc[0U]);
            csm_hal_sha256(&input_ctos[0U], input_ctos_size, &digest_ctos[0U]);
            digest_size = 32U;
            std::cout << "** Digest SHA256 (Manufacturer): " << std::endl;
        }

        std::cout << "** Computed StoC: ";
        Transport::Printer((char*)&digest_stoc[0U], digest_size, PRINT_HEX);
        std::cout << std::endl << "** Computed CtoS: ";
        Transport::Printer((char*)&digest_ctos[0U], digest_size, PRINT_HEX);
        std::cout << std::endl;
    }
    else if (mAssoState.auth_level == CSM_AUTH_HIGH_LEVEL_GMAC)
    {
        // FIXME
        result.SetError("** HLS5/GMAC not implemented!");
    }
    else if (mAssoState.auth_level == CSM_AUTH_HIGH_LEVEL_SHA256)
    {
        // FIXME
        result.SetError("** SHA256/GMAC not implemented!");
    }
    else
    {
        result.SetError("** Authentication pass 3 failure: mechanism not supported.");
    }

    if (digest_size > 0U)
    {
        // Initialize data array for Action SET part
        csm_array_init(&request.db_request.additional_data.data, &mAppBuffer[0], cAppBufferSize, 0, 0);
        request.db_request.additional_data.enable = TRUE;

        if (csm_axdr_wr_octetstring(&request.db_request.additional_data.data, &digest_stoc[0], digest_size))
        {
            // Send Action, parse result (GET part)
            result = AccessObject(meter, obj, request, response, app_array);
            if (result.success)
            {
                result.subject = "CONNECT COSEM (AARQ Pass 4)";
                // first, test if action is OK and has some data
                // Data in response contains the secured CtoS challenge
                // The size depends on the level used
                if ((response.access_result == CSM_ACCESS_RESULT_SUCCESS) &&
                    (response.has_data == TRUE))
                {
                    // Get digest
                    uint32_t size;
                    int valid = csm_axdr_rd_octetstring(&app_array, &size);
                    valid = valid && (digest_size == size);

                    if (valid)
                    {
                        std::cout << "** Recieved CtoS: ";
                        Transport::Printer((char*)csm_array_rd_data(&app_array), digest_size, PRINT_HEX);
                        std::cout << std::endl;

                        // Now compute the CtoS digest with the one we have computed
                        // FIXME: in GMAC, there is a security header
                        if (!std::memcmp(csm_array_rd_data(&app_array), &digest_ctos[0U], digest_size))
                        {
                            std::cout << "** HLS Pass 3 and 4 success! " << std::endl;
                        }
                        else
                        {
                            result.SetError("** HLS Pass 4 failure: bad received CtoS.");
                        }
                    }
                    else
                    {
                        result.SetError("** Bad Action response data size or contents for HLS Pass 3/4.");
                    }
                }
                else
                {
                    result.SetError("** Bad Action response for HLS Pass 3/4. (maybe a bad authentication key)");
                }
            }
            else
            {
                // Error string diagnostic is set elsewhere
                // Add some clue for this context (authentication)
                result.diagnostic += " (maybe a bad authentication key)";
            }
        }
        else
        {
            result.SetError("** Internal error, code 42.");
        }
    }
    else
    {
        // Error string diagnostic is set elsewhere
    }

    return result;
}

std::string CosemClient::AuthResultToString(enum csm_asso_result result)
{
    std::stringstream ss;
    switch (result)
    {

    case CSM_ASSO_ERR_NULL:
        ss << "success!";
        break;
    case CSM_ASSO_NO_REASON_GIVEN:
        ss << "No reason given";
        break;
    case CSM_ASSO_AUTH_NOT_RECOGNIZED:
        ss << "Authentication not recognized";
        break;
    case CSM_ASSO_AUTH_MECANISM_NAME_REQUIRED:
        ss << "Mechanism name required";
        break;
    case CSM_ASSO_ERR_AUTH_FAILURE:
        ss << "Authentication failure";
        break;
    case CSM_ASSO_AUTH_REQUIRED:
        ss << "Authentication required";
        break;
    default:
        ss << "Error code not supported!";
        break;
    }

    return ss.str();
}

Result CosemClient::ConnectAarq(Meter &meter)
{
    Result result;
    result.subject = "CONNECT COSEM (AARQ)";

    csm_array scratch_array;
    csm_array_init(&scratch_array, &mScratch[0], cBufferSize, 0, 3);

    mAssoState.auth_level = meter.cosem.GetAuthLevelFromString();
    mAssoState.ref = LN_REF;

    if (csm_asso_encoder(&mAssoState, &scratch_array, CSM_ASSO_AARQ))
    {
        std::string request_data = EncapsulateRequest(meter, &scratch_array);
        std::string data;

        if (HdlcProcess(meter, request_data, data, mConf.timeout_request, true))
        {
            Transport::Printer(data.c_str(), data.size(), PRINT_HEX);

            csm_array_init(&scratch_array, &mScratch[0], cBufferSize, 0, 0);
            csm_array_write_buff(&scratch_array, (const uint8_t *)data.c_str(), data.size());

            if (HasGoodLlc(&scratch_array))
            {
                // Good Cosem server packet
                if (csm_asso_decoder(&mAssoState, &scratch_array, CSM_ASSO_AARE))
                {
                    if (mAssoState.handshake.accepted)
                    {
                        if (mAssoState.auth_level <= CSM_AUTH_LOW_LEVEL)
                        {
                            std::cout << "** Authentication success: access granted." << std::endl;
                        }
                        else if (mAssoState.auth_level > CSM_AUTH_LOW_LEVEL)
                        {
                            if (mAssoState.handshake.result == CSM_ASSO_AUTH_REQUIRED)
                            {
                                std::cout << "** High authentication: Starting pass 3." << std::endl;
                                result = Pass3And4(meter);
                            }
                            else
                            {
                                result.SetError("** FAILURE: result must be in state: authentication-required");
                            }
                        }
                        else
                        {
                            std::stringstream ss;
                            ss <<  "** FAILURE: " << AuthResultToString(mAssoState.handshake.result);
                            result.SetError(ss.str());
                        }
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "** FAILURE: " << AuthResultToString(mAssoState.handshake.result);
                        result.SetError(ss.str());
                    }
                }
                else
                {
                    result.SetError("** Cannot decode Cosem AARE");
                }
            }
            else
            {
                result.SetError("** Not a valid Cosem AARE LLC");
            }
        }
        else
        {
            result.SetError("** Cannot send/receive Cosem AARQ/AARE");
        }
    }
    else
    {
        result.SetError("** Internal error: cannot encode Cosem AARQ frame");
    }
    return result;
}

std::string CosemClient::GetLls()
{
    std::string lls;

    if (mConf.meters.size() > 0)
    {
        lls = mConf.meters[mMeterIndex].cosem.auth_password;
    }
    return lls;
}

std::string CosemClient::ResultToString(csm_data_access_result result)
{
    std::stringstream ss;
    switch (result)
    {
        case CSM_ACCESS_RESULT_SUCCESS:
            ss << "success!";
            break;
        case CSM_ACCESS_RESULT_HARDWARE_FAULT:
            ss << "Hardware fault";
            break;
        case CSM_ACCESS_RESULT_TEMPORARY_FAILURE:
            ss << "temporary failure";
            break;
        case CSM_ACCESS_RESULT_READ_WRITE_DENIED:
            ss << "read write denied";
            break;
        case CSM_ACCESS_RESULT_OBJECT_UNDEFINED:
            ss << "object undefined";
            break;
        case CSM_ACCESS_RESULT_OBJECT_CLASS_INCONSISTENT:
            ss << "object class inconsistent";
            break;
        case CSM_ACCESS_RESULT_OBJECT_UNAVAILABLE:
            ss << "object unavailable";
            break;
        case CSM_ACCESS_RESULT_TYPE_UNMATCHED:
            ss << "type unmatched";
            break;
        case CSM_ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED:
            ss << "scope of access violated";
            break;
        case CSM_ACCESS_RESULT_DATA_BLOCK_UNAVAILABLE:
            ss << "data block unavailable";
            break;
        case CSM_ACCESS_RESULT_LONG_GET_ABORTED:
            ss << "long get aborted";
            break;
        case CSM_ACCESS_RESULT_NO_LONG_GET_IN_PROGRESS:
            ss << "no long get in progress";
            break;
        case CSM_ACCESS_RESULT_LONG_SET_ABORTED:
            ss << "long set aborted";
            break;
        case CSM_ACCESS_RESULT_NO_LONG_SET_IN_PROGRESS:
            ss << "no long set in progress";
            break;
        case CSM_ACCESS_RESULT_DATA_BLOCK_NUMBER_INVALID:
            ss << "data block number invalid";
            break;
        case CSM_ACCESS_RESULT_OTHER_REASON:
            ss << "other reason";
            break;
        default:
            break;
    }
    return ss.str();
}

static AxdrPrinter gPrinter;

static void AxdrData(uint8_t type, uint32_t size, uint8_t *data)
{
    gPrinter.Append(type, size, data);
}


std::string CosemClient::EncapsulateRequest(Meter &meter, csm_array *request)
{
    std::string request_data;

    if (request->offset != 3U)
    {
        std::cout << "Cosem array must have room for LLC" << std::endl;
    }
    else
    {
        // remove offset
        request->offset = 0;
        request->wr_index += 3U; // adjust size written

        csm_array_set(request, 0U, 0xE6U);
        csm_array_set(request, 1U, 0xE6U);
        csm_array_set(request, 2U, 0x00U);

        // Encode HDLC
        meter.hdlc.sender = HDLC_CLIENT;
        int send_size = hdlc_encode_data(&meter.hdlc, (uint8_t *)&mSndBuffer[0], cBufferSize, request->buff, csm_array_written(request));

        request_data.assign((char *)&mSndBuffer[0], send_size);
    }

    return request_data;
}


void DateToCosem(std::tm &date, csm_array *array)
{
    clk_datetime_t clk;

    // 1. Transform standard date into cosem format

    clk.date.year = date.tm_year + 1900;
    clk.date.month = date.tm_mon + 1;
    clk.date.day = date.tm_mday;
    clk.date.dow = 0xFFU; // not specified
    clk.time.hour = date.tm_hour;
    clk.time.minute = date.tm_min;
    clk.time.second = date.tm_sec;
    clk.time.hundredths = 0U;

    clk.deviation = static_cast<std::int16_t>(0x8000);
    clk.status = 0xFFU;

    // 2. Serialize
    clk_datetime_to_cosem(&clk, array);
}


Result CosemClient::AccessObject(Meter &meter, const Object &obj, csm_request &request, csm_response &response, csm_array &app_array)
{
    Result result;

    result.subject = obj.name;

    bool allowSelectiveAccess = false;
    bool hasEndDate = false;

    std::tm tm_start = {};
    std::tm tm_end = {};

    // Allow selective access only on profile get buffer attribute
    if ((obj.attribute_id == 2) && (obj.class_id == 7U))
    {
        allowSelectiveAccess = true;
    }

    if (allowSelectiveAccess)
    {
        if (mConf.start_date.size() > 0)
        {
            // Try to decode start date
            std::stringstream ss(mConf.start_date);
            ss >> std::get_time(&tm_start, "%Y-%m-%d.%H:%M:%S");

            if (ss.fail())
            {
                std::cout << "** Parse start date failed\r\n";
                allowSelectiveAccess = false;
            }
        }
        else
        {
            // No start date, disable selective access
            allowSelectiveAccess = false;
        }

        if (mConf.end_date.size() > 0)
        {
            std::stringstream ss2(mConf.end_date);
            ss2 >> std::get_time(&tm_end, "%Y-%m-%d.%H:%M:%S");
            if (ss2.fail())
            {
                std::cout << "** Parse end date failed\r\n";
                allowSelectiveAccess = false;
            }
            else
            {
            	hasEndDate = true;
            }
        }
        else
        {
            // No end date
        	hasEndDate = false;
            std::cout << "** No end date defined" << std::endl;
        }
    }

    if (allowSelectiveAccess)
    {
        // Setup selective access options
        request.db_request.sel_access.enable = TRUE;
        csm_array_init(&request.db_request.sel_access.data, &mSelectiveAccessBuff[0], cSelectiveAccessBufferSize, 0, 0);
        uint8_t clockBuffStart[12];
        uint8_t clockBuffEnd[12];
        csm_array clockStartArray;
        csm_array clockEndArray;

        csm_array_init(&clockStartArray, &clockBuffStart[0], 12, 0, 0);
        csm_array_init(&clockEndArray, &clockBuffEnd[0], 12, 0, 0);


        DateToCosem(tm_start, &clockStartArray);

        if (hasEndDate)
        {
        	DateToCosem(tm_end, &clockEndArray);
        }
        else
        {
        	clk_datetime_t clk;

        	// Set all Cosem DateTime fields as undefined
        	clk_set_undefined(&clk);
        	clk_datetime_to_cosem(&clk, &clockEndArray);
        }

        csm_object_t clockObj;

        clockObj.class_id = 8;
        clockObj.data_index = 0;
        clockObj.id = 2;
        clockObj.obis.A = 0;
        clockObj.obis.B = 0;
        clockObj.obis.C = 1;
        clockObj.obis.D = 0;
        clockObj.obis.E = 0;
        clockObj.obis.F = 255;

        csm_client_encode_selective_access_by_range(&request.db_request.sel_access.data, &clockObj, &clockStartArray, &clockEndArray);
    }
    else
    {
        request.db_request.sel_access.enable = FALSE;
    }

    request.db_request.logical_name.class_id = obj.class_id;

    std::vector<std::string> obis = Util::Split(obj.ln, ".");

    if (obis.size() == 6)
    {
        request.db_request.logical_name.obis.A = strtol(obis[0].c_str(), NULL, 10);
        request.db_request.logical_name.obis.B = strtol(obis[1].c_str(), NULL, 10);
        request.db_request.logical_name.obis.C = strtol(obis[2].c_str(), NULL, 10);
        request.db_request.logical_name.obis.D = strtol(obis[3].c_str(), NULL, 10);
        request.db_request.logical_name.obis.E = strtol(obis[4].c_str(), NULL, 10);
        request.db_request.logical_name.obis.F = strtol(obis[5].c_str(), NULL, 10);
    }
    else
    {
        result.SetError("Bad Cosem OBIS code format");
    }
    request.db_request.logical_name.id = obj.attribute_id;

    csm_array scratch_array;
    csm_array_init(&scratch_array, &mScratch[0], cBufferSize, 0, 3);

    if (result.success && svc_request_encoder(&request, &scratch_array))
    {
        std::cout << "** Sending request for object: " << obj.name << std::endl;

        std::string request_data = EncapsulateRequest(meter, &scratch_array);
        std::string data;
        bool loop = true;
        bool dump = false;
        uint32_t retries = 0U;

        do
        {
            data.clear();

            if (HdlcProcess(meter, request_data, data, mConf.timeout_request, true))
            {
                Transport::Printer(data.c_str(), data.size(), PRINT_HEX);
                csm_array_init(&scratch_array, &mScratch[0], cBufferSize, 0, 0);

                csm_array_write_buff(&scratch_array, (const uint8_t *)data.c_str(), data.size());

                if (HasGoodLlc(&scratch_array))
                {
                    // Good Cosem server packet
                    if (csm_client_decode(&response, &scratch_array))
                    {
                        bool isResponseValid = false;

                        if (response.service == request.db_request.service)
                        {
                            if (response.service == SVC_ACTION)
                            {
                                if (response.action_result == CSM_ACTION_RESULT_SUCCESS)
                                {
                                    // If not set, then we must have some data
                                    isResponseValid = true;
                                }
                            }
                            else
                            {
                                if (response.access_result == CSM_ACCESS_RESULT_SUCCESS)
                                {
                                    isResponseValid = true;
                                }
                            }
                        }


                        if (isResponseValid)
                        {
                            if (response.type == SVC_RESPONSE_NORMAL)
                            {
                                // We have the data, copy it to the application buffer and stop
                                csm_array_write_buff(&app_array, csm_array_rd_data(&scratch_array), csm_array_unread(&scratch_array));
                                loop = false;
                                dump = true;
                            }
                            else if (response.type == SVC_RESPONSE_WITH_DATABLOCK)
                            {
                            	// Copy data into app data
								uint32_t size = 0U;
								if (csm_axdr_decode_block(&scratch_array, &size))
								{
									std::cout << "** Block of data of size: " << size << std::endl;
									// FIXME: Test the size indicated in the packet and the real size received
									// Add it
									csm_array_write_buff(&app_array, csm_array_rd_data(&scratch_array), csm_array_unread(&scratch_array));

									// Check if last block
									if (csm_client_has_more_data(&response))
									{
										// Send next block
										request.type = SVC_REQUEST_NEXT;
										request.db_request.block_number = response.block_number;
										request.sender_invoke_id = response.invoke_id;

										csm_array_init(&scratch_array, &mScratch[0], cBufferSize, 0, 3);

										svc_request_encoder(&request, &scratch_array);

										hdlc_print_result(&meter.hdlc, HDLC_OK);
										request_data = EncapsulateRequest(meter, &scratch_array);

										printf("** Sending ReadProfile next...\r\n");
									}
									else
									{
										std::cout << "** No more data" << std::endl;
										loop = false;
										dump = true;
									}
								}
								else
								{
									std::cout << "** ERROR: must be a block of data" << std::endl;
									loop = false;
								}
                            }
                            else
                            {
                                std::cout << "** Service not supported" << std::endl;
                                loop = false;
                            }
                        }
                        else
                        {
                            // BAD response from meter, filter why
                            if ((response.service == SVC_GET) ||
                                (response.service == SVC_ACTION))
                            {
                                std::stringstream ss;
                                ss << "** Data access result: " << ResultToString(response.access_result);
                                result.SetError(ss.str());
                            }
                            else if (response.service == SVC_EXCEPTION)
                            {
                                std::stringstream ss;
                                ss << "** Received exception from meter: ";

                                if (response.exception.state_err == 1)
                                {
                                    ss << "Service not allowed.";
                                }
                                else
                                {
                                    ss << "Service unknown.";
                                }

                                if (response.exception.service_err == 1)
                                {
                                    ss << "Operation not possible.";
                                }
                                else if (response.exception.service_err == 2)
                                {
                                    ss << "Service not supported.";
                                }
                                else
                                {
                                    ss << "Other reason.";
                                }

                                result.SetError(ss.str());
                            }
                            else
                            {
                                result.SetError("** Error, service not found! ");
                            }
                            loop = false;
                        }
                    }
                    else
                    {
                        result.SetError("** Cannot decode Cosem response");
                        loop = false;
                    }
                }
                else
                {
                    result.SetError("** Not a compliant HDLC LLC");
                    loop = false;
                }
            }
            else
            {
                retries++;
                if (retries > mConf.retries)
                {
                    result.SetError("** Cannot get HDLC data");
                    loop = false;
                }
            }
        }
        while(loop);

        if (dump && obj.dump)
        {
            std::string infos = "Object=\"" + obj.name + "\"";
            gPrinter.Start(infos);
            csm_axdr_decode_tags(&app_array, AxdrData);
            gPrinter.End();

            std::string xml_data = gPrinter.Get();
            std::cout << xml_data << std::endl;

            std::string dirName = meter.meterId;
            std::string fileName = dirName + Util::DIR_SEPARATOR + obj.name + ".xml";

            std::cout << "Dumping into file: " << fileName << std::endl;

            std::fstream f;

            Util::Mkdir(dirName);
            f.open(fileName, std::ios_base::out | std::ios_base::binary);

            if (f.is_open())
            {
                f << xml_data << std::endl;
                f.close();
            }
            else
            {
                std::cout << "Cannot open file!" << std::endl;
            }
           // print_hex((const char *)&mAppBuffer[0], csm_array_written(&app_array));
        }
    }

    return result;
}

bool  CosemClient::PerformCosemRead(Meter &meter)
{
    bool ret = false;
    uint32_t retries = 0U;

    do
    {

        switch(mCosemState)
        {
            case CONNECT_HDLC:
            {
                Result result;
                result.subject = "CONNECT HDLC";
                printf("** Sending HDLC SNRM (addr: %d)...\r\n", meter.hdlc.phy_address);
                if (ConnectHdlc(meter) > 0)
                {
                   printf("** HDLC success!\r\n");
                   ret = true;
                   mCosemState = ASSOCIATION_PENDING;
                }
                else
                {
                    retries++;
                    if (retries > mConf.retries)
                    {
                        retries = 0;
                        if (meter.testHdlcAddr)
                        {
                            // Keep this state, no error, scan next HDLC address
                            meter.hdlc.phy_address++;
                            ret = true;
                        }
                        else
                        {
                            result.SetError("** Cannot connect to meter.");
                            mResults.push_back(result);
                            ret = false;
                        }
                    }
                    else
                    {
                        ret = true;
                    }
                }
            }
             break;
            case ASSOCIATION_PENDING:
            {
                printf("** Sending AARQ...\r\n");
                Result result = ConnectAarq(meter);
                if (result.success)
                {
                   printf("** AARQ success!\r\n");
                   ret = true;
                   mReadIndex = 0U;
                   mCosemState = ASSOCIATED;
                }
                else
                {
                   mResults.push_back(result);
                   ret = false;
                }
                break;
            }
            case ASSOCIATED:
            {
                if (mReadIndex < mConf.list.size())
                {
                    Object obj = mConf.list[mReadIndex];

                    csm_request request;
                    csm_response response;
                    request.db_request.service = SVC_GET;
                    request.type = SVC_REQUEST_NORMAL;
                    request.sender_invoke_id = 0xC1U;

                    csm_array app_array;
                    csm_array_init(&app_array, &mAppBuffer[0], cAppBufferSize, 0, 0);

                    Result result = AccessObject(meter, obj, request, response, app_array);

                    mResults.push_back(result);

                    if (result.success)
                    {
                        std::cout << "Object: " << result.subject << " access success!" << std::endl;
                        mReadIndex++;
                    }
                    else
                    {
                        // stop at first failure
                        ret = false;
                    }
                }
                else
                {
                    std::cout << "** No more data to read for that meter." << std::endl;
                    ret = false;
                }
                break;
            }
            default:
                ret = false;
                break;

        }
    } while (ret);

    return ret;
}


std::string now( const char* format = "%c" )
{
    std::time_t t = std::time(0) ;
    char cstr[128] ;
    std::strftime( cstr, sizeof(cstr), format, std::localtime(&t) ) ;
    return cstr ;
}

void CosemClient::PrintResult()
{
    std::string dirName = "result";
    std::string dateTime = now("_%Y%m%d_%H%M%S.xml");
    std::string fileName = dirName + Util::DIR_SEPARATOR + "result" + dateTime;

    std::cout << "=============================   RESULT  ============================= " << std::endl;

    std::fstream f;

    Util::Mkdir(dirName);
    f.open(fileName, std::ios_base::out | std::ios_base::binary);

    if (f.is_open())
    {
        if (mResults.size() > 0U)
        {
            f << "<Result status=\"failure\">" << std::endl;
            std::cout << "One or more problem was found." << std::endl;
            for (uint32_t i; i < mResults.size(); i++)
            {
               if (!mResults[i].success)
               {
                   std::stringstream ss;
                   ss << "Task: " << mResults[i].subject << " access failure: " << mResults[i].diagnostic << std::endl;
                   std::cout << ss.str() << std::endl;
                   f << "    <Diagnostic>" << ss.str() << "</Diagnostic>" << std::endl;
               }
            }

            f << "</Result>" << std::endl;
        }
        else
        {
            f << "<Result status=\"success\" />" << std::endl;
        }

        std::cout << "Result file generated: " << fileName << std::endl;
        f.close();
    }
    else
    {
       std::cout << "Cannot create result file!" << std::endl;
    }

}

// Global state chart
bool CosemClient::PerformTask()
{
    bool ret = false;

    switch (mModemState)
    {
        case DISCONNECTED:
        {
            std::string modemReply;
            Result result;
            result.subject = "MODEM TEST";

            if (SendModem(mConf.modem.init + "\r\n", "OK", modemReply, 2U) > 0)
            {
                std::cout << "** Modem test success!" << std::endl;

                mModemState = DIAL;
                ret = true;
            }
            else
            {
                result.SetError("** Modem test failed.");
                mResults.push_back(result);
            }

            break;
        }

        case DIAL:
        {
            Result result;
            result.subject = "MODEM DIAL";

            std::cout << "** Dial: " << mConf.modem.phone << std::endl;
            std::string dialRequest = std::string("ATD") + mConf.modem.phone + std::string("\r\n");
            std::string modemReply;
            if (SendModem(dialRequest, "CONNECT", modemReply, mConf.timeout_dial))
            {
               std::cout << "** Modem dial success!" << std::endl;
               ret = true;
               mModemState = CONNECTED;
            }
            else
            {
                if (modemReply.size())
                {
                    std::stringstream ss;
                    ss << "** Dial failed, modem response: " << modemReply;
                    result.SetError(ss.str());
                }
                else
                {
                    result.SetError("** Dial failed: no response from modem.");
                }
                mResults.push_back(result);
            }

            break;
        }

        case CONNECTED:
        {
            if (mMeterIndex < mConf.meters.size())
            {
                Meter meter = mConf.meters[mMeterIndex];

                std::cout << "** Meter ID: " << meter.meterId << std::endl;
                std::cout << "** Using Client: " << meter.cosem.client << std::endl;

                if (meter.transport == HDLC)
                {
                    meter.hdlc.sender = HDLC_CLIENT;
                    meter.hdlc.logical_device = meter.cosem.logical_device;
                    meter.hdlc.client_addr = meter.cosem.client;
                    std::cout << "** Using HDLC address: " << meter.hdlc.phy_address << std::endl;
                }

                if (meter.meterId.size() > 0U)
                {
                    ret = PerformCosemRead(meter);
                }
                else
                {
                    std::cout << "** Please specify a valid meter ID" << std::endl;
                }
                mMeterIndex++;
            }
            else
            {
                std::cout << "** No more meter to read, exiting." << std::endl;
                ret = false;
            }

            break;
        }
        default:
            break;
    }
    return ret;
}

