/*! @file
    @brief Implementation to express an exception in the M-Library.

    This class provides functions of adding error information or output files,
	and errors are stored in an array in the order in which they occur.
*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLCoreError.h"
#include <cstring>
#include <vector>
#include <ctime>
#include <cstdio>
#include <sstream>

using namespace std;

#define STDERR (char*)"STDERR"             // default output file

#define TIMELEN             (20+1)  // string representing time

/*!
    @brief Implementation to store error information.

*/
class AQLCoreErrorInfo
{
public:
    /*!
        @brief default constructor
    */
    AQLCoreErrorInfo() {}

    /*!
        @brief copy constructor
        @param[in] e original object
    */
    AQLCoreErrorInfo(const AQLCoreErrorInfo& e)
    {  // copy
        copy(e);
    }

    /*!
        @brief destructor
    */
    ~AQLCoreErrorInfo(void)
    {
        clear();
    }

    /*!
        @brief assignment operator
        @param[in] e assigned object
    */
    AQLCoreErrorInfo& operator=(const AQLCoreErrorInfo& e)
    {
        clear();
        copy(e);
        return *this;
    }

    /*!
        @brief addition assignment operator
        @param[in] e assigned object
    */
    AQLCoreErrorInfo& operator+=(const AQLCoreErrorInfo& e)
    {
        copy(e);
        return *this;
    }

    /*!
        @brief clear all the error information

		 clear the error message, file name and file line number information
    */
    void clear(void)
    {
        for (unsigned int i = 0; i < mMsgs.size(); ++i)
        {
            delete [] mMsgs[i];
            delete [] mFiles[i];
        }
        mMsgs.clear();
        mFiles.clear();
        mLines.clear();
    }

    /*!
        @brief copy error information
        @param[in] e original object
    */
    void copy(const AQLCoreErrorInfo& e)
    {
        char_t*     msgSet;
        char_t*     fileSet;
        for (unsigned int i = 0; i < e.mMsgs.size(); ++i)
        {
            try {
                msgSet = new char_t[STRLEN(e.mMsgs[i])+1];
            }
            catch (bad_alloc & e){
                return; // you can not allocate memory without copying back
            }

            try {
                fileSet = new char_t[STRLEN(e.mFiles[i])+1];
            }
            catch (bad_alloc & e){
                delete[] msgSet;
                return; // you can not allocate memory without copying back
            }

            try {
                strcpy(msgSet, e.mMsgs[i]);
                strcpy(fileSet, e.mFiles[i]);
                // set file name the error occurred
                mFiles.push_back(fileSet);
                // set Line number the error occurred
                mLines.push_back(e.mLines[i]);
                // set mMsgs finally
                mMsgs.push_back(msgSet);
            }
            catch (...)
            {
                return;
            }
        }
    }

    static FILE*      mspFile;     // pointer to FILE structure of the output file error message (the default is STDERR)

    static bool       msIsDefault; // whether or not leave the default (msFileName and mspFile) setting of the output file

    static char_t*    msFileName;  // out outfile name(the default is STDERR)

    vector<char_t* >  mMsgs;       // error message (new information is added sequentially)

    vector<char_t* >  mFiles;      // fle name that the error occurred (new information is added sequentially)

    vector<int>       mLines;      // fle line number that the error occurred (new information is added sequentially)
};

//////////////////// STATIC VARIABLESDEFINE /////////////////////////////
bool
AQLCoreErrorInfo::msIsDefault = true;    // default or not
FILE* 
AQLCoreErrorInfo::mspFile = stderr;      // default output file(STDERR)
char_t*  
AQLCoreErrorInfo::msFileName = STDERR;   // default output file name

//////////////////// STATIC FUNCTION //////////////////////////////////////
/*!
    @brief output error information in log file

    Output format of the error information is as follows. \n
    <b> [1 date and time of error] 2 error message [3 source file name : 4 line number of the source file] </b> \n

    (1) display in the format MM: YYYYMMDD HH the date and time of the error \n
    (2) display error message \n
    (3) display the source file name where the error occurred in the full path name \n
    (4) display the line number of the source file where the error occurred \n
           
	Also, if there is more than one error information, then add multi "<--" \n
	at the beginning of the message in orde to output an error message divided into a hierarchy \n

    @param[in] fp pointer to FILE structure of the output file
    @param[in] info error information to ouput
*/
static void printLog(FILE* fp, AQLCoreErrorInfo* info)
{
    // check info
    if ( info == NULL ) 
    {
        return;
    }
    // fp check
    if (fp == NULL)
    {
        fp = stderr;
    }

    // ouput to file
    unsigned int i, j;
    for (i = 0; i < info->mMsgs.size(); ++i)
    {
        for (j = 0; j < i; ++j)
        {
            fprintf(fp, "<---");
        }
        fprintf(fp, "%s[%s : %d]\n",
                info->mMsgs[i],
                info->mFiles[i],
                info->mLines[i]);
    } 
    // write contents
    fflush(fp);
}

