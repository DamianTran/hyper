/** ////////////////////////////////////////////////////////////////

    *** Hyper C++ - A simplified C++ experience ***

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

#include "hyper/toolkit/string.hpp"
#include "hyper/algorithm.hpp"

#include <codecvt>
#include <locale>

using namespace std;

namespace hyperC
{

void trimSpaces(string& tag)
{
    while((tag.size() > 0) && (tag.front() == ' ')) tag.erase(tag.begin());
    while((tag.size() > 0) && (tag.back() == ' ')) tag.pop_back();
}

void trim(string& str, const string& delim)
{
    while(!str.empty() && isCharType(str.back(), delim)) str.pop_back();
    while(!str.empty() && isCharType(str.front(), delim)) str.erase(str.begin());
}

void trim_all(string& str)
{
    while(!str.empty() && !isLetter(str.back()) && !isNumber(str.back())) str.pop_back();
    while(!str.empty() && !isLetter(str.front()) && !isNumber(str.front())) str.erase(str.begin());
}

unsigned int nline(const string& str)
{
    unsigned int output = 1;
    for(size_t i = 0; i < str.size(); ++i)
    {
        if((str[i] == '\n') && (i < str.size() - 1)) ++output;
    }
    return output;
}

unsigned int line_begin(const string& str, const unsigned int& index)
{

    if(index >= nline(str)) return UINT_MAX;

    if(index == 0) return 0;

    size_t i = 0, cLine = 0;

    while((i < str.size()) && (cLine < index))
    {
        if(str[i] == '\n')
        {
            ++cLine;
            if(cLine == index)
            {
                if(i < str.size() - 1) return i + 1;
                else return UINT_MAX;
            }
        }
        ++i;
    }

    return UINT_MAX;
}

unsigned int line_end(const string& str, const unsigned int& index)
{

    if(index >= nline(str)) return UINT_MAX;

    size_t i = 0, cLine = 0;

    while((i < str.size()) && (cLine <= index))
    {
        if(str[i] == '\n')
        {
            if(cLine == index)
            {
                return i;
            }
            ++cLine;
        }
        ++i;
    }

    return UINT_MAX;
}

// Unicode

string unicode(const int& code)
{

    wstringstream output;
    output << wchar_t(code);
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(output.str());

}

// Number manipulation

float getFirstDigit(const float& number)
{
    return floor(number/pow(10.0f, floor(log10(number))));
}

float placeDecimal(const float& number, int position)
{
    return number/pow(10.0f, floor(log10(number)) - position);
}

void processString(string& s)
{
    if(s.size() < 1) return;
    while((s.front() == ' ') || (s.front() == '"')) s.erase(s.begin());
    if(s.size() < 1) return;
    while((s.back() == ' ') || (s.back() == '"')) s.pop_back();
}

void splitString(const char* input, vector<string>& output,
                        const string& delim)
{
    if(!(*input))
    {
        return;
    }

    const char* lastPos;

    while(*input)
    {
        while((*input) && isCharType(*input, delim))
        {
            ++input;
        }

        if(*input)
        {
            lastPos = input;

            while((*input) && !isCharType(*input, delim))
            {
                ++input;
            }

            output.emplace_back();
            output.back().assign(lastPos, input);
        }

        if(*input) ++input;
    }
}

bool splitString(const string input, StringVector& output, const StringVector& delim,
                        const string& splitTrim, const unsigned char& params)
{

    unsigned int foundIndex = UINT_MAX, endIndex = 0, lastIndex = 0;

    vector<unsigned int> foundIndices;

    for(auto& str : delim)
    {
        foundIndex = findString(input, str, params);
        if(foundIndex != UINT_MAX) foundIndices.push_back(foundIndex);
    }

    rmDuplicates(foundIndices);
    order(foundIndices);

    for(auto& idx : foundIndices)
    {
        if(idx != lastIndex)
        {
            endIndex = idx-1;
            while(isCharType(input[endIndex], DELIM_STANDARD)) --endIndex;
            output.emplace_back();
            output.back().assign(input.begin() + lastIndex, input.begin() + endIndex+1);
            lastIndex = idx;
        }
        while((lastIndex < input.size()) && !isCharType(input[lastIndex], DELIM_STANDARD)) ++lastIndex;
        while((lastIndex < input.size()) && isCharType(input[lastIndex], DELIM_STANDARD)) ++lastIndex; // Shift to next word
    }

    if((foundIndices.size() > 0) && (lastIndex < input.size()))
    {
        output.emplace_back();
        output.back().assign(input.begin() + lastIndex, input.end());
    }

    return foundIndices.size() > 0;

}

void splitByString(const char* input,
                   vector<string>& output,
                   const string& term,
                   const unsigned char& params,
                   const float& threshold)
{

    bool bFound = false;

    for(size_t i = 0, j = 0, k = 0, l = 0, L = strlen(input); i < L; ++i)
    {

        if((i != j) && isDelimiter(input[i]))
        {
            string test(&input[j], &input[i]);

            if(cmpString(test, term, params, threshold))
            {
                if(l)
                {
                    output.emplace_back(&input[k], &input[l]);
                }
                bFound = true;
            }
            l = i;
            while(isDelimiter(input[i]))
            {
                ++i;
            }
            j = i;
            if(bFound)
            {
                k = i;
                bFound = false;
            }
        }
        else if(i + 1 == L)
        {
            string test(&input[k], &input[L]);
            if(!cmpString(test, term, params, threshold))
            {
                output.emplace_back(&input[k], &input[L]);
            }
            else
            {
                output.emplace_back(&input[k], &input[l]);
            }
        }

    }

}

void splitByStrings(const char* input,
                    vector<string>& output,
                    const vector<string>& terms,
                    const unsigned char& params,
                    const float& threshold)
{

    bool bFound = false;

    for(size_t i = 0, j = 0, k = 0, l = 0, L = strlen(input); i < L; ++i)
    {

        if((i != j) && isDelimiter(input[i]))
        {
            string test(&input[j], &input[i]);

            if(cmpStringToList(test, terms, params, threshold))
            {
                if(k < l)
                {
                    output.emplace_back(&input[k], &input[l]);
                }
                bFound = true;
            }
            l = i;
            while(isDelimiter(input[i]))
            {
                ++i;
            }
            j = i;
            if(bFound)
            {
                k = i;
                bFound = false;
            }
        }
        else if((k < L) && (i + 1 == L))
        {
            string test(&input[k], &input[L]);
            if(!cmpStringToList(test, terms, params, threshold))
            {
                output.emplace_back(&input[k], &input[L]);
            }
            else if(k < l)
            {
                output.emplace_back(&input[k], &input[l]);
            }
        }

    }

}

void concatenate_lateral(string& left, const string& right,
                                const unsigned int& spacing, const string& delim)
{

    if(right.empty() || delim.empty()) return;
    if(left.empty())
    {
        left = right;
        return;
    }

    size_t L1 = nline(left), L2 = nline(right), L = L1 > L2 ? L1 : L2,
           x = ncol(left, delim), rowLength = 1, cLine = 0;

    for(size_t i = 0; cLine < L; ++i)
    {
        if(i >= left.size())
        {

            if(cLine < L2)
            {
                while(rowLength < x + spacing)
                {
                    left.insert(left.end(), delim.front());
                    ++i;
                    ++rowLength;
                }
            }
            else
            {
                while(rowLength < x)
                {
                    left.insert(left.end(), delim.front());
                    ++i;
                    ++rowLength;
                }
            }

            left.insert(left.end(),
                        right.begin() + line_begin(right, cLine),
                        right.begin() + line_end(right, cLine));

            i += line_end(right, cLine) - line_begin(right, cLine);

            left.insert(left.end(), '\n');

            ++i;
            ++cLine;
            rowLength = 1;
        }
        else if(left[i] == '\n')
        {
            if(cLine < L2)
            {
                while(rowLength < x + spacing)
                {
                    left.insert(left.begin() + i, delim.front());
                    ++i;
                    ++rowLength;
                }
            }
            else
            {
                while(rowLength < x)
                {
                    left.insert(left.begin() + i, delim.front());
                    ++i;
                    ++rowLength;
                }
            }

            if(cLine < L2)
            {

                left.insert(left.begin() + i,
                            right.begin() + line_begin(right, cLine),
                            right.begin() + line_end(right, cLine));

                i += line_end(right, cLine) - line_begin(right, cLine);

            }
            ++cLine;
            rowLength = 1;
        }
        else if(isCharType(left[i], delim))
        {
            ++rowLength;
        }
    }

}

string concatenateColumn(const StringVector& info)
{
    string output;
    for(size_t i = 0; i < info.size(); ++i)
    {
        output += info[i];
        if(i < info.size()-1) output += '\n';
    }
    return output;
}

void concatenateString(const vector<string>& input, string& output,
                              const char delim)
{
    size_t L = input.size();
    unsigned int i = 0;
    for(auto str : input)
    {
        output += str;
        if(i < L-1) output += delim;
        ++i;
    }
}

unsigned int getStrSize(char* c)
{
    unsigned int output = 0;
    char* newC = c;
    while((*newC != '\t') && (*newC != '\n') && (*newC != '\0'))
    {
        ++output;
        ++newC;
    }
    newC = c-1;
    while((*newC != '\t') && (*newC != '\n') && (*newC != '\0'))
    {
        ++output;
        --newC;
    }
    return output;
}

bool atWord(const unsigned int& index,
            const string& body,
            const string& query)
{
    unsigned int L = body.size(), S = query.size(), j = 0;
    for(size_t i = index; (i < L) && (j < S); ++i, ++j)
    {
        if(body[i] != query[j]) return false;
    }
    return true;
}

bool check_letter(const char& c, const char& other_c)
{
    if(isLetter(c))
    {
        if(c == other_c) return true;
        if(isUpperCase(c) && isLowerCase(other_c))
        {
            if((c + 32) == other_c) return true;
        }
        else if(isLowerCase(c) && isUpperCase(other_c))
        {
            if((c - 32) == other_c) return true;
        }
    }

    return false;
}

bool isNumeric(char* c)
{
    char* dataPos = c;

    bool numericCheck = false;
    while((*dataPos == ' ') || (*dataPos == '-') || (*dataPos == '+')) ++dataPos;

    while((*dataPos != '\0') && (*dataPos != '\t') && (*dataPos != '\n'))
    {
        if(!isNumber(*dataPos) && (*dataPos != '.')) return false;
        if(((*dataPos == '-') || (*dataPos == '+')) && ((*(dataPos-1) != 'E') || (*(dataPos-1) != 'e'))) return false;
        if(((*dataPos == 'E') || (*dataPos == 'e')) && (dataPos == c)) return false;

        numericCheck = true;
        ++dataPos;
    }
    return numericCheck;
}

bool isNumeric(const string& s)
{
    bool numericCheck = false;
    unsigned int startIndex = 0;
    while((startIndex < s.size()) &&
            ((s[startIndex] == ' ') ||
             (s[startIndex] == '-') || (s[startIndex] == '+')))
    {
        ++startIndex;
    }

    for(size_t i = startIndex; i < s.size(); ++i)
    {
        if(!isNumber(s[i]) && (s[i] != '.')) return false;
        if(((s[i] == '-') || (s[i] == '+')) && ((s[i-1] != 'E') || (s[i-1] != 'e'))) return false;
        if(((s[i] == 'E') || (s[i] == 'e')) && (i == 0)) return false;

        numericCheck = true;
    }

    return numericCheck;
}

bool isEquation(const string& s)
{

    bool numericCheck = false;
    unsigned int startIndex = 0;
    while((startIndex < s.size()) &&
            ((s[startIndex] == ' ') ||
             (s[startIndex] == '-') || (s[startIndex] == '+')))
    {
        ++startIndex;
    }

    for(size_t i = startIndex; i < s.size(); ++i)
    {
        if(!isNumber(s[i]) && !isCharType(s[i], ".!+-*/^()[]{}= ")) return false;
        if(((s[i] == 'E') || (s[i] == 'e')) && !isLetter(s[i + 1])) return false;

        numericCheck = true;
    }

    return numericCheck;

}

