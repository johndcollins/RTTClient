// ------------------------------------------------------------------------------------------------
// Copyright (c) John collins. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// ------------------------------------------------------------------------------------------------

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <stdarg.h>

#include <libgen.h>
#include <unistd.h>
#include <linux/limits.h>
#include <fstream>

#include "CLogger.h"

using namespace std;
using namespace RTTClient::Common;

CLogger* CLogger::m_Instance = 0;

// Log file name. File name should be change from here only
const string logFileName = "RTTClient.log";

CLogger::CLogger()
{
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    std::string path;
    if (count != -1)
        path = dirname(result);
    else
    {
        printf("CLogger::CLogger() -- Startup : Unable to locate exe directory\n");
        exit(0);
    }

    path.append("/");
    path.append(logFileName);

    remove(logFileName.c_str());

    m_File.open(path.c_str(), ios::out | ios::app);
    m_LogLevel = LOG_LEVEL_TRACE;
    m_LogType = FILE_LOG;

    // Initialize mutex
    int ret = 0;
    ret = pthread_mutexattr_settype(&m_Attr, PTHREAD_MUTEX_ERRORCHECK_NP);
    if (ret != 0)
    {
        printf("CLogger::CLogger() -- Mutex attribute not initialize!!\n");
        exit(0);
    }
    ret = pthread_mutex_init(&m_Mutex, &m_Attr);
    if (ret != 0)
    {
        printf("CLogger::CLogger() -- Mutex not initialize!!\n");
        exit(0);
    }
}

CLogger::~CLogger()
{
    m_File.close();
    pthread_mutexattr_destroy(&m_Attr);
    pthread_mutex_destroy(&m_Mutex);
}

CLogger* CLogger::getInstance() throw ()
{
    if (m_Instance == 0)
    {
        m_Instance = new CLogger();
    }
    return m_Instance;
}

void CLogger::lock()
{
    pthread_mutex_lock(&m_Mutex);
}

void CLogger::unlock()
{
    pthread_mutex_unlock(&m_Mutex);
}

void CLogger::logIntoFile(std::string& data)
{
    lock();
    m_File << getCurrentTime() << "  " << data << endl;
    unlock();
}

void CLogger::logOnConsole(std::string& data)
{
    cout << getCurrentTime() << "  " << data << endl;
}

string CLogger::getCurrentTime()
{
    string currTime;
    //Current date/time based on current time
    time_t now = time(0);
    // Convert current time to string
    currTime.assign(ctime(&now));

    // Last charactor of currentTime is "\n", so remove it
    string currentTime = currTime.substr(0, currTime.size() - 1);
    return currentTime;
}

void CLogger::error(const char* text, ...) throw()
{
    char szLogString[LOG_BUFFER_MAX_LENGTH];
    va_list args;

    va_start(args, text);
    if (vsnprintf(szLogString, sizeof(szLogString), text, args) == -1)
        szLogString[sizeof(szLogString) - 1] = '\0';
    va_end(args);

    string data;
    data.append("[ERROR]: ");
    data.append(szLogString);

    // ERROR must be capture
    if (m_LogType == FILE_LOG)
    {
        logIntoFile(data);
    }
    else if (m_LogType == CONSOLE)
    {
        logOnConsole(data);
    }
}

// Interface for Error Log
void CLogger::error(std::string& text) throw()
{
    error(text.data());
}

void CLogger::error(std::ostringstream& stream) throw()
{
    string text = stream.str();
    error(text.data());
}

// Interface for Alarm Log 
void CLogger::alarm(const char* text, ...) throw()
{
    char szLogString[LOG_BUFFER_MAX_LENGTH];
    va_list args;

    va_start(args, text);
    if (vsnprintf(szLogString, sizeof(szLogString), text, args) == -1)
        szLogString[sizeof(szLogString) - 1] = '\0';
    va_end(args);

    string data;
    data.append("[ALARM]: ");
    data.append(szLogString);

    // ALARM must be capture
    if (m_LogType == FILE_LOG)
    {
        logIntoFile(data);
    }
    else if (m_LogType == CONSOLE)
    {
        logOnConsole(data);
    }
}

void CLogger::alarm(std::string& text) throw()
{
    alarm(text.data());
}

void CLogger::alarm(std::ostringstream& stream) throw()
{
    string text = stream.str();
    alarm(text.data());
}

// Interface for Always Log 
void CLogger::always(const char* text, ...) throw()
{
    char szLogString[LOG_BUFFER_MAX_LENGTH];
    va_list args;

    va_start(args, text);
    if (vsnprintf(szLogString, sizeof(szLogString), text, args) == -1)
        szLogString[sizeof(szLogString) - 1] = '\0';
    va_end(args);

    string data;
    data.append("[ALWAYS]: ");
    data.append(szLogString);

    // No check for ALWAYS logs
    if (m_LogType == FILE_LOG)
    {
        logIntoFile(data);
    }
    else if (m_LogType == CONSOLE)
    {
        logOnConsole(data);
    }
}

