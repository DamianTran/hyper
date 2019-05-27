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

#pragma once

#ifndef AIDA_EMAIL
#define AIDA_EMAIL

#include <string>
#include <map>
#include <vector>
#include <utility>
#include <sstream>

size_t email_info_read(void *ptr, size_t size, size_t nmemb, void *userp);
std::string generate_email_id();

class email_info : public std::stringstream
{
public:

    bool automated;

    std::string sender_name,
                sender_email,
                subject,
                body,
                server,
                port,
                password,
                date;

    std::map<std::string, std::string> rec_email_name,
                                        cc_email_name,
                                        bcc_email_name;

    struct attachment
    {
        std::string name,
                    encoding,
                    file_path;

        bool is_path() const;
        bool empty() const;

        void set_path(const std::string& newPath);
        void set_encoding(const std::string& newEncoding);

        attachment();
        attachment(const std::string& path);
        attachment(const std::string& name,
                   const std::string& encoding);
    };

    // Base-64-encoded attachment strings
    std::vector<attachment> attachments;

    void addRecipient(const std::string& email, const std::string& name = "");
    void addCC(const std::string& email, const std::string& name = "");
    void addBCC(const std::string& email, const std::string& name = "");

    void removeRecipient(const std::string& email);
    void removeCC(const std::string& email);
    void removeBCC(const std::string& email);

    void setDateNow();
    void setDate(const std::string& newDate);

    void addAttachment(const std::string& info,
                       const std::string& name = "");

    bool send();

    email_info(const char* info);
    email_info();

};

static char base64_table[] = {

    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'

};

static int mod_table[] = {0, 2, 1};


std::string base64_encode(const unsigned char *data,
                    size_t input_length);
std::string base64_encode(const std::string& filepath);

#endif // AIDA_NETWORK_EMAIL
