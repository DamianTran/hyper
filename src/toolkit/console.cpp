#include "EZC/toolkit/console.hpp"

namespace EZC
{

void clear_console(char fill)
{

    #if defined WIN32 || defined _WIN32 || defined __WIN32

    COORD tl({0,0});
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

    GetConsoleScreenBufferInfo(console, &bufferInfo);

    DWORD written, cells = bufferInfo.dwSize.X * bufferInfo.dwSize.Y;

    FillConsoleOutputCharacter(console, fill, cells, tl, &written);
    FillConsoleOutputAttribute(console, bufferInfo.wAttributes, cells, tl, &written);
    SetConsoleCursorPosition(console, tl);

    #endif

}

void get_args(const int& argc, char* argv[], std::map<std::string, std::string>& output)
{
    std::string newArg;

    for(int i = 1; i < argc; ++i)
    {

        if(argv[i][0] == '-')
        {

            if(!newArg.empty() && output[newArg].empty())
            {
                output[newArg] = "TRUE";
                newArg.assign(argv[i] + 1);

                if(i == (argc - 1))
                {
                    output[newArg] = "TRUE";
                }
            }
            else if(i == (argc - 1))
            {
                newArg.assign(argv[i] + 1);
                output[newArg] = "TRUE";
            }
            else
            {
                newArg.assign(argv[i] + 1);
            }

        }
        else
        {
            if(!newArg.empty())
            {
                if(!output[newArg].empty())
                {
                    output[newArg] += ';';
                }
                output[newArg].insert(output[newArg].size(), argv[i]);
            }
        }

    }
}

bool check_arg(const std::string& arg, const arg_map& args)
{
    try
    {
        args.at(arg);
        return true;
    }catch(...)
    {
        return false;
    }
}

}
