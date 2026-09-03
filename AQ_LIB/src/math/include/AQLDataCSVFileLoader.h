/*! @file
    @brief Class declaration to get data from CSV files.
		   The first column of the file does need to be object_t.
*/

#ifndef AQLCSVFileLoader_h
#define AQLCSVFileLoader_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLDataBase.h"

// AQLCSVFileLoader object name
#define    AQLCSVFILELOADER  "CSVFileLoader"
// file path(Data name)
#define    CALIBRATION_DATA_MD_FILEPATHS  "FilePaths"
/*! 
    @brief Class to get data from CSV files.
           
		   Item of the first column of the file needs to be object_t.
*/
class AQLCSVFileLoader : public AQLBase
{
public:
//  LIFECYCLE
    // constructor
    AQLCSVFileLoader(const AQLString& name=AQLCSVFILELOADER);
    // destructor
    virtual ~AQLCSVFileLoader(void);

//  QUERY

    // get the data
    virtual Records_var     get(const AQLObjectHolder& objHolder) const;

//  OPERATION
    // set the AQLDataInstance object
    void                setDataInstance(AQLDataInstance* dataInstance);

private:
    AQLDataInstance*             mpDataInstance;// pointer to DataInstance Object
    
};

#endif
