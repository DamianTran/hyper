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

#include "hyper/toolkit/tree.hpp"

#include "boost/property_tree/xml_parser.hpp"
#include "boost/filesystem.hpp"

namespace pt = boost::property_tree;
namespace fs = boost::filesystem;

namespace hyperC
{

std::ostream& print_tree(std::ostream& output, const pt::ptree& tree){
    std::function<void(const std::pair<std::string, pt::ptree>&, int)> recurse_print;

    recurse_print = [&](const std::pair<std::string, pt::ptree>& node,
                         int level = 0){
        for(size_t i = 0; i < level; ++i){
            output << "-";
        }
        output << node.first << '\n';
        for(auto& sub_node : node.second){
            if(!node.second.empty()){
                recurse_print(sub_node, level + 1);
            }
        }
    };

    for(auto& node : tree){
        recurse_print(node, 0);
    }

    return output;
}

std::vector<std::string> getData(const pt::ptree& tree,
                                       const std::string& property){

    std::vector<std::string> output;

    std::function<void(const std::pair<std::string, pt::ptree>&)> recurse_branch;

    recurse_branch = [&](const std::pair<std::string, pt::ptree>& node){

        if(node.first == property){
            output.push_back(node.second.get("", ""));
        }
        for(auto& sub_node : node.second){
            if(!node.second.empty()){
                recurse_branch(sub_node);
            }
        }
    };

    for(auto& node : tree){
        recurse_branch(node);
    }

    return output;
}

}
