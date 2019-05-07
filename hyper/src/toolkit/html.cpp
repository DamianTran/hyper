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

#include "hyper/toolkit/html.hpp"
#include "hyper/algorithm.hpp"

#include "hyper/toolkit/charsurf.hpp"
#include "hyper/toolkit/string.hpp"
#include "hyper/toolkit/string_search.hpp"

using namespace std;

namespace hyper
{

HTML_tree::HTML_tree(const char* str, const size_t& level):
    bNoContentType(false),
    level(level)
{
    read_text(str);
}

HTML_tree::HTML_tree():
    bNoContentType(false),
    level(0) { }

bool HTML_DOCUMENT_TYPE(const char* doc_ptr)
{
    return boyer_moore_search("<!DOCTYPE html", doc_ptr, true) != UINT_MAX;
}

bool HTML_BEGIN(const char* text_ptr)
{
    if((*text_ptr == '<') && (isLetter(*(text_ptr + 1))))
    {
        text_ptr += 2;
        while(*text_ptr)
        {
            if((*text_ptr == '>') && (*(text_ptr - 1) != '-') && (*(text_ptr - 1) != ']')) return true;
            ++text_ptr;
        }
    }
    return false;
}

bool HTML_END(const char* text_ptr)
{
    if(ptr_at_string(text_ptr, "</"))
    {
        while(*text_ptr)
        {
            if(*text_ptr == '>') return true;
            ++text_ptr;
        }
    }
    return false;
}

bool HTML_DECL(const char* text_ptr)
{
    return ptr_at_string(text_ptr, "<!DOCTYPE html", true);
}

bool HTML_COMMENT(const char* text_ptr)
{
    if(ptr_at_string(text_ptr, "<!"))
    {
        text_ptr += 2;
        while(*text_ptr)
        {
            if(ptr_at_string(text_ptr, "-->")) return true;
            ++text_ptr;
        }
    }
    return false;
}

bool HTML_NOCONTENT(const char* text_ptr)
{

    if(HTML_BEGIN(text_ptr))
    {
        string itemType = string("</") + HTML_CLASS(text_ptr) + ">";

        text_ptr += HTML_FORWARD(text_ptr);

        while(*text_ptr)
        {
            if(ptr_at_string(text_ptr, itemType.c_str())) return false;
            else if(HTML_BEGIN(text_ptr)) text_ptr += HTML_FORWARD(text_ptr);
            else ++text_ptr;
        }

        return true;
    }

    return false;
}

int HTML_FORWARD(const char* text_ptr)
{
    int distance = 0;

    if(*text_ptr == '<')
    {
        ++distance;

        bool bQuote = false;
        char quoteType;

        while(*text_ptr && (bQuote || (*text_ptr != '>')))
        {

            if(!bQuote && (isCharType(*text_ptr, "'\"")))
            {
                bQuote = true;
                quoteType = *text_ptr;
            }
            else if(*text_ptr == quoteType)
            {
                bQuote = false;
            }

            ++text_ptr;
            ++distance;
        }
    }

    return distance;
}

string HTML_NAME(const char* text_ptr)
{
    string output;
    if(HTML_BEGIN(text_ptr))
    {
        ++text_ptr;
        const char* c = text_ptr;
        while(*text_ptr && (*text_ptr != ' ') && (*text_ptr != '>'))
        {
            ++text_ptr;
        }
        output.assign(c, text_ptr);
    }
    return output;
}

string HTML_ATTR(const char* text_ptr, const char* attribute_name)
{
    string output;
    if(HTML_BEGIN(text_ptr))
    {
        bool bQuote = false;
        char quoteType;

        while(*text_ptr && !ptr_at_string(text_ptr, attribute_name))
        {
            ++text_ptr;
        }
        text_ptr += strlen(attribute_name);

        while(*text_ptr && isCharType(*text_ptr, " \t\r\n=")) ++ text_ptr;

        const char* c = text_ptr;
        while(*text_ptr && (bQuote || (*text_ptr != ' ')) && (*text_ptr != '>'))
        {
            if(!bQuote && isCharType(*text_ptr, "'\""))
            {
                bQuote = true;
                quoteType = *text_ptr;
            }
            else if(*text_ptr == quoteType)
            {
                bQuote = false;
            }

            ++text_ptr;
        }
        output.assign(c, text_ptr);
        trim(output, " \"'\t\n\r");
    }
    return output;
}

string HTML_CLASS(const char* text_ptr)
{

    string output;

    if(HTML_BEGIN(text_ptr))
    {
        const char* c = ++text_ptr;
        while(*text_ptr && (*text_ptr != ' ') && (*text_ptr != '>')) ++text_ptr;

        output.assign(c, text_ptr);
        trim(output, "\t\r\n ");

    }

    return output;

}

const std::string MS_APOSTROPHE = { -30, -128, -103 };
const std::string MS_QUOTE = { -30, -128, -90 };

string HTML_CLEAN_CONTENT(const char* text_ptr)
{

    string output(text_ptr);

    for(size_t i = 0, j; i < output.size();)
    {
        if(output[i] == '&')
        {
            for(j = i + 1; j < output.size(); ++j)
            {
                if((j == (output.size() - 1)) ||
                    isCharType(output[j], " ;&|-/\\:"))
                {

                    bool bErase = (output[j] == ';') || (j == (output.size() - 1));
                    string escape;

                    if(j - i > 1)
                    {
                        if(j == (output.size() - 1))
                        {
                            escape.assign(output.begin() + i, output.end());
                        }
                        else
                        {
                            escape.assign(output.begin() + i, output.begin() + j);
                        }
                    }
                    else
                    {
                        ++i;
                        break;
                    }

                    try
                    {
                        string replacement = HTML_ESCAPE_CHARS.at(escape);

                        if(bErase)
                        {
                            output.replace(output.begin() + i, output.begin() + j + 1, replacement);
                        }
                        else
                        {
                            output.replace(output.begin() + i, output.begin() + j, replacement);
                        }

                        i += replacement.size();
                    }
                    catch(...)
                    {
                        if(bErase) output.erase(output.begin() + i, output.begin() + j);
                        else ++i;
                    }

                    break;
                }
            }
        }
        else ++i;
    }

    replace_all(output, MS_APOSTROPHE, "'");
    replace_all(output, MS_QUOTE, "\"");

    return output;

}

string HTML_REMOVE_SYNTAX(const char* text_ptr)
{

    string output(text_ptr);

    for(size_t i = 0, j; i < output.size();)
    {
        if(HTML_BEGIN(&output[i]) || HTML_END(&output[i]))
        {
            j = i + HTML_FORWARD(&output[i]);
            output.erase(output.begin() + i, output.begin() + j);
        }
        else ++i;
    }

    return HTML_CLEAN_CONTENT(output);

}

void get_HTML_attributes(const char* text_ptr,
                         map<string, string>& output)
{

    while(*text_ptr)
    {

        if(HTML_BEGIN(text_ptr))
        {
            string attr, value;
            const char* c = NULL;
            bool bQuote = false;
            char quoteType;

            // Skip the class ID

            while(*text_ptr != ' ')
            {
                if(*text_ptr == '>')
                {
                    return; // No attributes
                }
                ++text_ptr;
            }

            while(*text_ptr)
            {

                if(attr.empty())
                {

                    if(!c)
                    {
                        while(*text_ptr && isCharType(*(text_ptr + 1), " \r\t\n")) ++text_ptr;
                        c = text_ptr;
                    }

                    if(*text_ptr == '>') return;

                    if(*text_ptr == '=')
                    {
                        attr.assign(c, text_ptr);
                        trim(attr, " \"'\t\n\r");
                        to_lowercase(attr);
                        c = NULL;
                    }

                }
                else if(value.empty())
                {
                    if(!c)
                    {
                        while(*text_ptr && isCharType(*(text_ptr + 1), " \r\t\n")) ++text_ptr;
                        if(*text_ptr == ' ')
                        {
                            c = ++text_ptr;
                        }
                        else c = text_ptr;
                    }

                    if(!bQuote && isCharType(*text_ptr, "'\""))
                    {
                        bQuote = true;
                        quoteType = *text_ptr;
                    }
                    else if(bQuote && (*text_ptr == quoteType))
                    {
                        bQuote = false;
                    }

                    if(!bQuote && isCharType(*text_ptr, " >"))
                    {

                        if(c != text_ptr)
                        {
                            value.assign(c, text_ptr);
                            trim(value, " \"'\t\n\r;,");
                        }

                        output[attr] = value;
                        while(*text_ptr && isCharType(*(text_ptr + 1), " \t\n\r;,")) ++text_ptr;
                        c = NULL;
                        value.clear();
                        attr.clear();
                    }
                }

                if(!bQuote && ((*text_ptr == '>') || (*text_ptr == '<'))) return;
                ++text_ptr;
            }

            return;
        }

        ++text_ptr;
    }
}

bool sub_HTML_content(string& text)
{

    size_t i = 0, j = 0,
           beginIdx,
           subIdx,
           level = 0;
    bool bQuote = false,
         bStatus = false;
    char quoteType;
    string content;

    while(i < text.size())
    {

        if(HTML_BEGIN(&text.c_str()[i]))
        {

            if(HTML_NOCONTENT(&text.c_str()[i]))
            {

                size_t s = i;
                i += HTML_FORWARD(&text.c_str()[i]);

                if(i < text.size())
                    text.erase(text.begin() + s, text.begin() + i);
                else text.clear();
                i = s;

            }
            else
            {

                beginIdx = i;
                i += HTML_FORWARD(&text.c_str()[i]);
                if(!level) subIdx = i;
                ++level;

            }

        }
        else if(HTML_END(&text.c_str()[i]))
        {

            --level;

            if(!level)
                content.assign(text.begin() + subIdx, text.begin() + i);

            i += HTML_FORWARD(&text.c_str()[i]);

            if(!level)
            {
                j = i - beginIdx - content.size();

                text.replace(text.begin() + beginIdx, text.begin() + i,
                             content.begin(), content.end());

                i -= j;

                bStatus = true;
            }

        }
        else ++i;

    }

    return bStatus;

}

bool HTML_tree::read_text(const char* text)
{

    itemClass.clear();
    ID.clear();
    attributes.clear();

    size_t L = strlen(text);
    size_t i = 0;

    bool bBracket = false;
    bool bNoContent = true;

    string subBuf;

    unsigned int beginIdx = UINT_MAX;
    unsigned int subIdx;
    unsigned int level = 0;

    if(HTML_DECL(&text[i])) i += HTML_FORWARD(&text[i]);

    for(; i < L;)
    {
        if(HTML_COMMENT(&text[i]))
        {
            i += HTML_FORWARD(&text[i]);
        }
        else if(HTML_NOCONTENT(&text[i]))
        {

            if(level > 1)
            {
                i += HTML_FORWARD(&text[i]);
            }
            else
            {

                if(!bNoContent && (beginIdx != UINT_MAX) && (i != beginIdx))
                {
                    string newContent;
                    newContent.assign(text + beginIdx, text + i);
                    trim(newContent, " \t\n\r");
                    content += newContent;
                }

                beginIdx = i;
                i += HTML_FORWARD(&text[i]);

                for(size_t j = i; j < L; ++j)
                {
                    if(text[j] == '<')
                    {
                        bNoContent = false;
                    }
                }

                if(bNoContent)
                {
                    get_HTML_attributes(&text[beginIdx], attributes);
                    itemClass = HTML_CLASS(&text[beginIdx]);
                    bNoContentType = true;
                }
                else
                {
                    subBuf.assign(text + beginIdx, text + i);
                    branches.emplace_back(subBuf.c_str(), this->level + 1);
                }

                beginIdx = i;

            }
        }
        else if(HTML_BEGIN(&text[i]))
        {
            if(!level)
            {
                beginIdx = i + HTML_FORWARD(&text[i]);
                get_HTML_attributes(&text[i], attributes);
                itemClass = HTML_CLASS(&text[i]);
            }
            else if(level == 1)
            {
                subIdx = i;
            }
            ++level;
            i += HTML_FORWARD(&text[i]);
        }
        else if(HTML_END(&text[i]))
        {
            --level;
            if(!level && (beginIdx != UINT_MAX) && (i != beginIdx))
            {
                string newContent;
                newContent.assign(text + beginIdx, text + i);
                trim(newContent, " \t\n\r");
                content += newContent;
                beginIdx = UINT_MAX;
            }
            else if(level == 1)
            {
                subBuf.assign(text + subIdx, text + i + HTML_FORWARD(&text[i]));
                branches.emplace_back(subBuf.c_str(), this->level + 1);
            }

            i += HTML_FORWARD(&text[i]);
        }
        else ++i;
    }

    return true;
}

bool HTML_tree::read_text(const string& text)
{
    return read_text(text.c_str());
}

bool HTML_tree::read_file(const string& filename)
{

    if(access(filename.c_str(), F_OK))
    {
        cout << "HTML tree: error reading file\n";
        return false;
    }

    FILE* inFILE = fopen(filename.c_str(), "rb");

    fseek(inFILE, 0, SEEK_END);
    size_t fileSIZE = ftell(inFILE);
    fseek(inFILE, 0, SEEK_SET);

    char* text = new char[fileSIZE];

    fread(text, fileSIZE, sizeof(char), inFILE);
    fclose(inFILE);

    bool status = read_text(text);

    delete[] (text);

    return status;

}

const string& HTML_tree::getClass() const
{
    return itemClass;
}

const string& HTML_tree::getID() const
{
    try
    {
        return attributes.at("id");
    }
    catch(...)
    {
        return string();
    }
}

string HTML_tree::getContent() const
{

    if(branches.empty())
    {
        if(bNoContentType)
        {
            stringstream ss;
            ss << *this;
            return ss.str();
        }
        else
        {
            return HTML_CLEAN_CONTENT(content);
        }
    }
    else
    {

        string output;

        if(!content.empty())
        {

            output = HTML_REMOVE_SYNTAX(content);

        }

        trim(output, " ");

        if(output.empty())
        {

            for(auto& branch : branches)
            {
                if(output.empty())
                {
                    output = branch.getContent();
                }
                else
                {
                    output.append("\n") += branch.getContent();
                }
            }

        }

        return output;
    }
}

bool HTML_tree::isClass(const string& class_type) const
{
    return cmpString(itemClass, class_type, CMP_STR_CASE_INSENSITIVE);
}

bool HTML_tree::isID(const string& id) const
{
    return getID() == id;
}

const HTML_tree& HTML_tree::getBranch(const string& itemClass,
                                      const string& id,
                                      const vector<string>& attributes) const
{

    for(auto& branch : branches)
    {
        if(branch.itemClass == itemClass)
        {

            bool bMatch = true;

            if(!id.empty())
            {
                bMatch = false;
                if(branch.getID() == id)
                {
                    bMatch = true;
                }
            }

            if(!attributes.empty())
            {
                bMatch = false;
                for(auto& attribute : attributes)
                {
                    if(branch.hasAttribute(attribute))
                    {
                        bMatch = true;
                    }
                }
            }

            if(bMatch)
            {
                return branch;
            }

        }

        try
        {
            return branch.getBranch(itemClass, id, attributes);
        }
        catch(...) { }
    }

    throw std::invalid_argument((string("HTML_tree: no branch with class \"" +
                                        itemClass + "\" found under class \"" +
                                        getClass() + "\"")).c_str());

}

string HTML_tree::getAttribute(const string& attr) const
{
    for(auto& pair : attributes)
    {
        if(cmpString(attr, pair.first, CMP_STR_CASE_INSENSITIVE))
        {
            return pair.second;
        }
    }
    return string();
}

bool HTML_tree::hasAttribute(const string& attr) const
{
    for(auto& pair : attributes)
    {
        if(cmpString(attr, pair.first, CMP_STR_CASE_INSENSITIVE))
        {
            return true;
        }
    }
    return false;
}

bool HTML_tree::getTreeContent(vector<string>& output,
                               const vector<string>& class_types,
                               const map<string, string>& attribute_filters) const
{

    bool status = false, match = false;

    if(cmpStringToList(getClass(), class_types, CMP_STR_CASE_INSENSITIVE))
    {

        if(!attribute_filters.empty())
        {
            try
            {
                for(auto& attr : attributes)
                {
                    for(auto& o_attr : attribute_filters)
                    {
                        if((attr.first == o_attr.first) &&
                                (attr.second == o_attr.second))
                        {
                            match = true;
                            goto match_found;
                        }
                    }
                }
            }
            catch(...) { }
        }
        else match = true;

    }

    match_found:;

    if(match)
    {
        output.emplace_back(getContent());
        status = true;
    }

    for(auto& tree : branches)
    {
        status |= tree.getTreeContent(output, class_types, attribute_filters);
    }

    return status;

}

bool HTML_tree::getTreeText(vector<string>& output,
                            const map<string, string>& attribute_filters) const
{

    getTreeContent(output,
    { "p", "h1", "h2", "h3", "h4", "h5", "h6", "blockquote" },
    attribute_filters);

    size_t i;

    for(size_t j = 0; j < output.size();)
    {

        while(sub_HTML_content(output[j])) { }

        trim(output[j], " \t\n\r");
        swap_chars(output[j], "\t\n\r", ' ');
        single_spaces(output[j]);

        if(output[j].empty())
        {
            output.erase(output.begin() + j);
        }
        else if(HTML_BEGIN(output[j].c_str()))
        {
            output.erase(output.begin() + j);
        }
        else
        {
            ++j;
        }

    }

    return !output.empty();
}

bool HTML_tree::getTreeClasses(vector<HTML_tree>& output,
                               const vector<string>& classes,
                               const map<string, string>& attribute_filters)
{

    if(itemClass.empty())
    {
        return false;
    }

    bool bStatus = false;

    if(cmpStringToList(itemClass, classes, CMP_STR_CASE_INSENSITIVE))
    {

        bool match = false;

        if(!attribute_filters.empty())
        {
            try
            {
                for(auto& attr : attributes)
                {
                    for(auto& o_attr : attribute_filters)
                    {
                        if((attr.first == o_attr.first) &&
                                (attr.second == o_attr.second))
                        {
                            match = true;
                            goto match_found;
                        }
                    }
                }
            }
            catch(...) { }
        }
        else match = true;

match_found:
        ;

        if(match)
        {
            output.emplace_back(*this);
            output.back().level = 0;
            bStatus = true;
        }
    }
    else
    {
        for(auto& branch : branches)
        {
            bStatus |= branch.getTreeClasses(output, classes, attribute_filters);
        }
    }

    return bStatus;

}

bool HTML_tree::getClassesWithAttributes(vector<HTML_tree>& output,
        const vector<string>& attributes)
{

    if(itemClass.empty())
    {
        return false;
    }

    bool bStatus = !attributes.empty();

    for(auto& attr : attributes)
    {
        if(!hasAttribute(attr))
        {
            bStatus = false;
        }
    }

    if(bStatus)
    {
        output.emplace_back(*this);
    }

    for(auto& branch : branches)
    {
        bStatus |= branch.getClassesWithAttributes(output, attributes);
    }

    return bStatus;
}

bool HTML_tree::getClassContent(string& output,
                                const vector<string>& class_types)
{
    if(itemClass.empty() || class_types.empty())
    {
        return false;
    }

    bool bStatus = false;

    if(anyEqual(itemClass, class_types))
    {
        if(output.empty())
        {
            output = getContent();
        }
        else
        {
            output.append("\n") += getContent();
        }

        bStatus |= true;
    }

    for(auto& branch : branches)
    {
        bStatus |= branch.getClassContent(output, class_types);
    }

    return bStatus;

}

ostream& operator<<(ostream& output, const HTML_tree& input)
{
    for(size_t i = 0; i < input.level; ++i)
    {
        output << '\t';
    }

    output << '<' << input.itemClass;

    if(!input.ID.empty())
    {
        output << ' ' << "id=\"" << input.ID << '\"';
    }

    for(auto& pair : input.attributes)
    {
        output << ' ' << pair.first << "=\"" << pair.second << '\"';
    }

    output << ">";

    if(input.branches.empty())
    {
        if(!input.content.empty() && !input.bNoContentType)
        {
            output << '\n';
            for(size_t i = 0; i < input.level + 1; ++i)
            {
                output << '\t';
            }
            output << input.content << '\n';
        }
    }
    else
    {
        output << '\n';
        for(auto& branch : input.branches)
        {
            output << branch << '\n';
        }
    }

    if(!input.bNoContentType)
    {
        if(!input.content.empty())
        {
            for(size_t i = 0; i < input.level; ++i)
            {
                output << '\t';
            }
        }
        output << "</" << input.itemClass << '>';
    }
    return output;
}

}
