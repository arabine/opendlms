/**
 * Communication transport layer
 *
 * Copyright (c) 2016, Anthony Rabine
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the BSD license.
 * See LICENSE.txt for more details.
 *
 */

#ifndef TRANSPORT_H_
#define TRANSPORT_H_

#include <string>
#include <cstdint>
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include "Semaphore.h"

enum PrintFormat
{
    NO_PRINT,
    PRINT_RAW,
    PRINT_HEX
};


class Transport
{
public:

    enum Type
    {
        SERIAL,
        TCP_IP
    };

    struct Params
    {
        Params()
            : type(SERIAL)
            , baudrate(9600)
        {

        }
        Type type;
        std::string address;
        std::string port;
        unsigned int baudrate;
    };

    Transport();

    void Start();
    void WaitForStop();

    bool Open(const Params &params);
    int Send(const std::string &data, PrintFormat format);
    bool WaitForData(std::string &data, int timeout);

    static void Printer(const char *text, int size, PrintFormat format);

private:

    static const uint32_t cBufferSize = 40U*1024U;
    char mRcvBuffer[cBufferSize];

    bool mStarted;
    Params mConf;
    bool mUseTcpGateway;
    int mSerialHandle;
    bool mTerminate;
    std::string mData;

    std::thread mThread;
    Semaphore mSem;
    std::mutex mMutex;

    static void EntryPoint(void *pthis);
    void Reader();
};



#endif /* COSEMCLIENT_LIB_TRANSPORT_H_ */
