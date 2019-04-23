#pragma once

#ifndef DATASTREAM_HPP
#define DATASTREAM_HPP

#include <EZC/algorithm.hpp>
#include <EZC/toolkit/clustered_vector.hpp>
#include <EZC/toolkit/string.hpp>

#ifdef AIDA_MODULE_GPU
#include "AIDA/kernel.hpp"
#endif

#include <boost/filesystem.hpp>

#define DATA_READOUT_REFRESH 1.0f/24
#define STREAM_SCAN_BUFFER_SIZE 1000000
#define STREAM_SEARCH_BUFFER_SIZE 300000
#define STREAM_CACHE_SHORT_SEARCH_LIMIT 100

#define ORIENTATION_NONE                0_BIT
#define ORIENTATION_ROW                 1_BIT
#define ORIENTATION_COLUMN              2_BIT
#define ORIENTATION_UNKNOWN             3_BIT

namespace EZC
{

/**  =============================================================

// Typedefs

============================================================== **/

typedef std::vector<unsigned int>                   dimArray;
typedef std::vector<std::vector<unsigned int>>      dimMatrix;
typedef std::vector<unsigned int>                   uint_v;
typedef std::vector<std::vector<unsigned int>>      uint_m;

typedef std::vector<std::string>                    tagList;
typedef EZC::VectorPair<std::string>                pairedTags;
typedef std::vector<EZC::Vector2<unsigned int>>     pairedCoords;
typedef EZC::Vector2<float>                         Vector2f;
typedef EZC::Vector2<unsigned int>                  Vector2u;

/**  =============================================================

// Stream classes

============================================================== **/

class _2Dstream;

class _1Dstream{
protected:

    FILE* stream;
    std::string streamFile;
    unsigned int entrySize, byteSize;
    std::vector<uint16_t> dataSizes;
    std::vector<uint32_t> dataIndex;
    std::vector<std::reference_wrapper<const std::string>> dataRefs;

    char* readBuffer;
    bool bIsReference; // If stream was extracted from another stream, access by data references

    void update();
    void getEntrySize();

public:

    inline const unsigned int& size() const{ return entrySize; }
    inline char *const readbuf() const { return readBuffer; }

    inline bool empty() const{ return entrySize == 0; }

    inline void reset(){
        fclose(stream);
        dataIndex.erase(dataIndex.begin(), dataIndex.end());
        dataSizes.erase(dataSizes.begin(), dataSizes.end());
    }

    void get(const unsigned int& i) const;
    std::string getString(const unsigned int& i) const;
    float getFloat(const unsigned int& i) const;

    void import(std::vector<std::string>& storage) const;

    bool check(const std::string& target) const;
    bool find(uint_v& output, const std::string& target) const;
    unsigned int getIndex(const std::string& query) const;

    std::string operator[](const unsigned int& i) const;
    float operator()(const unsigned int& i) const;
    _1Dstream operator[](const std::vector<unsigned int>& index);
    _1Dstream substream(const std::vector<unsigned int>& index);
    _1Dstream& reduce(const std::vector<unsigned int>& index);

    void openStream(const std::string& streamFile);

    friend std::ostream& operator<<(std::ostream& output, _1Dstream other){
        for(size_t i = 0; i < other.entrySize; ++i){
            output << other.getString(i);
            if(i < other.entrySize - 1) output << '\t';
        }
        return output;
    }

    _1Dstream& operator=(const _1Dstream& other);

    _1Dstream(const _1Dstream& other);
    _1Dstream(const std::string& filename, const std::vector<uint32_t>& dataIndex, const std::vector<uint16_t>& dataSizes);
    _1Dstream(const _2Dstream& dataset, const unsigned int& index, const bool& row);
    _1Dstream(const _1Dstream& other, const std::vector<unsigned int>& index);


    ~_1Dstream(){
        if(readBuffer != nullptr) delete[] readBuffer;
        if(stream != nullptr) fclose(stream); }

};

struct coord_string : public std::string
{
    coord_string(const std::string str,
                 const Vector2u coords):
        std::string(str),
        coords(coords){ }

