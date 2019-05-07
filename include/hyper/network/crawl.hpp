#pragma once

#ifndef hyper_CRAWL
#define hyper_CRAWL

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

#include "hyper/network/webio.hpp"
#include "hyper/toolkit/html.hpp"

namespace hyper
{

/** =====================================================================

    Web crawler class for easy automated web navigation

    ---

    Initialize the web crawler blank, and then use the go functions
    Or initialize the crawler on a URL and navigate from indexed links

    The web crawler automatically keeps track of its navigation
    history and supports browser-like backward and forward functions

    The web crawler is generally a no-throw class, and instead allows
    runtime read checks by using the read_success() function. The crawler
    will either navigate to the site upon a successful GET query, or
    return FALSE from the associated go function

===================================================================== */

class Spider
{
public:

    Spider();
    Spider(const std::string& URL);

    // Lower-level functions

    bool go_to(const std::string& URL);
    bool go_to(const size_t& index);
    bool go_back();                                     // Move backward in the navigation history
    bool go_forward();                                  // Move forward in the navigation history
    inline bool go_home(){ return go_to(iURL); }        // Return to initial directory
    inline bool go_to_main(){ return go_to(domain); }   // Go to current domain entry point

    /** Navigate to a link on the current page with a description containing a tag
    and links containing at least one context item */

    bool go_to_named(const std::string& tag,        // The description extracted with the link
                     const std::vector<std::string>& context =
                     std::vector<std::string>());   // Contextual soft string matches within the link

    inline const bool& read_success() const noexcept{ return bSuccess; }
    bool checkHistory(const std::string& link) const noexcept;
    inline void clearHistory() noexcept{ history.clear(); }

    inline const std::string& getContent() const
    {
        if(history_index < history.size())
        {
            return history[history_index].getContent();
        }
        throw std::out_of_range("Spider: no web page loaded to get content from");
    }

    inline const std::string& getURL() const noexcept{ return cURL; }
    inline const std::string& getDomain() const noexcept{ return domain; }

    const std::string& getLink(const size_t& index) const;
    const std::string& getLinkDescription(const size_t& index) const;

    inline bool hasLinks() const noexcept
    {
        try
        {
            return !getLinks().empty();
        }catch(...)
        {
            return false;
        }
    }

    inline const std::unordered_map<std::string, std::string>& getLinks() const
    {
        if(history_index < history.size())
        {
            return history[history_index].getLinks();
        }
        throw std::out_of_range("Spider: no web page loaded to get links from");
    }

    inline const WebPage& getPage() const
    {
        if(history_index < history.size())
        {
            return history[history_index];
        }
        throw std::out_of_range("Spider: requested web page reference but no web pages are loaded");
    }

    inline std::unordered_map<std::string, std::string>::const_iterator begin() const{ return getLinks().begin(); }
    inline std::unordered_map<std::string, std::string>::const_iterator end() const{ return getLinks().end(); }

    inline void setVerbose(const bool& state = true) noexcept{ bVerbose = state; }
    inline void setStayOnDomain(const bool& state = true) noexcept{ bStayOnDomain = state; }
    inline void setIndexMedia(const bool& state = true) noexcept{ bIndexMedia = state; }
    inline void setHistorySize(const size_t& newSize) noexcept{ max_cache_size = newSize; }

    inline void addExcludeTypes(const std::vector<std::string>& fileTypes) noexcept;

    // Higher-level functions

    /** Returns links accessible from the current URL to pages
        that contain all search terms and at least one context term. */

    bool find_pages(std::map<std::string, std::string>& output,
                    const std::vector<std::string>& search_terms,
                    const int& search_depth = -1,
                    const std::vector<std::string>& context =
                    std::vector<std::string>(),
                    std::unordered_set<std::string>* trace_path = nullptr);

    // I/O functions

    friend std::ostream& operator<<(std::ostream& output, const Spider& input);
    std::ostream& print_meta(std::ostream& output = std::cout) const;
    std::ostream& print_links(std::ostream& output = std::cout) const;

protected:

    bool bSuccess;
    bool bVerbose;
    bool bStayOnDomain;     // When crawling, only follow relative links/links matching this domain
    bool bIndexMedia;       // Keep an index of media files that the bot runs into

    std::string cURL;       // Current URL
    std::string iURL;       // Initial URL (return point)
    std::string domain;

    std::vector<WebPage> history;
    size_t history_index;
    size_t max_cache_size;  // Maximum number of web pages to store in history

    std::unordered_set<std::string> exclude_filetypes;  // File types to exclude when crawling
    std::unordered_set<std::string> media_URLs;         // Index of media files found

};

/** Wrapper for a basic console-based internet browser */
int WebCrawlProgram(int argc = 0, char** argv = nullptr);

}

#endif // hyper_CRAWL