bool isDigitalTime(const string& s)
{

    bool numeric = false;
    for(auto& c : s)
    {
        if(isNumeric(c))
        {
            numeric = true;
            break;
        }
    }

    if(numeric)
    {
        return ((s.find(':') < s.size()) ||
                cmpStringToList(s, {"am", "pm" }, CMP_STR_CASE_INSENSITIVE |
                                                  CMP_STR_SIZE_INSENSITIVE));
    }

    return false;

}

bool isGenetic(const string& s)
{

    if(s.size() < 3)
    {
        return false;
    }

    for(auto& c : s)
    {
        if(!isCharType(c, "atcguATCGU-35'"))
        {
            return false;
        }
    }

    return true;

}

bool isUpperCase(const string& s)
{

    for(auto& c : s)
    {
        if(isLowerCase(c))
        {
            return false;
        }
    }

    return true;

}

bool isLowerCase(const string& s)
{
    for(auto& c : s)
    {
        if(isUpperCase(c))
        {
            return false;
        }
    }

    return true;
}

float pctUpperCase(const string& s)
{

    float output = 0.0;

    for(auto& c : s)
    {
        if(isUpperCase(c))
        {
            ++output;
        }
    }

    return output / s.size();

}

string sigFigs(const string& numeric)
{
    if((numeric.find('.') >= numeric.size()) ||
            (numeric.find('E') >= numeric.size())) return numeric;

    string output = numeric;
    while(output.back() == '0') output.pop_back();
    while(output.back() == '.') output.pop_back();

    return output;
}

