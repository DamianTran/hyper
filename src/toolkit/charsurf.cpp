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

#include "EZC/toolkit/charsurf.hpp"
#include "EZC/toolkit/string.hpp"
#include "EZC/toolkit/string_search.hpp"

using namespace std;

namespace EZC
{

bool ptr_at_string(const char* ptr, const char* query,
                   bool case_insensitive)
{
    if(!(*ptr) || !(*query)) return false;
    while((*ptr) && (*query))
    {
        if(*ptr != *query)
        {
            if(!case_insensitive || !case_cmp(*ptr, *query))
            {
                return false;
            }
        }

        ++ptr;
        ++query;
    }
    return !(*query);
}

string read_to(const char** ptr, const char* query)
{

    string output;

    if(!(*ptr) || !query)
    {
        return output;
    }

    const char* begin = *ptr;

    while(**ptr)
    {
        if(ptr_at_string(*ptr, query))
        {
            output.assign(begin, *ptr);
            return output;
        }
        ++(*ptr);
    }

    return output;

}

bool replace_all(string& str,
                 const string& sequence,
                 const string& replacement)
{

    bool status = false;
    for(size_t i = 0; i < str.size(); ++i)
    {
        if(ptr_at_string(&str[i], sequence.c_str()))
        {
            str.replace(str.begin() + i, str.begin() + i + sequence.size(),
                        replacement);
            status = true;
        }
    }

    return status;
}

void remove_all(string& str,
                const string& sequence)
{
    for(size_t i = 0; i < str.size(); ++i)
    {
        if(ptr_at_string(&str[i], sequence.c_str()))
        {
            str.erase(str.begin() + i, str.begin() + i + sequence.size());
        }
    }

}

unsigned int num_matches(const char* query,
                         const char* background,
                         bool case_insensitive)
{

    unsigned int output = 0;

    if(!(*background) || !(*query))
    {
        return output;
    }

    unsigned int matchCheck = UINT_MAX;
    unsigned int L = strlen(query);

    vector<size_t> char_table;
    vector<size_t> match_table;

    boyer_moore_char_table(query, char_table, true);
    boyer_moore_match_table(query, match_table, true);

    while((matchCheck = boyer_moore_search(query, background, char_table, match_table, true)) != UINT_MAX)
    {
        background += matchCheck + L; // Roll forward to check for redundant matches
        ++output;
    }

    return output;

}

bool shiftTextIndex(unsigned int& index, const string& text,
                           const unsigned int& distance, const bool forward)
{

    size_t i = 0, L = text.size();
    if(forward)
    {
        if(index < L-1)
        {
            for(; (i < distance) && (index < L); ++i)
            {
                while(!isCharType(text[index], " ,;._+\\/?:\"") && (index < L))  // Shift to end of word if not already so
                {
                    ++index;
                }
                while(isCharType(text[index], " ,;._+\\/?:\"") && (index < L))  // Shift past punctuation
                {
                    ++index;
                }
            }
        }
    }
    else
    {
        if(index > 0)
        {
            for(; (i < distance) && (index > 0); ++i)
            {
                while(!isCharType(text[index], " ,;._+\\/?:\"") && (index > 0))  // Shift to beginning of word if not already so
                {
                    --index;
                }
                while(isCharType(text[index], " ,;._+\\/?:\"") && (index > 0))  // Shift past punctuation
                {
                    --index;
                }
                while(!isCharType(text[index], " ,;._+\\/?:\"") && (index > 0))  // Shift to beginning of next index word
                {
                    --index;
                }
            }
        }
    }

    return i == distance;
}

bool read_word(const char** ptr, string& output)
{
    if(!(**ptr)) return false;

    const char* begin = *ptr;

    while(**ptr && !isTextDelim(**ptr))
    {
        ++(*ptr);
    }

    if(*ptr != begin)
    {

        output.assign(begin, *ptr);

        while(**ptr && isTextDelim(**ptr))
        {
            ++(*ptr);
        }

        return true;

    }

    return false;
}

string get_word(const char* ptr)
{
    string output;
    const char* begin = ptr;

    while(*ptr && !isTextDelim(*ptr))
    {
        ++ptr;
    }

    output.assign(begin, ptr);

    return output;
}

string get_between(const char& begin,
                   const char& end,
                   const char* str)
{

    if(!(*str)) return string();

    string output;
    int level = 0;
    const char* begin_c = str;
    bool bOpen = false;

    while(*str)
    {

        if(!bOpen && (*str == begin) && *(str + 1))
        {
            if(!level)
            {
                begin_c = str + 1;
                if(begin == end) bOpen = true;
            }
            ++level;
            ++str;
        }
        else if(*str == end)
        {
            --level;
            if(!level)
            {
                if(str != begin_c)
                {
                    output.assign(begin_c, distance(begin_c, str));
                }

                return output;
            }
        }

        ++str;
    }

    return output;

}

}
