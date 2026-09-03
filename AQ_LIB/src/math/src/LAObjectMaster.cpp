/*! @file
    @brief Implementation to master object classes.

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LAObjectMaster.h"
#include "LAObject.h"

using namespace std;

/*!
    @brief default constructor

	LAObject class is registered in this Object via setUpDefault()

    @param[in] dataInstance LADataInstance object that this Object Maseter belongs to
*/
LAObjectMaster::LAObjectMaster(LADataInstance* dataInstance)
:mpDataInstance(dataInstance), mEntityCollection(), mObjectCollection()
{
	setUpDefault();
}

/*!
    @brief copy constructor

    @param[in] a original object
    @param[in] dataInstance LADataInstance object that this class belongs to
*/
LAObjectMaster::LAObjectMaster(const LAObjectMaster& a, LADataInstance* dataInstance)
:mpDataInstance(dataInstance)
{
	copy(a);
}

/*!
    @brief destructor
*/
LAObjectMaster::~LAObjectMaster(void)
{
}

/*!
    @brief get Object Holder corresponding to the name of the specified Object

    if you do not set the Object, return the temporary Holder

    @param[in] name Object name

    @return Object Holder
*/
const LAObjectHolder& 
LAObjectMaster::getObject(const LAString& name) const
{
	map<LAString, LAObjectHolder>::const_iterator it = mEntityCollection.find(name);
	if (it == mEntityCollection.end())
	{
		return NULL_OBJECT_HOLDER;
	}
	return it->second;
}

/*!
    @brief get Object Holder corresponding to the id of the specified Object

    if you do not set the Object, return the temporary Holder

    @param[in] id Object ID

    @return Object Holder
*/
const LAObjectHolder& 
LAObjectMaster::getObject(object_t id) const
{
	map<int, LAObjectHolder>::const_iterator it = mObjectCollection.find(id);
	if (it == mObjectCollection.end())
	{
		return NULL_OBJECT_HOLDER;
	}
	return it->second;
}

/*!
    @brief get registered Object ID Vector

    @return registered Object ID Vector
*/
vector<object_t>		
LAObjectMaster::getTypeArray(void) const
{
	vector<object_t> ret;
	ret.reserve(mObjectCollection.size());
	map<object_t, LAObjectHolder>::const_iterator it;
	for (it = mObjectCollection.begin(); it != mObjectCollection.end(); ++it)
	{
		ret.push_back(it->first);
	}
	return ret;
}

/*!
    @brief get registered Object Name Vector

    @return registered Object Name Vector
*/
vector<LAString>	
LAObjectMaster::getNameArray(void) const
{
	vector<LAString> ret;
	ret.reserve(mEntityCollection.size());
	map<LAString, LAObjectHolder>::const_iterator it;
	for (it = mEntityCollection.begin(); it != mEntityCollection.end(); ++it)
	{
		ret.push_back(it->first);
	}
	return ret;
}

/*!
    @brief  set Object name and Object object

	Object Master holds the ownership of this Object object (memory release is done automatically)

    @param[in] name Object name
    @param[in] e Object object

    @return Object Holder 
*/
const LAObjectHolder&	
LAObjectMaster::setEntity(const LAString& name, LAObject* e)
{
	LAObjectHolder& h = mEntityCollection[name];
	if (h.isDefined())
	{
		LAString msg("Duplicate name [");
		msg += name + "]";
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
	h.setEntity(e, true);  // ownership moves to Object Holder by delete flag=true
	h.setDataInstance(mpDataInstance);
	return h;
}

/*!
    @brief  register Object

    Object Master holds the ownership of this Object object (memory release is done automatically)

    @param[in] att Object object

    @return Object Holder to register 
*/
const LAObjectHolder&	
LAObjectMaster::setEntity(LAObject* att)
{
	LAObjectHolder& h = mObjectCollection[att->getType()];
	if (h.isDefined())
	{
		LAString msg("Duplicate object_t[");
		msg += LAString(att->getType()) + "]";
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
	h.setEntity(att, true);
	h.setDataInstance(mpDataInstance);
	return h;
}

/*!
    @brief  delete Object specified from the Object Master

    @param[in] name Object name to be deleted
*/
void
LAObjectMaster::removeEntity(const LAString& name)
{
	mEntityCollection.erase(name);
}

/*!
    @brief  delete Object with the specified Object ID from the Object Master

    @param[in] id Object ID to be deleted
*/
void
LAObjectMaster::removeEntity(object_t id)
{
	mObjectCollection.erase(id);
}

/*!
    @brief  delete all Entities from Object Master

	delete all registered Entities from Object Master
*/
void				
LAObjectMaster::clear()
{
	mObjectCollection.clear();
	mEntityCollection.clear();
}

/*!
    @brief set the LADataInstance object which belongs to the Object Master

    @param[in] dataInstance the LADataInstance object to be set
*/
void
LAObjectMaster::setDataInstance(LADataInstance* dataInstance)
{
	mpDataInstance = dataInstance;
	setDataInstanceToEntity();
}

//////////////////// PRIVATE ////////////////////////////////
/*!
    @brief set the LADataInstance object to all Entities registered the Object Master
*/
void
LAObjectMaster::setDataInstanceToEntity()
{
	map<object_t, LAObjectHolder>::iterator it;
	for (it = mObjectCollection.begin(); 
			it != mObjectCollection.end(); ++it)
	{
		it->second.setDataInstance(mpDataInstance);
	}
	map<LAString, LAObjectHolder>::iterator it2;
	for (it2 = mEntityCollection.begin(); 
			it2 != mEntityCollection.end(); ++it2)
	{
		it2->second.setDataInstance(mpDataInstance);
	}
}

/*!
    @brief shallow copy of the specified Object Master

    @param[in] e  original Object Master object

    @return the copied Object Master object
*/
LAObjectMaster&
LAObjectMaster::copy(const LAObjectMaster& e)
{
	mObjectCollection = e.mObjectCollection;
	mEntityCollection = e.mEntityCollection;
	setDataInstanceToEntity();
	return *this;
}

/*!
    @brief set LAObject into the Object Master
*/
void
LAObjectMaster::setUpDefault(void)
{
	setEntity(new LAObject());
}
