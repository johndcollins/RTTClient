// ------------------------------------------------------------------------------------------------
// Copyright (c) John Collins. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------

#include "CApplication.h"

static void ShowUsage(std::string name)
{
    std::cerr << "Usage: " << name << " <option(s)>\n"
        << "Options:\n"
        << "\t-h,--help\t\tShow this help message\n"
        << "\t-d,--debug\t\tTurn on log to debug level\n"
        << std::endl;
}


int main(int argc, char* argv[])
{
    bool logDebug = false;

    if (argc > 0)
    {
        if (argc > 2)
        {
            ShowUsage(argv[0]);
            return 1;
        }
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if ((arg == "-h") || (arg == "--help")) {
                ShowUsage(argv[0]);
                return 0;
            }
            else if ((arg == "-d") || (arg == "--debug")) {
                logDebug = true;
            }
        }
    }

    Application app(logDebug);

    app.Loop();

    return 0;
}