/*!
    @brief default constructor

    Allocates an empty AQLCoreErrorInfo rather than leaving mpErrInfo NULL - operator=(),
    operator+=() and the copy constructor all dereference mpErrInfo unconditionally (`*mpErrInfo =
    *(e.mpErrInfo)`), so a NULL mpErrInfo is a latent crash waiting for a copy/assignment of a
    default-constructed AQLCoreError. Observable behaviour for every existing accessor is
    unchanged - getSize() on an empty AQLCoreErrorInfo is still 0, getMsg()/getFile() still return
    "", exactly as the old NULL-check branches did - this only removes the unchecked-dereference
    path, it does not change what any caller sees.
*/
AQLCoreError::AQLCoreError() : mpErrInfo(new AQLCoreErrorInfo())
{
}

/*!
    @brief constructor

    set error data

    @param[in] msg error message
    @param[in] file file name that error occurs
    @param[in] line line number of the file that error occurs

    \code
    AQLCoreError e("Error!!!", __FILE__, __LINE__);
    \endcode
    
*/
AQLCoreError::AQLCoreError(const char_t*   msg, const char_t*   file, unsigned int line)
: mpErrInfo(NULL)
{
    // check argument
    if (msg == NULL)
    {
        msg = "";
    }
    if (file == NULL)
    {
        file = "";
    }

    // memory allocation for a set of error information
    char_t*   msgSet = NULL;
    char_t*   fileSet = NULL;
    try {
        mpErrInfo = new AQLCoreErrorInfo();
        msgSet = new char_t[STRLEN(msg)+1];
        fileSet = new char_t[STRLEN(file)+1];

		strcpy(msgSet, msg);
        strcpy(fileSet, file);
        // log error set
        mpErrInfo->mMsgs.push_back(msgSet);
        mpErrInfo->mFiles.push_back(fileSet);
        mpErrInfo->mLines.push_back(line);
	} 
    catch (...)
    {   // give up the error processing
        if (msgSet != NULL) delete[] msgSet;
        if (fileSet != NULL) delete[] fileSet;
		mpErrInfo->mFiles.clear();
        mpErrInfo->mMsgs.clear();
        mpErrInfo->mLines.clear();
    }
    return;
}

/*!
    @brief copy constructor
    @param[in] e original error object
*/
AQLCoreError::AQLCoreError(const AQLCoreError& e)
    : mpErrInfo(NULL)
{
    mpErrInfo = new AQLCoreErrorInfo(*(e.mpErrInfo));
}

/*!
    @brief destructor
*/
AQLCoreError::~AQLCoreError(void)
{
    delete mpErrInfo;
}

/*!
    @brief std::exception::what() override - see the header field comment for whatCache_.

    Debug builds append the [file:line] the constructor already captured via __FILE__/__LINE__;
    Release builds return the bare message only, so a source path never reaches a shipped product's
    UI (an Excel cell error, for instance).
*/
const char*
AQLCoreError::what() const
{
#ifdef _DEBUG
    if (mpErrInfo != NULL && mpErrInfo->mMsgs.size() > 0)
    {
        std::ostringstream oss;
        oss << getMsg() << "[" << getFile() << ":" << getLine() << "]";
        whatCache_ = oss.str();
        return whatCache_.c_str();
    }
#endif
    return getMsg();
}

///////////////////////////////////////////////////////////////////////////
/*!
    @brief get number of error information to be stored
    @return number of error information
*/
unsigned int
AQLCoreError::getSize(void) const
{
    if ( mpErrInfo == NULL ) 
    {   
		return 0;
    }
    return static_cast<unsigned int>(mpErrInfo->mMsgs.size());
}

