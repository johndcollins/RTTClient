// ------------------------------------------------------------------------------------------------
// Copyright (c) John Collins. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------

#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <errno.h>
#include <pthread.h>

namespace RTTClient::Common
{
	// Direct Interface for logging into log file or console using MACRO(s)
	#define LOG_ERROR(x)    CLogger::getInstance()->error(x)
	#define LOG_ALARM(x)	CLogger::getInstance()->alarm(x)
	#define LOG_ALWAYS(x)	CLogger::getInstance()->always(x)
	#define LOG_INFO(x)     CLogger::getInstance()->info(x)
	#define LOG_BUFFER(x)   CLogger::getInstance()->buffer(x)
	#define LOG_TRACE(x)    CLogger::getInstance()->trace(x)
	#define LOG_DEBUG(x)    CLogger::getInstance()->debug(x)

    #define LOG_BUFFER_MAX_LENGTH 512

   // enum for LOG_LEVEL
	typedef enum LOG_LEVEL
	{
		DISABLE_LOG = 1,
		LOG_LEVEL_INFO = 2,
		LOG_LEVEL_BUFFER = 3,
		LOG_LEVEL_TRACE = 4,
		LOG_LEVEL_DEBUG = 5,
		ENABLE_LOG = 6,
	} LogLevel;

	// enum for LOG_TYPE
	typedef enum LOG_TYPE
	{
		NO_LOG = 1,
		CONSOLE = 2,
		FILE_LOG = 3,
	} LogType;

	class CLogger
	{
    public:
        static CLogger* getInstance() throw ();

        // Interface for Error Log 
        void error(const char* text, ...) throw();
        void error(std::string& text) throw();
        void error(std::ostringstream& stream) throw();

        // Interface for Alarm Log 
        void alarm(const char* text, ...) throw();
        void alarm(std::string& text) throw();
        void alarm(std::ostringstream& stream) throw();

        // Interface for Always Log 
        void always(const char* text, ...) throw();
        void always(std::string& text) throw();
        void always(std::ostringstream& stream) throw();

        // Interface for Buffer Log 
        void buffer(const char* text) throw();
        void buffer(std::string& text) throw();
        void buffer(std::ostringstream& stream) throw();

        // Interface for Info Log 
        void info(const char* text, ...) throw();
        void info(std::string& text) throw();
        void info(std::ostringstream& stream) throw();

        // Interface for Trace log 
        void trace(const char* text, ...) throw();
        void trace(std::string& text) throw();
        void trace(std::ostringstream& stream) throw();

        // Interface for Debug log 
        void debug(const char* text, ...) throw();
        void debug(std::string& text) throw();
        void debug(std::ostringstream& stream) throw();

        // Error and Alarm log must be always enable
        // Hence, there is no interfce to control error and alarm logs

        // Interfaces to control log levels
        void updateLogLevel(LogLevel logLevel);
        void enaleLog();  // Enable all log levels
        void disableLog(); // Disable all log levels, except error and alarm

        // Interfaces to control log Types
        void updateLogType(LogType logType);
        void enableConsoleLogging();
        void enableFileLogging();

    protected:
        CLogger();
        ~CLogger();

        // Wrapper function for lock/unlock
        // For Extensible feature, lock and unlock should be in protected
        void lock();
        void unlock();

        std::string getCurrentTime();

    private:
        void logIntoFile(std::string& data);
        void logOnConsole(std::string& data);
        CLogger(const CLogger& obj) {}
        void operator=(const CLogger& obj) {}

    private:
        static CLogger*         m_Instance;
        std::ofstream           m_File;

        pthread_mutexattr_t     m_Attr;
        pthread_mutex_t         m_Mutex;

        LogLevel                m_LogLevel;
        LogType                 m_LogType;
    };
}