string sigFigs(const float& value)
{
    stringstream str;
    str << value;
    return sigFigs(str.str());
}

int hexToInt(const char* c)
{
    int L = strlen(c);
    if((L < 2) || (L % 2)) return -1;
    int output = 0, d1, d2;

    for(int i = L - 1, j = 0; i > 0; i -= 2, j += 2)
    {
        if((c[i] > 'f') || (c[i - 1] > 'f')) return -1;

        if(isNumber(c[i]))
        {
            d1 = int(c[i]) - 48;
        }
        else
        {
            d1 = int(c[i]) - 87;
        }

        if(isNumber(c[i - 1]))
        {
            d2 = int(c[i - 1]) - 48;
        }
        else
        {
            d2 = int(c[i - 1]) - 87;
        }

        output += d1*pow(16, j) + d2*pow(16, j+1);

    }

    return output;
}

int hexToInt(const string& s)
{
    return hexToInt(s.c_str());
}

void to_lowercase(string& s)
{
    size_t L = s.size();
    for(size_t i = 0; i < L; ++i)
    {
        if(isUpperCase(s[i])) s[i] += 32;
    }
}

string get_lowercase(const string& s)
{
    string output = s;
    to_lowercase(output);
    return output;
}

void to_uppercase(string& s)
{
    size_t L = s.size();
    for(size_t i = 0; i < L; ++i)
    {
        if(isLowerCase(s[i])) s[i] -= 32;
    }
}

