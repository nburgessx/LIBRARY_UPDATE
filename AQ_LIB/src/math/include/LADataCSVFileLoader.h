/*! @file
    @brief Class declaration to get data from CSV files.
		   The first column of the file does need to be object_t.
*/

#ifndef MDCSVFileLoader_h
#define MDCSVFileLoader_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LADataBase.h"

// MDCSVFileLoader object name
#define    MDCSVFILELOADER  "CSVFileLoader"
// file path(Data name)
#define    CALIBRATION_DATA_MD_FILEPATHS  "FilePaths"
/*! 
    @brief Class to get data from CSV files.
           
		   Item of the first column of the file needs to be object_t.
*/
class MDCSVFileLoader : public MDBase
{
public:
//  LIFECYCLE
    // constructor
    MDCSVFileLoader(const LAString& name=MDCSVFILELOADER);
    // destructor
    virtual ~MDCSVFileLoader(void);

//  QUERY

    // get the data
    virtual Records_var     get(const LAObjectHolder& objHolder) const;

//  OPERATION
    // set the LADataInstance object
    void                setDataInstance(LADataInstance* dataInstance);

private:
    LADataInstance*             mpDataInstance;// pointer to DataInstance Object
    
};

#endif
