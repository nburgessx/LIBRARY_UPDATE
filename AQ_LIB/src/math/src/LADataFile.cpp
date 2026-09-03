/*! @file
    @brief Implementation to provide an access to file.
		   It provides functions to get and record data from files.
*/


#ifdef __GNUG__
#pragma implementation
#else // VC++
#pragma warning(disable:4786)
#endif

#include "LADataFile.h"
#ifdef __GNUG__
#include <errno.h>
#endif

using namespace std;

// file open mode (read-only)
const char_t*   const readMode = "r";
// file open mode (write-only)
const char_t*   const writeMode = "w";

//=============================LIFECYCLE -==========================
/*!
    @brief constructor
*/
MDFile::MDFile() : mpFile(NULL), mModified(false)
{
}
/*!
    @brief constructor

    @param[in] fileName file name
    @param[in] openMode file open mode
*/
MDFile::MDFile(const char_t* fileName, OPENMODE openMode)
 : mpFile(NULL), mModified(false)
{
    openFile(fileName, openMode);
}
/*!
    @brief constructor

    @param[in] fileName file name
    @param[in] openMode file open mode
*/
MDFile::MDFile(const LAString& fileName, OPENMODE openMode)
 : mpFile(NULL), mModified(false)
{
    openFile(fileName, openMode);
}
/*!
    @brief destructor
*/
MDFile::~MDFile()
{
    closeFile();
}

