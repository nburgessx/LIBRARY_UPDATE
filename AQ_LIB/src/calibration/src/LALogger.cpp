/*! @file
    @brief Class to load txt file data
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       MALogger.cpp
//
//  DESCRIPTION :       Logger class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LALogger.h"

using namespace std;

//====================MALogger begin==============================
// constructor
/*!

*/
MALogger::MALogger(void)
{
}

// destructor
/*!

*/
MALogger::~MALogger(void)
{
}

//====================MALoggerImpl begin==============================

// logLevel is static
int MALoggerImpl::logLevel = 0;

// constructor
/*!

	file open in the constructor

	@param[in] file
	@param[in] mode

*/
MALoggerImpl::MALoggerImpl(const LAString &file, ios_base::openmode mode)
: MALogger(), mFilename(file), mFilestream(new ofstream(file.getCString(),mode)), mFilemode(mode)
{

}

// destructor
/*!


*/
MALoggerImpl::~MALoggerImpl(void)
{
	mFilestream->close();
	delete mFilestream;
}

// 
/*!
    @brief debug information output method
	
	logging method for debugging
	
	@param[in] msg
	@param[in] file
	@param[in] line
 
*/
void 
MALoggerImpl::debug(const LAString &msg, const char *file, unsigned int line)
{
	debug(msg.getCString(), file, line);
}


// 
/*!
    @brief normal information output method
	
	logging method for informing
	
	@param[in] msg
	@param[in] file
	@param[in] line
 
*/
void 
MALoggerImpl::info(const LAString &msg, const char *file, unsigned int line)
{
	info(msg.getCString(), file, line);
}


// 
/*!
    @brief error information output method
	
	logging method for error
	
	@param[in] msg
	@param[in] file
	@param[in] line
 
*/
void 
MALoggerImpl::error(const LAString &msg, const char *file, unsigned int line)
{
	error(msg.getCString(), file, line);
}


// 
/*!
    @brief debug information output method
	
	logging method for debugging
	
	@param[in] msg
	@param[in] file
	@param[in] line
 
*/
void 
MALoggerImpl::debug(const char *msg, const char *file, unsigned int line)
{
	if (logLevel >= MALogger::DEBUG_LEVEL)
	{
		writeMsg("DEBUG", msg, file, line);
	}
}

// 
/*!
    @brief normal information output method
	
	logging method for informing
	
	@param[in] msg
	@param[in] file
	@param[in] line
 
*/
void 
MALoggerImpl::info(const char *msg, const char *file, unsigned int line)
{
	if (logLevel >= MALogger::INFO_LEVEL)
	{
		writeMsg("INFO ", msg, file, line);
	}
}

// 
/*!
    @brief error information output method
	
	logging method for error
	
	@param[in] msg
	@param[in] file
	@param[in] line
 
*/
void 
MALoggerImpl::error(const char *msg, const char *file, unsigned int line)
{
	if (logLevel >= MALogger::ERROR_LEVEL)
	{
		writeMsg("ERROR", msg, file, line);
	}
}


// 
/*!
    @brief log writing method

	logging format is
	header [YYYY/MM/DD HH:MM] message [file : line]
	ex.)
	ERROR [2007/04/03 16:19] Message ERROR test [w:\m-library\libtestxlw3\mt\src\mtmain.cpp : 145]

	@param[in] header
	@param[in] msg
	@param[in] file
	@param[in] line
 
*/
void 
MALoggerImpl::writeMsg(const char *header, const char *msg, const char *file, unsigned int line)
{
	tm*     nowtime;
    time_t  local_t;
	int const TIMELEN = 21;
    char    strTime[TIMELEN] = "\0";

    if (header == 0)
    {
        header = "";
    }
    if (msg == 0)
    {
        msg = "";
    }
	if (file == 0)
    {
        file = "";
	}

	//get now time
	time(&local_t); 
	nowtime = localtime(&local_t);
    if (nowtime == 0)
    {   
        return;
    }
	sprintf(strTime, " [%04d/%02d/%02d %02d:%02d] ", nowtime->tm_year + 1900, nowtime->tm_mon + 1,
                                                   nowtime->tm_mday, nowtime->tm_hour, nowtime->tm_min);
	*mFilestream << header << strTime << msg << " [" << file << " : " << line << "]" << endl;

}


// 
/*!
    @brief close method
	
	close log file
*/
void 
MALoggerImpl::close()
{
	if (*mFilestream)
	{
		mFilestream->close();
	}
}


// 
/*!
    @brief open method
	
	open log file.
 
*/
void 
MALoggerImpl::open()
{
	if (*mFilestream)
	{
		mFilestream->close();
	}
	try
	{
		// open
		delete mFilestream;
		mFilestream = new ofstream(mFilename.getCString(), mFilemode);
	}
	catch (bad_alloc &e)
	{
		LAString msg = LAString(e.what()) + " Cannnot open log file.. File : " + mFilename;
		throw LACoreSystemError(msg.getCString(), __FILE__, __LINE__);
	}
	
	// check
	if (!*mFilestream)
	{
		LAString msg = "Cannnot open log file.. File : " + mFilename ; 
		throw LACoreSystemError(msg.getCString(), __FILE__, __LINE__);
	}

}


// 
/*!
    @brief check file is open 

	if file is open return true, else return false.
	
	@return bool 
 
*/
bool 
MALoggerImpl::is_open()
{
	return  mFilestream->is_open();
}
