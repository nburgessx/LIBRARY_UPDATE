#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLDataHolder.h"
#include "AQLObject.h"

#include <map>
#include <vector>

// Data ID vector
typedef std::vector<object_t>	AttrTVector;

/*!
    @brief Class to master dataValues.

    Class to master dataValues used in the application.
*/
class AQLPriceDataManager 
{
public:

    // default constructor
	AQLPriceDataManager(void);

	// destructor
    ~AQLPriceDataManager(void);

	// return the Data Holder that corresponds to the name of Data
    const AQLDataHolder& getData( const AQLString& name ) const;

	// return the Data Holder that corresponds to the name of Data or the alias
    const AQLDataHolder& getData( const AQLString& name, const AQLString& alias ) const;

    // return the Data Holder that corresponds to the id of Data
    const AQLDataHolder& getData( object_t id ) const;

    // return the Data Holder that corresponds to the id of Data  or the alias
    const AQLDataHolder& getData( object_t id, object_t alias_id ) const;

	// return the array of the registered Data ID
	AttrTVector			getDataArray(void) const;

    // return the array of name of the registered Data ID
	AQLStringVector		getNameArray(void) const;
 
	// register Data name and Data id
	const AQLDataHolder&	setData(const AQLString& name, object_t id);
    // register the Data
	const AQLDataHolder&	setData(AQLPriceDataType* att);

	// remove the Data with the specified the Data name
	void				removeData(const AQLString& name);

    // remove the Data with the specified the Data name
	void				removeData(object_t type);

    // clear the Data data
    void				clear();

private:
	// register default Data
	void				setUpDefault(void);

	AQLObject							mAttrCollection; // Object holding Attibute Master
	std::map<object_t, AQLDataHolder>	mObjectCollection; // pair pf Data ID and Data Holder

	// copy constructor
    // impossible to copy Master Object?
    AQLPriceDataManager(const AQLPriceDataManager& a);

	// Null Data Holder used when (Holder) has not been set yet Data corresponding to the name of the Data.
	AQLDataHolder NULL_DATA_HOLDER;

};

