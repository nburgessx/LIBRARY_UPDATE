#ifdef __GNUG__
#pragma implementation
#endif

#include "LAPriceDataType.h"
#include "LAObject.h"
#include "LADataHolder.h"

// comment out the following define if we update notification into Object automatically when we change Data 
//#define NO_AUTO_UPDATE 

/*!
    @brief constructor

    @param[in] attr Data ID
*/
LAPriceDataType::LAPriceDataType(const staticData_t attr)
			: mpHolder(NULL), mAttributeType(attr), mIsNull(true) 
{
}

/*!
    @brief copy constructor

    @param[in] attr original Data
*/
LAPriceDataType::LAPriceDataType(const LAPriceDataType& attr)
			:  mpHolder(NULL), 
				mAttributeType(attr.mAttributeType),
				mIsNull(attr.mIsNull)
{ 
}

/*!
    @brief destructor
*/
LAPriceDataType::~LAPriceDataType()
{
}

/*!
    @brief delete "(double quotes) from string in the first and end, and store the data
    
    @param[in] str string to be converted
    @param[out] data strign after converting

    @retval true if str is not defined or str is "NULL"
    @retval false otherwise

    @note if str is "NULL" then "" is stored in data
*/
bool				
LAPriceDataType::strToData(const LAString& str, LAString& data) const
{
	if (! str.isDefined() || str == NULL_STR)
	{
		data = "";
		return true; // expect that NULL will be set
	}

	if (str.size() > 2 && str[0] == '\"' && str[str.size() - 1] == '\"')
	{
		data=str.subString(1, str.size() - 2);
	}
	else
	{
		data=str;
	}
	// there is some data
	return false;
}

/*!
    @brief inform the Object Data that the Data has been updated. It is called when Data is updated.

    @param[in] type update type(TYPE_NORMAL or TYPE_BROADCAST)

    @note the selected NO_AUTO_UPDATE is neglected when set the argument by update type
*/
void
LAPriceDataType::update(int type)
{
	if (mpHolder != NULL)
		mpHolder->update(type);
}

/*!
    @brief inform the Object Data that the Data has been updated. It is called when Data is updated.

    @note nothing is informed when build is running with already defined NO_AUTO_UPDATE
*/
void
LAPriceDataType::update(void)
{
#ifndef NO_AUTO_UPDATE // no update if already Defined
	update(TYPE_NORMAL);
#endif
}

/*!
    @brief get the pointer to the Object object that holds the Data

    in case of not Object, return NULL
    
    @return the pointer to the Object object that holds the Data
*/
LAObject*
LAPriceDataType::getObject(void)
{
	if (mpHolder != NULL)
		return mpHolder->getObject();
	else
		return NULL;
}

/*!
    @brief get the pointer to the Object object that holds the Data

    in case of not Object, return NULL
    
    @return the pointer to the Object object that holds the Data
*/
const LAObject*
LAPriceDataType::getObject(void) const
{
	if (mpHolder != NULL)
		return mpHolder->getObject();
	else
		return NULL;
}

/*!
    @brief set a pointer to the Holder of the Data

   able to clear the relationship between the Data Holder by setting a NULL

    @param[in] holder Data Holder to be set
*/
void
LAPriceDataType::setHolder(LADataHolder* holder)
{
	mpHolder = holder;
}

