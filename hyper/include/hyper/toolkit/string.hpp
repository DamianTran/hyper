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

#ifndef EZ_STRING
#define EZ_STRING

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <sstream>
#include <cstring>
#include <codecvt>
#include <locale>

#define CMP_STR_MATCH               0b0
#define CMP_STR_CASE_INSENSITIVE    0b1
#define CMP_STR_SIZE_INSENSITIVE    0b10
#define CMP_STR_DISCRETE            0b100
#define CMP_STR_SMALL_DISCRETE      0b1000
#define CMP_STR_SW                  0b10000

#define CMP_STR_DEFAULT 0b1011

#define SW_PTR_DIAG                     0
#define SW_PTR_LEFT                     1
#define SW_PTR_UP                       2
#define SW_PTR_NONE                     3

/////////////////////////////////////////////////////////////////////////////

/* Global operator overrides */

/////////////////////////////////////////////////////////////////////////////

namespace hyper
{

template<typename data_t>
std::ostream& operator<<(std::ostream& output, const std::vector<data_t>& V)
{
    size_t L = V.size();
    output << '[';
    for(size_t i = 0; i < L; ++i)
    {
        output << V[i];
        if((L > 1) & (i < L - 1)) output << ',';
    }
    output << ']';
    return output;
}

template<typename T1, typename T2> std::ostream& operator<<(std::ostream& output, const std::map<T1, T2>& M)
{

    if(!M.empty())
    {
        auto endIT = M.end();
        for(auto IT = M.begin(); IT != endIT; ++IT)
        {
            output << IT->first << ": " << IT->second << '\n';
        }
    }

    return output;
}

const static std::string DELIM_NONE                    =  "";
const static std::string DELIM_BASIC                   =  " ,:;\t\n\r";
const static std::string DELIM_STANDARD                =  " ,.?\\\"/\t\n\r!:;&=#";
const static std::string DELIM_STANDARD_MISC           =  ",.?\\\"/\t\n\r!:;&=#";
const static std::string DELIM_CODE                    =  "\"\t\n\r.;:&$+=#<>{}[]";
const static std::string DELIM_ALL                     =  " ,.?\"'/+-=\t\n\r*~!_:;^\\&#()[]{}<>";

const static int SW_MATCH_SCORE = 5,
                 SW_MISMATCH_PENALTY = 4,
                 SW_GAP_OPEN_PENALTY = 12,
                 SW_GAP_EXTEND_PENALTY = 4;

typedef std::vector<std::string> StringVector;
typedef std::vector<std::vector<std::string>> StringMatrix;

/////////////////////////////////////////////////////////////////////////////

/* Character checks */

/////////////////////////////////////////////////////////////////////////////

constexpr bool isLetter(const char& c)
{
    return (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')));
}

constexpr bool isUpperCase(const char& c)
{
    return ((c >= 'A') && (c <= 'Z'));
}

constexpr bool isLowerCase(const char& c)
{
    return ((c >= 'a') && (c <= 'z'));
}

inline bool case_cmp(const char& c1, const char& c2)
{
    return (isUpperCase(c1) && (c1 == (c2 - 32))) ||
            (isLowerCase(c1) && (c1 == (c2 + 32)));
}

constexpr bool isNumber(const char& c)
{
    return ((c >= '0') && (c <= '9'));
}

constexpr bool isDelimiter(const char& c)
{
    return ((c == ' ') || ( c == '\t') || (c == '\n') ||
            (c == ',') || (c == '-'));
}

inline bool isTextDelim(const char& c)
{
    return(((c < 48) && (c != 39)) ||
           ((c > 57) && (c < 65)) ||
           ((c > 90) && (c < 97)) ||
           (c > 122));
}

inline bool isSentenceDelim(const char& c)
{
    return ((c == '.') || (c == '?') || (c == '!'));
}

inline bool at_word_begin(const char* c)
{
    return(isTextDelim(*(c - 1)) && !(isTextDelim(*c)));
}

inline bool at_word_end(const char* c)
{
    return(!isTextDelim(*c) && isTextDelim(*(c + 1)));
}

constexpr bool isSpecial(const char& c)
{
    return(((c < 'A') || (c > 'Z')) &&
           ((c < 'a') || (c > 'z')) &&
           ((c < '0') || (c > '9')) &&
           (c != '-') && (c != '+') &&
           (c != '/'));
}

constexpr bool isTypeChar(const char& c)
{
    return (((c > 31) && (c < 127)) || (c == '\t') || (c == '\n'));
}

constexpr bool isNumeric(const char& c)
{
    return ((c > 39) && (c < 58));
}

bool isNumeric(char* c);

unsigned int getStrSize(char* c);

inline bool isCharType(const char& c, const char* type)
{
    for(size_t i = 0, L = strlen(type); i < L; ++i)
    {
        if(type[i] == c) return true;
    }
    return false;
}
template<typename basic_string_t>
bool isCharType(const char& c, const basic_string_t& type)
{
    return isCharType(c, type.c_str());
}

bool atWord(const unsigned int& index,
            const std::string& body,
            const std::string& query);
bool check_letter(const char& c, const char& other_c);

unsigned int charMatchNum(const std::string& query, const std::string& other);

unsigned int ncol(const std::string& str, const std::string& delim = "\t");
unsigned int nrow(const std::string& str);

unsigned int nline(const std::string& str);
unsigned int line_begin(const std::string& str, const unsigned int& index);
unsigned int line_end(const std::string& str, const unsigned int& index);

/////////////////////////////////////////////////////////////////////////////

/* Conversions */

/////////////////////////////////////////////////////////////////////////////

std::string unicode(const int& code); // Convert unicode number to character sequence

inline std::string UTF16_to_UTF8(const std::wstring& str)
{
    try
    {
        return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(str);
    }catch(...)
    {
        return std::string();
    }
}
inline std::wstring UTF8_to_UTF16(const std::string& str)
{
    try
    {
        return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(str);
    }catch(...)
    {
        return std::wstring();
    }
}

/////////////////////////////////////////////////////////////////////////////

/* Numeric string manipulation */

/////////////////////////////////////////////////////////////////////////////

float getFirstDigit(const float& number);
float placeDecimal(const float& number, int position = 0);

/////////////////////////////////////////////////////////////////////////////

/* Data visualization */

/////////////////////////////////////////////////////////////////////////////

template<typename T>
std::string bytes(const T& val)
{
    std::stringstream output;
    unsigned char* ptr = (unsigned char*)&val;
    int j;

    for(size_t i = 0; i < sizeof(T); ++i, ++ptr)
    {
        for(j = 7; j >= 0; --j)
        {
            output << bool(*ptr & (0b1 << j));
        }

        if(i < sizeof(T) - 1)
        {
            output << ' ';
        }
    }

    return output.str();

}

/////////////////////////////////////////////////////////////////////////////

/* Numeric operations */

/////////////////////////////////////////////////////////////////////////////

bool isNumeric(const std::string& s);
std::string sigFigs(const std::string& numeric);
std::string sigFigs(const float& value);
int hexToInt(const char* c);

template<typename T> std::string strDigits(const T& numeric, const unsigned int& numDecimals)  // Only works with classes with to_string support in std
{
    std::string numStr = std::to_string(numeric);
    if(!isNumeric(numStr)) return numStr;

    unsigned int index = numStr.find('.');
    if(index > numStr.size())
    {
        if(numDecimals < 1) return numStr;
        index = numStr.size();
        numStr += '.';
    }
    else
    {
        if(numDecimals == 0)
        {
            numStr.resize(index);
            return numStr;
        }
        numStr.resize(index + numDecimals + 1);
    }

    size_t L = index + numDecimals + 1;
    while(numStr.size() < L)
    {
        numStr += '0';
    }

    return numStr;

}

inline void operator+=(std::string& text, const float& numeric)
{
    text += sigFigs(std::to_string(numeric));
}

inline void operator+=(std::string& text, const unsigned int& numeric)
{
    text += sigFigs(std::to_string(numeric));
}

inline void operator+=(std::string& text, const int& numeric)
{
    text += sigFigs(std::to_string(numeric));
}

inline std::string operator+(const std::string& text, const float& numeric)
{
    return text + sigFigs(std::to_string(numeric));
}

inline std::string operator+(const std::string& text, const unsigned int& numeric)
{
    return text + sigFigs(std::to_string(numeric));
}

inline std::string operator+(const std::string& text, const int& numeric)
{
    return text + sigFigs(std::to_string(numeric));
}

template<typename T> std::string operator+(const std::string& text, const std::vector<T>& V)
{
    std::string output = text;
    output += '(';
    for(auto item : V)
    {
        output += item;
        output += ',';
    }
    output.back() = ')';
    return output;
}

/////////////////////////////////////////////////////////////////////////////

/* Manipulation */

/////////////////////////////////////////////////////////////////////////////

void to_lowercase(std::string& s);
std::string get_lowercase(const std::string& s);
void to_uppercase(std::string& s);
std::string get_uppercase(const std::string& s);
void capitalize(std::string& s);
void uncapitalize(std::string& s);
std::string capitalized(std::string s);
std::string non_capitalized(std::string s);
bool isWord(const std::string& s);

template<typename string_t>
void swap_chars(string_t& str,
                const std::string& targets,
                const char& substitute)
{
    for(auto& c : str)
    {
        if(isCharType(c, targets))
        {
            c = substitute;
        }
    }
}

template<typename string_t>
void single_spaces(string_t& str)
{
    for(size_t i = 0, L = str.size(); i < L; ++i)
    {
        if(str[i] == ' ')
        {
            while((i + 1 < L) && (str[i + 1] == ' '))
            {
                str.erase(str.begin() + i + 1);
                --L;
            }
        }
    }
}

template<typename string_t>
void swap_chars(std::vector<string_t>& strs,
                const std::string& targets,
                const char& substitute)
{
    for(auto& str : strs)
    {
        swap_chars(str, targets, substitute);
    }
}

std::string rep(const std::string& str, const unsigned int& N);

void trimSpaces(std::string& tag);
void trim(std::string& str, const std::string& delim = DELIM_BASIC);
void trim_all(std::string& str);

void concatenate_lateral(std::string& left, const std::string& right,
                                const unsigned int& spacing = 1,
                                const std::string& delim = "\t");
std::string concatenateColumn(const StringVector& info);

void concatenateString(const std::vector<std::string>& input,
                       std::string& output,
                            const char delim);

template<typename T> std::string concatenateString(const std::vector<T>& input,
                                                   const std::string& delim,
                                                   const std::string& terminator = "")
{

    std::stringstream oss;
    size_t L = input.size();
    unsigned int i = 0;
    for(auto item : input)
    {
        oss << item;
        if(terminator.size() > 0)
        {
            if(L > 1)
            {
                if(i == L-2) oss << (terminator);
                else if(i < L-1) oss << delim;
            }
        }
        else
        {
            if(i < L-1) oss << delim;
        }
        ++i;
    }
    return oss.str();
}



void processString(std::string& s);
void splitString(const char* input,
                 std::vector<std::string>& output,
                 const std::string& delim = DELIM_BASIC);
inline void splitString(const std::string& input,
                        std::vector<std::string>& output,
                        const std::string& delim = DELIM_BASIC)
{
    return splitString(input.c_str(), output, delim);
}
bool splitString(const std::string input,
                 std::vector<std::string>& output,
                 const std::vector<std::string>& delim,
                 const std::string& splitTrim = DELIM_BASIC,
                 const unsigned char& params = CMP_STR_DEFAULT);

void splitByString(const char* input,
                   std::vector<std::string>& output,
                   const std::string& term,
                   const unsigned char& params = CMP_STR_DEFAULT,
                   const float& threshold = 1.0f);
inline void splitByString(const std::string& input,
                          std::vector<std::string>& output,
                          const std::string& term,
                          const unsigned char& params = CMP_STR_DEFAULT,
                          const float& threshold = 1.0f)
{
    return splitByString(input.c_str(), output, term, params, threshold);
}

void splitByStrings(const char* input,
                    std::vector<std::string>& output,
                    const std::vector<std::string>& terms,
                    const unsigned char& params = CMP_STR_DEFAULT,
                    const float& threshold = 1.0f);
inline void splitByStrings(const std::string& input,
                           std::vector<std::string>& output,
                           const std::vector<std::string>& terms,
                           const unsigned char& params = CMP_STR_DEFAULT,
                           const float& threshold = 1.0f)
{
    splitByStrings(input.c_str(), output, terms, params, threshold);
}

/////////////////////////////////////////////////////////////////////////////

/* Matching and alignment */

/////////////////////////////////////////////////////////////////////////////

bool sw_align(const char* query, const char* background,
                     const float& threshold = 0.6f,
                     unsigned int* output = NULL,
                     bool case_insensitive = true,
                     unsigned int* out_pos = NULL);
bool sw_align(const std::string& query, const std::string& background,
                     const float& threshold = 0.6f,
                     unsigned int* output = NULL,
                     bool case_insensitive = true,
                     unsigned int* out_pos = NULL);
bool cmpString(const char* focus, const char* other,
                      const unsigned char& params = CMP_STR_DEFAULT,
                      const float& threshold = 1.0f,
                      float* output = NULL);
bool cmpString(const std::string& focus, const std::string& other,
                      const unsigned char& params = CMP_STR_DEFAULT,
                      const float& threshold = 1.0f,
                      float* output = NULL);
size_t findString(const char* focus, const char* other,
                      const unsigned char& params = CMP_STR_DEFAULT,
                      const float& threshold = 1.0f,
                      float* output = NULL);
inline size_t findString(const std::string& focus, const std::string& other,
                         const unsigned char& params = CMP_STR_DEFAULT,
                         const float& threshold = 1.0f,
                         float* output = NULL)
{
    return findString(focus.c_str(), other.c_str(),
                      params, threshold, output);
}

size_t findStrings(const std::vector<std::string>& strings,
                   const char* background,
                   const unsigned char& params = CMP_STR_DEFAULT,
                   const float& threshold = 1.0f,
                   float* output = NULL);
inline size_t findStrings(const std::vector<std::string>& strings,
                   const std::string& background,
                   const unsigned char& params = CMP_STR_DEFAULT,
                   const float& threshold = 1.0f,
                   float* output = NULL)
{
    return findStrings(strings, background.c_str(),
                       params, threshold, output);
}

template<typename string_container_t>
bool cmpStringToList(const std::string& focus,
                     const string_container_t& list,
                    const unsigned char& params = CMP_STR_DEFAULT,
                    const float& threshold = 0.6f,
                        float* output = NULL)
{
    float cmpOutput;
    if(output) *output = 0.0f;

    for (auto& str : list)
    {
        if(!output)
        {
            if(cmpString(focus, str, params, threshold)) return true;
        }
        else
        {

            cmpString(focus, str, params, threshold, &cmpOutput);
            if(cmpOutput > *output) *output = cmpOutput;

        }
    }

    if(output) return *output;
    return false;
}

inline bool cmpStringToList(const std::string& focus,
                     std::initializer_list<std::string> list,
                     const unsigned char& params = CMP_STR_DEFAULT,
                     const float& threshold = 0.6f,
                     float* output = NULL)
{
    return cmpStringToList(focus, std::vector<std::string>(list),
                           params, threshold, output);
}

template<typename string_t1,
            typename string_t2>
bool cmpStringToList(const std::vector<string_t1>& query,
                     const std::vector<string_t2>& other,
                    const unsigned char& params = CMP_STR_DEFAULT,
                    const float& threshold = 0.6f)
{
    for(auto& str: query)
    {
        if(cmpStringToList(str, other, params)) return true;
    }
    return false;
}

bool cmpStringIncludeList(const std::string& focus, const std::vector<std::string>& list,
                             const unsigned char& params = CMP_STR_DEFAULT,
                             const float& threshold = 0.6f);
bool replace(std::string& str,
                const std::string& sequence,
                const std::string& replacement);

std::string getBestStringMatch(const std::string& tag,
                                  const std::vector<std::string>& list,
                                  const float& threshold = 0.6f);

unsigned int getBestStringMatchIndex(const std::string& tag,
        const std::vector<std::string>& list,
        const unsigned char& params = CMP_STR_DEFAULT);

std::string getMatchingTag(const std::string& tag,
                           const std::vector<std::string>& list,
                           const unsigned char& params = CMP_STR_DEFAULT);
unsigned int getMatchingIndex(const std::string& focus,
                              const std::vector<std::string>& list,
                              const unsigned char& params = CMP_STR_DEFAULT,
                              const float& threshold = 0.6f);

bool checkString(const char* query, const char* background);
bool removeStrings(StringVector& prompt, const StringVector& strings);

size_t num_matches(const std::string& query,
                   const std::vector<std::string>& background,
                   const unsigned char& params = CMP_STR_DEFAULT,
                   const float& threshold = 1.0f);

template<typename T> T* matching_item(const std::string& query,
                                      std::map<std::string, T>& M,
                                      const unsigned char& params = CMP_STR_DEFAULT)
{
    if(M.empty()) return nullptr;

    std::vector<std::string> tags;
    tags.reserve(M.size());

    auto endIT = M.end();
    for(auto IT = M.begin(); IT != endIT; ++IT)
    {
        tags.emplace_back(IT->first);
    }

    size_t matchIndex = getMatchingIndex(query, tags, params);
    if(matchIndex != UINT_MAX)
    {
        return &M[tags[matchIndex]];
    }

    return nullptr;
}

}

#endif // EZ_STRING
