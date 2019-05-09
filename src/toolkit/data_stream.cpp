#include "hyper/toolkit/data_stream.hpp"
#include "hyper/toolkit/string.hpp"
#include "hyper/toolkit/string_search.hpp"

using namespace std;

namespace hyperC
{

_1Dstream::_1Dstream(const _2Dstream& dataset, const unsigned int& index, const bool& row):
    stream(nullptr),
    readBuffer(nullptr),
    bIsReference(dataset.isImported())
{
    if(bIsReference)
    {
        if(row)
        {
            dataRefs.reserve(dataset.importData[index].size());
            for(auto& x : dataset.importData[index])
            {
                dataRefs.emplace_back(x);
            }
        }
        else
        {
            dataRefs.reserve(dataset.nrow());
            for(auto& y : dataset.importData)
            {
                if(index < y.size())
                {
                    dataRefs.emplace_back(y[index]);
                }
            }
        }
        this->entrySize = dataRefs.size();
    }
    else
    {

        this->streamFile = dataset.getSourceFile();
        if(row)
        {
            this->entrySize = dataset.rowSize(index);
            this->dataSizes = dataset.getRowIndexSizes(index);
            this->dataIndex = dataset.getRowIndexPos(index);
        }
        else
        {

            this->entrySize = dataset.nrow();
            this->dataSizes.reserve(dataset.nrow());
            this->dataIndex.reserve(dataset.nrow());

            for(size_t i = 0; i < dataset.nrow(); ++i)
            {
                if(index < dataset.rowSize(i))
                {
                    dataSizes.push_back(dataset.size_of(index, i));
                    dataIndex.push_back(dataset.position(index, i));
                }
                else
                {

                    dataSizes.push_back(0);
                    dataIndex.push_back(0);

                }
            }
        }

        readBuffer = new char[max(dataSizes)*10];
        readBuffer[max(dataSizes)] = '\0';
        openStream(this->streamFile);
    }
}

_1Dstream::_1Dstream(const _1Dstream& stream, const vector<unsigned int>& index):
    stream(nullptr),
    streamFile(stream.streamFile),
    entrySize(0),
    bIsReference(stream.bIsReference)
{
    if(!bIsReference)
    {
        size_t L = index.size();
        dataIndex.reserve(L);
        dataSizes.reserve(L);

        for(size_t i = 0; i < L; ++i)
        {
            if(index[i] >= stream.size()) continue;
            dataIndex.push_back(stream.dataIndex[index[i]]);
            dataSizes.push_back(stream.dataSizes[index[i]]);
            ++entrySize;
        }

        readBuffer = new char[max(dataSizes)+1];
        readBuffer[max(dataSizes)] = '\0';
        openStream(streamFile);
    }
    else
    {
        size_t L = index.size();
        dataRefs.reserve(L);

        for(size_t i = 0; i < L; ++i)
        {
            if(index[i] >= stream.size()) continue;
            dataRefs.push_back(stream.dataRefs[index[i]]);
            ++entrySize;
        }
    }
}

_1Dstream::_1Dstream(const _1Dstream& other):
    stream(nullptr),
    streamFile(other.streamFile),
    entrySize(other.entrySize),
    dataIndex(other.dataIndex),
    dataSizes(other.dataSizes),
    dataRefs(other.dataRefs),
    readBuffer(new char[max(dataSizes)+1]),
    bIsReference(other.bIsReference)
{
    if(!bIsReference)
    {
        readBuffer[max(dataSizes)] = '\0';
        openStream(streamFile);
    }
}
_1Dstream::_1Dstream(const string& filename, const vector<uint32_t>& dataIndex, const vector<uint16_t>& dataSizes):
    stream(nullptr),
    streamFile(filename),
    entrySize(dataIndex.size()),
    dataIndex(dataIndex),
    dataSizes(dataSizes),
    readBuffer(new char[max(dataSizes)+1]),
    bIsReference(false)
{
    readBuffer[max(dataSizes)] = '\0';
    openStream(filename);
}

_1Dstream& _1Dstream::operator=(const _1Dstream& other)
{
    entrySize = other.entrySize;
    bIsReference = other.bIsReference;
    dataRefs = other.dataRefs;

    if(!bIsReference)
    {
        streamFile = other.streamFile;
        dataIndex = other.dataIndex;
        dataSizes = other.dataSizes;

        delete[] readBuffer;
        readBuffer = new char[max(dataSizes)+1];
        readBuffer[max(dataSizes)] = '\0';
        memcpy(readBuffer, other.readBuffer, max(dataSizes));

        openStream(streamFile);
    }
    return *this;
}


void _1Dstream::openStream(const string& streamFile)
{
    if(stream != nullptr)
    {
        reset();
    }

    if(access(streamFile.c_str(), F_OK | W_OK))
    {
        cout << ">> ERROR: could not open stream to " << streamFile << '\n';
        return;
    }

    this->streamFile = streamFile;
    stream = fopen(streamFile.c_str(), "rwb");
}

inline void _1Dstream::get(const unsigned int& i) const
{
    if(dataSizes[i] < 1)
    {
        readBuffer[0] = '\0';
        return;
    }
    readBuffer[dataSizes[i]] = '\0';
    fseek(stream, dataIndex[i], SEEK_SET);
    fread(readBuffer, 1, dataSizes[i], stream);
}

string _1Dstream::getString(const unsigned int& i) const
{
    if(!bIsReference)
    {
        get(i);
        string s(readBuffer);
        processString(s);
        return(s);
    }
    else
    {
        string s = dataRefs[i];
        processString(s);
        return(s);
    }
}

float _1Dstream::getFloat(const unsigned int& i) const
{
    if(!bIsReference)
    {
        get(i);
        if(!isNumeric(readBuffer))
        {
            return NAN;
        }
        else
        {
            return strtod(readBuffer, nullptr);
        }
    }
    else
    {
        if(!isNumeric(dataRefs[i])) return NAN;
        else return stof(dataRefs[i]);
    }
}

void _1Dstream::import(StringVector& storage) const
{
    storage.reserve(storage.size() + entrySize);
    for(size_t i = 0; i < entrySize; ++i)
    {
        storage.push_back(getString(i));
    }
}

string _1Dstream::operator[](const unsigned int& i) const
{
    if(!bIsReference)
    {
        get(i);
        string s(readBuffer);
        processString(s);
        return s;
    }
    else
    {
        string s = dataRefs[i];
        processString(s);
        return s;
    }
}

float _1Dstream::operator()(const unsigned int& i) const
{
    if(!bIsReference)
    {
        get(i);
        if(!isNumeric(readBuffer))
        {
            return NAN;
        }
        else
        {
            return strtod(readBuffer, nullptr);
        }
    }
    else
    {
        if(!isNumeric(dataRefs[i])) return NAN;
        else return stof(dataRefs[i]);
    }
}

_1Dstream _1Dstream::substream(const vector<unsigned int>& index)
{
    return _1Dstream(*this, index);
}

_1Dstream& _1Dstream::reduce(const vector<unsigned int>& index)
{
    if(!bIsReference)
    {
        unsigned int cIndex = 0;
        for(size_t i = 0; i < entrySize; ++cIndex)
        {
            if(!anyEqual(cIndex, index))
            {
                dataIndex.erase(dataIndex.begin() + i);
                dataSizes.erase(dataSizes.begin() + i);
                --entrySize;
            }
            else ++i;
        }
    }
    else
    {
        unsigned int cIndex = 0;
        for(size_t i = 0; i < entrySize; ++cIndex)
        {
            if(!anyEqual(cIndex, index))
            {
                dataRefs.erase(dataRefs.begin() + i);
                --entrySize;
            }
            else ++i;
        }
    }
    return *this;
}

_1Dstream _1Dstream::operator[](const vector<unsigned int>& index)
{
    return substream(index);
}

bool _1Dstream::check(const string& target) const
{
    if(!bIsReference)
    {
        for(size_t i = 0; i < entrySize; ++i)
        {
            if(cmpString(getString(i), target)) return true;
        }
    }
    else
    {
        for(auto& data : dataRefs)
        {
            if(cmpString(data, target)) return true;
        }
    }
    return false;
}

bool _1Dstream::find(uint_v& output, const string& target) const
{
    size_t L = output.size();
    if(!bIsReference)
    {
        for(size_t i = 0; i < entrySize; ++i)
        {
            if(cmpString(getString(i), target)) output.push_back(i);
        }
    }
    else
    {
        unsigned int i = 0;
        for(auto& data : dataRefs)
        {
            if(cmpString(data, target)) output.push_back(i);
            ++i;
        }
    }

    return output.size() > L;
}

unsigned int _1Dstream::getIndex(const string& target) const
{
    uint_v coords;
    find(coords, target);

    size_t outSIZE = coords.size();
    if(outSIZE < 1) return UINT_MAX;
    if(outSIZE < 2) return coords.front();
    size_t L = target.size();

    unsigned int minSizeDiff = L, outIndex = 0;

    if(outSIZE < 2) return coords.front();

    for(size_t i = 0; i < outSIZE; ++i)
    {
        get(coords[i]);
        unsigned int othercmp = charMatchNum(target, readBuffer);
        unsigned int sizecmp = othercmp > L ? othercmp - L : L - othercmp;
        if(sizecmp < minSizeDiff)
        {
            minSizeDiff = sizecmp;
            outIndex = i;
        }
    }

    return outIndex;
}

_2Dstream::_2Dstream():
    stream(nullptr),
    streamSize(0),
    numRows(0),
    numColumns(0),
    readBuffer(new char[STREAM_SCAN_BUFFER_SIZE]),
    delim("\t"),
    cached_alignment(ORIENTATION_UNKNOWN),
    verbose(false),
    active(false),
    imported(false),
    search_index(nullptr) { }

_2Dstream::_2Dstream(const bool& verbose,
                     const string& delim):
    stream(nullptr),
    streamSize(0),
    numRows(0),
    numColumns(0),
    readBuffer(new char[STREAM_SCAN_BUFFER_SIZE]),
    delim(delim),
    cached_alignment(ORIENTATION_UNKNOWN),
    verbose(verbose),
    active(false),
    imported(false),
    search_index(nullptr) { }

_2Dstream::_2Dstream(const string& filename,
                     const bool& verbose,
                     const string& delim):
    stream(nullptr),
    streamSize(0),
    numRows(0),
    numColumns(0),
    readBuffer(new char[STREAM_SCAN_BUFFER_SIZE]),
    delim(delim),
    cached_alignment(ORIENTATION_UNKNOWN),
    verbose(verbose),
    active(false),
    imported(false),
    search_index(nullptr)
{
    openStream(filename, true, delim);
}

_2Dstream::_2Dstream(const _2Dstream& other):
    stream(fopen(other.streamFile.c_str(), "rb")),
    streamFile(other.streamFile),
    streamSize(other.streamSize),
    numRows(other.numRows),
    numColumns(other.numColumns),
    readBuffer(new char[STREAM_SCAN_BUFFER_SIZE]),
    delim(other.delim),
    rowSizes(other.rowSizes),
    rowDataSizes(other.rowDataSizes),
    indexPos(other.indexPos),
    indexSizes(other.indexSizes),
    importData(other.importData),
    verbose(other.verbose),
    active(other.active),
    imported(other.imported)
{

    if(other.search_index)
    {
        search_index = new hyperC::tree_vector<char, coord_string>(*other.search_index);
    }
    else
    {
        search_index = nullptr;
    }

}

_2Dstream::_2Dstream(string filename, vMatrix<uint32_t>& indexPos, vMatrix<uint16_t>& indexSizes,
                     dimArray& rowSizes, dimArray& rowDataSizes,
                     const string& delim):
    stream(nullptr),
    streamFile(filename),
    numRows(rowSizes.size()),
    numColumns(maxSize(indexPos)),
    readBuffer(new char[STREAM_SCAN_BUFFER_SIZE]),
    delim(delim),
    cached_alignment(ORIENTATION_UNKNOWN),
    rowSizes(rowSizes),
    rowDataSizes(rowDataSizes),
    indexPos(indexPos),
    indexSizes(indexSizes),
    verbose(false),
    active(false),
    imported(false),
    search_index(nullptr)
{
    openStream(filename, false, delim);
}

_2Dstream& _2Dstream::operator=(const _2Dstream& other)
{
    streamFile = other.streamFile;
    readBuffer = new char[STREAM_SCAN_BUFFER_SIZE];
    streamSize = other.streamSize;
    numRows = other.numRows;
    numColumns = other.numColumns;
    rowSizes = other.rowSizes;
    rowDataSizes = other.rowDataSizes;
    indexPos = other.indexPos;
    indexSizes = other.indexSizes;
    importData = other.importData;
    verbose = other.verbose;
    active = other.active;
    imported = other.imported;
    delim = other.delim;

    if(other.search_index)
    {
        search_index = new hyperC::tree_vector<char, coord_string>(*other.search_index);
    }
    else
    {
        search_index = nullptr;
    }

    openStream(streamFile, false, delim);
    return *this;
}

void _2Dstream::index(const int& max_levels)
{

    if(search_index)
    {
        if(verbose)
        {
            cout << "Stream is already indexed\n";
        }
        return;
    }

    if(stream)
    {
//        if(verbose)
//        {
            cout << "Indexing...";
//        }
        if(search_index)
        {
            delete(search_index);
        }
        if(!coord_index.empty())
        {
            coord_index.clear();
        }

        for(size_t y = 0, x; y < nrow(); ++y)
        {
            for(x = 0; (x < rowSize(y)) && (x < ncol()); ++x)
            {
                string str = getString(x, y);
                if(!isNumeric(str))
                {
                    to_lowercase(str);
                    coord_index.emplace_back(str, x, y);
                }
            }
        }

        if(!coord_index.empty())
        {

            reference_vector<coord_string> refs(coord_index);
            search_index = new tree_vector<char, coord_string>(refs, max_levels);

            if(verbose)
            {
                cout << " success\n";
            }
        }
        else if(verbose)
        {
            cout << " failed\n";
        }

    }

}

void _2Dstream::reset()
{
    fclose(stream);
    numRows = 0;
    numColumns = 0;
    streamSize = 0;
    streamFile.clear();
    rowDataSizes.clear();
    rowSizes.clear();
    indexPos.clear();
    indexSizes.clear();
    cached_alignment = ORIENTATION_UNKNOWN;
    importData.clear();
    imported = false;

    delete(search_index);
    search_index = nullptr;

}

void _2Dstream::reload()
{
    openStream(getSourceFile());
}

bool _2Dstream::openStream(const string& filename, const bool& update,
                           const string& delim)
{

    if(stream)
    {
        reset();
    }

    if(access(filename.c_str(), F_OK))
    {
        if(verbose) cout << ">> ERROR: could not access file at " << filename << "\n";
        return false;
    }

    if(verbose) cout << ">> Opening data stream from " << filename << '\n';

    stream = fopen(filename.c_str(), "rb");

    if(!stream)
    {
        cout << "Error opening stream to " << filename << '\n';
        return false;
    }

    fseek(stream, 0, SEEK_END);
    streamSize = ftell(stream);
    fseek(stream, 0, SEEK_SET);

    streamFile = filename;

    // Determine the appropriate delimiter

    if(delim.empty())
    {
        if(filename.find(".csv") < filename.size())
        {
            if(getCharCount(";") > getCharCount(","))
            {
                this->delim = ";";
            }
            else
            {
                this->delim = ",";
            }
        }
        else if(filename.find(".tsv") < filename.size())
        {
            this->delim = "\t";
        }
        else
        {
            map<string, size_t> delim_counts;
            delim_counts["\t"] = getCharCount("\t");
            delim_counts[","] = getCharCount(",");
            delim_counts[";"] = getCharCount(";");

            size_t maxCount = 0,
                   maxCountIndex = 0;

            int i = 0;

            for(auto& pair : delim_counts)
            {
                if(pair.second > maxCount)
                {
                    maxCount = pair.second;
                    maxCountIndex = i;
                }

                ++i;
            }

            i = 0;
            for(auto& pair : delim_counts)
            {
                if(i == maxCountIndex)
                {
                    this->delim = pair.first;
                    break;
                }
                ++i;
            }
        }
    }
    else
    {
        this->delim = delim;
    }

    if(update) this->update();

    if(verbose) cout << "\r>> Finished\t\t\t\n";

    return true;
}

void _2Dstream::update()
{

    if(imported)
    {
        numRows = importData.size();
        numColumns = maxSize(importData);
        if(verbose) cout << ">> Updating imported stream... ";
        for(size_t i = 0; i < numColumns; ++i)
        {
            if(i >= rowSizes.size()) rowSizes.push_back(importData[i].size());
            else rowSizes[i] = importData[i].size();
        }
        if(verbose) cout << "done\n";
        return;
    }

    if(nrow() > 0)
    {
        indexPos.clear();
        indexSizes.clear();
        rowSizes.clear();
        rowDataSizes.clear();
    }

    numRows = 0;

    if(!streamSize)
    {
        cout << "Warning: loaded stream is empty\n";
        return;
    }

    unsigned int lastRowSize = 0;
    unsigned long long lastIndexPos = 0ULL, lastRowIndex = 0ULL, i = 0LL;
    thread* readout;

    if(verbose)
    {
        readout = new thread([&]()
        {
            cout << ">> Updating: 0%";
            while(i < streamSize)
            {
                cout << "\r>> Updating: " << i << "b / " <<
                     streamSize << "b (" << (unsigned int)(float(i)/streamSize*100) << "%)";
                this_thread::sleep_for(chrono::duration<float>(DATA_READOUT_REFRESH));
            }
        });
    }

    fseek(stream, 0, SEEK_SET);

    char* c = readBuffer;
    unsigned long long bufferPos = 0ULL;
    bool bNewLine = true;
    bool bQuote = false;

    for(; i < streamSize; ++i, ++c)
    {

        if(bufferPos <= i)
        {
            unsigned int bufferSize = STREAM_SCAN_BUFFER_SIZE > streamSize - i ? streamSize - i : STREAM_SCAN_BUFFER_SIZE;
            bufferPos += bufferSize;
            fread(readBuffer, 1, bufferSize, stream);

            c = readBuffer;
        }

        if((*c == '\n') || (*c == '\r'))
        {

            ++rowSizes.back();

            indexSizes.back().push_back(i-lastIndexPos);
            indexPos.back().push_back(lastIndexPos);
            rowDataSizes.back() = i - lastRowIndex;

            if(i < streamSize - 1)
            {
                if((*c + 1) && ((*(c + 1) == '\r')))
                {
                    lastIndexPos = i+2;
                    lastRowIndex = i+2;
                    ++i;
                    ++c;
                }
                if((*c + 1) && ((*(c + 1) == '\n')))
                {
                    lastIndexPos = i+2;
                    lastRowIndex = i+2;
                    ++i;
                    ++c;
                }
                else
                {
                    lastIndexPos = i+1;
                    lastRowIndex = i+1;
                }
            }

            bNewLine = true;

        }
        else if(*c == '\"')
        {
            bQuote = !bQuote;
        }
        else if(isCharType(*c, delim) && !bQuote)
        {

            indexPos.back().push_back(lastIndexPos);
            indexSizes.back().push_back(i-lastIndexPos);

            ++rowSizes.back();

            if(*(c + 1) && (*(c + 1) == '\r'))
            {
                lastIndexPos = i+2;
                ++i;
                ++c;
            }
            else
            {
                lastIndexPos = i+1;
            }
        }
        else if((i < streamSize - 1) && bNewLine)
        {
            ++numRows;

            indexSizes.emplace_back();
            indexPos.emplace_back();
            rowSizes.emplace_back(0);
            rowDataSizes.emplace_back(0);

            bNewLine = false;
        }

    }
    if(i > lastIndexPos)
    {
        indexSizes.back().push_back(i-lastIndexPos);
        indexPos.back().push_back(lastIndexPos);
        rowDataSizes.push_back(i-lastRowIndex);
    }
    numColumns = max(rowSizes);

    if(verbose)
    {
        if(readout->joinable()) readout->join();
        delete(readout);
        cout << "\r>> Updating: " << streamSize << "b / " <<
             streamSize << "b (100%)\n";
    }
}

size_t _2Dstream::getCharCount(const string& chars) const
{

    fpos_t streamPos = ftell(stream);

    fseek(stream, 0, SEEK_SET);
    char* c = readBuffer;
    unsigned long long bufferPos = 0;
    unsigned int bufferSize = 0;
    size_t output = 0;

    for(size_t coord = 0; coord < streamSize; ++coord)
    {
        if(bufferPos <= coord)
        {
            bufferSize = STREAM_SCAN_BUFFER_SIZE > streamSize - coord ? streamSize - coord : STREAM_SCAN_BUFFER_SIZE;
            bufferPos += bufferSize;
            fread(readBuffer, 1, bufferSize, stream);
            c = readBuffer;
        }

        if(isCharType(*c, chars))
        {
            ++output;
        }

        ++c;
    }

    fsetpos(stream, &streamPos);

    return output;

}

unsigned int _2Dstream::colSize(const unsigned int& col) const
{
    unsigned int output = 0;
    for(auto& row : rowSizes)
    {
        if(col < row)
        {
            ++output;
        }
    }

    return output;
}

inline void _2Dstream::get(const unsigned int& x, const unsigned int& y) const
{
    if(!indexSizes[y][x])
    {
        readBuffer[0] = '\0';
        return;
    }

    readBuffer[indexSizes[y][x]] = '\0';
    fseek(stream, indexPos[y][x], SEEK_SET);
    fread(readBuffer, 1, indexSizes[y][x], stream);

}

_1Dstream _2Dstream::getCol(const unsigned int& col) const
{
    return _1Dstream(*this, col, false);
}

void _2Dstream::importCol(StringVector& storage, const unsigned int& col) const
{
    storage.reserve(storage.size() + numRows);
    if(!imported)
    {
        for(size_t i = 0; i < numRows; ++i)
        {
            if(col >= rowSizes[i]) continue;
            storage.push_back(getString(col, i));
        }
    }
    else
    {
        for(size_t i = 0; i < nrow(); ++i)
        {
            if(col >= rowSizes[i]) continue;
            storage.push_back(importData[i][col]);
        }
    }
}

void _2Dstream::importRow(StringVector& storage, const unsigned int& row) const
{
    if(!imported)
    {
        storage.reserve(storage.size() + rowSizes[row]);
        size_t L = rowSizes[row];
        for(size_t i = 0; i < L; ++i)
        {
            storage.push_back(getString(i, row));
        }
    }
    else
    {
        storage = importData[row];
    }
}

void _2Dstream::import()
{
    if(imported) update();

    importData.clear();
    importData.resize(numRows);
    for(size_t i = 0; i < numRows; ++i)
    {
        importRow(importData[i], i);
        if(verbose) cout << "\r>> Importing stream: " << i*100/numRows << "%";
    }
    if(verbose) cout << ">> Importing stream: 100%\n";
    imported = true;

    fclose(stream);
    stream = nullptr;
    delete[] readBuffer;
    readBuffer = nullptr;
    indexPos.clear();
    indexSizes.clear();
}

void _2Dstream::release()
{
    if(verbose) cout << ">> Releasing stream data ... ";
    imported = false;
    importData.clear();

    readBuffer = new char[STREAM_SCAN_BUFFER_SIZE];
    openStream(streamFile);
    update();
    if(verbose) cout << "complete\n";
}

string _2Dstream::getString(const unsigned int& x, const unsigned int& y) const
{
    if(!imported)
    {
        get(x, y);
        string s(readBuffer);
        processString(s);
        return s;
    }
    else
    {
        string s = importData[y][x];
        processString(s);
        return s;
    }
}

float _2Dstream::getFloat(const unsigned int& x, const unsigned int& y) const
{
    if(!imported)
    {
        get(x, y);
        if(!isNumeric(readBuffer))
        {
            return NAN;
        }
        else
        {
            return strtod(readBuffer, nullptr);
        }
    }
    else
    {
        if(isNumeric(importData[y][x])) return stof(importData[y][x]);
        else return NAN;
    }
}

_1Dstream _2Dstream::getRow(const unsigned int& row) const
{
    return _1Dstream(*this, row, true);
}

_1Dstream _2Dstream::operator[](const unsigned int& row) const
{
    return _1Dstream(*this, row, true);
}

_2Dstream _2Dstream::operator[](const vector<unsigned int>& rows) const
{
    size_t L = rows.size();

    vMatrix<uint32_t> newIndexPos;
    vMatrix<uint16_t> newIndexSizes;
    dimArray newRowSizes;
    dimArray newRowDataSizes;

    newIndexPos.reserve(L);
    newIndexSizes.reserve(L);
    newRowSizes.reserve(L);
    newRowDataSizes.reserve(L);

    for(size_t i = 0; i < L; ++i)
    {
        newIndexPos.emplace_back(indexPos[rows[i]]);
        newIndexSizes.emplace_back(indexSizes[rows[i]]);
        newRowSizes.push_back(newIndexSizes.back().size());
        newRowDataSizes.push_back(sum(newIndexSizes.back()));
    }

    return _2Dstream(streamFile, newIndexPos, newIndexSizes, newRowSizes, newRowDataSizes, "");
}

_2Dstream _2Dstream::operator[](const VectorPairU& bounds) const
{

    size_t L = bounds.y.size();

    vMatrix<uint32_t> newIndexPos;
    vMatrix<uint16_t> newIndexSizes;
    dimArray newRowSizes;
    dimArray newRowDataSizes;

    newIndexPos.reserve(L);
    newIndexSizes.reserve(L);
    newRowSizes.reserve(L);
    newRowDataSizes.reserve(L);

    for(size_t i = 0; i < L; ++i)
    {
        newIndexPos.emplace_back(at_index(indexPos[bounds.y[i]], bounds.x));
        newIndexSizes.emplace_back(at_index(indexSizes[bounds.y[i]], bounds.x));
        newRowSizes.push_back(newIndexSizes.back().size());
        newRowDataSizes.push_back(sum(newIndexSizes.back()));
    }

    return _2Dstream(streamFile, newIndexPos, newIndexSizes, newRowSizes, newRowDataSizes, "");
}

bool _2Dstream::check(const string& target, const float& size_threshold,
                      const float& match_threshold) const
{

    // Use the pre-assembled search index if available

    if(search_index)
    {
        try
        {

            string case_target = target;
            to_lowercase(case_target);

            return search_index->search(case_target);

        }
        catch(...)
        {
            return false;
        }
    }

    // Case-insensitive Boyer-Moore search

    if(!imported)
    {

        if(match_threshold == 1.0f)
        {

            size_t L = target.size();
            if(L > streamSize) return false;

            vector<size_t> char_table;
            vector<size_t> match_table;

            boyer_moore_char_table(target, char_table, true);
            boyer_moore_match_table(target, match_table, true);

            fseek(stream, 0, SEEK_SET);

            unsigned long long matchCoord = 0;
            unsigned long long bufferPos = 0;
            unsigned long long matchCheck;

            char* match_c = readBuffer;

            unsigned int bufferSize = 0;
            unsigned int strSize;
            unsigned int maxStrSize = L/size_threshold;

            do
            {

                // 2048b overlap to account for boundary matches
                bufferSize = STREAM_SCAN_BUFFER_SIZE > streamSize - bufferPos ? streamSize - bufferPos : STREAM_SCAN_BUFFER_SIZE - 2048;

                fread(readBuffer, 1, bufferSize, stream);

                matchCheck = boyer_moore_search(target.c_str(), match_c, char_table, match_table, true);

                if(matchCheck != UINT_MAX) // Match found in the stream segment
                {

                    match_c = readBuffer + matchCheck; // Roll forward to check for redundant matches

                    if(bufferPos + matchCheck > matchCoord) // Account for repetitions in the overlap region
                    {

                        matchCoord = bufferPos + matchCheck;

                        // Check the size against requirements

                        strSize = getStrSize(match_c);

                        if(strSize < maxStrSize)
                        {

                            return true;

                        }

                    }


                }

                bufferPos += bufferSize;

            }
            while(bufferPos + bufferSize < streamSize);

        }
        else
        {

            // Brute force search for partial string matches

            size_t L = target.size();

            unsigned long long coord = 0;
            unsigned long long bufferPos = 0;

            unsigned int matchCoord = 0;
            unsigned int initCoord = 0;
            unsigned int sizeFactor = L/size_threshold;
            unsigned int matchFactor = L*match_threshold;
            unsigned int bufferSize = 0;

            char* c = readBuffer;

            // Initialize the trim

            while(matchCoord < L)
            {
                if(isSpecial(target[matchCoord])) ++matchCoord;
                else
                {
                    initCoord = matchCoord;
                    matchCoord = L-1;
                    break;
                }
            }

            while(matchCoord > initCoord + 1)
            {
                if(isSpecial(target[matchCoord]))
                {
                    --L;
                    --matchCoord;
                }
                else
                {
                    break;
                }
            }

            L-= initCoord;
            if(L < 1) L = target.size();
            if(initCoord >= L-1) initCoord = 0;

            if(L > streamSize) return false;
            size_t S = streamSize - L + 1;

            matchCoord = initCoord;

            fseek(stream, 0, SEEK_SET);

            // Perform the search

            for(; coord < S; ++coord)
            {
                if(bufferPos <= coord)
                {
                    bufferSize = STREAM_SCAN_BUFFER_SIZE > streamSize - coord ? streamSize - coord : STREAM_SCAN_BUFFER_SIZE;
                    bufferPos += bufferSize;
                    fread(readBuffer, 1, bufferSize, stream);
                    c = readBuffer;
                }

                if(*c == target[matchCoord])
                {
                    ++matchCoord;
                }
                else if(isLowerCase(*c))
                {
                    if((*c - 32) == target[matchCoord])
                    {
                        ++matchCoord;
                    }
                }
                else if(isUpperCase(*c))
                {
                    if((*c + 32) == target[matchCoord])
                    {
                        ++matchCoord;
                    }
                }
                else if(matchCoord >= matchFactor)
                {
                    if(getStrSize(c) <= sizeFactor)
                    {
                        return true;
                    }
                    matchCoord = initCoord;
                }
                else matchCoord = initCoord;
                ++c;
            }
        }

    }
    else
    {
        size_t sizeFactor = target.size()/size_threshold;
        for(const auto& y : importData)
        {
            for(const auto& x : y)
            {
                if((sizeFactor > x.size()) && cmpString(target, x)) return true;
            }
        }
    }

    return false;
}

bool _2Dstream::find(VectorPairU& output, const string& target, const float& size_threshold,
                     const float& match_threshold)
{

    if(target.empty())
    {
        return false;
    }

    // Use the pre-assembled search index if available

    if(search_index)
    {
        try
        {

            string case_target = target;
            to_lowercase(case_target);

            vector<coord_string> search_results;
            search_index->get_equal(case_target, search_results);

            if(!search_results.empty())
            {
                for(auto& result : search_results)
                {
                    output.emplace_back(result.coords);
                }

                return true;
            }

            return false;

        }
        catch(...)
        {
            return false;
        }
    }

    unsigned int initSIZE = output.size();

    // If all else fails, perform a linear string search

    if(!imported)
    {

        if(match_threshold == 1.0f)
        {

            // Case-insensitive Boyer-Moore search for exact length matches

            size_t L = target.size();
            if(L > streamSize) return false;

            vector<size_t> char_table;
            vector<size_t> match_table;

            boyer_moore_char_table(target, char_table, true);
            boyer_moore_match_table(target, match_table, true);

            fseek(stream, 0, SEEK_SET);

            unsigned long long matchCoord = 0;
            unsigned long long bufferPos = 0;
            unsigned long long matchCheck;

            char* match_c = readBuffer;

            unsigned int bufferSize = 0;
            unsigned int strSize;
            unsigned int maxStrSize = L/size_threshold;

            do
            {

                // 2048b overlap to account for boundary matches
                bufferSize = STREAM_SCAN_BUFFER_SIZE > streamSize - bufferPos ? streamSize - bufferPos : STREAM_SCAN_BUFFER_SIZE - 2048;

                fread(readBuffer, 1, bufferSize, stream);
                match_c = readBuffer;

                size_t i = 0;

                // Search along the stream segment until no match is found

                while((matchCheck = boyer_moore_search(target.c_str(), match_c, char_table, match_table, true)) != UINT_MAX)
                {

                    match_c += matchCheck + L; // Roll forward to check for redundant matches
                    i += matchCheck;

                    if(bufferPos + i > matchCoord) // Account for repetitions in the overlap region
                    {

                        matchCoord = bufferPos + i;

                        // Check the size against requirements

                        strSize = getStrSize(match_c);

                        if(strSize < maxStrSize)
                        {

                            // Register the index of this find

                            for(size_t y = 1; y < nrow(); ++y)
                            {
                                if(indexPos[y].front() > matchCoord)
                                {

                                    for(int x = rowSize(y-1) - 1; x >= 0; --x)
                                    {
                                        if(indexPos[y-1][x] <= matchCoord)
                                        {
                                            output.emplace_back((size_t)x, y-1);

                                            break;
                                        }
                                    }

                                    break;
                                }
                            }

                        }

                    }

                    i += L;

                }

                bufferPos += bufferSize;

            }
            while(bufferPos + bufferSize < streamSize);

        }
        else
        {

            // Brute force search for partial string matches

            size_t L = target.size();

            unsigned long long coord = 0;
            unsigned long long bufferPos = 0;

            unsigned int matchCoord = 0;
            unsigned int initCoord = 0;

            unsigned int bufferSize = 0;

            char* c = readBuffer;

            // Initialize the trim

            while((matchCoord < L) && (L > 0))
            {
                if(isSpecial(target[matchCoord]))
                {
                    ++matchCoord;
                    --L;
                }
                else
                {
                    initCoord = matchCoord;
                    matchCoord = L-1;
                    break;
                }
            }

            while((matchCoord > initCoord + 1) && (L > 0))
            {
                if(isSpecial(target[matchCoord]))
                {
                    --L;
                    --matchCoord;
                }
                else
                {
                    break;
                }
            }

            L-= initCoord;
            if(L < 1) L = target.size();
            if(initCoord >= L-1) initCoord = 0;

            if(L > streamSize) return false;
            size_t S = streamSize - L + 1;

            matchCoord = initCoord;

            unsigned int sizeFactor;
            if(L > 3) sizeFactor = L/size_threshold;
            else sizeFactor = L;

            unsigned int matchFactor;
            if(L > 3) matchFactor = L*match_threshold;
            else matchFactor = L;

            fseek(stream, 0, SEEK_SET);

            // Perform the search

            if(!L) return false;

            for(; coord < S; ++coord)
            {

                if(bufferPos <= coord)
                {
                    bufferSize = STREAM_SCAN_BUFFER_SIZE > streamSize - coord ? streamSize - coord : STREAM_SCAN_BUFFER_SIZE;
                    bufferPos += bufferSize;
                    fread(readBuffer, 1, bufferSize, stream);
                    c = readBuffer;
                }

                if(*c == target[matchCoord])
                {
                    ++matchCoord;
                }
                else if(isLowerCase(*c))
                {
                    if((*c - 32) == target[matchCoord])
                    {
                        ++matchCoord;
                    }
                }
                else if(isUpperCase(*c))
                {
                    if((*c + 32) == target[matchCoord])
                    {
                        ++matchCoord;
                    }
                }
                else if(matchCoord >= matchFactor)
                {

                    if(getStrSize(c) <= sizeFactor)
                    {
                        output.x.push_back(0);
                        output.y.push_back(0);
                        while((output.y.back() < nrow() - 1) && (coord > indexPos[output.y.back()].front() + rowDataSizes[output.y.back()]))
                        {
                            ++output.y.back();
                        }
                        while((output.x.back() < rowSize(output.y.back()) - 1) && (coord > indexPos[output.y.back()][output.x.back()+1]))
                        {
                            ++output.x.back();
                        }
                    }

                    matchCoord = initCoord;
                }
                else matchCoord = initCoord;
                ++c;
            }

        }

    }
    else
    {
        unsigned int xCoord, yCoord = 0;
        unsigned int sizeFactor = target.size()/size_threshold;

        for(const auto& y : importData)
        {
            xCoord = 0;
            for(const auto& x : y)
            {
                if((sizeFactor > x.size()) && cmpString(target, x))
                {
                    output.x.push_back(xCoord);
                    output.y.push_back(yCoord);
                }
                ++xCoord;
            }
            ++yCoord;
        }
    }

    return output.size() > initSIZE;

}

Vector2u _2Dstream::getCoords(const string& target, const float& threshold)
{
    VectorPairU coords;
    find(coords, target);

    size_t outSIZE = coords.size();
    if(outSIZE < 1) return Vector2u(UINT_MAX,UINT_MAX);
    if(outSIZE < 2) return Vector2u(coords.x.front(), coords.y.front());
    size_t L = target.size();

    unsigned int minSizeDiff = L, outIndex = 0;

    for(size_t i = 0; i < outSIZE; ++i)
    {
        get(coords.x[i], coords.y[i]);
        unsigned int othercmp = charMatchNum(target, readBuffer);
        unsigned int sizecmp = othercmp > L ? othercmp - L : L - othercmp;
        if(sizecmp < minSizeDiff)
        {
            minSizeDiff = sizecmp;
            outIndex = i;
        }
    }

    return Vector2u(coords.x[outIndex], coords.y[outIndex]);
}

string _2Dstream::findMatch(const string& target, const float& threshold)
{
    VectorPairU coords;
    find(coords, target, threshold);

    if(coords.empty()) return string();

    size_t outSIZE = coords.size();
    if(outSIZE < 2) return getString(coords.x.front(), coords.y.front());
    size_t L = target.size();

    std::vector<std::string> candidates;
    candidates.reserve(coords.size());

    for(size_t i = 0; i < coords.size(); ++i)
    {
        candidates.emplace_back(getString(coords[i]));
    }

    return getBestStringMatch(target, candidates);
}

unsigned int _2Dstream::getCount(const string& query, bool exact, const float& threshold)
{
    unsigned int output = 0;
    VectorPairU coords;
    find(coords, query, threshold);

    size_t L = coords.size();

    if(exact)
    {
        for(size_t i = 0; i < L; ++i)
        {
            if(getString(coords[i]) == query)
            {
                ++output;
            }
        }
    }
    else
    {
        output = coords.size();
    }

    return output;
}

bool isNumeric(const _1Dstream& stream, const unsigned int index)
{
    stream.get(index);
    bool output = isNumeric(stream.readbuf());
    return output;
}

string getMatchingString(const string& query, _2Dstream& stream)
{
    VectorPairU coords;
    stream.find(coords, query);
    Vector2u matchCoords = getBestStreamMatch(coords, stream, query);
    return stream.getString(matchCoords.x, matchCoords.y);
}

Vector2u getBestStreamMatch(VectorPairU& coords,
                            _2Dstream& stream,
                            const string& target,
                            const unsigned char& params,
                            const float& threshold)
{

    if(coords.empty()) return Vector2u(UINT_MAX, UINT_MAX);

    size_t outSIZE = coords.size(), L = target.size();
    unsigned int minSizeDiff = L, outIndex = 0;

    vector<string> candidates;

    if(!stream.isImported())
    {
        for(size_t i = 0; i < outSIZE; ++i)
        {
            candidates.emplace_back(stream.getString(coords[i]));
        }
    }
    else
    {
        for(size_t i = 0; i < outSIZE; ++i)
        {
            candidates.emplace_back(stream.importData[coords.y[i]][coords.x[i]]);
        }
    }

    unsigned int matchIndex = getMatchingIndex(target, candidates, params, threshold);

    if(matchIndex != UINT_MAX)
    {
        return coords[matchIndex];
    }

    return Vector2u(UINT_MAX, UINT_MAX);
}

void getBestStreamMatchCoords(VectorPairU& coords, _2Dstream& stream, const string& target)
{
    size_t outSIZE = coords.size();
    if(outSIZE < 2) return;
    size_t L = target.size();

    vector<string> candidates;
    for(size_t i = 0; i < outSIZE; ++i)
    {
        candidates.emplace_back(stream.getString(coords[i]));
    }

    unsigned int matchIndex = getMatchingIndex(target, candidates, CMP_STR_DEFAULT | CMP_STR_SW, 0.4);

    if(matchIndex != UINT_MAX)
    {

        string bestMatch = candidates[matchIndex];

        for(size_t i = 0, j = 0; i < outSIZE; ++j)
        {
            if(candidates[j] != bestMatch)
            {
                coords.erase(i);
                --outSIZE;
            }
            else ++i;
        }
    }


}

bool filterStreamMatch(StringVector& prompt, _2Dstream& stream, bool getMatches)
{
    for(size_t i = 0; i < prompt.size();)
    {
        if(getMatches)
        {
            prompt[i] = stream.findMatch(prompt[i]);
            if(prompt[i].size() < 1) prompt.erase(prompt.begin() + i);
            else ++i;
        }
        else if(!getMatches && !stream.check(prompt[i])) prompt.erase(prompt.begin() + i);
        else ++i;
    }
    return prompt.size() > 0;
}

bool checkAnyStreamMatch(StringVector& query, _2Dstream& stream, bool getMatches)
{
    bool output = false;
    for(auto& tag : query)
    {
        if(getMatches)
        {
            string matchStr = stream.findMatch(tag);
            if(matchStr.size() > 0)
            {
                tag = matchStr;
                output = true;
            }
        }
        else
        {
            if(stream.check(tag)) return true;
        }
    }
    return output;
}

ByteNum assessCoordAlignment(const VectorPairU& coords, _2Dstream& dataSource)
{

    float orientationX = 0.0f,
          orientationY = 0.0f,
          yCTR = (float)dataSource.nrow()/2,
          xCTR = (float)dataSource.ncol()/2;

    size_t tSIZE = coords.size();

    if(tSIZE < 1) return ORIENTATION_NONE;

    for(size_t i = 0; i < tSIZE; ++i)
    {
        orientationX += absolute((float)(coords.x[i]) - xCTR)/dataSource.nrow();
        orientationY += absolute((float)(coords.y[i]) - yCTR)/dataSource.ncol();
    }

    orientationX /= numUnique(coords.x);
    orientationY /= numUnique(coords.y);

    if(orientationX > orientationY) return ORIENTATION_COLUMN; // Horizontal distribution is more diffuse
    if(orientationY > orientationX) return ORIENTATION_ROW; // Vertical distribution is more diffuse

    return ORIENTATION_NONE;
}

ByteNum assessCoordAlignment(const VectorPairU& inCoords, const VectorPairU& outCoords)
{
    size_t L1 = inCoords.size(), L2 = outCoords.size();

    if((L1 < 1) || (L2 < 1)) return ORIENTATION_NONE;

    size_t xMatch = 0, yMatch = 0;

    for(size_t i = 0; i < L1; ++i)
    {
        if(anyEqual(inCoords.x[i], outCoords.x)) ++xMatch;
        if(anyEqual(inCoords.y[i], outCoords.y)) ++yMatch;
    }

    if(xMatch > yMatch) return ORIENTATION_ROW;
    if(yMatch > xMatch) return ORIENTATION_COLUMN;

    return ORIENTATION_NONE;
}

ByteNum assessCoordAlignment(const VectorPairU& coords)
{
    size_t xUnique = numUnique(coords.x);
    size_t yUnique = numUnique(coords.y);

    if(xUnique > yUnique) return ORIENTATION_ROW;
    if(yUnique > xUnique) return ORIENTATION_COLUMN;

    return ORIENTATION_NONE;
}

ByteNum getSearchOrientation(const string& query, _2Dstream& stream)
{
    VectorPairU coords;
    stream.find(coords, query);
    return assessCoordAlignment(coords, stream);
}

}
