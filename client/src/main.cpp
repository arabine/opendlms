/**
 * Cosem client entry point
 *
 * Copyright (c) 2016, Anthony Rabine
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the BSD license.
 * See LICENSE.txt for more details.
 *
 */

#include <iostream>
#include "JsonReader.h"
#include "CosemClient.h"
#include "Configuration.h"
#include "csm_array.h"
#include "Util.h"

CosemClient client;

extern "C" void csm_hal_get_lls_password(uint8_t sap, uint8_t *array, uint8_t max_size)
{
    (void)sap;
    std::string lls = client.GetLls();

    uint32_t size = (lls.size() > max_size) ? max_size : lls.size();

    lls.copy((char*)array, size);
}

int main(int argc, char **argv)
{
    setbuf(stdout, NULL); // disable printf buffering

   std::cout << "DLMS/Cosem client tool version " <<  COSEM_CLIENT_VER <<  " build date: " << __DATE__ << " " <<  __TIME__ << std::endl;

    if (argc >= 3)
    {
        std::string meterFile(argv[1]); // First file is the communication parameters
        std::string objectsFile(argv[2]); // Second is the objects to retrieve
        std::string commFile(argv[3]); // Second is the objects to retrieve

        if (argc >= 5)
        {
            client.SetStartDate(std::string(argv[4])); // startDate for the profiles
        }

        if (argc >= 6)
        {
            client.SetEndDate(std::string(argv[5])); // endDate for the profiles
        }

        // Before application, test connectivity
        if (client.Initialize(commFile, objectsFile, meterFile))
        {
            while (client.PerformTask());
        }

        client.PrintResult();
    }
    else
    {
        printf("\r\nUsage: cosem_client /path/session.json /another/objectlist.json /path/comm.json [start date] [end date]\r\n");
        printf("\r\nExample: cosem_client session.json objectlist.json comm.json 2017-08-01.00:00:00 2017-10-23.14:55:02\r\n");
        puts("\r\nTwo last parameters are start and end dates for selective access of data. Start date only is supported (means until now), no any date means getting all the data.");
        puts("\r\nDate-time format: %Y-%m-%d.%H:%M:%S");
    }

    printf("** Exit task loop, waiting for reading thread...\r\n");
    client.WaitForStop();

    return 0;

}