string get_uppercase(const string& s)
{
    string output = s;
    to_uppercase(output);
    return output;
}

void capitalize(string& s)
{
    if(isLowerCase(s.front()))
    {
        s.front() -= 32;
    }
}

void uncapitalize(string& s)
{
    if(isUpperCase(s.front()))
    {
        s.front() += 32;
    }
}

string capitalized(string s)
{
    capitalize(s);
    return s;
}

string non_capitalized(string s)
{
    uncapitalize(s);
    return s;
}

bool isWord(const string& s)
{
    for(auto& c : s)
    {
        if(isCharType(c, DELIM_BASIC))
        {
            return false;
        }
    }
    return true;
}

bool sw_align(const char* query, const char* background,
                     const float& threshold,
                     unsigned int* output,
                     bool case_insensitive,
                     unsigned int* out_pos)
{
    unsigned int xL = strlen(query)+1;
    unsigned int yL = strlen(background)+1;

    vMatrix<int> scores(yL, vector<int>(xL, 0));
    vMatrix<int> dir(yL, vector<int>(xL, 3));

    vector<int> assessment(4, 0);

    for(size_t y = 1, x; y < yL; ++y)
    {
        for(x = 1; x < xL; ++x)
        {
            if((query[x-1] == background[y-1]) || // Case-insensitive alignment
                    (case_insensitive &&
                     ((isUpperCase(query[x-1]) && (query[x-1] == background[y-1] - 32)) ||
                      (isLowerCase(query[x-1]) && (query[x-1] == background[y-1] + 32)))))
            {
                assessment[0] = scores[y-1][x-1] + SW_MATCH_SCORE;
            }
            else
            {
                assessment[0] = scores[y-1][x-1] - SW_MISMATCH_PENALTY;
            }

            if((x > 1) && (dir[y][x-1] == dir[y][x-2])) assessment[1] = scores[y][x-1] - SW_GAP_EXTEND_PENALTY; // Gap extension penalty
            else
            {
                assessment[1] = scores[y][x-1] - SW_GAP_OPEN_PENALTY; // Gap opening penalty
            }

            if((y > 1) && (dir[y-1][x] == dir[y-2][x]))
            {
                assessment[2] = scores[y-1][x] - SW_GAP_EXTEND_PENALTY;
            }
            else assessment[2] = scores[y-1][x] - SW_GAP_OPEN_PENALTY;

            dir[y][x] = maxIndex(assessment);
            scores[y][x] = assessment[dir[y][x]];
        }
    }

    int maxScore = max(scores);
    int thresScore = maxScore * threshold;

    if(output)
    {
        *output = maxScore;
    }

    for(size_t y = 1, x; y < yL; ++y)
    {
        for(x = 1; x < xL; ++x)
        {
            if(out_pos &&
               (scores[y][x] == maxScore))
            {
                   *out_pos = x - 1;
            }
        }
    }

    return (float)maxScore > ((float)(strlen(background))+(float)(strlen(query)))/2*5*threshold;

}

