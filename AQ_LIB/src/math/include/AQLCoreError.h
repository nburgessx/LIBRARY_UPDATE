#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#define char_t  char     // char_t type (modify char into wchar_t when using WideChar)
#define SPRINTF sprintf  // SPRINTF function (modify sprintf into swprintf when using WideChar) 
#define FPRINTF fprintf  // FPRINTF function (modify fprintf into fwprintf when using WideChar)
#define FPUTC   fputc    // FPUTC function (modify fputc into fputwc when using WideChar)
#define FGETC   fgetc    // FGETC function (modify fgetc into fgetwc when using WideChar)
#define STRCPY  strcpy   // STRCPY function (modify strcpy into wcscpy when using WideChar)
#define STRCMP  strcmp   // STRCMP function (modify strcmp into wcscmp when using WideChar)
#define STRCAT  strcat   // STRCAT function (modify strcat into wcscat when using WideChar)
#define STRLEN  strlen   // STRLEN function (modify strlen into wcslen when using WideChar)


#if 0
#define char_t  wchar_t 
#define SPRINTF swprintf
#define FPRINTF fwprintf
#define FPUTC   fputwc
#define FGETC   fgetwc
#define STRCPY  wcscpy
#define STRCMP  wcscmp
#define STRCAT  wcscat
#define STRLEN  wcslen
#endif


#include <cstdio>
#include <stdexcept>
#include <string>

class AQLCoreErrorInfo;

/*! 
    @brief Class declaration to express an exception in the M-Library.

	This class provides functions of adding error information or output files,
	and errors are stored in an array in the order in which they occure.
*/

// TODO: Allow AQLCoreError to inherit from std::exception on all platforms.
// With Linux / Intel Compiler we currently would have to decorate the destructor with the throw() qualifier.
#if defined(WIN32) || defined(WIN64)
class AQLCoreError : public virtual std::exception
#else
class AQLCoreError
#endif
{
public:
    // default constructor
    AQLCoreError(void);
    // constructor
    AQLCoreError(const char_t* msg, const char_t* file, unsigned int line);
    // copy constructor
    AQLCoreError(const AQLCoreError& e);
    // destructor
    virtual ~AQLCoreError(void);

    // In a Debug build, appends "[file:line]" to the message (the file/line every AQLCoreError
    // already captures at the throw site via __FILE__/__LINE__, but which never reached what() -
    // getMsg()/getFile()/getLine() exposed them individually, yet the actual catch boundary this
    // library surfaces errors through everywhere (Excel cell text, GTEST failure output, any
    // `catch(const std::exception& e) { ...e.what()... }`) only ever called what(), so file/line
    // were captured but effectively never seen). Release strips it back to the bare message - a
    // hardcoded source path/line number in a user-facing Excel cell error is not something a
    // shipped product should show. See the .cpp for why this needs a cached std::string rather
    // than just formatting inline.
    /* virtual */ const char* what() const;

    // get number of error information to be stored
    unsigned int        getSize(void) const;

    // get an error message that is stored in the (0 starts) i-th
    const char_t*       getMsg(unsigned int i=0) const;

    // get a source file in which the error occurred that is stored in the (0 starts) i-th
	const char_t*       getFile(unsigned int i=0) const;

    // get a source file line number in which the error occurred that is stored in the (0 starts) i-th
	int        getLine(unsigned int i=0) const; 

    // get a file that output error information
    static const char_t*    getLogFileName(void);

    // output error information into an error log file
    AQLCoreError&            print(void);
    const AQLCoreError&      print(void) const;
    AQLCoreError&            print(const char_t* fileName);
    const AQLCoreError&      print(const char_t* fileName) const;

    // add an error message at the beginning of the 0-th error message
    void                addMsg(const char_t* msg);

    // select log file name to print error information
    static void         setLogFile(const char_t* fileName);

    // close log file name to print error information
    static void         closeLogFile(void);

    // assignment operator
    const AQLCoreError&      operator=(const AQLCoreError& e);
    // addition assignment operator 
    const AQLCoreError&      operator+=(const AQLCoreError& e);

private:
    AQLCoreErrorInfo*        mpErrInfo;  // error information

    // Lazily-built cache backing what()'s returned const char* - what() must return a pointer
    // that stays valid after the call returns (a caller does `const char* m = e.what(); ...use m
    // later...` routinely), so it can't return a temporary std::ostringstream's .c_str(). Built on
    // first call to what(), not in the constructor - most AQLCoreError instances are thrown, caught
    // once and never have what() called at all (many call sites catch, translate to their own
    // message, and never touch it), so paying the formatting cost unconditionally would be waste.
    mutable std::string     whatCache_;
};
