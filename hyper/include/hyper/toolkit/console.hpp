#pragma once

#ifndef TOOLKIT_CONSOLE
#define TOOLKIT_CONSOLE

#include <stdio.h>
#include <iostream>
#include <map>
#include <string>

#if defined WIN32 || defined _WIN32 || defined __WIN32
#include <windows.h>
#endif

namespace hyper
{

typedef std::map<std::string, std::string> arg_map;

void clear_console(char fill = ' ');
void get_args(const int& argc, char* args[], arg_map& output);
bool check_arg(const std::string& arg, const arg_map& args);

}

#endif // TOOLKIT_CONSOLE
