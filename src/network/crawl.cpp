#include "hyper/network/crawl.hpp"
#include "hyper/network/webio.hpp"

#include "hyper/toolkit/html.hpp"
#include "hyper/toolkit/console.hpp"
#include "hyper/toolkit/string.hpp"
#include "hyper/toolkit/string_search.hpp"

#include <boost/filesystem.hpp>

#define hyper_SPIDER_CONSTRUCT        \
    bSuccess(false),                \
    bVerbose(false),                \
    bStayOnDomain(false),           \
    bIndexMedia(true),              \
    history_index(0),               \
    max_cache_size(500)

namespace fs = boost::filesystem;
using namespace std;

namespace hyperC
{

Spider::Spider():
    hyper_SPIDER_CONSTRUCT{ }

Spider::Spider(const string& URL):
    hyper_SPIDER_CONSTRUCT
{
    go_to(URL);
}

bool Spider::go_to(const string& URL)
{

    if(URL.front() == '#') return false; // On-page link navigation is irrelevant for crawler

    string newURL;
    if(!URL_is_relative(URL))
    {
        newURL = URL;
    }
    else
    {
        newURL = cURL;
        newURL = URL_append(URL_get_directory(newURL), URL);
    }

    if(bVerbose)
    {
        cout << "Going to: " << newURL << '\n';
    }

    bSuccess = false;

    stringstream ss;
    string redirect_url;
    bool bCached = false;

    for(size_t i = 0; i < history.size(); ++i)
    {
        if(history[i].getURL() == newURL)
        {

            if(history.size() + 1 > max_cache_size)
            {
                history.pop_back();
            }

            history.insert(history.begin() + history_index, WebPage(history[i]));
            if(bVerbose)
            {
                cout << "Revisited cached website: " << history[history_index].getURL() << '\n';
            }

            bCached = true;

            break;
        }
    }

    if(!bCached && getNetworkData(&ss, newURL, &redirect_url))
    {

        history.insert(history.begin() + history_index, WebPage(ss.str(), newURL));

        try
        {

            const WebPage& page = getPage();

            if(!redirect_url.empty())
            {
                if(bVerbose)
                {
                    cout << "Redirected to: " << redirect_url << '\n';
                }
                newURL = redirect_url;
            }

            if(iURL.empty()) iURL = newURL;

            cURL = newURL;
            domain = URL_get_domain(cURL);

            if(cmpString(page.getTitle(), "404") ||
               cmpString(page.getTitle(), "Not Found"))
            {
                bSuccess = false;
            }
            else
            {

                if(bVerbose)
                {
                    cout << "Page: " << page.getTitle() << '\n';
                }
                bSuccess = true;
            }

        }catch(...)
        {
            bSuccess = false;
        }
    }
    else if(bCached)
    {

        try
        {

            const WebPage& page = getPage();

            cURL = newURL;
            domain = URL_get_domain(cURL);

            if(cmpString(page.getTitle(), "404") ||
               cmpString(page.getTitle(), "Not Found"))
            {
                bSuccess = false;
            }
            else
            {

                if(bVerbose)
                {
                    cout << "Page: " << page.getTitle() << '\n';
                }
                bSuccess = true;
            }

        }catch(...)
        {
            bSuccess = false;
        }

    }

    return bSuccess;

}

bool Spider::go_to(const size_t& index)
{
    // Will throw out_of_range if no valid page loaded
    if(index > getLinks().size())
    {
        throw out_of_range("Web Crawler: requested index was out of range of available links");
    }

    size_t i = 0;
    for(auto& pair : getLinks())
    {
        if(i == index) return go_to(pair.first);
        ++i;
    }

    return false;
}

bool Spider::go_to_named(const string& tag,
                              const vector<string>& context)
{

    vector<string> candidates;
    vector<string> candidate_links;

    for(auto& pair : getLinks())
    {
        if(cmpString(pair.second, tag, CMP_STR_DEFAULT | CMP_STR_SW, 0.4f))
        {
            if(context.empty())
            {
                candidates.emplace_back(pair.second);
                candidate_links.emplace_back(pair.first);
            }
            else
            {
                for(auto& term : context)
                {
                    if(!cmpString(pair.first, term))
                    {
                        goto next;
                    }
                }

                candidates.emplace_back(pair.second);
                candidate_links.emplace_back(pair.first);
            }
        }

        next:;
    }

    if(!candidates.empty())
    {
        unsigned int matchIndex = getMatchingIndex(tag, candidates, CMP_STR_DEFAULT | CMP_STR_SW, 0.4f);

        if(matchIndex != UINT_MAX)
        {
            return go_to(candidate_links[matchIndex]);
        }
    }

    return false;

}

bool Spider::checkHistory(const string& link) const noexcept
{

    if(history.empty())
    {
        return false;
    }

    string test_URL;

    if(URL_is_relative(link))
    {
        try
        {
            const WebPage& page = getPage();
            test_URL = URL_append(URL_get_directory(page.getURL()), link);
        }catch(...)
        {
            return false;
        }
    }
    else
    {
        test_URL = link;
    }

    for(auto& page : history)
    {
        if(page.getURL() == test_URL) return true;
    }

    return false;
}

bool Spider::go_back()
{
    if(history_index + 1 < history.size())
    {
        cURL = history[++history_index].getURL();
        return true;
    }

    return false;
}

bool Spider::go_forward()
{

    if(history_index)
    {
        cURL = history[--history_index].getURL();
        return true;
    }

    return false;

}

void Spider::addExcludeTypes(const vector<string>& fileTypes) noexcept{
    for(auto ext : fileTypes)
    {
        if(ext.front() != '.')
        {
            ext.insert(ext.begin(), '.');
        }

        exclude_filetypes.emplace(ext);
    }
}

bool Spider::find_pages(map<string, string>& output,
                        const vector<string>& search_terms,
                        const int& search_depth,
                        const vector<string>& context,
                        unordered_set<string>* tracePath)
{

    bool bStatus = false;

    try
    {

        const WebPage& page = getPage();

        if(!page.canFollowLinks())
        {
            cout << "WARNING: page does not allow bots to follow links.\n";
        }

        auto links = getLinks(); // Create a copy to maintain the top-level reference

        unordered_set<string> trace;

        if(!tracePath)
        {
            tracePath = &trace;
        }

        if(bVerbose)
        {
            cout << "Initiating search branch with depth: " << search_depth << '\n';
        }

        for(auto& pair : links)
        {

            string fileExt = fs::path(pair.first).extension().string();

            if(bStayOnDomain &&
               (!URL_is_relative(pair.first) &&
               !cmpString(URL_get_domain(pair.first), domain)))
            {
               if(bVerbose)
               {
                   cout << "Skipping outer domain link: " << pair.first << '\n';
               }
               continue;
            }

            if(!fileExt.empty())
            {
                if(exclude_filetypes.find(fileExt) != exclude_filetypes.end())
                {
                    if(bVerbose)
                    {
                        cout << "Skipping excluded file type: " << pair.first << '\n';
                    }

                    continue;
                }
                else if(anyEqual(fileExt, WEB_MEDIA_TYPES))
                {
                    if(bIndexMedia && (media_URLs.find(pair.first) != media_URLs.end()))
                    {
                        media_URLs.emplace(pair.first);
                    }

                    if(bVerbose)
                    {
                        cout << "Found media file: " << pair.first << '\n';
                    }

                    continue;
                }
            }

            if(tracePath->find(pair.first) == tracePath->end())
            {

                tracePath->emplace(pair.first);

                if(go_to(pair.first))
                {

                    if(bVerbose)
                    {
                        cout << "Navigated to: " << pair.first << '\n';
                    }

                    vector<string> textSections;

                    getPage().getTree().getTreeText(textSections);

                    for(auto& section : textSections)
                    {

                        for(auto& term : search_terms)
                        {
                            if(boyer_moore_search(term, section, true) == UINT_MAX)
                            {
                                goto nextSection;
                            }
                        }

                        if(!context.empty())
                        {
                            for(auto& term : context)
                            {
                                if(boyer_moore_search(term, section, true) != UINT_MAX)
                                {
                                    output.emplace(pair);
                                    bStatus |= true;
                                    goto nextLink;
                                }
                            }
                        }
                        else
                        {
                            bStatus |= true;
                            output.emplace(pair);
                            break;
                        }

                        nextSection:;

                    }

                    nextLink:;

                    if(search_depth && !getLinks().empty())
                    {
                        if(search_depth < 0)
                        {
                            bStatus |= find_pages(output, search_terms, search_depth, context, tracePath);
                        }
                        else if(search_depth > 0)
                        {
                            bStatus |= find_pages(output, search_terms, search_depth - 1, context, tracePath);
                        }

                        if(bVerbose)
                        {
                            cout << "Following link: " << pair.first << '\n';
                        }
                    }
                }

            }
            else if(bVerbose)
            {
                cout << "Already visited link: " << pair.first << '\n';
            }
        }

    }catch(...){ }

    return bStatus;

}

ostream& Spider::print_meta(ostream& output) const
{
    output << "URL: " << cURL;
    output << "\nDomain: " << domain << '\n';
    try
    {
        const WebPage& page = getPage();
        const HTML_tree& header = page.getTree().getBranch("head");

        for(auto& branch : header)
        {
            if(branch.isClass("title"))
            {
                output << "Title: " << branch.getContent() << '\n';
            }
            else if(branch.isClass("meta"))
            {
                if(branch.hasAttribute("name"))
                {
                    output << capitalized(branch.getAttribute("name")) << ": " << branch.getAttribute("content") << '\n';
                }
                else if(branch.hasAttribute("property"))
                {
                    output << capitalized(branch.getAttribute("property")) << ": " << branch.getAttribute("content") << '\n';
                }
                else if(branch.hasAttribute("itemprop"))
                {
                    output << capitalized(branch.getAttribute("itemprop")) << ": " << branch.getAttribute("content") << '\n';
                }
                else
                {
                    for(auto& pair : branch.getAttributes())
                    {
                        output << capitalized(pair.first) << ": " << pair.second << '\n';
                    }
                }
            }
        }

    }catch(...){ }

    return output;
}

ostream& Spider::print_links(ostream& output) const
{
    try
    {
        const WebPage& page = getPage();

        if(!page.getLinks().empty())
        {
            output << "Links:\n";
            size_t index = 0;
            for(auto& pair : page.getLinks())
            {
                output << '\t' << index << ": " << pair.first << " (" << pair.second << ")\n";
                ++index;
            }
        }
        else
        {
            output << "No links found\n";
        }

    }catch(...)
    {
        output << "No web pages loaded\n";
    }

    return output;
}

ostream& operator<<(ostream& output, const Spider& input)
{
    input.print_meta(output);
    try
    {
        const HTML_tree& tree = input.getPage().getTree();
        vector<string> textBlocks;
        output << "\n\nText Content: ***********************************\n\n";
        tree.getTreeText(textBlocks);
        for(auto& block : textBlocks)
        {
            output << block << "\n\n";
        }
        output << "End of Text Content *********************************\n\n";
    }catch(...)
    {
        output << "\n\n[ No text content to show ]\n\n";
    }
    input.print_links(output);
    return output;
}

int WebCrawlProgram(int argc, char** argv)
{
    cout << "Web crawler program: type a command to begin: ";

    Spider crawler;
    crawler.setVerbose();

    string usrInput;

    while(getline(cin, usrInput))
    {

        bool bRefresh = false;

        if(!usrInput.empty())
        {

            if((usrInput == "q") || (usrInput == "exit") || (usrInput == "quit"))
            {
                break;
            }
            else if(usrInput == "content")
            {
                try
                {
                    cout << crawler.getContent() << '\n';
                }catch(...){
                    cout << "Error: no active page\n";
                }
            }
            else if(usrInput == "tree")
            {
                try
                {
                    cout << crawler.getPage().getTree() << '\n';
                }catch(...){
                    cout << "Error: No active page\n";
                }
            }
            else if(usrInput == "back")
            {
                bRefresh = crawler.go_back();
            }
            else if(usrInput == "forward")
            {
                bRefresh = crawler.go_forward();
            }
            else{

                vector<string> parse;
                splitString(usrInput, parse, " ");

                if(parse.size() > 1)
                {
                    if(parse.front() == "goto")
                    {

                        if(parse[1] == "domain")
                        {
                            bRefresh = crawler.go_to_main();
                        }
                        else if(parse[1] == "home")
                        {
                            bRefresh = crawler.go_home();
                        }
                        else
                        {

                            try
                            {
                                bRefresh = crawler.go_to(stoi(parse[1]));
                            }catch(...)
                            {

                                if((parse[1].front() == '"') &&
                                   (parse[1].back() == '"'))
                                {
                                    trim(parse[1], "\"");
                                    vector<string> context;
                                    if(parse.size() > 2)
                                    {
                                        for(size_t i = 2; i < parse.size(); ++i)
                                        {
                                            context.emplace_back(parse[i]);
                                        }
                                    }

                                    bRefresh = crawler.go_to_named(parse[1], context);
                                }
                                else
                                {
                                    bRefresh = crawler.go_to(parse.back());
                                }
                            }

                        }
                    }
                    else if(parse.front() == "find")
                    {
                        vector<string> terms;
                        map<string, string> links;

                        int depth = -1;

                        for(size_t i = 1; i < parse.size(); ++i)
                        {
                            if(isNumeric(parse[i]) && (depth == -1))
                            {
                                depth = stoi(parse[i]);
                            }
                            else
                            {
                                terms.emplace_back(parse[i]);
                            }
                        }

                        if(!terms.empty())
                        {
                            clear_console();
                            cout << "Searching for terms: " << terms << '\n';
                            if(depth == -1)
                            {
                                cout << "Searching with infinite depth\n";
                            }
                            else
                            {
                                cout << "Searching " << depth << " links from the current location\n";
                            }

                            if(crawler.find_pages(links, terms, depth))
                            {
                                cout << "Found pages:\n";
                                size_t i = 0;
                                for(auto& link : links)
                                {
                                    cout << i << ": " << link.first << " (" << link.second << ")\n";
                                    ++i;
                                }
                            }
                            else
                            {
                                cout << "Failed to find pages with requested content\n";
                            }
                        }
                    }
                }

            }
        }

        if(bRefresh)
        {
            clear_console();
            cout << crawler;
        }

    }

    return 0;
}

}