bool sw_align(const string& query, const string& background,
                     const float& threshold,
                     unsigned int* output,
                     bool case_insensitive,
                     unsigned int* out_pos)
{
    return sw_align(query.c_str(), background.c_str(), threshold,
                    output, case_insensitive, out_pos);
}

bool cmpString(const char* focus, const char* other,
              const BYTE& params,
              const float& threshold,
              float* output)
{

    if(!focus || !other) return false;

    size_t s1SIZE = strlen(focus),
           s2SIZE = strlen(other),
           maxSIZE = s1SIZE > s2SIZE ? s1SIZE : s2SIZE;

    if(!(params & CMP_STR_SIZE_INSENSITIVE) && (s1SIZE != s2SIZE))
    {

        if(output) *output = 0.0f;
        return false;

    }

    unsigned int i = 0, j, k,
                 matchSize,
                 match1 = UINT_MAX,
                 match2 = UINT_MAX;

    for(; i < s1SIZE; ++i)
    {

        matchSize = 0;

        for(j = 0; j < s2SIZE; ++j)
        {

            for(k = i; k < s1SIZE; ++k, ++j)
            {

                if((focus[k] == other[j]) ||
                        ((params & CMP_STR_CASE_INSENSITIVE) &&
                         ((isUpperCase(focus[k]) && (focus[k] == (other[j] - 32))) ||
                          (isLowerCase(focus[k]) && (focus[k] == (other[j] + 32))))))
                {

                    ++matchSize;

                    if(match1 == UINT_MAX)
                    {

                        match1 = i;
                        match2 = j;

                    }

                    if(matchSize == s1SIZE || matchSize == s2SIZE)
                    {

                        if(((params & CMP_STR_SMALL_DISCRETE) &&
                                ((s1SIZE < 4) || (s2SIZE < 4))) ||
                                (params & CMP_STR_DISCRETE))
                        {

                            if((match1 &&
                                    !isTextDelim(focus[match1 - 1])) ||
                                    ((match1 + matchSize < s1SIZE) &&
                                     !isTextDelim(focus[match1 + matchSize])) ||
                                    (match2 &&
                                     !isTextDelim(other[match2 - 1])) ||
                                    ((match2 + matchSize < s2SIZE) &&
                                     !isTextDelim(other[match2 + matchSize])))
                            {

                                matchSize = 0;
                                match1 = UINT_MAX;
                                match2 = UINT_MAX;

                            }
                            else
                            {

                                if(output) *output = (float)matchSize/maxSIZE;
                                return true;

                            }

                        }
                        else
                        {

                            if(output) *output = (float)matchSize/maxSIZE;
                            return true;

                        }

                    }

                }
                else
                {

                    if(!params)
                    {

                        if(output) *output = 0.0f;
                        return false;

                    }

                    matchSize = 0;
                    match1 = UINT_MAX;
                    match2 = UINT_MAX;

                    break;

                }

            }

        }

    }

    if(params & CMP_STR_SW)
    {

        if((s1SIZE < s2SIZE) && ((float)s1SIZE/s2SIZE < threshold))
        {
            if(output)
            {
                *output = 0.0f;
            }

            return false;
        }
        else if((s2SIZE < s1SIZE) && ((float)s2SIZE/s1SIZE < threshold))
        {
            if(output)
            {
                *output = 0.0f;
            }

            return false;
        }

        unsigned int sw_output;

        if(sw_align(focus, other, threshold, &sw_output, params & CMP_STR_CASE_INSENSITIVE) &&
                (sw_output > SW_MATCH_SCORE*2))
        {

            if(output)
            {

                *output = (float)sw_output / (maxSIZE * SW_MATCH_SCORE);

                return *output >= threshold;

            }
            else
            {

                return ((float)sw_output / (maxSIZE * SW_MATCH_SCORE) >= threshold);

            }

        }
        else
        {

            if(output) *output = 0.0f;
            return false;

        }

    }

    if(output) *output = 0.0f;
    return false;
}

