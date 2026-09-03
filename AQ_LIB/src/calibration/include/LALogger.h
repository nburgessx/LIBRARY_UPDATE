#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdio>

#include "AQLCoreSystemError.h"
#include "AQLString.h"


class MALogManager;


//===================== Class Declare MALogger==================================
/*! 
    @brief  Logger Interface class

   
*/
class MALogger
{
public:
	// constructor
	MALogger(void);
	// destructor
	virtual ~MALogger(void);
	                         //=============
	                         // debug  
	virtual void             debug(const char *msg, const char *file, unsigned int line) = 0;
		                     //==============
	                         // info  
	virtual void            info(const char *msg, const char *file, unsigned int line) = 0;
		                     //==============
	                         // error
	virtual void             error(const char *msg, const char *file, unsigned int line) = 0;
	                         //=============
	                         // debug  
	virtual void             debug(const AQLString &msg, const char *file, unsigned int line) = 0;
		                     //==============
	                         // info  
	virtual void             info(const AQLString &msg, const char *file, unsigned int line) = 0;
		                     //==============
	                         // error
	virtual void              error(const AQLString &msg, const char *file, unsigned int line) = 0;
	                         //==============
	                         // ! log close method
	virtual void              close() = 0;
	                         //==============
	                         // ! log open method
	virtual void              open() = 0;
	                         //==============
	                         // ! check log isopen
	virtual bool              is_open() = 0 ;

protected:
	// log level. The log level is specified in the property file
	enum LOG_LEVEL
	{ 
		DEBUG_LEVEL = 60, 
		INFO_LEVEL  = 40, 
		ERROR_LEVEL = 20
	};

};


//===================== Class Declare MALoggerImpl==================================
/*! 
    @brief  Logger concrete class

   
*/
class MALoggerImpl : public MALogger
{
public:
	                         //=============
	                         // debug  
	virtual void             debug(const char *msg, const char *file, unsigned int line);
		                     //==============
	                         // info  
	virtual void            info(const char *msg, const char *file, unsigned int line);
		                     //==============
	                         // error
	virtual void             error(const char *msg, const char *file, unsigned int line);
	                         //=============
	                         // debug  
	virtual void             debug(const AQLString &msg, const char *file, unsigned int line);
		                     //==============
	                         // info  
	virtual void             info(const AQLString &msg, const char *file, unsigned int line);
		                     //==============
	                         // error
	virtual void              error(const AQLString &msg, const char *file, unsigned int line);
	                         //==============
	                         // ! log close method
	virtual void              close();
	                         //==============
	                         // ! log open method
	virtual void              open();
	                         //==============
	                         // ! check log isopen
	virtual bool              is_open();

	friend class MALogManager;

private:
	// constructor
	explicit MALoggerImpl(const AQLString &file, std::ios_base::openmode mode = std::ios_base::app);
	// destructor
	virtual ~MALoggerImpl(void);
	// copy constructor
	MALoggerImpl(const MALoggerImpl &rhs);
	const MALoggerImpl &operator =(const MALoggerImpl &rhs);

	void writeMsg(const char *header, const char *msg, const char *file, unsigned int line);
	
	const AQLString mFilename;               // file name
	std::ofstream *mFilestream;             // file stream
	const std::ios_base::openmode mFilemode; // open mode
	static int logLevel;                  // log level
};