/*!
    @brief get an error message that is stored in the (0 starts) i-th
    
    i starts from 0 and i=0 means the oldest information
    if it does not exist, return a string NULL
    @param[in] i number of error message(0 start)
    @return error message
*/
const char_t*
AQLCoreError::getMsg(unsigned int i) const
{
    if ( mpErrInfo == NULL || i >= mpErrInfo->mMsgs.size())
    {
        return "";
    }
    return mpErrInfo->mMsgs[i];
}

/*!
    @brief get a source file in which the error occurred that is stored in the (0 starts) i-th
    
    i starts from 0 and i=0 means the oldest information
    if it does not exist, return a string NULL
    @param[in] i number of file name in which the error occurred(0 start)
    @return file name
*/
const char_t*
AQLCoreError::getFile(unsigned int i) const
{
    if (mpErrInfo == NULL ||  i >= mpErrInfo->mFiles.size())
    {
        return "";
    }
    return mpErrInfo->mFiles[i];
}

/*!
    @brief get a source file line number in which the error occurred that is stored in the (0 starts) i-th
    
    i starts from 0 and i=0 means the oldest information
	if it does not exist, return -1
    @param[in] i line number of file name in which the error occurred(0 start)
    @return a source file line number in which the error occurred
*/
int
AQLCoreError::getLine(unsigned int i) const
{
    if ( mpErrInfo == NULL || i >= mpErrInfo->mLines.size())
    {
        return -1;
    }
    return mpErrInfo->mLines[i];
}

/*!
    @brief get a file that output error information
    
    @return name of the file that output error information
*/
const char_t*
AQLCoreError::getLogFileName(void)
{
    return AQLCoreErrorInfo::msFileName;
}

/////////// Log output//////////////////////////////////////////////////////
/*!
    @brief output error information into an error log file
	
	Output format of the error information is as follows. \n
    <b> [1 date and time of error] 2 error message [3 source file name : 4 line number of the source file] </b> \n

    (1) display in the format MM: YYYYMMDD HH the date and time of the error \n
    (2) display error message \n
    (3) display the source file name where the error occurred in the full path name \n
    (4) display the line number of the source file where the error occurred \n
           
	Also, if there is more than one error information, then add multi "<--" \n
	at the beginning of the message in orde to output an error message divided into a hierarchy \n

    @return the object
*/
AQLCoreError&
AQLCoreError::print(void)
{
    if ( mpErrInfo != NULL ) 
    {
        printLog(mpErrInfo->mspFile, mpErrInfo);
    }
    return *this;
}

/*!
    @brief output error information into anr erro log file

    Output format of the error information is as follows. \n
    <b> [1 date and time of error] 2 error message [3 source file name : 4 line number of the source file] </b> \n

    (1) display in the format MM: YYYYMMDD HH the date and time of the error \n
    (2) display error message \n
    (3) display the source file name where the error occurred in the full path name \n
    (4) display the line number of the source file where the error occurred \n
           
	Also, if there is more than one error information, then add multi "<--" \n
	at the beginning of the message in orde to output an error message divided into a hierarchy \n

    @return the object
*/
const AQLCoreError&
AQLCoreError::print(void) const
{
    if ( mpErrInfo != NULL ) 
    {
        printLog(mpErrInfo->mspFile, mpErrInfo);
    }
    return *this;
}

/*!
    @brief output error information into a specified error log file
    
    Output format of the error information is as follows. \n
    <b> [1 date and time of error] 2 error message [3 source file name : 4 line number of the source file] </b> \n

    (1) display in the format MM: YYYYMMDD HH the date and time of the error \n
    (2) display error message \n
    (3) display the source file name where the error occurred in the full path name \n
    (4) display the line number of the source file where the error occurred \n
           
	Also, if there is more than one error information, then add multi "<--" \n
	at the beginning of the message in orde to output an error message divided into a hierarchy \n

    @param[in] fileName log file name to output

    @return the object
*/
AQLCoreError&
AQLCoreError::print(const char_t* fileName)
{
    // FILE OPEN
    FILE* fp=NULL;
    if (fileName != NULL && STRLEN(fileName) != 0)
    {
        fp = fopen(fileName, "a");
    }
    printLog(fp, mpErrInfo);
    if (fp != NULL) fclose(fp);
    return *this;
}

