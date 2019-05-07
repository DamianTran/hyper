/** ////////////////////////////////////////////////////////////////

    *** EZ-C++ - A simplified C++ experience ***

        Yet (another) open source library for C++

        Original Copyright (C) Damian Tran 2019

        By aiFive Technologies, Inc. for developers

    Copying and redistribution of this code is freely permissible.
    Inclusion of the above notice is preferred but not required.

    This software is provided AS IS without any expressed or implied
    warranties.  By using this code, and any modifications and
    variants arising thereof, you are assuming all liabilities and
    risks that may be thus associated.

////////////////////////////////////////////////////////////////  **/

#pragma once

#ifndef EZ_ZIP
#define EZ_ZIP

#include "stdio.h"
#include "unistd.h"
#include "cstdlib"

#include <string>
#include <iostream>

namespace hyper
{

std::ostream& print_archive(std::ostream& output, const std::string& filename);
bool extract(const std::string& filename, const std::string& outPath = "");
bool archive(const std::string& directory, const std::string& outName = "");

// Decompress to memory
size_t decompress(const std::string& filename, void** output);
bool decompress(const std::string& filename,
                const std::string& outFileName);

}

#endif // EZ_ZIP
