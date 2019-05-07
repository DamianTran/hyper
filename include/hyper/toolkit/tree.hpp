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

#ifndef EZ_TREE
#define EZ_TREE

#include "boost/property_tree/ptree.hpp"

#include <vector>
#include <string>
#include <iostream>

namespace hyperC
{

std::ostream& print_tree(std::ostream& output, const boost::property_tree::ptree& tree);
std::vector<std::string> getData(const boost::property_tree::ptree& tree,
                                       const std::string& property);

}

#endif // EZ_TREE