void CLogger::always(std::string& text) throw()
{
    always(text.data());
}

void CLogger::always(std::ostringstream& stream) throw()
{
    string text = stream.str();
    always(text.data());
}

// Interface for Buffer Log 
void CLogger::buffer(const char* text) throw()
{
    // Buffer is the special case. So don't add log level
    // and timestamp in the buffer message. Just log the raw bytes.
    if ((m_LogType == FILE_LOG) && (m_LogLevel >= LOG_LEVEL_BUFFER))
    {
        lock();
        m_File << text << endl;
        unlock();
    }
    else if ((m_LogType == CONSOLE) && (m_LogLevel >= LOG_LEVEL_BUFFER))
    {
        cout << text << endl;
    }
}

void CLogger::buffer(std::string& text) throw()
{
    buffer(text.data());
}

void CLogger::buffer(std::ostringstream& stream) throw()
{
    string text = stream.str();
    buffer(text.data());
}

// Interface for Info Log
void CLogger::info(const char* text, ...) throw()
{
    char szLogString[LOG_BUFFER_MAX_LENGTH];
    va_list args;

    va_start(args, text);
    if (vsnprintf(szLogString, sizeof(szLogString), text, args) == -1)
        szLogString[sizeof(szLogString) - 1] = '\0';
    va_end(args);

    string data;
    data.append("[INFO]: ");
    data.append(szLogString);

    if ((m_LogType == FILE_LOG) && (m_LogLevel >= LOG_LEVEL_INFO))
    {
        logIntoFile(data);
    }
    else if ((m_LogType == CONSOLE) && (m_LogLevel >= LOG_LEVEL_INFO))
    {
        logOnConsole(data);
    }
}

void CLogger::info(std::string& text) throw()
{
    info(text.data());
}

void CLogger::info(std::ostringstream& stream) throw()
{
    string text = stream.str();
    info(text.data());
}

// Interface for Trace Log
void CLogger::trace(const char* text, ...) throw()
{
    char szLogString[LOG_BUFFER_MAX_LENGTH];
    va_list args;

    va_start(args, text);
    if (vsnprintf(szLogString, sizeof(szLogString), text, args) == -1)
        szLogString[sizeof(szLogString) - 1] = '\0';
    va_end(args);

    string data;
    data.append("[TRACE]: ");
    data.append(szLogString);

    if ((m_LogType == FILE_LOG) && (m_LogLevel >= LOG_LEVEL_TRACE))
    {
        logIntoFile(data);
    }
    else if ((m_LogType == CONSOLE) && (m_LogLevel >= LOG_LEVEL_TRACE))
    {
        logOnConsole(data);
    }
}

void CLogger::trace(std::string& text) throw()
{
    trace(text.data());
}

void CLogger::trace(std::ostringstream& stream) throw()
{
    string text = stream.str();
    trace(text.data());
}

// Interface for Debug Log
void CLogger::debug(const char* text, ...) throw()
{
    char szLogString[LOG_BUFFER_MAX_LENGTH];
    va_list args;
    va_start(args, text);
    if (vsnprintf(szLogString, sizeof(szLogString), text, args) == -1)
        szLogString[sizeof(szLogString) - 1] = '\0';
    va_end(args);

    string data;
    data.append("[DEBUG]: ");
    data.append(szLogString);
    
    if ((m_LogType == FILE_LOG) && (m_LogLevel >= LOG_LEVEL_DEBUG))
    {
        logIntoFile(data);
    }
    else if ((m_LogType == CONSOLE) && (m_LogLevel >= LOG_LEVEL_DEBUG))
    {
        logOnConsole(data);
    }
}

void CLogger::debug(std::string& text) throw()
{
    debug(text.data());
}

void CLogger::debug(std::ostringstream& stream) throw()
{
    string text = stream.str();
    debug(text.data());
}

// Interfaces to control log levels
void CLogger::updateLogLevel(LogLevel logLevel)
{
    m_LogLevel = logLevel;
}

// Enable all log levels
void CLogger::enaleLog()
{
    m_LogLevel = ENABLE_LOG;
}

// Disable all log levels, except error and alarm
void CLogger::disableLog()
{
    m_LogLevel = DISABLE_LOG;
}

// Interfaces to control log Types
void CLogger::updateLogType(LogType logType)
{
    m_LogType = logType;
}

void CLogger::enableConsoleLogging()
{
    m_LogType = CONSOLE;
}

void CLogger::enableFileLogging()
{
    m_LogType = FILE_LOG;
}