//===========================QUERY====================
/*!
    @brief get the name of the currently open file

    @return file name
*/
const LAString&         
MDFile::getFileName() const
{
    return mFileName;
}
/*!
    @brief get the number of records in the currently open file

    @return the number of records
*/
const unsigned int          
MDFile::getRowCounts() const
{
    return mRecords.size();
}
/*!
    @brief retrieve the rec-th record

    @param[in] rec the number of the record, first record is 0 
    @return record
*/
const LAString&
MDFile::getRecord(
    unsigned int rec) const
{
    return mRecords[rec];
}
/*!
    @brief get Item[from, to] of the rec-th record
	
    @param[in] rec number of records
    @param[in] from start position of Item to get
    @param[in] to end position of Item to get
    @return Item
*/
LAString                
MDFile::getItem(
    unsigned int rec, 
    unsigned int from, 
    unsigned int to) const
{
    if (mRecords[rec].size() < from || mRecords[rec].size() < to)
    {
        return LAString("");
    }
    return mRecords[rec].subString(from, to);
}
/*!
    @brief get num-th Item of the rec-th record separated by "del"

    @param[in] rec number of records
    @param[in] num start position of Item to get
    @param[in] del delimiter
    @return Item
*/
LAString                
MDFile::getItem(
    unsigned int rec, 
    unsigned int num, 
    char_t del) const
{
    unsigned int i, j;
    unsigned int delCount;
    const char_t* str = mRecords[rec].getCString();
    bool  isInString = false;

    // search Index of first item
    for(i = 0, delCount = 0;
        i < mRecords[rec].size() && delCount < num;
        ++i)
    {
        if(str[i] == '\"')
        {
            isInString = isInString ? false : true;
        }
        if (!isInString && str[i] == del) 
        {
            ++delCount;
        }
    }
    // search Index of last item
    for (j = i; 
        j < mRecords[rec].size(); 
        ++j)
    {
        if(str[j] == '\"')
        {
            isInString = isInString ? false : true;
        }
        if (!isInString && (str[j] == del || str[j] == '\n'))
        {
            break;
        }
        // in case of NULL
        if (str[j] == '\0') 
        {
            break;
        }
    }
    // when not found, return NULL
    if (i == j) 
    {
        return LAString("");
    }
    return mRecords[rec].subString(i,j-1);
}
/*!
    @brief get number of Items of the rec-th record separated by "del"

    @param[in] rec number of records
    @param[in] del delimiter
    @return Item
*/
unsigned int            
MDFile::getItemsize(
    unsigned int rec, 
    char_t del) const
{
    unsigned int i;
    unsigned int delCount;
    const char_t* str = mRecords[rec].getCString();
    bool         isInString = false;

    for (i = 0, delCount = 0;i < mRecords[rec].size();++i)
    {
        if (str[i] == '\"')
        {
            isInString = isInString ? false : true;
        }
        // count only matching delimiter
        if (!isInString && (str[i] == del))
        {
            delCount++;
        }
        if (str[i] == '\n' || str[i] == '\0')
        {
            break;
        }
    }
    return delCount++;
}
/*!
    @brief get Items in the array of the rec-th record separated by "del"

    @param[in] rec number of records
    @param[in] del delimiter
    @return Item in the array
*/
vector<LAString>        
MDFile::getItems(
    unsigned int rec, 
    char_t del) const
{
    unsigned int i;
    const char_t* str = mRecords[rec].getCString();
    bool         isInString = false;
    LAString    item;
    vector<LAString>    itemVar;

    for (i = 0;i < mRecords[rec].size();i++)
    {
        if ( str[i] == '\"')
        {
            isInString = isInString ? true : false;
            continue;
        }
        // added to the array at the timing of the delimiter
        if (!isInString && str[i] == del)
        {
            itemVar.push_back(item);
            item = "";
            continue;
        }
        if (str[i] == '\n' || str[i] == '\0')
        {
            itemVar.push_back(item);
            break;
        }
        item += str[i];
    }
    return itemVar;
}
//=================== OPERATION ===========================
/*!
    @brief open file

    @param[in] fileName file name
    @param[in] openMode file open mode
                MODE_R: read only mode
                MODE_W: write mode
*/
void                    
MDFile::openFile(
    const char_t* fileName,
    OPENMODE openMode)
{
    closeFile();
    mModified = false;
    mFileName = fileName;
    mOpenMode = getOpenMode(openMode);
    init();
}
/*!
    @brief open file

    @param[in] fileName file name
    @param[in] openMode file open mode
                MODE_R: read only mode
                MODE_W: write mode
*/
void                    
MDFile::openFile(
    const LAString& fileName,
    OPENMODE openMode)
{
    openFile(fileName.getCString(), openMode);
}
/*!
    @brief close file

*/
void                    
MDFile::closeFile(void)
{
    flush();
    mModified = false;
    mFileName = "";
    mOpenMode = "";
    mRecords.clear();
}
/*!
    @brief write to a file the contents ofthe buffer area
*/
void
MDFile::flush(void)
{
    if (mRead)
    {
        if (mpFile != NULL)
        {
            if (fclose(mpFile))
            {
                throw LACoreSystemError(__FILE__, __LINE__);
            }
            mpFile = NULL;
        }
        return;
    }
    else if (mModified == true)
    {
        // BackUp
        LAString bkup = mFileName + ".bak";
        rename(mFileName.getCString(), bkup.getCString());
        unsigned int i;

        for (i = 0; i < mRecords.size(); ++i) 
        {
            fputs(mRecords[i].getCString(), mpFile);
        }
        if (fclose(mpFile) != 0) 
        {
            throw LACoreSystemError(__FILE__, __LINE__);
        }
        mpFile = NULL;
        mModified = false;
        remove(bkup.getCString());
        
    }
}
/*!
    @brief clear contents of the file
*/
void
MDFile::clear()
{
    if (mpFile != NULL)
    {
        fclose(mpFile);
        mpFile = NULL;
    }
    mOpenMode = getOpenMode(MODE_W);
    mpFile = fopen(mFileName.getCString(), mOpenMode.getCString());
    if (mpFile == NULL)
    {
        throw LACoreSystemError(__FILE__, __LINE__);
    }
    mRecords.clear();
    mModified = true;
}

