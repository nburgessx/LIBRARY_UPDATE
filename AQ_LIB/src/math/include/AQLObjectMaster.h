#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLString.h"
#include "AQLObjectHolder.h"
#include "AQLCoreTemplateType.h"
#include <map>
#include <vector>

class AQLDataInstance;

typedef std::vector<object_t>		EntityTVector; // Object ID vector


/*! 
    @brief Class to master Object used in the application.
*/
class AQLObjectMaster 
{
public:
//  LIFECYCLE
    // default constructor
	AQLObjectMaster(AQLDataInstance* dataInstance = NULL);
    // copy constructor
    AQLObjectMaster(const AQLObjectMaster& a, AQLDataInstance* dataInstance = NULL);
    // destructor
    ~AQLObjectMaster(void);

//  QUERY
    // get Object Holder corresponding to the name of the specified Object
    const AQLObjectHolder&	getObject(const AQLString& name) const;

    // get Object Holder corresponding to the id of the specified Object
    const AQLObjectHolder&	getObject(object_t id) const;

    // get registered Object ID Vector
	EntityTVector			getTypeArray(void) const;

    // get registered Object Name Vector
	AQLStringVector			getNameArray(void) const;

//  OPERATION
    // set Object name and Object object
	const AQLObjectHolder&	setEntity(const AQLString& name, AQLObject* e);

    // set Object
	const AQLObjectHolder&	setEntity(AQLObject* object);

    // delete Object specified from the Object Master
	void					removeEntity(const AQLString& name);


    // delete Object with the specified Object ID from the Object Master
	void					removeEntity(object_t id);

    // delete all Entities from Object Master
    void					clear();

    // set the AQLDataInstance object which belongs to the Object Master
	void					setDataInstance(AQLDataInstance* dataInstance);

    // assignment operator
    AQLObjectMaster&			operator=(const AQLObjectMaster& e) 
							{return copy(e);}
private:
    // set the AQLDataInstance object to all entities which belongs to the Object Master
	void					setDataInstanceToEntity();

    // shallow copy of the specified Object Master
	AQLObjectMaster&			copy(const AQLObjectMaster& e);

	// set AQLObject into the Object Master
	void					setUpDefault(void);
	

	mutable AQLDataInstance*			mpDataInstance;     // pointer to the AQLDataInstance object the Object Master belongs to

	std::map<AQLString, AQLObjectHolder>	mEntityCollection; // map of Object(Holder) adminstrated from this Object Master(Object Holder name and Object Holder)
	std::map<object_t, AQLObjectHolder>	mObjectCollection; // map of Object(Holder) adminstrated from this Object Master(Object ID and Object Holder)
	AQLObjectHolder NULL_OBJECT_HOLDER;

};
