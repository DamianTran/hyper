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

#include "hyper/network/webio.hpp"
#include "hyper/toolkit/html.hpp"
#include "hyper/toolkit/string.hpp"
#include "hyper/toolkit/filesystem.hpp"

#include <stdio.h>
#include <unistd.h>
#include <sstream>

#include <boost/filesystem.hpp>
#include <curl/curl.h>

namespace fs = boost::filesystem;
using namespace std;

<<<<<<< HEAD
namespace hyperC
=======
namespace hyper
>>>>>>> a2aca39971b964f2fe0f560682673f2341f2e7a9
{

const vector<string> description_attributes = {
    "title",
    "name",
    "tag",
    "description",
    "rel",
    "type",
    "id",
};

WebPage::WebPage(const std::string& data,
                 const std::string& URL):
    bCanFollow(true),
    bCanArchive(true),
    bCanIndex(true),
    data(data),
    URL(URL)
{

    if(HTML_DOCUMENT_TYPE(data.c_str()))
    {

        DOM.read_text(data.c_str());

        try
        {
            const HTML_tree& meta = DOM.getBranch("head");

            string parse;

            for(auto& branch : meta)
            {
                if(branch.isClass("title"))
                {
                    title = branch.getContent();
                }
                else if(branch.isClass("meta"))
                {
                    if(branch.getAttribute("name") == "keywords")
                    {
                        parse = branch.getAttribute("content");
                    }
                    else if(branch.getAttribute("name") == "description")
                    {
                        description = branch.getAttribute("content");
                    }
                    else if(branch.getAttribute("name") == "robots")
                    {
                        string robots = branch.getAttribute("content");

                        cout << "Robots header: " << robots;

                        if(cmpString(robots, "nofollow"))
                        {
                            bCanFollow = false;
                        }

                        if(cmpString(robots, "noindex"))
                        {
                            bCanIndex = false;
                        }

                        if(cmpString(robots, "noarchive"))
                        {
                            bCanArchive = false;
                        }
                    }
                }
            }

            if(!parse.empty())
            {
                splitString(parse, keywords, ",;");
                if(keywords.empty())
                {
                    keywords.emplace_back(parse);
                }
            }

        }catch(...){ }

        links.clear();

        vector<HTML_tree> linkClasses;
        if(DOM.getClassesWithAttributes(linkClasses, { "href" }))
        {
            for(size_t i = 0, L = linkClasses.size(); i < L; ++i)
            {
                string link = linkClasses[i].getAttribute("href");
                if(!link.empty())
                {
                    string description;
                    if(linkClasses[i].hasContent()) description = linkClasses[i].getContent();
                    else
                    {
                        for(size_t j = 0; j < description_attributes.size(); ++j)
                        {
                            if(linkClasses[i].hasAttribute(description_attributes[j]))
                            {
                                description = linkClasses[i].getAttribute(description_attributes[j]);
                                break;
                            }
                        }
                    }

                    trim(description, " \t\n\r");
                    links[link] = description;
                }
            }
        }

    }
}

const string& WebPage::getHeaderContent(const string& metatag) const
{

    const HTML_tree& header = DOM.getBranch("head");

    for(auto& branch : header)
    {
        if(branch.isClass("meta") && (branch.getAttribute("name") == metatag))
        {
            return branch.getAttribute("content");
        }
    }

    throw invalid_argument((string("HTML_tree: no header field for requested metatag \"") +
                           metatag + "\"").c_str());

}

bool URL_is_relative(const string& URL)
{

    if(((!URL.find("https://")) ||
           (!URL.find("http://")) ||
           (!URL.find("ftp://")) ||
          ((!URL.find("www")) &&
          ((URL.find(".") == 3) || (URL.find(".") == 4))))) return false;

    size_t found_index = URL.find(".");
    size_t L = URL.size();

    if(found_index + 1 < L)
    {

        size_t TLD_index = URL.find("/", ++found_index);

        if(TLD_index > L)
        {
            TLD_index = L;
        }

        string TLD;
        TLD.assign(URL.begin() + found_index, URL.begin() + TLD_index);

        for(auto& term : URL_TLD_NAMES)
        {
            if(term == TLD) return false;
        }

        return true;

    }

    return false;

}

string URL_get_domain(const string& URL)
{

    if(URL_is_relative(URL)) return string();

    size_t L = URL.size();
    size_t found_index = URL.find(".");
    size_t end_index = 0;
    if(found_index + 1 < L)
    {

        end_index = URL.find(".", ++found_index);

        if(URL.find("/", found_index) < end_index)
        {
            end_index = URL.find("/", found_index);

            if(end_index > L)
            {
                end_index = L;
            }

            string output;
            output.assign(URL.begin(), URL.begin() + end_index);

            return output;
        }
        else if(end_index < L)
        {

            end_index = URL.find("/", end_index);
            if(end_index > L)
            {
                end_index = L;
            }

            string output;
            output.assign(URL.begin(), URL.begin() + end_index);

            return output;

        }
        else
        {

            end_index = URL.find("/", found_index);

            if(end_index > L)
            {
                end_index = L;
            }

            string TLD;
            TLD.assign(URL.begin() + found_index, URL.begin() + end_index);

            for(auto& term : URL_TLD_NAMES)
            {
                if(term == TLD) return URL;
            }

        }

    }

    return string();

}

string URL_get_directory(const std::string& URL)
{
    string output = fs::path(URL).parent_path().string();
    if(output.empty())
    {
        return URL;
    }
    return output;
}

string URL_append(string URL, string extension)
{

    if(extension.front() == '.')
    {
        int up_levels = -1;
        while(extension.front() == '.')
        {
            ++up_levels;
            extension.erase(extension.begin());
        }

        trim(extension, "/");

        while(up_levels > 0)
        {
            string up_level = fs::path(URL).parent_path().string();

            if(up_level.empty()) break; // Top-level directory
            else URL = up_level;

            --up_levels;
        }
    }
    else if(extension.front() == '/')
    {
        return URL_get_domain(URL) + extension;
    }

    trim(extension, "/");
    if(URL.back() != '/')
    {
        URL += '/';
    }

    return URL + extension;

}

struct text_stream
{
    size_t size;
    char* data;

    text_stream():
        size(0),
        data(new char[1])
    {
        data[0] = 0;
    }

    ~text_stream()
    {
        delete[] data;
    }
};

size_t net_write_data(void* buf, size_t size, size_t nmemb, void* userp)
{

    size_t tSIZE = size*nmemb;

    if(!tSIZE)
    {
        return 0;
    }

    text_stream* stream = (text_stream*)userp;

    char* realloc_stream = new char[stream->size + tSIZE + 1];
    memcpy(realloc_stream, stream->data, stream->size);
    delete[] stream->data;
    stream->data = realloc_stream;

    if(!stream->data)
    {
        cout << "Error in net write: out of memory\n";
        return 0;
    }

    memcpy(&stream->data[stream->size], buf, tSIZE);

    stream->size += tSIZE;
    stream->data[stream->size] = 0;

    return tSIZE;
}

bool getNetworkData(ostream* output,
                    const string& url,
                    string* redirect_url)
{

    CURL* curl = curl_easy_init();
    CURLcode res;

    text_stream data;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &net_write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&data);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);

    if(res != CURLE_OK)
    {
        cout << "Failed to obtain network data: " << curl_easy_strerror(res) << '\n';
        return false;
    }

    if(redirect_url)
    {
        char* url = NULL;
        curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &url);

        if(!url)
        {
            curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);
        }

        if(url)
        {
            redirect_url->assign(url);
        }
    }

    *output << data.data;

    curl_easy_cleanup(curl);

    return true;

}