/*!
    @brief replace rec-th record between "from" and "to" with item, number of string of item is necessary to be to-from+1

    @param[in] rec number of records
    @param[in] from start position
    @param[in] to end position
    @param[in] item Item to be replaced
*/      
void                    
MDFile::setItem(
    unsigned int rec, 
    unsigned int from, 
    unsigned int to, 
    const LAString &item)
{
    if (mRead) // if the read-only, throw an exception.
    {
        LAString    msg("File Open Mode Error"); 
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    mModified = true;
    if(item.size() != to-from+1)
    {
        throw LACoreInvalidData(item.getCString(), __FILE__, __LINE__);
    }
    mRecords[rec].replace(from, item);
}
/*!
    @brief replace num-th element with item when separated by del delimiter character

    @param[in] rec number of records
    @param[in] num number of Item to be replaced
    @param[in] del delimiter
    @param[in] item Item to be replaced
*/      
void                    
MDFile::setItem(
    unsigned int rec, 
    unsigned int num, 
    char_t del, 
    const LAString& item)
{
    unsigned int i, j;
    unsigned int delCount;
    const char_t* str = mRecords[rec].getCString();
    bool  isInString = false;

    if (mRead)  // if the read-only, throw an exception.
    {
        LAString    msg("File Open Mode Error"); 
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    mModified = true;

    // search Index of first item
    for(i = 0, delCount = 0;
        i < mRecords[rec].size() && delCount < num;
        ++i)
    {
        if(str[i] == '\"')
        {
            isInString = isInString ? false : true;
        }
        if (!isInString && str[i] == del)
        {
            ++delCount;
        }
    }
    // search Index of last item
    for (j = i; 
        j < mRecords[rec].size(); 
        ++j)
    {
        if(str[j] == '\"')
        {
            isInString = isInString ? false : true;
        }
        if (!isInString && (str[j] == del || str[j] == '\n'))
        {
            break;
        }
        if (str[j] == '\0') 
        {
            throw LACoreInvalidData(str, __FILE__, __LINE__);
        }
    }
    // return NULL when it is not found
    if (delCount < num) 
    {
        LAString msg = "Invalid index : ";
        msg += LAString(int(num));
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    if (i != j)
    {
        mRecords[rec].remove(i,j-i);
    }
    mRecords[rec].insert(i, item);
    if (mRecords[rec][mRecords[rec].size() - 1] != '\n')
    {
        mRecords[rec] += '\n';
    }
}

/*!
    @brief add the record

    @param[in] str record to be added
*/      
void                    
MDFile::addRecord(
    const LAString& str)
{
    if (mRead) // if the read-only, throw an exception.
    {
        LAString    msg("File Open Mode Error"); 
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    mModified = true;
    mRecords.push_back(str);
}
/*!
    @brief insert the record

	specify the insertion point from the first record that you specify,
	and insert a record into a previous position.
	(if you specify 0, then it is inserted just before the first record.)
    
    @param[in] before position to insert record from ths first record(0-)
    @param[in] str record to be inserted
*/  
void                    
MDFile::insertRecord(
    unsigned int before, 
    const LAString& str)
{
    if (mRead)  // if the read-only, throw an exception.
    {
        LAString    msg("File Open Mode Error"); 
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    mModified = true;
    mRecords.insert(mRecords.begin() + before, str);
}
/*!
    @brief change from READ mode into WRITE mode

*/  
void
MDFile::changeOpenMode()
{
    if (mpFile != NULL)
    {
        if (fclose(mpFile) != 0) 
        {
            throw LACoreSystemError(__FILE__, __LINE__);
        }
        mpFile = NULL;
    }
    mOpenMode = getOpenMode(MODE_W);
    mpFile = fopen(mFileName.getCString(), mOpenMode.getCString());
    if (mpFile == NULL) 
    {
        throw LACoreSystemError(__FILE__, __LINE__);
    }
}
/*!
    @brief perform the initialization

*/  
void
MDFile::init()
{
    //must be set file name in mFileName
    mRecords.clear();
    if (mpFile != NULL) 
    {
        fclose(mpFile);
    }
    mpFile = fopen(mFileName.getCString(), mOpenMode.getCString());
    if (mpFile == NULL) 
    {
        LAString msg = "File Open Error for:";
        msg+=mFileName;
        LACoreSystemError e(msg.getCString(),__FILE__, __LINE__);
        e+=LACoreSystemError(__FILE__, __LINE__);
        throw e;
    }
    for(;!feof(mpFile);)
    {
        LAString str="";
        int c;
        for(;(c=FGETC(mpFile)) != EOF && c != '\n';)
        {
            str += (char)c;
        }
        if (c == '\n') 
        {
            str += (char)c;
            mRecords.push_back(str);
        }
        else if (str.size() != 0) 
        {
            str += '\n';
            mRecords.push_back(str);
        }
        if (c == EOF) 
        {
            break;
        }
    }
}
/*!
    @brief converte a file open mode to a string

    @param[in] openMode file open mode
    @return file open mode that has been converted to a string
*/
const char_t*           
MDFile::getOpenMode(
    OPENMODE openMode)
{
    switch(openMode)
    {
        case MODE_R:
            mRead = true;
            return readMode;
        case MODE_W:
            mRead = false;
            return writeMode;
        default:
            return readMode;
    }
}
