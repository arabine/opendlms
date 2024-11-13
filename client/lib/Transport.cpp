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

#include <chrono>
#include <stdio.h>
#include "Transport.h"
#include "serial.h"
#include "os_util.h"
#include "Util.h"

Transport::Transport()
    : mStarted(false)
    , mUseTcpGateway(false)
    , mSerialHandle(0)
    , mTerminate(false)
{

}

void Transport::Printer(const char *text, int size, PrintFormat format)
{
    std::cout << Util::CurrentDateTime("%Y-%m-%d.%X") << ": ";

    if (format != NO_PRINT)
    {
        if (format == PRINT_RAW)
        {
            fwrite(text, size, 1, stdout);
        }
        else
        {
            print_hex(text, size);
        }
    }
}



bool Transport::Open(const Transport::Params &params)
{
    bool ret = false;

    mConf = params;

    std::cout << "** Opening serial port " << mConf.port << " at " << mConf.baudrate << std::endl;
    mSerialHandle = serial_open(mConf.port.c_str());

    if (mSerialHandle >= 0)
    {
        if (serial_setup(mSerialHandle, mConf.baudrate) == 0)
        {
            printf("** Serial port success!\r\n");
            ret = true;
        }
    }

    return ret;
}

int Transport::Send(const std::string &data, PrintFormat format)
{
    int ret = -1;

    // Print request
    puts("====> Sending: ");
    Printer(data.c_str(), data.size(), format);
    puts("\r\n");

    if (mUseTcpGateway)
    {
        // TODO
      //  socket.write(data);
      //  socket.flush();
    }
    else
    {
        ret = serial_write(mSerialHandle, data.c_str(), data.size());
    }

    return ret;
}

bool Transport::WaitForData(std::string &data, int timeout)
{
    bool notified = mSem.Wait(timeout*1000);

    if (!notified)
    {
        puts("** Serial read timeout!\r\n");
    }

    mMutex.lock();
    data += mData;
    mData.clear();
    mMutex.unlock();

    return notified;
}


void Transport::WaitForStop()
{
    mTerminate = true;

    if (mStarted)
    {
        mThread.join();
    }
}



void Transport::Reader()
{
    mStarted = true;

    while (!mTerminate)
    {
        int ret = serial_read(mSerialHandle, &mRcvBuffer[0], cBufferSize, 10);

        if (ret > 0)
        {
            printf("<==== Got data: %d bytes: ", ret);
            std::string data(&mRcvBuffer[0], ret);

            Printer(data.c_str(), data.size(), PRINT_HEX);

            puts("\r\n");

            // Add data
            mMutex.lock();
            mData += data;
            mMutex.unlock();

            // Signal new data available
            mSem.Notify();
        }
        else if (ret == 0)
        {
            puts("Still waiting for data...\r\n");
        }
        else
        {
            puts("Serial read error, exiting...\r\n");
            mTerminate = true;
            exit(1);
        }
    }
}

void Transport::Start()
{
    mThread = std::thread(Transport::EntryPoint, this);
}

void Transport::EntryPoint(void *pthis)
{
    Transport *pt = static_cast<Transport *>(pthis);
    pt->Reader();
}

