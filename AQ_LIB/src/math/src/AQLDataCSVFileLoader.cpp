/*! @file
    @brief Implementation to get data from CSV files.

*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#ifdef __MDEBUG__
#define __MDEBUG_DUMP__
#endif

#include "AQLDataCSVFileLoader.h"
#include "AQLDataFile.h"
#include "AQLMathDefine.h"
#include "AQLDataBasics.h"
#include "AQLPriceDataManager.h"
#include "AQLObjectMaster.h"
#include "AQLDataVector.h"
#include "AQLDataInstance.h"
#include <vector>

using namespace std;

//=============================================================================
/*!
    @brief constructor

    @param[in] name name of the object
*/
MDCSVFileLoader::MDCSVFileLoader(const AQLString& name) 
:MDBase(name)
{
}
/*!
    @brief destructor
*/
MDCSVFileLoader::~MDCSVFileLoader(void)
{
}
/*!
    @brief get data from file with CSV format
           It is required to set RootObject by setDataInstance method before calling this function.

    @param[in] objHolder store the path to the file to read and the path is stored in the class of AQLDataStrings
				  whose data is CALIBRATION_DATA_MD_FILEPATHS="FilePaths"
    @return data
*/
Records_var     
MDCSVFileLoader::get(const AQLObjectHolder& objHolder) const
{
    // define Reterun value
    Records_var ret(new vector<AQLObjectHolder>());
    // get File array
    const AQLDataHolder dh = objHolder.getData(CALIBRATION_DATA_MD_FILEPATHS);
    if (! dh.isDefined() || dh.isNull())
    {
        AQLString msg = "FilePaths are not set";
        throw AQLCoreInvalidData(msg.getCString(),__FILE__, __LINE__);
    }
    const AQLDataStrings& filePaths = 
        dynamic_cast<const AQLDataStrings&>(dh.get());
    
    // get AttributeMaster
    AQLPriceDataManager& dm = mpDataInstance->getDataMaster();
    // get EntityMaster
    AQLObjectMaster& em = mpDataInstance->getObjectMaster();
    // for each file
    AQLStringVector::const_iterator it;
    unsigned int i, j, k;
    for (it = filePaths.begin(); it != filePaths.end(); ++it)
    {
        MDFile file(*it, MODE_R);
        if (file.getRowCounts() == 0) continue;
        k = 0;
        for (;;)
        {
            // extract part of Header
            vector<AQLString> header = file.getItems(k, ',');
            // delete blank
            for (i=0; i < header.size(); ++i)
            {
                header[i].trimLeft().trimRight();
            }
            // check Header
            if (header[0] != "object_t")
            {
                AQLString msg("Invalid Format : First item is not object_t [");
                msg += *it + " ]";
                throw AQLCoreInvalidData(msg.getCString(),__FILE__, __LINE__);
            }
            ++k;
            // 
            for (i = k; i < file.getRowCounts(); ++i)
            {
                // specify Object from object_t
                AQLString data = file.getItem(i, 0, ',');
                // 
                if (data == "") continue;
                // get Type
                int type = data.getIntValue();
                // if 0 then Header
                if (type == 0) break;

                // start read
                const AQLObjectHolder& h = em.getObject(data.getIntValue());
                if (! h.isDefined())
                {
                    AQLString msg("Invalid Format : Not exist object_t (");
                    msg += data + ") [";
                    msg += *it + " line-";
                    msg += AQLString((int)i) + "]";
                    throw AQLCoreInvalidData(msg.getCString(),__FILE__, __LINE__);
                }
                // clone object
                AQLObject* e = h.get().clone();
                AQLString name("");
                for (j = 1; j < header.size(); ++j)
                {
                    // process for each Data item
                    data = file.getItem(i, j, ',');
                    //
                    data.trimLeft().trimRight();
                    if(data == "")
                    {
                        continue;
                    }
                    // get Data
                    AQLDataHolder* hp = &(e->getData(header[j]));
                    if (! hp->isDefined())
                    {
                        const AQLDataHolder& att = dm.getData(header[j]);
                        hp = &(e->add(header[j], att));
                    }
                    hp->convertFromString(data);
                    if (header[j] == CALIBRATION_DATA_NAME)
                    {
                        name = dynamic_cast<AQLDataString&>(hp->get()).get();
                    }
                }
                //
				AQLObjectHolder objHolder(e, false);
                objHolder.setName(name);
                ret->push_back(objHolder);
            }
            if (i == file.getRowCounts())
            {
                break;
            }
            k = i;
        }
    }
    return ret;
}

/*!
    @brief set the AQLDataInstance object

    @param[in] dataInstance pointer to RootObject
*/
void                
MDCSVFileLoader::setDataInstance(AQLDataInstance* dataInstance) 
{
    mpDataInstance = dataInstance;
}