/*!
    @brief output error information into a specified error log file
    
    If the specified file already exists, error information will be appended to existing files.
	If it fails to open the specified file, then it is output to standard error.

    Output format of the error information is as follows. \n
    <b> [1 date and time of error] 2 error message [3 source file name : 4 line number of the source file] </b> \n

    (1) display in the format MM: YYYYMMDD HH the date and time of the error \n
    (2) display error message \n
    (3) display the source file name where the error occurred in the full path name \n
    (4) display the line number of the source file where the error occurred \n
           
	Also, if there is more than one error information, then add multi "<--" \n
	at the beginning of the message in orde to output an error message divided into a hierarchy \n

    @param[in] fileName log file name to output

    @return the object
*/
const AQLCoreError&
AQLCoreError::print(const char_t* fileName) const
{
    FILE* fp=NULL;
    if (fileName != NULL && STRLEN(fileName) != 0)
    {
        fp = fopen(fileName, "a");
    }
    printLog(fp, mpErrInfo);
    if (fp != NULL) fclose(fp);
    return *this;
}

///////////////////////////////////////////////////////////////////////////
/*!
    @brief add an error message at the beginning of the 0-th error message
    
    If the specified file already exists, error information will be appended to existing files.
	If it fails to open the specified file, then it is output to standard error.

    @param[in] msg error message to be added
*/
void
AQLCoreError::addMsg(const char_t*   msg)
{
    if (mpErrInfo ==0 || mpErrInfo->mMsgs.size() == 0 || msg == NULL)
    {
        return;
    }
    try {
        // The "+3" (1 for the null terminator, 2 spare) was always sized for a 2-character
        // separator between msg and the existing message, but nothing ever wrote one - the two
        // strings landed jammed together with no space, e.g. AQLCoreSystemError's
        // addMsg(callerMsg) after constructing from strerror(errno) produced
        // "File not foundNo such file or directory" instead of a readable
        // "File not found: No such file or directory". Filling in the ": " separator the
        // reserved bytes were already sized for - no buffer size change needed.
        char_t*   msgSet = new char_t[STRLEN(msg)+ STRLEN(mpErrInfo->mMsgs[0]) + 3 ];

        strcpy(msgSet, msg);
        strcat(msgSet, ": ");
        strcat(msgSet, mpErrInfo->mMsgs[0]);
        delete[] mpErrInfo->mMsgs[0];
        mpErrInfo->mMsgs[0] = msgSet;
    }
    catch (...)
    {
        return;
    }
}

/*!
    @brief select log file name to print error information
    
    If fail to set, the error information is output to standard error.
    @param[in] fileName log file name
*/
void
AQLCoreError::setLogFile(const char_t*   fileName)
{
    // close and defalut set
    closeLogFile();
    // return if file name is wrong;
    if (fileName == NULL || STRLEN(fileName) == 0)
    {
        return;
    }
    // file open
    AQLCoreErrorInfo::mspFile = fopen(fileName, "w");
    if (AQLCoreErrorInfo::mspFile == NULL)
    {
        AQLCoreErrorInfo::mspFile = stderr;
        return; 
    }
    try {
        AQLCoreErrorInfo::msIsDefault = false;
        AQLCoreErrorInfo::msFileName = new char_t[STRLEN(fileName)+1];

        strcpy(AQLCoreErrorInfo::msFileName, fileName);
    }
    catch (...)
    {
		closeLogFile();
    }
    return;
}

/*!
    @brief close log file name to print error information
    
    setting is returned to its default state.
*/
void
AQLCoreError::closeLogFile(void)
{
    if ( ! AQLCoreErrorInfo::msIsDefault)
    {// close file and set default
        fclose(AQLCoreErrorInfo::mspFile);
        delete [] AQLCoreErrorInfo::msFileName;
		AQLCoreErrorInfo::msFileName = STDERR;
        AQLCoreErrorInfo::mspFile = stderr;
        AQLCoreErrorInfo::msIsDefault = true;
    }
}
///////////////////////////////////////////////////////////////////////////
/*!
    @brief assignment operator
    
    existing data in the left-hand side is discarded

    @return the object
*/
const AQLCoreError&
AQLCoreError::operator=(const AQLCoreError& e)
{
    *mpErrInfo = *(e.mpErrInfo);
    return *this;
}

/*!
    @brief addition assignment operator
    
    additionally copy the right-hand side to the left-hand side

    @return the object
*/
const AQLCoreError&
AQLCoreError::operator+=(const AQLCoreError& e)
{
    *mpErrInfo += *(e.mpErrInfo);
    return *this;
}
