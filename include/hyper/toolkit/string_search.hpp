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

#ifndef TOOLKIT_STRING_SEARCH
#define TOOLKIT_STRING_SEARCH

#include <string>
#include <vector>
#include <map>
#include <unistd.h>

#include "hyper/toolkit/clustered_vector.hpp"
#include "hyper/toolkit/string.hpp"

namespace hyperC
{

template<class string_container_t, typename string_t>
class clustered_stringvector : public clustered_vector<char, string_container_t, string_t>
{
protected:

    bool bCaseInsensitive;

public:

    clustered_stringvector():
        bCaseInsensitive(true){ }

    clustered_stringvector(string_container_t& V,
                          const bool& caseInsensitive = true):
        bCaseInsensitive(caseInsensitive)
    {
        assemble(V);
    }

    void assemble(string_container_t& V)
    {
        char newKey;

        for(auto& string : V)
        {

            newKey = string.front();

            if(case_insensitive())
            {
                if(isLowerCase(newKey))
                {
                    newKey -= 32;
                }
            }

            try
            {
                (*this).at(newKey);
            }catch(...)
            {
                (*this)[newKey] = string_container_t();
            }

            string_container_t& v_bin = (*this).at(newKey);

            if(v_bin.empty())
            {
                v_bin.emplace_back(string);
            }
            else
            {
                for(size_t i = 0; i < v_bin.size(); ++i)
                {
                    if(string < v_bin[i])
                    {
                        v_bin.insert(v_bin.begin() + i, string);
                        goto inserted;
                    }
                }

                v_bin.emplace_back(string);

                inserted:;
            }
        }
    }

    inline void set_case_insensitive(const bool& status = true)
    {
        bCaseInsensitive = status;
    }
    inline const bool& case_insensitive() const
    {
        return bCaseInsensitive;
    }

    template<typename string_key_t>
    bool get_references(string_key_t keys,
                        reference_vector<string_t>& output)
    {
        if(case_insensitive())
        {
            for(auto& char_t : keys)
            {
                if(isLowerCase(char_t))
                {
                    char_t -= 32;
                }
            }
        }

        return clustered_vector<char, string_container_t, string_t>::get_references(keys, output);
    }

    template<typename string_key_t>
    bool get_values(string_key_t keys,
               std::vector<string_t>& output)
    {
        if(case_insensitive())
        {
            for(auto& char_t : keys)
            {
                if(isLowerCase(char_t))
                {
                    char_t -= 32;
                }
            }
        }

        return clustered_vector<char, string_container_t, string_t>::get_values(keys, output);
    }

