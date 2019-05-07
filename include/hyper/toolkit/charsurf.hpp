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

#ifndef EZ_CHARSURF
#define EZ_CHARSURF

#include <string>
#include <vector>

namespace hyperC
{

bool ptr_at_string(const char* ptr, const char* query,
                   bool case_insensitive = false);
std::string read_to(const char** ptr, const char* query);
bool replace_all(std::string& str,
                        const std::string& sequence,
                        const std::string& replacement);
void remove_all(std::string& str,
                const std::string& sequence);

unsigned int num_matches(const char* query, const char* background,
                         bool case_insensitive = false);
inline unsigned int num_matches(const std::string& query, const std::string& background,
                            bool case_insensitive = false)
{
    return num_matches(query.c_str(), background.c_str(), case_insensitive);
}

bool read_word(const char** ptr, std::string& output);
std::string get_word(const char* ptr);

std::string get_between(const char& begin,
                        const char& end,
                        const char* str);
inline std::string get_between(const char& begin,
                               const char& end,
                               const std::string& str)
{
    return get_between(begin, end, str.c_str());
}

bool shiftTextIndex(unsigned int& index, const std::string& text,
                           const unsigned int& distance = 1, const bool forward = true);

}

#endif // EZ_CHARSURF
