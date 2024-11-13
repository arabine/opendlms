/**
 * AXDR printer to several output formats (currently XML)
 *
 * Copyright (c) 2016, Anthony Rabine
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the BSD license.
 * See LICENSE.txt for more details.
 *
 */

#ifndef AXDR_PRINTER
#define AXDR_PRINTER

#include <vector>
#include <sstream>
#include <cstdint>

struct Element
{
    uint32_t counter;
    uint32_t size;
    uint8_t type;
};



class AxdrPrinter
{

public:

    void Clear()
    {
        mStream.str("");
        mLevels.clear();
    }

    void Start(const std::string &infos = "");
    void End();
    void Append(uint8_t type, uint32_t size, uint8_t *data);
    std::string Get();

private:
    void PrintIndent();
    static std::string DataToString(uint8_t type, uint32_t size, uint8_t *data, std::string &hint);

    std::vector<Element> mLevels;
    std::stringstream mStream;

};


#endif // AXDR_PRINTER

