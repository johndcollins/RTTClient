#pragma once

#include <iostream>
#include <map>

namespace RTTClient::Common
{
    // Defining ConfigReader as singleton class
    // Easy to access across the multiple classes
    //
    // The responsibility of this class is to parse the
    // Config file and store it in the std::map
    // Defined getter function getValue() to get the
    // data from the std::map.
    //
    // To use this class, pass the config file path to
    // the function getInstance()
    //
    // This is one of the advance config reader, because this
    // class is handling the comment line as well. Comment line 
    // will start from hash(#). So all the string after
    // semicolon will be discarded.

    class CConfigReader
    {
    private:

        // Define the map to store data from the config file
        std::map<std::string, std::string> m_ConfigSettingMap;

        // Static pointer instance to make this class singleton.
        static CConfigReader* m_pInstance;

    public:

        // Public static method getInstance(). This function is
        // responsible for object creation.
        static CConfigReader* getInstance();

        // Parse the config file.
        bool parseFile(std::string fileName = "/tmp/default_config");

        // Overloaded getValue() function.
        // Value of the tag in cofiguration file could be
        // string or integer. So the caller need to take care this.
        // Caller need to call appropiate function based on the
        // data type of the value of the tag.

        bool getValue(std::string tag, int& value);
        bool getValue(std::string tag, std::string& value);
        bool getValue(std::string tag, bool& value);

        // Function dumpFileValues is for only debug purpose
        void dumpFileValues();

    private:

        // Define constructor in the private section to make this
        // class as singleton.
        CConfigReader();

        // Define destructor in private section, so no one can delete 
        // the instance of this class.
        ~CConfigReader();

        // Define copy constructor in the private section, so that no one can 
        // voilate the singleton policy of this class
        CConfigReader(const CConfigReader& obj) {}

        // Define assignment operator in the private section, so that no one can 
        // voilate the singleton policy of this class
        void operator=(const CConfigReader& obj) {}

        // Helper function to trim the tag and value. These helper function is
        // calling to trim the un-necessary spaces.
        std::string trim(const std::string& str, const std::string& whitespace = " \t");
        std::string reduce(const std::string& str,
            const std::string& fill = " ",
            const std::string& whitespace = " \t");
    };
};