    coord_string(const std::string& str,
                 const unsigned int& x,
                 const unsigned int& y):
        std::string(str),
        coords(x, y){ }

    inline friend std::ostream& operator<<(std::ostream& output, const coord_string& input)
    {
        return output << input.c_str() << '[' << input.coords.x << ',' << input.coords.y << ']';
    }

    EZC::Vector2u coords;
};

class _2Dstream{
protected:

    FILE*                       stream;

    std::string                 streamFile;

    unsigned long long          streamSize;

    unsigned int                numRows;
    unsigned int                numColumns;

    char*                       readBuffer;     // Search buffer - pre-allocate for rapid search

    std::string                 delim;          // Delimiting characters (may be more than one)

    uint8_t                     cached_alignment;

    dimArray                    rowSizes;
    dimArray                    rowDataSizes;

    EZC::vMatrix<uint16_t>      indexSizes;
    EZC::vMatrix<uint32_t>      indexPos;

    bool verbose;
    bool active;                                // Prevent conflicts between separate processes using this stream
    bool imported;                              // Switch access modes if dataset has been loaded into RAM

    std::vector<coord_string> coord_index;
    EZC::tree_vector<char, coord_string> *       search_index;

public:

    void reset();
    inline void close(){ reset(); }

    void reload();
    void index(const int& max_levels = -1);   // Create a search index to speed up repeated find operations

    void update();

    inline bool empty() const{ return rowSizes.empty(); }

    std::vector<std::vector<std::string>> importData; // Import matrix - copy data to RAM for rapid-access tasks, and for writing

    inline char *const readbuf() const{ return readBuffer; }

    inline const bool& isActive() const{ return active; }
    inline const bool& isImported() const{ return imported; }

    inline const unsigned int& rowSize(const unsigned int& row) const{ return rowSizes[row]; }
    inline const unsigned int& rowDataSize(const unsigned int& row) const{ return rowDataSizes[row]; }

    size_t getCharCount(const std::string& chars) const;

    unsigned int colSize(const unsigned int& col) const;

    inline const std::vector<uint16_t>& getRowIndexSizes(const unsigned int& row) const{ return indexSizes[row]; }
    inline const std::vector<uint32_t>& getRowIndexPos(const unsigned int& row) const{ return indexPos[row]; }

    inline const uint16_t& size_of(const unsigned int& x, const unsigned int& y) const{ return indexSizes[y][x]; }
    inline const uint32_t& position(const unsigned int& x, const unsigned int& y) const{ return indexPos[y][x]; }

    inline const unsigned int& nrow() const{ return numRows; }
    inline const unsigned int& ncol() const{ return numColumns; }

    inline void setVerbose(bool verboseStatus){ verbose = verboseStatus; }

    const unsigned long long size() const{ return streamSize; }

    _1Dstream getRow(const unsigned int& row) const;
    _1Dstream getCol(const unsigned int& col) const;

    void importRow(std::vector<std::string>& storage, const unsigned int& row) const;
    void importCol(std::vector<std::string>& storage, const unsigned int& col) const;
    void import();  // Load stream data into RAM and perform further operations on it
    void release(); // Deallocate imported data and return to pure stream status

    _1Dstream operator[](const unsigned int& y) const;
    _1Dstream operator[](const std::string& x) const;

    _2Dstream operator[](const std::vector<unsigned int>& rows) const;
    _2Dstream operator[](const EZC::VectorPair<unsigned int>& bounds) const;

    bool find(EZC::VectorPairU& output, const std::string& target, const float& size_threshold = 0.1f,
              const float& match_threshold = 1.0f);
    Vector2u getCoords(const std::string& target, const float& threshold = 0.1f);
    bool check(const std::string& target,
               const float& threshold = 0.1f,
               const float& match_threshold = 1.0f) const;
    std::string findMatch(const std::string& target, const float& threshold = 0.1f);
    unsigned int getCount(const std::string& query, bool exact = true, const float& threshold = 0.1f);

