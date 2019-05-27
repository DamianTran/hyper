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

#ifndef EZ_FILESYSTEM
#define EZ_FILESYSTEM

#include <boost/filesystem.hpp>

#define FILE_STATUS_EXISTS              0b1
#define FILE_STATUS_WRITABLE            0b10
#define FILE_STATUS_READABLE            0b100
#define FILE_STATUS_DIR                 0b1000

#define DIR_EXISTS(DIR)                 boost::filesystem::exists(DIR)
#define FILE_EXISTS(FILE)               boost::filesystem::exists(FILE)

#define PRIVATE_MEM_HASH_SIZE           "F_MHS"
#define PRIVATE_MEM_HASH                "F_MH"
#define PRIVATE_MEM_USERNAME            "UNAME"

#define BASE_HASH                       "$%056AB7&bg@#*yshYH54B*%[[-"

#define CMP_STR_DEFAULT 0b1011

#include "hyper/toolkit/string.hpp"

namespace hyperC
{

typedef std::vector<boost::filesystem::path> PathList;
typedef boost::filesystem::directory_iterator DIR_ITERATOR;

boost::filesystem::path base_directory(const boost::filesystem::path& path);

bool getPathListNames(const PathList& input, std::vector<std::string>& output);
bool getPathListStrings(const PathList& input, std::vector<std::string>& output);
void getFilesInDir(PathList& output,
                   const boost::filesystem::path& directory,
                   const std::string& extension = "");
void getFilesInDir(PathList& output,
                   const boost::filesystem::path& directory,
                   const std::vector<std::string>& extensions);
std::string getBestPathMatch(const std::string& query, const boost::filesystem::path& directory,
                             const float& threshold = 0.6f);
unsigned int dirFileCount(const std::string& directory);
bool searchForDir(const std::string& query, const boost::filesystem::path& directory,
                                  PathList& output);
bool searchForDir(const std::string& query, const PathList& directories,
                  PathList& output);
bool searchDir(const std::string& query, const boost::filesystem::path& directory,
               PathList& output);
bool searchDir(const std::string& query, const PathList& directories,
               PathList& output);
bool fileExists(const std::string& filename, const std::string& directory);

bool assembleVM(const std::string& directory, const std::string& rootName, // Assembly for verbal matrix class
              const bool& cleanUp = true, const bool& verbose = true);

boost::filesystem::path getBranchDirectory(const PathList& paths);

size_t numFilesInDir(const std::string& directory);
bool directory_empty(const std::string& directory);

void activateFile(const std::string& filename);

size_t getFileSize(const std::string& filename);
size_t getFileSize(FILE* inFILE);

size_t getFileData(const std::string& filename,
                void** output);
bool saveFileData(void* input,
                 const size_t& inputSIZE,
                 const std::string& filename);

unsigned int getMatchingIndex(const boost::filesystem::path& query,
                              const std::vector<boost::filesystem::path>& list,
                              const unsigned char& params = CMP_STR_DEFAULT);

/* ================================================ //

// Templated C-stream read/write functions

// ================================================ */

inline void fwrite(const uint8_t& data, FILE* outFILE)          { fwrite(&data, 1, sizeof(data), outFILE); }
inline void fwrite(const uint16_t& data, FILE* outFILE)         { fwrite(&data, 1, sizeof(data), outFILE); }
inline void fwrite(const uint32_t& data, FILE* outFILE)         { fwrite(&data, 1, sizeof(data), outFILE); }
inline void fwrite(const uint64_t& data, FILE* outFILE)         { fwrite(&data, 1, sizeof(data), outFILE); }

inline void fwrite(const int8_t& data, FILE* outFILE)           { fwrite(&data, 1, sizeof(data), outFILE); }
inline void fwrite(const int16_t& data, FILE* outFILE)          { fwrite(&data, 1, sizeof(data), outFILE); }
inline void fwrite(const int32_t& data, FILE* outFILE)          { fwrite(&data, 1, sizeof(data), outFILE); }
inline void fwrite(const int64_t& data, FILE* outFILE)          { fwrite(&data, 1, sizeof(data), outFILE); }

inline void fwrite(const float& data, FILE* outFILE)            { fwrite(&data, 1, sizeof(data), outFILE); }
inline void fwrite(const double& data, FILE* outFILE)           { fwrite(&data, 1, sizeof(data), outFILE); }
inline void fwrite(const long double& data, FILE* outFILE)      { fwrite(&data, 1, sizeof(data), outFILE); }

inline void fwrite(const char& data, FILE* outFILE)             { fwrite(&data, 1, sizeof(data), outFILE); }
inline void fwrite(const bool& data, FILE* outFILE)             { fwrite(&data, 1, sizeof(data), outFILE); }

inline void fwrite(const char* data, FILE* outFILE)
{
    uint64_t L = strlen(data);
    fwrite(&L, 1, sizeof(L), outFILE);
    fwrite(data, L, sizeof(char), outFILE);
}

inline void fwrite(const std::string& data, FILE* outFILE)
{
    fwrite(data.c_str(), outFILE);
}

template<typename vector_t>
void vwrite(const vector_t& list, FILE* outFILE)
{
    uint64_t L = list.size();
    fwrite(&L, 1, sizeof(L), outFILE);
    for(auto& item : list)
    {
        fwrite(item, outFILE);
    }
}

template<typename map_t>
void mwrite(const map_t& item_map, FILE* outFILE)
{
    uint64_t L = item_map.size();
    fwrite(&L, 1, sizeof(L), outFILE);
    for(auto& pair : item_map)
    {
        fwrite(pair.first, outFILE);
        fwrite(pair.second, outFILE);
    }
}

inline void fread(uint8_t& data, FILE* inFILE)          { fread(&data, 1, sizeof(data), inFILE); }
inline void fread(uint16_t& data, FILE* inFILE)         { fread(&data, 1, sizeof(data), inFILE); }
inline void fread(uint32_t& data, FILE* inFILE)         { fread(&data, 1, sizeof(data), inFILE); }
inline void fread(uint64_t& data, FILE* inFILE)         { fread(&data, 1, sizeof(data), inFILE); }

inline void fread(int8_t& data, FILE* inFILE)           { fread(&data, 1, sizeof(data), inFILE); }
inline void fread(int16_t& data, FILE* inFILE)          { fread(&data, 1, sizeof(data), inFILE); }
inline void fread(int32_t& data, FILE* inFILE)          { fread(&data, 1, sizeof(data), inFILE); }
inline void fread(int64_t& data, FILE* inFILE)          { fread(&data, 1, sizeof(data), inFILE); }

inline void fread(float& data, FILE* inFILE)            { fread(&data, 1, sizeof(data), inFILE); }
inline void fread(double& data, FILE* inFILE)           { fread(&data, 1, sizeof(data), inFILE); }
inline void fread(long double& data, FILE* inFILE)      { fread(&data, 1, sizeof(data), inFILE); }

inline void fread(char& data, FILE* inFILE)             { fread(&data, 1, sizeof(data), inFILE); }
inline void fread(bool& data, FILE* inFILE)             { fread(&data, 1, sizeof(data), inFILE); }

inline void fread(char* data, FILE* inFILE)
{
    uint64_t L;
    fread(&L, 1, sizeof(L), inFILE);
    data = new char[L + 1];
    data[L] = '\0';
    fread(data, L, sizeof(char), inFILE);
}

inline void fread(std::string& data, FILE* inFILE)
{
    uint64_t L;
    fread(&L, 1, sizeof(L), inFILE);
    char* newstr = new char[L + 1];
    newstr[L] = '\0';
    fread(newstr, L, sizeof(char), inFILE);

    data = newstr;
    delete[] newstr;
}

template<typename vector_t>
void vread(vector_t& list, FILE* inFILE)
{
    uint64_t L;
    fread(&L, 1, sizeof(L), inFILE);

    if(L)
    {
        list.resize(L);

        for(size_t i = 0; i < L; ++i)
        {
            fread(list[i], inFILE);
        }
    }
}

template<typename map_t>
void mread(map_t& list, FILE* inFILE)
{
    uint64_t L;
    fread(&L, 1, sizeof(L), inFILE);

    if(L)
    {

        typename map_t::key_type first;
        typename map_t::mapped_type second;

        for(size_t i = 0; i < L; ++i)
        {
            fread(first, inFILE);
            fread(second, inFILE);
            list.emplace(first, second);
        }

    }
}

std::string getUserDataPath();

}
#endif // EZ_FILESYSTEM
