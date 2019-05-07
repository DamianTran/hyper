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

#ifndef EZ_HTML
#define EZ_HTML

#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <unordered_map>

namespace hyperC
{

/** HTML escape character table from 'https://www.werockyourweb.com/html-escape-characters/' */

const static std::unordered_map<std::string, std::string> HTML_ESCAPE_CHARS = {
    std::make_pair( "&sp"      ,   " " ),
    std::make_pair( "&excl"    ,   "!" ),
    std::make_pair( "&quot"    ,   "\"" ),
    std::make_pair( "&lsquo"   ,   "\"" ),
    std::make_pair( "&rsquo"   ,   "\"" ),
    std::make_pair( "&rsquor"  ,   "\"" ),
    std::make_pair( "&sbquo"   ,   "\"" ),
    std::make_pair( "&lsquor"  ,   "\"" ),
    std::make_pair( "&ldquo"   ,   "\"" ),
    std::make_pair( "&rdquo"   ,   "\"" ),
    std::make_pair( "&rdquor"  ,   "\"" ),
    std::make_pair( "&bdquo"   ,   "\"" ),
    std::make_pair( "&ldquor"  ,   "\"" ),
    std::make_pair( "&num"     ,   "#" ),
    std::make_pair( "&dollar"  ,   "$" ),
    std::make_pair( "&percnt"  ,   "%" ),
    std::make_pair( "&amp"     ,   "&" ),
    std::make_pair( "&apos"    ,   "'" ),
    std::make_pair( "&lpar"    ,   "(" ),
    std::make_pair( "&rpar"    ,   ")" ),
    std::make_pair( "&ast"     ,   "*" ),
    std::make_pair( "&plus"    ,   "+" ),
    std::make_pair( "&comma"   ,   "," ),
    std::make_pair( "&hyphen"  ,   "-" ),
    std::make_pair( "&dash"    ,   "-" ),
    std::make_pair( "&ndash"   ,   "–" ),
    std::make_pair( "&mdash"   ,   "–" ),
    std::make_pair( "&period"  ,   "." ),
    std::make_pair( "&sol"     ,   "/" ),
    std::make_pair( "&colon"   ,   ":" ),
    std::make_pair( "&semi"    ,   ";" ),
    std::make_pair( "&equals"  ,   "=" ),
    std::make_pair( "&quest"   ,   "?" ),
    std::make_pair( "&commat"  ,   "@" ),
    std::make_pair( "&lsqb"    ,   "[" ),
    std::make_pair( "&bsol"    ,   "\\" ),
    std::make_pair( "&rsqb"    ,   "]" ),
    std::make_pair( "&lt"      ,   "<" ),
    std::make_pair( "&gt"      ,   ">" ),
    std::make_pair( "&lsaquo"  ,   "<" ),
    std::make_pair( "&rsaquo"  ,   ">" ),
    std::make_pair( "&caret"   ,   "^" ),
    std::make_pair( "&lowbar"  ,   "_" ),
    std::make_pair( "&lcub"    ,   "{" ),
    std::make_pair( "&verbar"  ,   "|" ),
    std::make_pair( "&rcub"    ,   "}" ),
    std::make_pair( "&tilde"   ,   "~" ),
    std::make_pair( "&sim"     ,   "~" ),
    std::make_pair( "&nbsp"    ,   " " ),
    std::make_pair( "&iexcl"   ,   "¡" ),
    std::make_pair( "&cent"    ,   "¢" ),
    std::make_pair( "&pound"   ,   "£" ),
    std::make_pair( "&curren"  ,   "¤" ),
    std::make_pair( "&yen"     ,   "¥" ),
    std::make_pair( "&brkbar"  ,   "¦" ),
    std::make_pair( "&sect"    ,   "§" ),
    std::make_pair( "&uml"     ,   "¨" ),
    std::make_pair( "&die"     ,   "¨" ),
    std::make_pair( "&copy"    ,   "(c)" ),
    std::make_pair( "&ordf"    ,   "^a" ),
    std::make_pair( "&laquo"   ,   "«" ),
    std::make_pair( "&not"     ,   "¬" ),
    std::make_pair( "&shy"     ,   "-" ),
    std::make_pair( "&reg"     ,   "(R)" ),
    std::make_pair( "&hibar"   ,   "¯" ),
    std::make_pair( "&deg"     ,   "°" ),
    std::make_pair( "&plusmn"  ,   "±" ),
    std::make_pair( "&sup2"    ,   "^2" ),
    std::make_pair( "&sup3"    ,   "^3" ),
    std::make_pair( "&acute"   ,   "´" ),
    std::make_pair( "&micro"   ,   "µ" ),
    std::make_pair( "&para"    ,   "\n" ),
    std::make_pair( "&middot"  ,   "·" ),
    std::make_pair( "&cedil"   ,   "¸" ),
    std::make_pair( "&sup1"    ,   "^1" ),
    std::make_pair( "&ordm"    ,   "º" ),
    std::make_pair( "&raquo"   ,   "»" ),
    std::make_pair( "&frac14"  ,   "1/4" ),
    std::make_pair( "&half"    ,   "1/2" ),
    std::make_pair( "&frac34"  ,   "3/4" ),
    std::make_pair( "&iquest"  ,   "¿" )
};

bool HTML_DOCUMENT_TYPE(const char* doc_ptr);
bool HTML_BEGIN(const char* text_ptr);
bool HTML_END(const char* text_ptr);
bool HTML_COMMENT(const char* text_ptr);
bool HTML_DECL(const char* text_ptr);
bool HTML_NOCONTENT(const char* text_ptr);
int HTML_FORWARD(const char* text_ptr);

std::string HTML_NAME(const char* text_ptr);
std::string HTML_ATTR(const char* text_ptr, const char* attribute_name);
std::string HTML_CLASS(const char* text_ptr);
std::string HTML_CLEAN_CONTENT(const char* text_ptr);
inline std::string HTML_CLEAN_CONTENT(const std::string& str)
{
    return HTML_CLEAN_CONTENT(str.c_str());
}
std::string HTML_REMOVE_SYNTAX(const char* text_ptr);
inline std::string HTML_REMOVE_SYNTAX(const std::string& text)
{
    return HTML_REMOVE_SYNTAX(text.c_str());
}

void get_HTML_attributes(const char* text_ptr, std::map<std::string, std::string>& output);

bool sub_HTML_content(std::string& text);

// Html recursive tree class - provides total access to class and attribute hierachy of an HTML file

class HTML_tree{
public:

    HTML_tree(const char* str, const size_t& level = 0);
    HTML_tree();

    bool read_text(const char* text);
    bool read_text(const std::string& text);

    bool read_file(const std::string& filename);

    const std::string& getClass() const;
    const std::string& getID() const;
    std::string getContent() const;

    bool isClass(const std::string& class_type) const;
    bool isID(const std::string& id) const;
    inline bool hasContent() const noexcept{ return !bNoContentType && !getContent().empty(); }
    const HTML_tree& getBranch(const std::string& itemClass,
                               const std::string& id = "",
                               const std::vector<std::string>& attributes =
                               std::vector<std::string>()) const;

    std::string getAttribute(const std::string& attr) const;
    inline const std::map<std::string, std::string>& getAttributes() const noexcept{ return attributes; }
    bool hasAttribute(const std::string& attr) const;

    /** getTreeContent - fetch data recursively from class elements belonging to class_types,
                        with attributes matching attribute_filters if applicable, and store
                        in output   */
    bool getTreeContent(std::vector<std::string>& output,
                        const std::vector<std::string>& class_types,
                        const std::map<std::string, std::string>& attribute_filters =
                        std::map<std::string, std::string>()) const;

    /** getTreeText - get all text element content in the DOM, including p and h derivatives */
    bool getTreeText(std::vector<std::string>& output,
                        const std::map<std::string, std::string>& attribute_filters =
                        std::map<std::string, std::string>()) const;

    /** getTreeClasses - Get all HTML elements with class name equal to one of class_types,
        and potentially with at least one attribute in attribute_filters if not empty.
        Equivalent to an advanced version of getElementsByClassName() in js */
    bool getTreeClasses(std::vector<HTML_tree>& output,
                        const std::vector<std::string>& class_types,
                        const std::map<std::string, std::string>& attribute_filters =
                        std::map<std::string, std::string>());

    /** getClassesWithAttributes - Get classes that contain all attributes requested */
    bool getClassesWithAttributes(std::vector<HTML_tree>& output,
                                  const std::vector<std::string>& attributes);

    /** getClassContent - Recursively search the tree and concatenate the content of all
        elements with class name equal to class_types */
    bool getClassContent(std::string& output,
                         const std::vector<std::string>& class_types);

    // Iterators for range loop access to member branches

    inline std::vector<HTML_tree>::const_iterator begin() const{ return branches.begin(); }
    inline std::vector<HTML_tree>::const_iterator end() const{ return branches.end(); }

    friend std::ostream& operator<<(std::ostream& output, const HTML_tree& tree);

protected:

    bool bNoContentType;    // Contentless-HTML class (ie. img, input)

    std::string itemClass, ID, content;
    std::map<std::string, std::string> attributes;
    std::vector<HTML_tree> branches;

    size_t level;

};

}

#endif // EZ_HTML