bool cmpString(const string& focus, const string& other,
                      const BYTE& params,
                      const float& threshold,
                      float* output)
{
    return cmpString(focus.c_str(), other.c_str(), params, threshold, output);
}

size_t findString(const char* focus, const char* other,
                      const BYTE& params,
                      const float& threshold,
                      float* output)
{

    if(!focus || !other) return UINT_MAX;

    size_t s1SIZE = strlen(focus),
           s2SIZE = strlen(other),
           maxSIZE = s1SIZE > s2SIZE ? s1SIZE : s2SIZE;

    if(!(params & CMP_STR_SIZE_INSENSITIVE) && (s1SIZE != s2SIZE))
    {

        if(output) *output = 0.0f;
        return UINT_MAX;

    }

    if((params & CMP_STR_DISCRETE) ||
       ((params & CMP_STR_SMALL_DISCRETE) && (strlen(focus) > 3)))
    {
        for(size_t i = 0, j = 0; i < s2SIZE; ++i)
        {

            if((j < i) &&
               (isDelimiter(other[i]) ||
               (i + 1 == s2SIZE)))
            {

                string test(&other[j], &other[i]);

                if(cmpString(focus, test.c_str(), params, threshold, output))
                {
                    return j;
                }

                while(isDelimiter(other[i]))
                {
                    ++i;
                }

                j = i;

            }

        }

        return UINT_MAX;
    }

    unsigned int i = 0, j, k,
                 matchSize,
                 match1 = UINT_MAX,
                 match2 = UINT_MAX;

    for(; i < s1SIZE; ++i)
    {

        matchSize = 0;

        for(j = 0; j < s2SIZE; ++j)
        {

            for(k = i; k < s1SIZE; ++k, ++j)
            {

                if((focus[k] == other[j]) ||
                        ((params & CMP_STR_CASE_INSENSITIVE) &&
                         ((isUpperCase(focus[k]) && (focus[k] == (other[j] - 32))) ||
                          (isLowerCase(focus[k]) && (focus[k] == (other[j] + 32))))))
                {

                    ++matchSize;

                    if(match1 == UINT_MAX)
                    {

                        match1 = i;
                        match2 = j;

                    }

                    if(matchSize == s1SIZE || matchSize == s2SIZE)
                    {

                        if(((params & CMP_STR_SMALL_DISCRETE) &&
                                ((s1SIZE < 4) || (s2SIZE < 4))) ||
                                (params & CMP_STR_DISCRETE))
                        {

                            if((match1 &&
                                    !isTextDelim(focus[match1 - 1])) ||
                                    ((match1 + matchSize < s1SIZE - 1) &&
                                     !isTextDelim(focus[match1 + matchSize])) ||
                                    (match2 &&
                                     !isTextDelim(other[match2 - 1])) ||
                                    ((match2 + matchSize < s2SIZE - 1) &&
                                     !isTextDelim(other[match2 + matchSize])))
                            {

                                matchSize = 0;
                                match1 = UINT_MAX;
                                match2 = UINT_MAX;

                            }
                            else
                            {

                                if(output) *output = (float)matchSize/maxSIZE;
                                return match2;

                            }

                        }
                        else
                        {

                            if(output) *output = (float)matchSize/maxSIZE;
                            return match2;

                        }

                    }

                }
                else
                {

                    if(!params)
                    {

                        if(output) *output = 0.0f;
                        return UINT_MAX;

                    }

                    matchSize = 0;
                    match1 = UINT_MAX;
                    match2 = UINT_MAX;

                    break;

                }

            }

        }

    }

    if(params & CMP_STR_SW)
    {

        unsigned int sw_output,
                        match_pos;

        if(sw_align(focus, other, threshold, &sw_output, params & CMP_STR_CASE_INSENSITIVE) &&
                (sw_output > SW_MATCH_SCORE*2))
        {

            if(output)
            {

                *output = (float)sw_output / (maxSIZE * SW_MATCH_SCORE);
                if(*output >= threshold)
                {
                    return match_pos;
                }

            }
            else
            {

                if(((float)sw_output / (maxSIZE * SW_MATCH_SCORE) >= threshold))
                {
                    return match_pos;
                }

            }

        }
        else
        {

            if(output) *output = 0.0f;
            return UINT_MAX;

        }

    }

    if(output) *output = 0.0f;
    return UINT_MAX;
}

