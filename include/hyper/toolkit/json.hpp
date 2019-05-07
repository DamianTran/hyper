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

#ifndef EZ_JSON
#define EZ_JSON

#include <string>
#include <vector>
#include <map>

namespace hyper
{

void get_JSON_name_type(const std::string& filename,
                        const std::string& var_name,
                        std::vector<std::string>& output);

enum class JSON_type
{
    None = -1,
    String = 0,
    Numeric = 1,
    Logical,
    Vector,
    Node
};

class JSON_object{
protected:

    std::map<std::string, JSON_object>                nodes;
    std::vector<JSON_object>                          branches;

    std::string property;

    JSON_type value_type;

    int level;

public:

    JSON_object(const char* str, const int& level = 0);
    JSON_object(const std::string& str, const int& level = 0);
    JSON_object();

    ~JSON_object() = default;

    bool load(const std::string& filename);
    bool save(const std::string& filename) const;

    inline const std::string& getProperty() const{ return property; }
    inline const std::string& getProperty(const std::string& field) const{ return nodes.at(field).getProperty(); }
    inline const JSON_object& getObject(const std::string& field) const{ return nodes.at(field); }

    bool getObjects(const std::string& field, std::vector<JSON_object>& output);
    bool getValues(const std::string& field, std::vector<std::string>& output);
    std::vector<std::string> getAttributes() const;

    inline std::vector<JSON_object>::iterator begin() noexcept{ return branches.begin(); }
    inline std::vector<JSON_object>::const_iterator begin() const noexcept{ return branches.begin(); }
    inline std::vector<JSON_object>::iterator end() noexcept{ return branches.end(); }
    inline std::vector<JSON_object>::const_iterator end() const noexcept{ return branches.end(); }

    inline JSON_object& front() noexcept{ return *begin(); }
    inline const JSON_object& front() const noexcept{ return *begin(); }
    inline JSON_object& back() noexcept{ return *(end() - 1); }
    inline const JSON_object& back() const noexcept{ return *(end() - 1); }

    inline const JSON_type& getType() const noexcept{ return value_type; }

    bool empty() const;
    inline bool hasBranches() const noexcept{ return !branches.empty(); }
    void clear();

    JSON_object& operator[](const std::string& tag);
    const JSON_object& operator[](const std::string& tag) const;

    friend std::ostream& operator<<(std::ostream& output, const JSON_object& input);

    void parse(const char* str);

};

}

#endif // EZ_JSON
