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

#include "hyper/toolkit/string_search.hpp"
#include "hyper/toolkit/charsurf.hpp"

using namespace std;

namespace hyperC
{

bool getClusterMap(const char* str,
                   ez_cluster_map& output,
                   const int& offset)
{

    string word;
    const char* last_ptr = str;

    size_t i = 0;

    while(read_word(&str, word))
    {
        output[word.front()].emplace_back(last_ptr + offset);
        last_ptr = str;

        ++i;
    }

    return !output.empty();

}

void getClusterCodex(const char* str,
                            string& output,
                            const int& offset)
{

    char c = *(str + offset);

    int i = 0;

    if(output.empty())
    {
        output.push_back(c);
    }
    else
    {
        for(i = 0; i < output.size(); ++i)
        {
            if(c < output[i])
            {
                output.insert(output.begin() + i, c);
                goto next;
            }
            else if(c == output[i]) goto start;
        }

        output.push_back(c);

        start:;
    }

    while(*str)
    {
        while(*str && !isTextDelim(*str))
        {
            ++str;
        }

        while(*str && isTextDelim(*str))
        {
            ++str;
        }

        if(*str)
        {

            c = *(str + offset);

            for(i = 0; i < output.size(); ++i)
            {
                if(c < output[i])
                {
                    output.insert(output.begin() + i, c);
                    goto next;
                }
                else if(c == output[i]) goto next;
            }

            output.push_back(c);

            next:;

        }
    }

}

void getTreeCodex(const char* str,
                  tree_search_codex& output,
                  bool case_insensitive)
{

    string word;

    while(read_word(&str, word))
    {
        for(size_t i = 0; i < word.size(); ++i)
        {
            if(i >= output.size())
            {
                output.resize(i + 1);
            }

            if(output[i].empty())
            {
                output[i].push_back(word[i]);
            }
            else
            {
                for(size_t j = 0; j < output[i].size(); ++j)
                {
                    if(word[i] < output[i][j])
                    {
                        output[i].insert(output[i].begin() + j, word[i]);
                        goto inserted;
                    }
                    else if(word[i] == output[i][j])
                    {
                        goto inserted;
                    }
                }

                output[i].push_back(word[i]);

                inserted:;

                if(case_insensitive && !i && isLetter(word[i]))
                {

                    char c;
                    if(isUpperCase(word[i]))
                    {
                        c = word[i] + 32;
                    }
                    else if(isLowerCase(word[i]))
                    {
                        c = word[i] - 32;
                    }

                    for(size_t j = 0; j < output[i].size(); ++j)
                    {
                        if(c < output[i][j])
                        {
                            output[i].insert(output[i].begin() + j, c);
                            goto case_inserted;
                        }
                        else if(c == output[i][j])
                        {
                            goto case_inserted;
                        }
                    }

                    output[i].push_back(c);

                    case_inserted:;
                }
            }
        }
    }

}

size_t boyer_moore_search(const char* pattern,
                          const char* background,
                          const bool& case_insensitive)
{

    if(!(*pattern))
    {
        return UINT_MAX;
    }

    vector<size_t> char_table;
    vector<size_t> match_table;

    boyer_moore_char_table(pattern, char_table, case_insensitive);
    boyer_moore_match_table(pattern, match_table, case_insensitive);

    const size_t Lb = strlen(background);
    const size_t Lp = strlen(pattern);

    size_t i = Lp - 1;

    while(i < Lb)
    {

        int j = Lp - 1;

        while((j >= 0) && ((background[i] == pattern[j]) ||
                           (case_insensitive && case_cmp(background[i], pattern[j]))))
        {
            --i;
            --j;
        }

        if(j < 0)
        {
            return i + 1;
        }

        i += (char_table[uint8_t(background[i])] > match_table[j] ?
              char_table[uint8_t(background[i])] : match_table[j]);

    }

    return UINT_MAX;

}

size_t boyer_moore_search(const char* pattern,
                          const char* background,
                          const vector<size_t>& char_table,
                          const vector<size_t>& match_table,
                          const bool& case_insensitive)
{

    if(!(*pattern))
    {
        return UINT_MAX;
    }

    const size_t Lb = strlen(background);
    const size_t Lp = strlen(pattern);

    size_t i = Lp - 1;

    while(i < Lb)
    {

        int j = Lp - 1;

        while((j >= 0) && ((background[i] == pattern[j]) ||
                           (case_insensitive && case_cmp(background[i], pattern[j]))))
        {
            --i;
            --j;
        }

        if(j < 0)
        {
            return i + 1;
        }

        i += (char_table[uint8_t(background[i])] > match_table[j] ?
              char_table[uint8_t(background[i])] : match_table[j]);

    }

    return UINT_MAX;

}

void boyer_moore_char_table(const char* pattern,
                            vector<size_t>& alignments,
                            const bool& case_insensitive)
{
    alignments.resize(256);
    const size_t L = strlen(pattern);

    for(auto& n : alignments)
    {
        n = L;
    }

    for(size_t i = 0; i < L - 1; ++i)
    {
        alignments[uint8_t(pattern[i])] = L - 1 - i;
        if(case_insensitive)
        {
            if(isUpperCase(pattern[i]))
            {
                alignments[uint8_t(pattern[i]) + 32] = L - 1 - i;
            }
            else if(isLowerCase(pattern[i]))
            {
                alignments[uint8_t(pattern[i]) - 32] = L - 1 - i;
            }
        }
    }

}

/*
    Determine if the string section past pos
    is also repeated at the beginning of the word
*/

bool is_prefix(const char* str, size_t pos, const bool& case_insensitive)
{
    const size_t L = strlen(str);
    const size_t suffix_len = L - pos;

    for(size_t i = 0; i < suffix_len; ++i)
    {
        if((str[i] != str[pos + i]) &&
           (!case_insensitive || !case_cmp(str[i], str[pos + i])))
        {
            return false;
        }
    }

    return true;
}

/*
    Determine the longest suffix length of a word ending at position pos
*/

size_t suffix_length(const char* str, size_t pos, const bool& case_insensitive)
{

    const size_t L = strlen(str);
    size_t i;

    for(i = 0;
        ((str[pos - i] == str[L - 1 - i]) ||
         (case_insensitive && case_cmp(str[pos - i], str[L - 1 - i]))) &&
         (i < pos);
        ++i);

    return i;

}

void boyer_moore_match_table(const char* pattern,
                             vector<size_t>& alignments,
                             const bool& case_insensitive)
{

    const size_t L = strlen(pattern);
    size_t last_prefix_index = L - 1;

    alignments.resize(L);

    int i;

    for(i = last_prefix_index; i >= 0; --i)
    {
        if(is_prefix(pattern, i + 1, case_insensitive))
        {
            last_prefix_index = i + 1;
        }
        alignments[i] = last_prefix_index + (L - 1 - i);
    }

    for(i = 0; i < L - 1; ++i)
    {
        int suffix_len = suffix_length(pattern, i, case_insensitive);
        if((pattern[i - suffix_len] != pattern[L - 1 - suffix_len]) &&
           (!case_insensitive || !case_cmp(pattern[i - suffix_len], pattern[L - 1 - suffix_len])))
        {
            alignments[L - 1 - suffix_len] = L - 1 - i + suffix_len;
        }
    }


}

string getAcronym(const char* text,
                       const char* acronym)
{
    int pos = 0, surf_pos;
    const char* surf, *ac_surf, *match = NULL;
    string output;

    if(!(*text) || !(*acronym)) return output;

    while(*text)
    {
        if(pos && ptr_at_string(text, acronym) &&
           (*(text - 1) == '(') && (*(text + strlen(acronym)) == ')'))
        {

            ac_surf = acronym + strlen(acronym) - 1;
            surf = text - 1;
            surf_pos = pos - 1;

            while(*surf && isTextDelim(*surf))
            {
                --surf;
                --surf_pos;
            }

            match = surf + 1;

            while((surf_pos >= 0) && !isSentenceDelim(*surf))
            {

                if(!surf_pos || at_word_begin(surf))
                {
                    if((*surf == *ac_surf) || case_cmp(*surf, *ac_surf))
                    {

                        if(ac_surf == acronym)
                        {
                            output.assign(surf, match);
                            return output;
                        }
                        else
                        {
                            --ac_surf;
                        }
                    }
                }

                --surf;
                --surf_pos;
            }
        }

        ++text;
        ++pos;
    }

    return output;
}

bool sentence_context(const char* text,
                      const char* focus,
                      const char* context,
                      bool case_insensitive)
{

    bool bFocus = false,
        bContext = false;

    while(*text)
    {

        if(isSentenceDelim(*text))
        {
            bFocus = false;
            bContext = false;
        }

        if(!bFocus && ptr_at_string(text, focus, case_insensitive))
        {
            bFocus = true;
            if(bContext) return true;
            text += strlen(focus);
        }
        else if(!bContext && ptr_at_string(text, context, case_insensitive))
        {
            bContext = true;
            if(bFocus) return true;
            text += strlen(context);
        }
        else ++text;
    }

    return false;
}

bool sentence_context(const char* text,
                      const vector<string>& terms,
                      bool case_insensitive,
                      bool ordered)
{

    size_t i = 0;

    if(ordered)
    {

        while(*text)
        {

            if(ptr_at_string(text, terms[i].c_str(), case_insensitive))
            {
                ++i;

                if(i == terms.size())
                {
                    return true;
                }
            }

            ++text;
        }

    }
    else
    {

        vector<bool> matches(terms.size(), false);

        while(*text)
        {

            for(i = 0; i < terms.size(); ++i)
            {
                if(!matches[i] && ptr_at_string(text, terms[i].c_str(), case_insensitive))
                {
                    matches[i] = true;

                    for(auto flag : matches)
                    {
                        if(!flag)
                        {
                            goto next;
                        }
                    }

                    return true;

                    next:;

                    text += strlen(terms[i].c_str());
                }
            }

            ++text;

        }

    }

    return false;

}

string word_after(const char* ptr,
                  const char* skip,
                  const char* stop)
{

    if(!(*ptr))
    {
        return string();
    }

    while(*ptr && !isCharType(*ptr, skip))
    {
        ++ptr;
    }

    while(*ptr && isCharType(*ptr, skip))
    {
        ++ptr;
    }

    const char* init_c = ptr;

    while(*ptr && !isCharType(*ptr, stop))
    {
        ++ptr;
    }

    while(*ptr && isCharType(*(ptr - 1), skip))
    {
        --ptr;
    }

    string output;
    int L = distance(init_c, ptr);

    if(L > 0)
    {
        output.assign(init_c, L);
    }

    return output;
}

string word_after_match(const char* pattern,
                        const char* background,
                        const char* skip,
                        const char* stop)
{

    size_t match = boyer_moore_search(pattern, background, true);

    if(match != UINT_MAX)
    {
        return word_after(background + match + strlen(pattern), skip, stop);

    }

    return string();

}

string word_before(const char* ptr,
                   const char* skip,
                   const char* stop)
{
    if(!(*ptr))
    {
        return string();
    }

    while(!isCharType(*ptr, skip))
    {
        --ptr;
    }

    while(isCharType(*ptr, skip))
    {
        --ptr;
    }

    const char* end_c = ptr + 1;

    while(!isCharType(*ptr, stop))
    {
        --ptr;
    }

    string output;
    size_t L = distance(ptr + 1, end_c);

    if(L)
    {
        output.assign(ptr + 1, distance(ptr + 1, end_c));
    }

    return output;
}

string word_before_match(const char* pattern,
                         const char* background,
                         const char* skip,
                         const char* stop)
{

    size_t match = boyer_moore_search(pattern, background, true);

    if(match != UINT_MAX)
    {

        return word_before(background + match, skip, stop);

    }

    return string();

}

}
