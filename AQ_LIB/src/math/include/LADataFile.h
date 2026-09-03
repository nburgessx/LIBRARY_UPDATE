/*! @file
    @brief Class declaration to provide an access to file.
		   It provides functions to get and record data from files.
*/


#ifndef MDFile_h
#define MDFile_h

#ifdef __GNUG__
#pragma interface
#endif


#include "LACoreSystemError.h"
#include "LACoreAppError.h"
#include "LAString.h"
#include "LACoreTemplateType.h"
#include <cstdio>



enum OPENMODE// file open mode
{
	MODE_R,// read mode
	MODE_W// write mode
};


/*! 
    @brief Class to provide an access to file.
		   It provides functions to get and record data from files.	
*/
class MDFile
{
public:
//  LIFECYCLE
	// constructor
	MDFile();
	explicit MDFile(const char_t* fileName, OPENMODE openMode);
	explicit MDFile(const LAString& fileName, OPENMODE openMode);
	// destructor
	virtual ~MDFile();

//  QUERYS
	// check whether it is opened
	bool					isDefined(void) {return mpFile ? true : false;}
	// check whether it is changed
	bool					isModified(void) {return mModified;}
	// check whether the read mode
	bool					isReadMode(void) {return mRead;}
	// get the name of the currently open file
	const LAString&			getFileName() const;
	// get the number of records in the currently open file
	const unsigned int		getRowCounts() const;
	// retrieve the rec-th record
	const LAString&			getRecord(unsigned int rec) const;
	// get Item[from, to] of the rec-th record
	LAString				getItem(unsigned int rec, unsigned int from, 
									unsigned int to) const;
	// get num-th Item of the rec-th record separated by "del"
	LAString				getItem(unsigned int rec, unsigned int num, 
									char_t del) const;
	// get number of Items of the rec-th record separated by "del"
	unsigned int			getItemsize(unsigned int rec, char_t del) const;
	// get Items in the array of the rec-th record separated by "del"
	LAStringVector			getItems(unsigned int rec, char_t del) const;

//  OPERATION
	// open file
	void					openFile(const char_t* fileName, 
									 OPENMODE openMode);
	void					openFile(const LAString& fileName,
									 OPENMODE openMode);
	// close file, and write buffer space into the file when there is any change
	void					closeFile(void);
	// write to a file the contents ofthe buffer area
	void                    flush(void);
	// clear the contents of the file
	void                    clear(void);
	// replace rec-th record between "from" and "to" with item, number of string of item is necessary to be to-from+1
	void					setItem(unsigned int rec, unsigned int from, 
									unsigned int to, const LAString &item);

	// replace num-th element with item when separated by del delimiter character
	void					setItem(unsigned int rec, unsigned int num, 
									char_t del, const LAString& item);
	// add the record
	void                    addRecord(const LAString& str);
	// insert the record
	void					insertRecord(unsigned int before, 
										 const LAString& str);
	// change from READ mode into WRITE mode
	void					changeOpenMode();

private:
	// perform the initialization
	void                    init(void);
	// converte a file open mode to a string
	const char_t*			getOpenMode(OPENMODE openMode);

	LAString				mFileName;// file name
	LAString				mOpenMode;// file open mode
	FILE*					mpFile;// file pointer
	LAStringVector			mRecords;// records
	bool					mModified;// modified flag
	bool					mRead;// read flag

//member functions which are not available
	MDFile(const MDFile& file);
	MDFile& operator=(const MDFile& file);
};

#endif