size_t getNetworkData(void** output,
                    const string& url,
                    string* redirect_url)
{

    CURL* curl = curl_easy_init();
    CURLcode res;

    text_stream data;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &net_write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&data);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);

    if(res != CURLE_OK)
    {
        cout << "Failed to obtain network data: " << curl_easy_strerror(res) << '\n';
        return 0;
    }

    if(redirect_url)
    {
        char* url = NULL;
        curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &url);

        if(!url)
        {
            curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);
        }

        if(url)
        {
            redirect_url->assign(url);
        }
    }

    if(data.size)
    {
        unsigned char* out_data = new unsigned char[data.size];
        memcpy(out_data, data.data, data.size);

        *output = (void*)out_data;
    }

    curl_easy_cleanup(curl);

    return data.size;

}

bool downloadFile(const string& URL,
                  const string& filename)
{

    unsigned char* data;
    size_t L;

    if(L = getNetworkData((void**)&data, URL))
    {

        FILE* outFILE = fopen(filename.c_str(), "wb");

        if(!outFILE)
        {
            cout << "Error: failed to write to hard drive\n";
            return false;
        }

        fwrite(data, sizeof(unsigned char), L, outFILE);
        fclose(outFILE);

        delete[] data;

        return true;
    }

    cout << "Error: download failed\n";

    return false;

}

void getLinks(const char* data, map<string, string>& link_output)
{

    HTML_tree tree(data);
    vector<HTML_tree> classes;
    tree.getClassesWithAttributes(classes, { "href" });

    for(auto& link : classes)
    {
        link_output[link.getContent()] = link.getAttribute("href");
    }

}

void getLinks(const char* data, vector<string>& link_output)
{

    HTML_tree tree(data);
    vector<HTML_tree> classes;
    tree.getClassesWithAttributes(classes, { "href" });

    for(auto& link : classes)
    {
        link_output.emplace_back(link.getAttribute("href"));
    }

}

AsyncDownloadProcess::AsyncDownloadProcess():
    bComplete(false),
    data_ptr(nullptr),
    size(0),
    process(nullptr){ }

AsyncDownloadProcess::~AsyncDownloadProcess()
{
    if(process)
    {
        process->join();
        delete(process);
    }

    if(data_ptr)
    {
        delete(data_ptr);
    }
}

void AsyncDownloadProcess::init(const std::string& URL)
{
    process = new thread([&]()
    {

        download(URL);
        bComplete = true;

    });

}

void AsyncDownloadProcess::download(const std::string& URL)
{
    size = getNetworkData((void**)&data_ptr, URL);
}

}
