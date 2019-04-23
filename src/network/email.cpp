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

#include "EZC/network/email.hpp"
#include <ctime>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include <curl/curl.h>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

const static std::string TX_BOUNDARY = "-------@01056@XXX$5032";

struct text_stream
{
    int pos;
    const char* data;

    text_stream(const char* data):
        pos(0),
        data(data)
    {

    }
};

size_t email_info_read(void *ptr, size_t size, size_t nmemb, void *userp)
{

    if(!(size*nmemb)) {
        return 0;
    }

    text_stream* basic_stream = (text_stream*)userp;
    const char* data = basic_stream->data + basic_stream->pos;

    if(data) {
        size_t len = strlen(data);

        if(len > CURL_MAX_WRITE_SIZE)
        {
            len = CURL_MAX_WRITE_SIZE;
        }

        memcpy(ptr, data, len);
        basic_stream->pos += len;

        return len;
    }

    return 0;
}

std::string generate_email_id()
{

    const int MESSAGE_ID_LEN = 37;

    std::string ret;
    ret.resize(15);

    auto t = time(NULL);
    auto tm = *std::gmtime(&t);

    strftime(const_cast<char *>(ret.c_str()),
             MESSAGE_ID_LEN,
             "%Y%m%d%H%M%S.",
             &tm);

    ret.reserve(MESSAGE_ID_LEN);

    static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

    while (ret.size() < MESSAGE_ID_LEN) {
        ret += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return ret;

}

email_info::attachment::attachment()
{

}

email_info::attachment::attachment(const std::string& path):
    file_path(path)
{
    name = fs::path(path).filename().string();
}

email_info::attachment::attachment(const std::string& name,
                                   const std::string& encoding):
                                       name(name),
                                       encoding(encoding)
{

}

bool email_info::attachment::is_path() const
{
    return !file_path.empty();
}

bool email_info::attachment::empty() const
{
    return !file_path.empty() && !encoding.empty();
}

void email_info::attachment::set_path(const std::string& newPath)
{
    file_path = newPath;
    name = fs::path(newPath).stem().string();
    encoding.clear();
}

void email_info::attachment::set_encoding(const std::string& newEncoding)
{
    encoding = newEncoding;
    file_path.clear();
}

email_info::email_info():
    automated(false)
{

}

email_info::email_info(const char* data):
    automated(false)
{

}

void email_info::addRecipient(const std::string& email,
                         const std::string& name)
{
    rec_email_name[email] = name;
}

void email_info::addCC(const std::string& email,
                 const std::string& name)
{
    cc_email_name[email] = name;
}

void email_info::addBCC(const std::string& email,
                 const std::string& name)
{
    bcc_email_name[email] = name;
}

void email_info::removeRecipient(const std::string& email)
{
    rec_email_name.erase(email);
}

void email_info::removeCC(const std::string& email)
{
    cc_email_name.erase(email);
}

void email_info::removeBCC(const std::string& email)
{
    bcc_email_name.erase(email);
}

void email_info::setDate(const std::string& date)
{
    this->date = date;
}

void email_info::setDateNow()
{
    std::stringstream ss;

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    ss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S %z");
    date = ss.str();
}

void email_info::addAttachment(const std::string& info,
                               const std::string& name)
{
    fs::path newPath(info);
    if(boost::filesystem::exists(newPath))
    {
        attachments.emplace_back(newPath.string());
    }
    else
    {
        attachments.emplace_back(name, info);
    }
}

bool email_info::send()
{
    int i = 0;

    str("");

    *this <<
      "Date: ";

    if(date.empty())
    {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        *this << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S %z") << "\r\n";
    }
    else
    {
        *this << date << "\r\n";
    }

    *this << "To: ";

    for(auto& pair : rec_email_name)
    {

        *this << '\"' << pair.second << "\" <" << pair.first << ">";

        if(i < rec_email_name.size() - 1)
        {
            *this << ',';
        }

        ++i;
    }

    *this << "\r\n" <<
      "From: \"" << sender_name << "\" <" << sender_email << ">";

    *this << "\r\n" <<
      "Cc: ";

    i = 0;
    for(auto& pair : cc_email_name)
    {

         *this << '\"' << pair.second << "\" <" << pair.first << ">";

        if(i < cc_email_name.size() - 1)
        {
            *this << ',';
        }

        ++i;
    }

    *this << "\r\n" <<
        "Bcc: ";

    i = 0;
    for(auto& pair : bcc_email_name)
    {

         *this << '\"' << pair.second << "\" <" << pair.first << ">";

        if(i < bcc_email_name.size() - 1)
        {
            *this << ',';
        }

        ++i;
    }

    bool bMIMEencode = false;

    if(!attachments.empty())
    {

        bMIMEencode = true;

        *this << "\r\n" <<
            "MIME-Version: 1.0\r\n" <<
            "Content-Type: multipart/mixed; " <<
            "boundary=" << TX_BOUNDARY;

    }

    *this << "\r\n" <<
      "Message-ID: <" << generate_email_id() << "@" <<
      sender_email.substr(sender_email.find("@") + 1) << ">\r\n" <<
      "Subject: " << subject << "\r\n\r\n";

    if(bMIMEencode)
    {
        *this << "--" << TX_BOUNDARY << "\r\n" <<
        "Content-Type: text/plain\r\n\r\n";
    }

    *this << body;

    for(auto& attachment : attachments)
    {
        if(attachment.empty())
        {
            continue;
        }

        *this << "\n--" << TX_BOUNDARY <<
        "\r\nContent-Type: application/octet-stream;" <<
        "\r\nContent-Disposition: attachment; filename=" << attachment.name <<
        "\r\nContent-Transfer-Encoding: base64\r\n\r\n";

        if(attachment.is_path())
        {
            *this << base64_encode(attachment.file_path);
        }
        else
        {
            *this << attachment.encoding;
        }

    }

    if(bMIMEencode)
    {
        *this << "--" << TX_BOUNDARY << "--";
    }

    *this << "\n\r";


    CURL *curl;

    CURLcode res = CURLE_OK;

    curl = curl_easy_init();

    if(curl)
    {

        std::stringstream ss;
        struct curl_slist * recipients = NULL;

        text_stream text_buffer(str().c_str());

        std::string URL = server;
        if(!port.empty())
        {
            if(URL.back() == '/') URL.pop_back();
            URL = URL + ":" + port;
        }

        if(URL.back() != '/') URL += '/';

        curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());

        if(!sender_email.empty())
        {
            curl_easy_setopt(curl, CURLOPT_USERNAME, sender_email.c_str());
        }

        if(!password.empty())
        {
            curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
        }

        if(!automated && !sender_email.empty())
        {
            curl_easy_setopt(curl, CURLOPT_MAIL_FROM, (std::string("<") + sender_email + ">").c_str());
        }

        std::string newEmail;
        for(auto& pair : rec_email_name)
        {
            newEmail = std::string("<") + pair.first + ">";
            recipients = curl_slist_append(recipients, newEmail.c_str());
        }

        for(auto& pair : cc_email_name)
        {
            newEmail = std::string("<") + pair.first + ">";
            recipients = curl_slist_append(recipients, newEmail.c_str());
        }

        for(auto& pair : bcc_email_name)
        {
            newEmail = std::string("<") + pair.first + ">";
            recipients = curl_slist_append(recipients, newEmail.c_str());
        }

        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        std::string send_data = str();

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, &email_info_read);
        curl_easy_setopt(curl, CURLOPT_READDATA, &text_buffer);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE, str().size());

        // Set flags

        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

        // Instruct upon SSL cert locations

        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

        #ifndef __APPLE__
        curl_easy_setopt(curl, CURLOPT_CAPATH, "ssl/certs/");
        curl_easy_setopt(curl, CURLOPT_CAINFO, "ssl/certs/ca-bundle.crt");
        #endif

        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
        {
            std::cout << "Email attempt to "
                        << rec_email_name.size() +
                            cc_email_name.size() +
                            bcc_email_name.size()
                        << " recipients failed: "
                        << curl_easy_strerror(res) << '\n';
            return false;
        }

        curl_slist_free_all(recipients);

        curl_easy_cleanup(curl);

        return true;

    }

    return false;

}