size_t findStrings(const vector<string>& strings,
                   const char* background,
                   const unsigned char& params,
                   const float& threshold,
                   float* output)
{

    size_t i = UINT_MAX;
    size_t j;

    for(auto& str : strings)
    {
        j = findString(str, background, params, threshold, output);
        if(j < i)
        {
            i = j;
        }
    }

    return i;

}

size_t num_matches(const string& query,
                   const vector<string>& background,
                   const unsigned char& params,
                   const float& threshold)
{
    size_t output = 0;

    for(auto& str : background)
    {
        if(cmpString(query, str, params, threshold))
        {
            ++output;
        }
    }

    return output;

}

bool cmpStringIncludeList(const string& focus,
                          const vector<string>& list,
                                 const BYTE& params,
                                 const float& threshold)
{
    for (auto& str : list)
    {
        if(!cmpString(focus, str, params)) return false;
    }
    return true;
}

bool replace(string& str,
                    const string& sequence,
                    const string& replacement)
{

    unsigned int beginIndex = str.find(sequence);
    if(beginIndex != UINT_MAX)
    {

        str.replace(str.begin() + beginIndex,
                    str.begin() + beginIndex + sequence.size(),
                    replacement);

        return true;

    }

    return false;
}


string rep(const string& str, const unsigned int& N)
{

    string output;

    for(unsigned int i = 0; i < N; ++i)
    {

        output.append(str);

    }

    return output;

}

string getBestStringMatch(const string& tag,
                               const vector<string>& list,
                               const float& threshold)
{

    if(tag.empty()) return string();

    size_t tagSIZE = tag.size();
    if(list.empty()) return string();
    if(list.size() == 1) return list.front();

    unsigned int maxScore = 0, maxIndex = UINT_MAX, cScore = 0, cIndex = 0;
    for(auto& term : list)
    {
        if(sw_align(tag, term, threshold, &cScore))
        {
            cScore /= absolute((int)term.size() - (int)tag.size()) + 1;
            if(cScore > maxScore)
            {
                maxScore = cScore;
                maxIndex = cIndex;
            }
        }

        ++cIndex;
    }

    if(maxIndex == UINT_MAX) return string();
    return list[maxIndex];

}

unsigned int charMatchNum(const string& query, const string& other)  // Case insensitive match comparison
{
    size_t L1 = query.size(), L2 = other.size(), minL = L1 > L2 ? L2 : L1;
    unsigned int matchNum, bestMatchNum = 0;

    for(size_t i = 0; i < L1; ++i)
    {
        for(size_t j = 0; j < L2; ++j)
        {

            matchNum = 0;

            if(isLowerCase(query[i]))  // Find start match position
            {
                while(((i < L1) && (j < L2)) && (query[i] != other[j]))
                {
                    if(query[i] == (other[j] + 32)) break;
                    ++j;
                }
            }
            else if(isUpperCase(query[i]))
            {
                while(((i < L1) && (j < L2)) && (query[i] != other[j]))
                {
                    if(query[i] == (other[j] - 32)) break;
                    ++j;
                }
            }
            else
            {
                while(((i < L1) && (j < L2)) && (query[i] != other[j]))
                {
                    ++j;
                }
            }

            if(j >= L2) break;

            unsigned int startPos = i;

            while((startPos < L1) && (j < L2) && (matchNum < minL))
            {
                if(query[startPos] == other[j])
                {
                    ++startPos;
                    ++j;
                    ++matchNum;
                }
                else if(isLowerCase(query[startPos]))
                {
                    if(query[startPos] == (other[j] + 32))
                    {
                        ++startPos;
                        ++j;
                        ++matchNum;
                    }
                    else
                    {
                        ++startPos;
                        ++j;
                    }
                }
                else if(isUpperCase(query[startPos]))
                {
                    if(query[startPos] == (other[j] - 32))
                    {
                        ++startPos;
                        ++j;
                        ++matchNum;
                    }
                    else
                    {
                        ++startPos;
                        ++j;
                    }
                }
                else
                {
                    ++startPos;
                    ++j;
                }
            }

            if(matchNum > bestMatchNum) bestMatchNum = matchNum;
        }
    }

    return bestMatchNum;
}

