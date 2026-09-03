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

#include "LADataCSVFileLoader.h"
#include "LADataFile.h"
#include "LAMathDefine.h"
#include "LADataBasics.h"
#include "LAPriceDataManager.h"
#include "LAObjectMaster.h"
#include "LADataVector.h"
#include "LADataInstance.h"
#include <vector>

using namespace std;

//=============================================================================
/*!
    @brief constructor

    @param[in] name name of the object
*/
MDCSVFileLoader::MDCSVFileLoader(const LAString& name) 
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

    @param[in] objHolder store the path to the file to read and the path is stored in the class of LADataStrings
				  whose data is CALIBRATION_DATA_MD_FILEPATHS="FilePaths"
    @return data
*/
Records_var     
MDCSVFileLoader::get(const LAObjectHolder& objHolder) const
{
    // define Reterun value
    Records_var ret(new vector<LAObjectHolder>());
    // get File array
    const LADataHolder dh = objHolder.getData(CALIBRATION_DATA_MD_FILEPATHS);
    if (! dh.isDefined() || dh.isNull())
    {
        LAString msg = "FilePaths are not set";
        throw LACoreInvalidData(msg.getCString(),__FILE__, __LINE__);
    }
    const LADataStrings& filePaths = 
        dynamic_cast<const LADataStrings&>(dh.get());
    
    // get AttributeMaster
    LAPriceDataManager& dm = mpDataInstance->getDataMaster();
    // get EntityMaster
    LAObjectMaster& em = mpDataInstance->getObjectMaster();
    // for each file
    LAStringVector::const_iterator it;
    unsigned int i, j, k;
    for (it = filePaths.begin(); it != filePaths.end(); ++it)
    {
        MDFile file(*it, MODE_R);
        if (file.getRowCounts() == 0) continue;
        k = 0;
        for (;;)
        {
            // extract part of Header
            vector<LAString> header = file.getItems(k, ',');
            // delete blank
            for (i=0; i < header.size(); ++i)
            {
                header[i].trimLeft().trimRight();
            }
            // check Header
            if (header[0] != "object_t")
            {
                LAString msg("Invalid Format : First item is not object_t [");
                msg += *it + " ]";
                throw LACoreInvalidData(msg.getCString(),__FILE__, __LINE__);
            }
            ++k;
            // 
            for (i = k; i < file.getRowCounts(); ++i)
            {
                // specify Object from object_t
                LAString data = file.getItem(i, 0, ',');
                // 
                if (data == "") continue;
                // get Type
                int type = data.getIntValue();
                // if 0 then Header
                if (type == 0) break;

                // start read
                const LAObjectHolder& h = em.getObject(data.getIntValue());
                if (! h.isDefined())
                {
                    LAString msg("Invalid Format : Not exist object_t (");
                    msg += data + ") [";
                    msg += *it + " line-";
                    msg += LAString((int)i) + "]";
                    throw LACoreInvalidData(msg.getCString(),__FILE__, __LINE__);
                }
                // clone object
                LAObject* e = h.get().clone();
                LAString name("");
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
                    LADataHolder* hp = &(e->getData(header[j]));
                    if (! hp->isDefined())
                    {
                        const LADataHolder& att = dm.getData(header[j]);
                        hp = &(e->add(header[j], att));
                    }
                    hp->convertFromString(data);
                    if (header[j] == CALIBRATION_DATA_NAME)
                    {
                        name = dynamic_cast<LADataString&>(hp->get()).get();
                    }
                }
                //
				LAObjectHolder objHolder(e, false);
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
    @brief set the LADataInstance object

    @param[in] dataInstance pointer to RootObject
*/
void                
MDCSVFileLoader::setDataInstance(LADataInstance* dataInstance) 
{
    mpDataInstance = dataInstance;
}