std::string base64_encode(const unsigned char *data,
                    size_t input_length) {

    unsigned char *out, *pos;
    const unsigned char *end, *in;

    size_t olen;

    olen = 4*((input_length + 2) / 3);  // Convert 4-byte block counts to 3-byte

    if (olen < input_length)
        return std::string();   // Integer overflow

    std::string outStr;
    outStr.resize(olen);
    out = (unsigned char*)&outStr[0];

    end = data + input_length;
    in = data;
    pos = out;
    while (end - in >= 3) {
        *pos++ = base64_table[in[0] >> 2];
        *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = base64_table[in[2] & 0x3f];
        in += 3;
    }

    if (end - in) {
        *pos++ = base64_table[in[0] >> 2];
        if (end - in == 1) {
            *pos++ = base64_table[(in[0] & 0x03) << 4];
            *pos++ = '=';
        }
        else {
            *pos++ = base64_table[((in[0] & 0x03) << 4) |
                (in[1] >> 4)];
            *pos++ = base64_table[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
    }

    return outStr;

}

std::string base64_encode(const std::string& filepath)
{
    if(access(filepath.c_str(), F_OK))
    {
        return base64_encode((unsigned char*)filepath.c_str(), filepath.size());
    }

    FILE* inFILE = fopen(filepath.c_str(), "rb");

    if(!inFILE)
    {
        return std::string();
    }

    fseek(inFILE, 0, SEEK_END);
    size_t fileSIZE = ftell(inFILE);
    fseek(inFILE, 0, SEEK_SET);

    unsigned char* data = new unsigned char[fileSIZE];

    fread(data, sizeof(unsigned char), fileSIZE, inFILE);
    fclose(inFILE);

    return base64_encode(data, fileSIZE);

}
