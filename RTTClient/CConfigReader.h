// ------------------------------------------------------------------------------------------------
// Copyright (c) John collins. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------

#pragma once

#include <iostream>
#include <map>

namespace RTTClient::Common
{
    class CConfigReader
    {
    private:

        std::map<std::string, std::string> m_ConfigSettingMap;
        static CConfigReader* m_pInstance;

    public:

        static CConfigReader* getInstance();

        bool parseFile(std::string fileName = "/tmp/default_config");

        bool getValue(std::string tag, int& value);
        bool getValue(std::string tag, std::string& value);
        bool getValue(std::string tag, bool& value);

        void dumpFileValues();

    private:

        CConfigReader();
        ~CConfigReader();
        CConfigReader(const CConfigReader& obj) {}

        void operator=(const CConfigReader& obj) {}

        std::string trim(const std::string& str, const std::string& whitespace = " \t");
        std::string reduce(const std::string& str,
            const std::string& fill = " ",
            const std::string& whitespace = " \t");
    };
};