unsigned int getBestStringMatchIndex(const string& tag,
                                    const vector<string>& list,
                                    const unsigned char& params)
{
    size_t tagSIZE = tag.size();
    if(tagSIZE < 1) return UINT_MAX;
    if(list.size() < 1) return UINT_MAX;
    if(list.size() == 1) return 0U;

    if(params & CMP_STR_SW)
    {

        unsigned int maxScoreIndex = 0, cIndex = 0, lastScore = 0;
        float maxScore = 0.0f, newScore;

        for(auto& term : list)
        {
            sw_align(tag, term, 0.5f, &lastScore);
            newScore = (float)lastScore/term.size();

            if(newScore > maxScore)
            {
                maxScoreIndex = cIndex;
                maxScore = newScore;
            }

            ++cIndex;

        }

        return maxScoreIndex;

    }

    unsigned int closeSizeIndex = 0, cIndex = 0, minSizeDiff = tagSIZE;

    for(auto& term : list)
    {
        if(term.size() == tagSIZE) return cIndex;
        else
        {
            unsigned int matchNum = charMatchNum(tag, term);
            if(tagSIZE > matchNum)
            {
                if((tagSIZE - matchNum) < minSizeDiff)
                {
                    minSizeDiff = tagSIZE - matchNum;
                    closeSizeIndex = cIndex;
                }
            }
            else
            {
                if((matchNum - tagSIZE) < minSizeDiff)
                {
                    minSizeDiff = matchNum - tagSIZE;
                    closeSizeIndex = cIndex;
                }
            }
        }
        ++cIndex;
    }

    return closeSizeIndex;

}

string getMatchingTag(const string& tag,
                      const vector<string>& list,
                      const BYTE& params,
                      const float& threshold)
{
    vector<string> matches;
    for(auto& item : list)
    {
        if(tag == item) return tag;
        else if(cmpString(tag, item, params, threshold)) matches.push_back(item);
    }
    if(matches.size() > 1)
    {
        return getBestStringMatch(tag, matches, threshold);
    }
    else if(matches.size() == 1) return matches.front();
    else return string();
}

unsigned int getMatchingIndex(const string& focus,
                              const vector<string>& list,
                              const BYTE& params,
                              const float& threshold)
{
    unsigned int matchIndex = 0;
    vector<string> matches;
    vector<unsigned int> matchIndices;

    for(auto& item : list)
    {
        if(focus == item) return matchIndex;
        if(cmpString(focus, item, params, threshold))
        {
            matches.push_back(item);
            matchIndices.push_back(matchIndex);
        }
        ++matchIndex;
    }

    if(matches.size() > 1) return matchIndices[getBestStringMatchIndex(focus, matches, params)];
    else if(matches.size() == 1) return matchIndices.front();
    else return UINT_MAX;
}

bool checkString(const char* query, const char* background)
{
    size_t S = strlen(query),
           L = strlen(background),
           matchIdx = 0;

    if(S > L) return false;

    for(size_t i = 0; i < L - S + 1; ++i)
    {
        if(background[i] == query[matchIdx])
        {
            ++matchIdx;
        }
        else
        {
            matchIdx = 0;
        }

        if(matchIdx == S)
        {
            return true;
        }
    }

    return false;
}

bool removeStrings(StringVector& prompt, const StringVector& strings)
{
    for(size_t i = 0; i < prompt.size();)
    {
        if(cmpStringToList(prompt[i], strings))
        {
            prompt.erase(prompt.begin() + i);
        }
        else ++i;
    }
    return prompt.size() > 0;
}

unsigned int ncol(const string& str, const string& delim)
{
    unsigned int output = 0, rowLength = 1;

    for(const auto& c : str)
    {
        if(c == '\n')
        {
            if(rowLength > output)
            {
                output = rowLength;
            }
            rowLength = 1;
        }
        else if(isCharType(c, delim)) ++rowLength;
    }

    return output;
}

unsigned int nrow(const string& str)
{
    return nline(str);
}

namespace English
{

bool isPlural(const string& str)
{

    return((str.find("s") == (str.size() - 1)) ||
           (str.find("ae") == (str.size() - 2)) ||
           (str.find("i") == (str.size() - 1)) ||
           (str.find("ny") == (str.size() - 2)) ||
           (str.find("ese") == (str.size() - 3)));

}

}

}
