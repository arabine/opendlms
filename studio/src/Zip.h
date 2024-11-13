/*=============================================================================
 * TarotClub - Zip.h
 *=============================================================================
 * Zip file reader and extractor
 *=============================================================================
 * TarotClub ( http://www.tarotclub.fr ) - This file is part of TarotClub
 * Copyright (C) 2003-2999 - Anthony Rabine
 * anthony@tarotclub.fr
 *
 * TarotClub is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TarotClub is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TarotClub.  If not, see <http://www.gnu.org/licenses/>.
 *
 *=============================================================================
 */

#ifndef ZIP_H
#define ZIP_H

#include <cstdint>
#include <string>
#include <vector>

#ifndef ZIP_CPP_INCLUDED
#define MINIZ_HEADER_FILE_ONLY
#include "miniz.c"
#endif

/*****************************************************************************/
class Zip
{
public:
    Zip();
    ~Zip();

    bool Open(const std::string &fileName);
    std::uint32_t NumberOfFiles() { return mNumberOfFiles; }
    bool GetFile(const std::string &fileName, std::string &contents);
    void Close();

private:
    std::int32_t cZipMaxSize = 2 * 1024 * 1024; // 2MB max

    mz_zip_archive mZipArchive;
    bool mIsValid;
    std::uint32_t mNumberOfFiles;
};

#endif // ZIP_H

//=============================================================================
// End of file Zip.cpp
//=============================================================================
