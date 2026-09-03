/*! @file
    @brief Class declaration to provide an access to file.
		   It provides functions to get and record data from files.
*/


#ifndef AQLFile_h
#define AQLFile_h

#ifdef __GNUG__
#pragma interface
#endif


#include "AQLCoreSystemError.h"
#include "AQLCoreAppError.h"
#include "AQLString.h"
#include "AQLCoreTemplateType.h"
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
class AQLFile
{
public:
//  LIFECYCLE
	// constructor
	AQLFile();
	explicit AQLFile(const char_t* fileName, OPENMODE openMode);
	explicit AQLFile(const AQLString& fileName, OPENMODE openMode);
	// destructor
	virtual ~AQLFile();

//  QUERYS
	// check whether it is opened
	bool					isDefined(void) {return mpFile ? true : false;}
	// check whether it is changed
	bool					isModified(void) {return mModified;}
	// check whether the read mode
	bool					isReadMode(void) {return mRead;}
	// get the name of the currently open file
	const AQLString&			getFileName() const;
	// get the number of records in the currently open file
	const unsigned int		getRowCounts() const;
	// retrieve the rec-th record
	const AQLString&			getRecord(unsigned int rec) const;
	// get Item[from, to] of the rec-th record
	AQLString				getItem(unsigned int rec, unsigned int from, 
									unsigned int to) const;
	// get num-th Item of the rec-th record separated by "del"
	AQLString				getItem(unsigned int rec, unsigned int num, 
									char_t del) const;
	// get number of Items of the rec-th record separated by "del"
	unsigned int			getItemsize(unsigned int rec, char_t del) const;
	// get Items in the array of the rec-th record separated by "del"
	AQLStringVector			getItems(unsigned int rec, char_t del) const;

//  OPERATION
	// open file
	void					openFile(const char_t* fileName, 
									 OPENMODE openMode);
	void					openFile(const AQLString& fileName,
									 OPENMODE openMode);
	// close file, and write buffer space into the file when there is any change
	void					closeFile(void);
	// write to a file the contents ofthe buffer area
	void                    flush(void);
	// clear the contents of the file
	void                    clear(void);
	// replace rec-th record between "from" and "to" with item, number of string of item is necessary to be to-from+1
	void					setItem(unsigned int rec, unsigned int from, 
									unsigned int to, const AQLString &item);

	// replace num-th element with item when separated by del delimiter character
	void					setItem(unsigned int rec, unsigned int num, 
									char_t del, const AQLString& item);
	// add the record
	void                    addRecord(const AQLString& str);
	// insert the record
	void					insertRecord(unsigned int before, 
										 const AQLString& str);
	// change from READ mode into WRITE mode
	void					changeOpenMode();

private:
	// perform the initialization
	void                    init(void);
	// converte a file open mode to a string
	const char_t*			getOpenMode(OPENMODE openMode);

	AQLString				mFileName;// file name
	AQLString				mOpenMode;// file open mode
	FILE*					mpFile;// file pointer
	AQLStringVector			mRecords;// records
	bool					mModified;// modified flag
	bool					mRead;// read flag

//member functions which are not available
	AQLFile(const AQLFile& file);
	AQLFile& operator=(const AQLFile& file);
};

#endif
