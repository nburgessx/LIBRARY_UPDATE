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

class AQLCoreErrorInfo;

/*! 
    @brief Class declaration to express an exception in the M-Library.

	This class provides functions of adding error information or output files,
	and errors are stored in an array in the order in which they occure.
	This class depends only AQLCoreErrorLog.
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
//  LIFECYCLE
    // default constructor
    AQLCoreError(void);
    // constructor
    AQLCoreError(const char_t* msg, const char_t* file, unsigned int line);
    // copy constructor
    AQLCoreError(const AQLCoreError& e);
    // destructor
    virtual ~AQLCoreError(void);

	/* virtual */ const char* what() const {return getMsg();}

//  QUERY
    // get number of erro information to be stored
    unsigned int        getSize(void) const;

    // get an error message that is stored in the (0 starts) i-th
    const char_t*       getMsg(unsigned int i=0) const;

    // get a source file in which the error occurred that is stored in the (0 starts) i-th
	const char_t*       getFile(unsigned int i=0) const;

    // get a source file line number in which the error occurred that is stored in the (0 starts) i-th
	int        getLine(unsigned int i=0) const; 

    // get a file that output error information
    static const char_t*    getLogFileName(void);

    // output erro information into an erro log file
    AQLCoreError&            print(void);
    const AQLCoreError&      print(void) const;
    AQLCoreError&            print(const char_t* fileName);
    const AQLCoreError&      print(const char_t* fileName) const;

//  OPERATION
    // add an error message at the beginning of the 0-th error message
    void                addMsg(const char_t* msg);

    // select log file name to print error information
    static void         setLogFile(const char_t* fileName);

    // close log file name to print error information
    static void         closeLogFile(void);

//  OPERATOR
    // assignment operator
    const AQLCoreError&      operator=(const AQLCoreError& e);
    // addition assignment operator 
    const AQLCoreError&      operator+=(const AQLCoreError& e);

private:
    AQLCoreErrorInfo*        mpErrInfo;  // error information
};
