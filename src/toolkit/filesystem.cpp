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

#include "hyper/toolkit/filesystem.hpp"
#include "hyper/toolkit/data_stream.hpp"

#include "hyper/algorithm.hpp"

#include <stdio.h>
#include <unistd.h>

#if defined WIN32 || defined _WIN32
#include <windows.h>
#endif

namespace fs = boost::filesystem;
using namespace std;

namespace hyperC
{

fs::path base_directory(const fs::path& path)
{
    fs::path output = path;
    fs::path sub_path;
    do
    {
        sub_path = output.parent_path();
        if(!sub_path.empty() && (sub_path.string() != "/") && (sub_path.string() != "\\"))
        {
            output = sub_path;
        }
        else
        {
            break;
        }
    }while(!sub_path.empty());
    return output;
}

bool getPathListStrings(const PathList& input, vector<string>& output)
{
    output.reserve(output.size() + input.size());
    const c_itr<fs::path> itEND = input.end();
    for(c_itr<fs::path> it = input.begin(); it != itEND; ++it)
    {
        output.push_back(it->string());
    }
    return output.size() > 0;
}

bool getPathListNames(const PathList& input, vector<string>& output)
{
    output.reserve(output.size() + input.size());
    const c_itr<fs::path> itEND = input.end();
    for(c_itr<fs::path> it = input.begin(); it != itEND; ++it)
    {
        output.push_back(it->stem().string());
    }
    return output.size() > 0;
}

void getFilesInDir(PathList& output,
                   const fs::path& directory,
                   const string& extension)
{
    if(!fs::is_directory(directory)) return;

    try
    {
        fs::directory_iterator end_itr;
        for(fs::directory_iterator itr(directory);
                itr != end_itr; ++itr)
        {
            if(fs::is_directory(itr->status()))
            {
                getFilesInDir(output, itr->path(), extension);
            }
            else if((!extension.empty()) &&
                    (itr->path().extension().string().find(extension) <
                     itr->path().extension().string().size()))
            {
                output.push_back(itr->path());
            }
            else if(extension.size() < 1) output.push_back(itr->path());
        }
    }catch(...){ }
}

void getFilesInDir(PathList& output,
                   const fs::path& directory,
                   const vector<string>& extensions)
{
    if(!fs::is_directory(directory)) return;

    try
    {
        fs::directory_iterator end_itr;
        for(fs::directory_iterator itr(directory);
                itr != end_itr; ++itr)
        {
            if(fs::is_directory(itr->status()))
            {
                getFilesInDir(output, itr->path(), extensions);
            }
            else if((!extensions.empty()) &&
                    cmpStringToList(itr->path().extension().string(),
                                    extensions,
                                    CMP_STR_CASE_INSENSITIVE | CMP_STR_SIZE_INSENSITIVE))
            {
                output.push_back(itr->path());
            }
        }
    }catch(...){ }
}

string getBestPathMatch(const string& query, const fs::path& directory,
                             const float& threshold)
{

    if(!DIR_EXISTS(directory)) return string();
    if(DIR_EXISTS(query)) return query;

    StringVector candidates;
    DIR_ITERATOR end_itr;
    for(DIR_ITERATOR itr(directory); itr != end_itr; ++itr)
    {
        if(cmpString(itr->path().string(), query)) candidates.push_back(itr->path().string());
        if(fs::is_directory(itr->status()))
        {
            string newCandidate = getBestPathMatch(query, itr->path(), threshold);
            if(!newCandidate.empty()) candidates.push_back(newCandidate);
        }
    }

    return getBestStringMatch(query, candidates, threshold);
}

unsigned int dirFileCount(const string& directory)
{
    if(!DIR_EXISTS(directory)) return 0;
    DIR_ITERATOR end_itr;
    unsigned int count = 0;
    for(DIR_ITERATOR itr(directory); itr != end_itr; ++itr)
    {
        ++count;
    }
    return count;
}

bool searchForDir(const string& query, const fs::path& directory,
                  PathList& output)
{

    if(!fs::exists(directory) ||
            !fs::is_directory(directory)) return false;

    bool check = false;

    fs::directory_iterator end_itr;
    for(fs::directory_iterator itr(directory);
            itr != end_itr; ++itr)
    {
        try
        {
            if(fs::is_directory(itr->status()))
            {
                if(cmpString(itr->path().string(), query))
                {

                    bool dup = false;
                    for(auto& out : output)
                    {
                        if(cmpString(itr->path().string(), out.string()))
                        {
                            dup = true;
                            break;
                        }
                    }
                    if(!dup)
                    {
                        output.emplace_back(itr->path());
                        check = true;
                    }
                }
                check |= searchDir(query, itr->path(), output);
            }
        }
        catch(...)
        {
            continue;
        }
    }

    return check;
}

size_t numFilesInDir(const string& directory)
{

    if(!DIR_EXISTS(directory))
    {
        return 0;
    }

    size_t output = 0;

    fs::directory_iterator end_itr;
    for(fs::directory_iterator itr(directory);
        itr != end_itr; ++itr)
    {
        ++output;
    }

    return output;
}

bool directory_empty(const string& directory)
{
    if(!DIR_EXISTS(directory))
    {
        return true;
    }

    fs::directory_iterator end_itr;
    for(fs::directory_iterator itr(directory);
        itr != end_itr; ++itr)
    {
        return false;
    }

    return true;
}

bool searchForDir(const string& query, const PathList& directories,
                  PathList& output)
{

    bool check = false;

    for(auto& directory : directories)
    {

        if(!fs::exists(directory) ||
                !fs::is_directory(directory)) return false;

        fs::directory_iterator end_itr;
        for(fs::directory_iterator itr(directory);
                itr != end_itr; ++itr)
        {
            try
            {
                if(cmpString(itr->path().string(), query))
                {
                    output.emplace_back(itr->path());
                    check = true;
                }
                if(fs::is_directory(itr->status()))
                {
                    check |= searchDir(query, itr->path(), output);
                }
            }
            catch(...)
            {
                continue;
            }
        }

    }

    return check;
}

bool searchDir(const string& query, const fs::path& directory,
               PathList& output)
{

    bool check = false;

    if(!fs::exists(directory) ||
            !fs::is_directory(directory)) return false;

    fs::directory_iterator end_itr;
    for(fs::directory_iterator itr(directory);
            itr != end_itr; ++itr)
    {
        try
        {
            if(cmpString(itr->path().string(), query))
            {
                output.emplace_back(itr->path());
                check = true;
            }
            if(fs::is_directory(itr->status()))
            {
                check |= searchDir(query, itr->path(), output);
            }
        }
        catch(...)
        {
            continue;
        }
    }

    return check;
}

bool searchDir(const string& query, const PathList& directories,
               PathList& output)
{

    bool check = false;

    for(auto& directory : directories)
    {
        if(!fs::exists(directory) ||
                !fs::is_directory(directory)) return false;

        fs::directory_iterator end_itr;
        for(fs::directory_iterator itr(directory);
                itr != end_itr; ++itr)
        {
            try
            {
                if(cmpString(itr->path().string(), query))
                {
                    output.emplace_back(itr->path());
                    check = true;
                }
                if(fs::is_directory(itr->status()))
                {
                    check |= searchDir(query, itr->path(), output);
                }
            }
            catch(...)
            {
                continue;
            }
        }

    }

    return check;
}

bool fileExists(const string& query, const string& directory)
{
    if(!DIR_EXISTS(directory)) return false;

    fs::directory_iterator end_itr;
    for(fs::directory_iterator itr(directory);
            itr != end_itr; ++itr)
    {
        if(itr->path().stem().string() == query) return true;
    }
    return false;
}

fs::path getBranchDirectory(const PathList& paths)
{

    fs::path tmp, output;

    if(paths.empty()) return output;

    output = paths.front();
    if(paths.size() == 1) return output.parent_path();

    for(size_t i = 1; i < paths.size(); ++i)
    {

        while(!output.empty())
        {

            if(!fs::is_directory(paths[i]))
            {
                tmp = paths[i].parent_path();
            }
            else
            {
                tmp = paths[i];
            }

            while(!tmp.empty())
            {
                if(output.stem().string() == tmp.stem().string())
                {
                    goto nextLoop;
                }
                else
                {
                    tmp = tmp.parent_path();
                }
            }

            if(tmp.empty())
            {
                output = output.parent_path();
            }
        }

        break;

        nextLoop:;

    }

    return output;

}

void activateFile(const string& filename)
{
#if defined WIN32 || defined _WIN32
    ShellExecute(NULL, NULL, filename.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
    system(string("open ").append(filename).c_str());
#endif
}

size_t getFileSize(const string& filename)
{
    try{
        FILE* inFILE = fopen(filename.c_str(), "rb");

        if(!inFILE)
        {
            return 0;
        }

        return getFileSize(inFILE);
    }catch(...)
    {
        return 0;
    }
}

size_t getFileSize(FILE* inFILE)
{

    if(!inFILE)
    {
        return -1;
    }

    fpos_t initPos = ftell(inFILE);

    fseek(inFILE, 0, SEEK_END);
    size_t output = ftell(inFILE);
    fsetpos(inFILE, &initPos);

    return output;

}

size_t getFileData(const string& filename,
                void** output)
{
    try
    {
        FILE* inFILE = fopen(filename.c_str(), "rb");

        if(!inFILE)
        {
            return 0;
        }

        size_t L = getFileSize(inFILE);

        *output = new unsigned char[L];

        fread(*output, sizeof(unsigned char), L, inFILE);

        fclose(inFILE);

        return L;
    }catch(...)
    {
        *output = NULL;
    }

    return 0;
}



bool saveFileData(void* input,
                 const size_t& inputSIZE,
                 const string& filename)
{
    if(!input || !inputSIZE)
    {
        return false;
    }

    FILE* outFILE = fopen(filename.c_str(), "wb");

    if(!outFILE)
    {
        return false;
    }

    fwrite(input, sizeof(unsigned char), inputSIZE, outFILE);

    fclose(outFILE);

    return true;
}

unsigned int getMatchingIndex(const fs::path& focus,
                              const vector<fs::path>& list,
                              const BYTE& params)
{
    unsigned int matchIndex = 0;
    vector<string> matches;
    vector<unsigned int> matchIndices;
    for(auto& item : list)
    {
        if(focus == item) return matchIndex;
        if(cmpString(focus.string(), item.string(), params))
        {
            matches.push_back(item.string());
            matchIndices.push_back(matchIndex);
        }
        ++matchIndex;
    }
    if(matches.size() > 1) return matchIndices[getBestStringMatchIndex(focus.string(), matches)];
    else if(matches.size() == 1) return matchIndices.front();
    else return UINT_MAX;
}

std::string getUserDataPath()
{
#ifdef __WIN32 || defined _WIN32 || defined WIN32
    return string(getenv("USERPROFILE"));
#elif defined __APPLE__
    return string(getenv("HOME"));
#else
    return string();
#endif
}

}
