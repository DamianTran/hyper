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

#include "hyper/toolkit/zip.hpp"
#include "hyper/toolkit/filesystem.hpp"
#include "boost/filesystem.hpp"

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <fcntl.h>
#include <cmath>

#include "zlib.h"
#include <zip.h>

#define __zlib_check_err(msg) { \
    if (err != Z_OK) { \
        cerr << msg << " error: " << err << endl; \
        exit(1); \
    } \
}

using namespace boost::filesystem;
using namespace std;

namespace hyperC
{

const static size_t zip_inflate_buf_size = 16384;

ostream& print_archive(ostream& output, const string& filename)
{

    if(access(filename.c_str(), F_OK))
    {
        return output << "File \"" << filename << "\" does not exist\n";
    }

    int err, len;

    struct zip* archive;
    struct zip_stat sb;

    if((archive = zip_open(filename.c_str(), 0, &err)) == NULL)
    {
        output << "Unable to open or access archive file\n";
        return output;
    }

    size_t N = zip_get_num_entries(archive, 0);
    output << "Archive with " << N << " entries =============\n";
    for(size_t i = 0; i < N; ++i)
    {
        if(zip_stat_index(archive, i, 0, &sb) == 0)  // Valid archive item
        {
            output << '\t' << i + 1 << ": " << sb.name << " (" << sb.size << "b)\n";
        }
    }

    zip_close(archive);
    return output;
}

bool extract(const string& filename,
             const string& outPath)
{

    if(access(filename.c_str(), F_OK))
    {
        cout << "Extraction failed (file \"" << filename << "\" does not exist)\n";
        return false;
    }

    int err, len, fd;
    long long tSIZE;

    struct zip* archive;
    struct zip_file* zf;
    struct zip_stat sb;

    string archive_path = outPath;
    if(outPath.empty()) archive_path = path(filename).parent_path().string()
                                           + "/" + path(filename).stem().string() + "/";
    else if(archive_path.back() != '/') archive_path.append("/");

    create_directories(archive_path);

    if((archive = zip_open(filename.c_str(), 0, &err)) == NULL)
    {
        return false;
    }

    size_t N = zip_get_num_entries(archive, 0);

    for(size_t i = 0; i < N; ++i)
    {
        if(zip_stat_index(archive, i, 0, &sb) == 0)  // Valid archive item
        {
            cout << "Extracting " << sb.name << " (" << sb.size << "b)\n";
            path sub_file(archive_path + sb.name);
            create_directories(sub_file.parent_path());

            len = strlen(sb.name);
            zf = zip_fopen_index(archive, i, 0);
            unsigned char* buf = new unsigned char[sb.size];

#if defined __WIN32 || defined _WIN32 || defined WIN32
            fd = open(sub_file.string().c_str(), O_RDWR | O_TRUNC | O_CREAT | O_BINARY, 0644);
#else
            fd = open(sub_file.string().c_str(), O_RDWR | O_TRUNC | O_CREAT, 0644);
#endif

            if((len = zip_fread(zf, buf, sb.size)) < 0)
            {
                cout << "Failed to extract " << sb.name << "...\n";
                continue;
            }

            write(fd, buf, len);
            close(fd);
            zip_fclose(zf);

            delete[] buf;
        }
    }

    zip_close(archive);

    return true;
}

bool archive(const string& directory,
             const string& outName)
{

    if(!is_directory(directory)) return false;
    int errorp;

    string archive_path = outName;
    if(archive_path.empty())
    {
        archive_path = path(directory).parent_path().string() + "/" +
                       path(directory).stem().string() + ".zip";
    }

    unsigned int L = path(directory).string().size();

    PathList files;
    getFilesInDir(files, path(directory), "");

    zip_t* zipper = zip_open(archive_path.c_str(), ZIP_CREATE | ZIP_EXCL, &errorp);
    if(zipper == nullptr) return false;

    zip_source_t* source;

    bool bStatus = false;

    for(auto& file : files)
    {
        if((source = zip_source_file(zipper, file.string().c_str(), 0,0)) == nullptr)
        {
            cout << "Failed to source: " << file.string() << '\n';
            continue;
        }
        cout << "Archiving: " << file.string() << '\n';
        if(zip_file_add(zipper, file.string().substr(L + 1).c_str(), source, ZIP_FL_ENC_UTF_8) < 0)
        {
            cout << "Failed to add to archive: " << file.string() << '\n';
            zip_source_free(source);
            continue;
        }

        bStatus |= true;
    }

    zip_close(zipper);

    return bStatus;

}

size_t decompress(const string& filename,
                void** output)
{
    unsigned char* data = nullptr;
    size_t dataSIZE = getFileData(filename, (void**)&data);

    if(!dataSIZE || !data) return 0;

    z_stream d_stream;

    d_stream.zalloc = Z_NULL;
    d_stream.zfree = Z_NULL;
    d_stream.opaque = Z_NULL;
    d_stream.avail_in = 0;
    d_stream.next_in = Z_NULL;

    int err;

    if(filename.find(".gz") < filename.size())
    {
        err = inflateInit2(&d_stream, 16+MAX_WBITS);
    }
    else
    {
        err = inflateInit(&d_stream);
    }

    d_stream.avail_in = dataSIZE;
    d_stream.next_in = data;

    __zlib_check_err("Inflate init")

    size_t outPos = 0;

    unsigned char* output_data = new unsigned char;

    for(size_t i = 0, inflateSize;;++i)
    {

        inflateSize = zip_inflate_buf_size*pow(1.4, i);

        unsigned char* d_buffer = new unsigned char[inflateSize];

        d_stream.next_out = d_buffer;
        d_stream.avail_out = inflateSize;

        err = inflate(&d_stream, Z_NO_FLUSH);

        if(err == Z_STREAM_END)
        {

            delete[] data;

            unsigned char* realloc_data = new unsigned char[outPos + inflateSize - d_stream.avail_out];
            memcpy(realloc_data, output_data, outPos);
            delete[] output_data;
            output_data = realloc_data;

            if(!output_data)
            {
                cout << "Error: failed to reallocate\n";
                delete[] d_buffer;
                break;
            }

            memcpy(output_data + outPos, d_buffer, inflateSize - d_stream.avail_out);

            outPos += inflateSize - d_stream.avail_out;

            if(!d_stream.avail_in)
            {
                delete[] d_buffer;
                break;
            }
        }
        else if(err)
        {
            delete[] data;

            break;
        }

        __zlib_check_err("Inflate");

        unsigned char* realloc_data = new unsigned char[outPos + inflateSize];
        memcpy(realloc_data, output_data, outPos);
        delete[] output_data;
        output_data = realloc_data;

        memcpy(output_data + outPos, d_buffer, inflateSize);

        outPos += inflateSize;
        delete[] d_buffer;
    }

    err = inflateEnd(&d_stream);
    __zlib_check_err("Inflate end");

    *output = (void*) output_data;

    return outPos;

}

bool decompress(const string& filename,
                const string& outFileName)
{

    unsigned char* data = nullptr;
    size_t fileSIZE = decompress(filename, (void**)&data);

    if(data)
    {
        if(!fileSIZE)
        {
            delete[] data;
            return false;
        }

        if(!saveFileData(data, fileSIZE, outFileName))
        {
            delete[] data;
            return false;
        }

        delete[] data;

        return true;
    }

    return false;

}

}