    friend std::ostream& operator<<(std::ostream& output, const clustered_stringvector& input)
    {
        size_t idx = 0;
        for(auto& pair : input)
        {
            output << pair.first << "\t=======================\n[";
            for(size_t i = 0; i < pair.second.size(); ++i)
            {
                output << pair.second[i];
                if(i < pair.second.size() - 1)
                {
                    output << ',';
                }
            }

            output << ']';

            if(idx < input.size() - 1) output << '\n';
        }
        return output;
    }

};

///////////////////////////////////////////////////////////////////

/* Codex acquire methods                                         */

///////////////////////////////////////////////////////////////////

typedef std::map<char, std::vector<const char*>> ez_cluster_map;

bool getClusterMap(const char* str,
                   ez_cluster_map& output,
                   const int& offset = 0);
inline bool getClusterMap(const std::string& str,
                          ez_cluster_map& output)
{
    return getClusterMap(str.c_str(), output);
}

void getClusterCodex(const char* str,
                    std::string& output,
                    const int& offset = 0);
inline void getClusterCodex(std::string& str,
                            std::string& output,
                                const int& offset = 0)
{
    return getClusterCodex(str.c_str(), output, offset);
}

typedef std::vector<std::string> tree_search_codex;

void getTreeCodex(const char* str,
                  tree_search_codex& output,
                  bool case_insensitive = false);
inline void getTreeCodex(const std::string& str,
                         tree_search_codex& output,
                         bool case_insensitive = false)
{
    return getTreeCodex(str.c_str(), output, case_insensitive);
}

///////////////////////////////////////////////////////////////////

/* Fast string search implementations                            */

///////////////////////////////////////////////////////////////////

/**  Boyer-Moore  **/

// Wrapped search function - form tables denovo
size_t boyer_moore_search(const char* pattern,
                          const char* background,
                          const bool& case_insensitive = false);
inline size_t boyer_moore_search(const std::string& pattern,
                                 const std::string& background,
                                 const bool& case_insensitive = false)
{
    return boyer_moore_search(pattern.c_str(), background.c_str(), case_insensitive);
}

// Bare search function - use for repeated searches with pre-processed pattern
size_t boyer_moore_search(const char* pattern,
                          const char* background,
                          const std::vector<size_t>& char_table,
                          const std::vector<size_t>& match_table,
                          const bool& case_insensitive = false);
inline size_t boyer_moore_search(const std::string& pattern,
                                 const std::string& background,
                                 const std::vector<size_t>& char_table,
                                 const std::vector<size_t>& match_table,
                                 const bool& case_insensitive = false)
{
    return boyer_moore_search(pattern.c_str(), background.c_str(), char_table, match_table, case_insensitive);
}

// Create the character skip reference
void boyer_moore_char_table(const char* pattern,
                            std::vector<size_t>& alignments,
                            const bool& case_insensitive = false);
inline void boyer_moore_char_table(const std::string& pattern,
                                   std::vector<size_t>& alignments,
                                   const bool& case_insensitive = false)
{
    boyer_moore_char_table(pattern.c_str(), alignments, case_insensitive);
}

// Create the prefix/suffix skip reference
void boyer_moore_match_table(const char* pattern,
                             std::vector<size_t>& alignments,
                             const bool& case_insensitive = false);
inline void boyer_moore_match_table(const std::string& pattern,
                                   std::vector<size_t>& alignments,
                                   const bool& case_insensitive = false)
{
    boyer_moore_match_table(pattern.c_str(), alignments, case_insensitive);
}

///////////////////////////////////////////////////////////////////

/* Advanced string analyses                                      */

///////////////////////////////////////////////////////////////////

std::string getAcronym(const char* text,
                       const char* acronym);

template<typename string_t1,
            typename string_t2>
std::string getAcronym(const string_t1& text,
                              const string_t2& acronym)
{
    return getAcronym(text.c_str(), acronym.c_str());
}

bool sentence_context(const char* text,
                      const char* focus,
                      const char* context,
                      bool case_insensitive = false);

inline bool sentence_context(const std::string& text,
                         const std::string& focus,
                         const std::string& context,
                         bool case_insensitive = false)
{
    return sentence_context(&text[0], &focus[0], &context[0], case_insensitive);
}

bool sentence_context(const char* text,
                      const std::vector<std::string>& terms,
                      bool case_insensitive = false,
                      bool ordered = true);
template<typename string_t>
bool sentence_context(const string_t& text,
                      std::initializer_list<std::string> list,
                      bool case_insensitive = false,
                      bool ordered = true)
{
    return sentence_context(&text[0], std::vector<std::string>(list), case_insensitive, ordered);
}

std::string word_after(const char* ptr,
                       const char* skip = ":;, ",
                       const char* stop = ".!?");
std::string word_after_match(const char* pattern,
                             const char* background,
                             const char* skip = ":;, ",
                             const char* stop = ".!?");
template<typename basic_string_t>
std::string word_after_match(const char* pattern,
                             const basic_string_t& background,
                             const char* skip = ":;, ",
                             const char* stop = ".!?")
{
    return word_after_match(pattern, background.c_str(), skip, stop);
}
template<typename basic_string_t, typename delim_string_t>
std::string word_after_match(const basic_string_t& pattern,
                             const basic_string_t& background,
                             const delim_string_t& skip = ":;, ",
                             const delim_string_t& stop = ".!?")
{
    return word_after_match(pattern.c_str(), background.c_str(), skip.c_str(), stop.c_str());
}

std::string word_before(const char* ptr,
                        const char* skip = ":;, ",
                        const char* stop = ".!?");
std::string word_before_match(const char* pattern,
                              const char* background,
                              const char* skip = ":;, ",
                              const char* stop = ".!?");
template<typename basic_string_t, typename delim_string_t>
std::string word_before_match(const basic_string_t& pattern,
                              const basic_string_t& background,
                              const delim_string_t& skip = ":;, ",
                              const delim_string_t& stop = ".!?")
{
    return word_before_match(pattern.c_str(), background.c_str(), skip.c_str(), stop.c_str());
}

template<typename string_constructable>
void extract_after(std::vector<string_constructable>& output,
                   const char* pattern,
                   const char* background,
                   const char* skip = ":;, ",
                   const char* stop = ".!?")
{

    size_t match = 0;

    std::vector<size_t> char_table;
    std::vector<size_t> match_table;

    boyer_moore_char_table(pattern, char_table);
    boyer_moore_match_table(pattern, match_table);

    size_t L = strlen(pattern);

    const char* c = background;

    while((match = boyer_moore_search(pattern, c, char_table, match_table, true)) != UINT_MAX)
    {
        output.emplace_back(word_after(c + match + L, skip, stop));
        c += match + L;
    }

}

template<typename output_string_t>
void extract_after(std::vector<output_string_t>& output,
                   const std::string& pattern,
                   const std::string& background,
                   const char* skip = ":;, ",
                   const char* stop = ".!?")
{
    extract_after(output, pattern.c_str(), background.c_str(), skip, stop);
}

}

#endif // TOOLKIT_STRING_SEARCH
