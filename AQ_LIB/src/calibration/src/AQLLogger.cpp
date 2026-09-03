/*! @file
    @brief Class to load txt file data
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLLogger.cpp
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


#include "AQLLogger.h"

using namespace std;

//====================AQLLogger begin==============================
// constructor
/*!

*/
AQLLogger::AQLLogger(void)
{
}

// destructor
/*!

*/
AQLLogger::~AQLLogger(void)
{
}

//====================AQLLoggerImpl begin==============================

// logLevel is static
int AQLLoggerImpl::logLevel = 0;

// constructor
/*!

	file open in the constructor

	@param[in] file
	@param[in] mode

*/
AQLLoggerImpl::AQLLoggerImpl(const AQLString &file, ios_base::openmode mode)
: AQLLogger(), mFilename(file), mFilestream(new ofstream(file.getCString(),mode)), mFilemode(mode)
{

}

// destructor
/*!


*/
AQLLoggerImpl::~AQLLoggerImpl(void)
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
AQLLoggerImpl::debug(const AQLString &msg, const char *file, unsigned int line)
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
AQLLoggerImpl::info(const AQLString &msg, const char *file, unsigned int line)
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
AQLLoggerImpl::error(const AQLString &msg, const char *file, unsigned int line)
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
AQLLoggerImpl::debug(const char *msg, const char *file, unsigned int line)
{
	if (logLevel >= AQLLogger::DEBUG_LEVEL)
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
AQLLoggerImpl::info(const char *msg, const char *file, unsigned int line)
{
	if (logLevel >= AQLLogger::INFO_LEVEL)
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
AQLLoggerImpl::error(const char *msg, const char *file, unsigned int line)
{
	if (logLevel >= AQLLogger::ERROR_LEVEL)
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
AQLLoggerImpl::writeMsg(const char *header, const char *msg, const char *file, unsigned int line)
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
AQLLoggerImpl::close()
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
AQLLoggerImpl::open()
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
		AQLString msg = AQLString(e.what()) + " Cannnot open log file.. File : " + mFilename;
		throw AQLCoreSystemError(msg.getCString(), __FILE__, __LINE__);
	}
	
	// check
	if (!*mFilestream)
	{
		AQLString msg = "Cannnot open log file.. File : " + mFilename ; 
		throw AQLCoreSystemError(msg.getCString(), __FILE__, __LINE__);
	}

}


// 
/*!
    @brief check file is open 

	if file is open return true, else return false.
	
	@return bool 
 
*/
bool 
AQLLoggerImpl::is_open()
{
	return  mFilestream->is_open();
}
