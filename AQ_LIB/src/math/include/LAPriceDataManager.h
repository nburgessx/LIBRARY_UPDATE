#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LADataHolder.h"
#include "LAObject.h"

#include <map>
#include <vector>

// Data ID vector
typedef std::vector<object_t>	AttrTVector;

/*!
    @brief Class to master dataValues.

    Class to master dataValues used in the application.
*/
class LAPriceDataManager 
{
public:

    // default constructor
	LAPriceDataManager(void);

	// destructor
    ~LAPriceDataManager(void);

	// return the Data Holder that corresponds to the name of Data
    const LADataHolder& getData( const LAString& name ) const;

	// return the Data Holder that corresponds to the name of Data or the alias
    const LADataHolder& getData( const LAString& name, const LAString& alias ) const;

    // return the Data Holder that corresponds to the id of Data
    const LADataHolder& getData( object_t id ) const;

    // return the Data Holder that corresponds to the id of Data  or the alias
    const LADataHolder& getData( object_t id, object_t alias_id ) const;

	// return the array of the registered Data ID
	AttrTVector			getDataArray(void) const;

    // return the array of name of the registered Data ID
	LAStringVector		getNameArray(void) const;
 
//  OPERATION
	// register Data name and Data id
	const LADataHolder&	setData(const LAString& name, object_t id);
    // register the Data
	const LADataHolder&	setData(LAPriceDataType* att);

	// remove the Data with the specified the Data name
	void				removeData(const LAString& name);

    // remove the Data with the specified the Data name
	void				removeData(object_t type);

    // clear the Data data
    void				clear();

private:
	// register default Data
	void				setUpDefault(void);

	LAObject							mAttrCollection; // Object holding Attibute Master
	std::map<object_t, LADataHolder>	mObjectCollection; // pair pf Data ID and Data Holder

	// copy constructor
    // impossible to copy Master ObjectÅH
    LAPriceDataManager(const LAPriceDataManager& a);

	// Null Data Holder used when (Holder) has not been set yet Data corresponding to the name of the Data.
	LADataHolder NULL_DATA_HOLDER;

};

