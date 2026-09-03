#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAString.h"
#include "LAObjectHolder.h"
#include "LACoreTemplateType.h"
#include <map>
#include <vector>

class LADataInstance;

typedef std::vector<object_t>		EntityTVector; // Object ID vector


/*! 
    @brief Class to master Object used in the application.
*/
class LAObjectMaster 
{
public:
//  LIFECYCLE
    // default constructor
	LAObjectMaster(LADataInstance* dataInstance = NULL);
    // copy constructor
    LAObjectMaster(const LAObjectMaster& a, LADataInstance* dataInstance = NULL);
    // destructor
    ~LAObjectMaster(void);

//  QUERY
    // get Object Holder corresponding to the name of the specified Object
    const LAObjectHolder&	getObject(const LAString& name) const;

    // get Object Holder corresponding to the id of the specified Object
    const LAObjectHolder&	getObject(object_t id) const;

    // get registered Object ID Vector
	EntityTVector			getTypeArray(void) const;

    // get registered Object Name Vector
	LAStringVector			getNameArray(void) const;

//  OPERATION
    // set Object name and Object object
	const LAObjectHolder&	setEntity(const LAString& name, LAObject* e);

    // set Object
	const LAObjectHolder&	setEntity(LAObject* object);

    // delete Object specified from the Object Master
	void					removeEntity(const LAString& name);


    // delete Object with the specified Object ID from the Object Master
	void					removeEntity(object_t id);

    // delete all Entities from Object Master
    void					clear();

    // set the LADataInstance object which belongs to the Object Master
	void					setDataInstance(LADataInstance* dataInstance);

    // assignment operator
    LAObjectMaster&			operator=(const LAObjectMaster& e) 
							{return copy(e);}
private:
    // set the LADataInstance object to all entities which belongs to the Object Master
	void					setDataInstanceToEntity();

    // shallow copy of the specified Object Master
	LAObjectMaster&			copy(const LAObjectMaster& e);

	// set LAObject into the Object Master
	void					setUpDefault(void);
	

	mutable LADataInstance*			mpDataInstance;     // pointer to the LADataInstance object the Object Master belongs to

	std::map<LAString, LAObjectHolder>	mEntityCollection; // map of Object(Holder) adminstrated from this Object Master(Object Holder name and Object Holder)
	std::map<object_t, LAObjectHolder>	mObjectCollection; // map of Object(Holder) adminstrated from this Object Master(Object ID and Object Holder)
	LAObjectHolder NULL_OBJECT_HOLDER;

};
