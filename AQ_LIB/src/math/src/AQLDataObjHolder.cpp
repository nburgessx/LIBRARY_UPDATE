/*! @file
    @brief Class to store classes (such as MDBase or derived class).
		   This class is used for the registration of database class.
		   There is ownership of the pointer if mDeleteFlag is true.
		   The default mDeleteFlag is false. it is able to be selected
		   by setDeleteFlag(bool).

*/


#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "AQLDataObjHolder.h"


/*!
    @brief constructor

	@param[in] e pointer to the data get class
	@param[in] deleteFlag flag whether delete or not the data get class
*/
MDObjHolder::MDObjHolder(MDBase* e, bool deleteFlag)
:mpObj(e), mDeleteFlag(deleteFlag)
{
}
/*!
    @brief copy constructor

	@param[in] objectHolder original MDObjHolder class
*/
MDObjHolder::MDObjHolder(const MDObjHolder& objectHolder)
:mpObj(objectHolder.mpObj), mDeleteFlag(false)
{
}
/*!
    @brief destructor

*/
MDObjHolder::~MDObjHolder()
{
	if (mDeleteFlag == true)
		delete mpObj;
}
/*!
    @brief relational operator through the comparison with Name 


	@param[in] objectHolder MDObjHolder class to store data get class to compare
    @return true if it is the same as objectHolder, and false otherwise.
*/
bool                
MDObjHolder::operator==(const MDObjHolder& objectHolder) const
{
	return (mpObj->getName() == objectHolder.mpObj->getName());
}
/*!
    @brief relational operator through the comparison with Name


	@param[in] objectHolder MDObjHolder class to store data get class to compare
    @return true if this object < objectHolder true, and false otherwise
*/
bool                
MDObjHolder::operator < (const MDObjHolder& objectHolder) const
{
	return (mpObj->getName() < objectHolder.mpObj->getName());
}
/*!
    @brief set delete flag


	@param[in] b delete or not
*/
void				
MDObjHolder::setDeleteFlag(bool b)
{
	mDeleteFlag = b;
}
/*!
    @brief copy the class to get data


	@param[in] objectHolder MDObjHolder class which stores original data
    @return reference to the object
*/
MDObjHolder&   
MDObjHolder::copy(const MDObjHolder& objectHolder)
{
	if (mpObj == objectHolder.mpObj) return *this;
	if (mDeleteFlag)
		delete mpObj;

	mpObj = objectHolder.mpObj; 
	return *this;
}