    void get(const unsigned int& x, const unsigned int& y) const; // Returns copied data - must delete pointer after every call
    std::string getString(const unsigned int& x, const unsigned int& y) const;
    template<typename T> std::string getString(const EZC::Vector2<T>& coords) const{
        return getString((unsigned int)coords.x, (unsigned int)coords.y);
    }
    float getFloat(const unsigned int& x, const unsigned int& y) const;

    bool openStream(const boost::filesystem::path& filePath, const bool& update = true,
                    const std::string& delim = ""){
            return openStream(filePath.string(), update); }
    bool openStream(const std::string& filename, const bool& update = true,
                    const std::string& delim = "");
    inline const std::string& getSourceFile() const{ return streamFile; }

    friend std::ostream& operator<<(std::ostream& output, const _2Dstream& other){
        for(size_t i = 0; i < other.nrow(); ++i){
            for(size_t j = 0; j < other.ncol(); ++j){
                if(j >= other.rowSizes[i]) break;
                output << other.getString(j, i);
                if((other.rowSizes[i] > 1) && (j < other.rowSizes[i] - 1)) output << '\t';
            }
            output << '\n';
        }
        return output;
    }

    _2Dstream(const _2Dstream& other); // Copy constructor - duplicates info but opens entirely new stream
    _2Dstream& operator=(const _2Dstream& other);

    explicit _2Dstream();
    explicit _2Dstream(const std::string& filename, const bool& verbose = true,
                       const std::string& delim = "");
    explicit _2Dstream(const bool& verbose, const std::string& delim = "\t");
    explicit _2Dstream(std::string filename, EZC::vMatrix<uint32_t>& indexPos,
                       EZC::vMatrix<uint16_t>& indexSizes,
                    dimArray& rowSizes, dimArray& rowDataSizes,
                    const std::string& delim);

    ~_2Dstream(){
        if(readBuffer) delete[] readBuffer;
        if(stream) fclose(stream);
        if(search_index) delete(search_index);
    }
};

inline size_t nrow(const _2Dstream& stream){
    return stream.nrow();
}

inline size_t ncol(const _2Dstream& stream){
    return stream.ncol();
}

inline size_t rowSize(const _2Dstream& stream, const unsigned int& index){
    return stream.rowSize(index);
}

typedef _2Dstream matrixStream;
typedef _2Dstream m_stream;
typedef _1Dstream vectorStream;
typedef _1Dstream v_stream;

// Datastream functions

inline unsigned int numDelim(char* c, const unsigned int L){
    unsigned int delimCount = 0;
    char* newC = c;
    for(size_t i = 0; i < L; ++i, ++newC){
        if((*newC == '\t') || (*newC == '\n')) ++delimCount;
    }
    return delimCount;
}

bool isNumeric(const _1Dstream& stream, const unsigned int index);

std::string getMatchingString(const std::string& query, _2Dstream& stream);
Vector2u getBestStreamMatch(EZC::VectorPairU& coords,
                            _2Dstream& stream,
                            const std::string& target,
                            const unsigned char& params = CMP_STR_DEFAULT | CMP_STR_SW,
                            const float& threshold = 0.5f);
void getBestStreamMatchCoords(EZC::VectorPairU& coords, _2Dstream& stream, const std::string& target);

bool filterStreamMatch(std::vector<std::string>& prompt, _2Dstream& stream, bool getMatches = true);
bool checkAnyStreamMatch(std::vector<std::string>& query, _2Dstream& stream, bool getMatches = false);

// Get orientation of search results relative to data center
uint8_t getSearchOrientation(const std::string& query, _2Dstream& dataSource);

// Assess alignment of coordinates given back by a stream search query
uint8_t assessCoordAlignment(const EZC::VectorPairU& coords, _2Dstream& dataSource); // Relative to global boundaries
uint8_t assessCoordAlignment(const EZC::VectorPairU& inCoords, const EZC::VectorPairU& outCoords); // Relative to other coordinates
uint8_t assessCoordAlignment(const EZC::VectorPairU& coords); // Relative to self

// Data visualization

//class _2DStreamDataViewer: public CVViewPanel{
//protected:
//public:
//
//    _2DStreamDataViewer()
//};

}

#endif // DATASTREAM_HPP
