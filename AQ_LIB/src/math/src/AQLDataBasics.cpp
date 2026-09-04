/*! @file
    @brief Implementation of the basic data class.

    Implement the following atrribute classes(AQLDataBool, AQLDataInt, AQLDataDouble, AQLDataString,
    AQLDataDate, AQLDataDateTime)
*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLDataBasics.h"

using namespace std;

//============ AQLDataBool  =============================
/*!
    @brief default constructor
*/
AQLDataBool::AQLDataBool(void) : 
AQLPriceDataType(DATA_BOOL), mData(true)
{
}

/*!
    @brief copy constructor

    @param[in] b original object
*/
AQLDataBool::AQLDataBool(const AQLDataBool& b) 
				: AQLPriceDataType(b), mData(b.mData)
{
}

/*!
    @brief constructor

    @param[in] b boolian data to be set
*/
AQLDataBool::AQLDataBool(bool b) 
				: AQLPriceDataType(DATA_BOOL), mData(b)
{
	setNull(false);
}

/*!
    @brief destructor
*/
AQLDataBool::~AQLDataBool(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

	@note mpHolder which is one of member in coping object (base class, AQLPriceDataType class) 
	is not copied, and Null is set


*/
AQLPriceDataType*	
AQLDataBool::clone() const
{
    try {
        AQLDataBool*	pAttr = new AQLDataBool(*this);
    	return pAttr;
    }
    catch (bad_alloc & e){
		throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
	}
}

/*!
    @brief  // convert boolian data of the data into string format


	@return string type of AQLString
    @note if the data is NULL, it returns the string "NULL" and if true/false then, return "TRUE","FALSE"

*/
AQLString      
AQLDataBool::convertToString(void) const
{
	return isNull() ? NULL_STR : (mData ? "TRUE": "FALSE");
}

/*!
    @brief set boolian data from a specific string format

	true and false data is set by selecting "TRUE" or "FALSE"(allowded to include lower-case character)
	if selected "NULL", then data becomes NULL, and other string raises an exception

    @param[in] str select "TRUE" or "FALSE"
*/
void          
AQLDataBool::convertFromString(const AQLString& str)
{
	AQLString data;
	bool ret = strToData(str, data); // true if str is "NULL" or ""

	// notify of the change
	update();
	if (ret || data.size() == 0) 
	{
		setNull();
	}
	else
	{
		setNull(false);
		data.toUpper();
		if (data == "TRUE")
		{
			mData = true;
		}
		else if (data == "FALSE") 
		{
			mData = false;
		}
		else 
		{
			setNull();
			// exception
			AQLString err = "InvalidData for AQLDataBool : ";
			err += str;
			throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
		}
	}
}

/*!
    @brief set boolian data

    @param[in] b true or false
*/
void 
AQLDataBool::set(bool b)
{
	setNull(false);
	mData = b;
	update();
}

/*!
    @brief assignment operator
    
    existing data in the left-hand side is discarded

    @param[in] b true or false

    @return the object
*/
AQLDataBool&
AQLDataBool::operator=(bool b)
{
	set(b); // Method
	return *this;
}

/*!
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
AQLPriceDataType&
AQLDataBool::assignment(const AQLPriceDataType& a)
{
	// not copy when a == this
	if (this == &a) return *this;

	if (a.getType() != DATA_BOOL) 
	{	// if something is wrong with type check
		// exception
		AQLString err = "Assignment error for AQLDataBool : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

	// copy
	mData = dynamic_cast<const AQLDataBool&>(a).mData;
	setNull(a.isNull());
	// notify of the change
	update();
	return *this;
}

/*!
    @brief compare the contents against the other objecet

    @param[in] a the object to be compared

    @retval  0 cotents of object is the same(or both are NULL)
    @retval  1 this object is true and object a is false
			   (also this object is not NULL and object a is NULL)
    @retval -1 this object is false and object a is true
			   (also this object is NULL and object a is not NULL)
*/
int
AQLDataBool::compare(const AQLPriceDataType& a) const
{
	if (a.getType() != DATA_BOOL) 
	{
		// exception
		AQLString err = "Compare error for AQLDataBool : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	if (isNull() && a.isNull()) 
	{
		return 0;
	}
	if (isNull()) 
	{
		return -1;
	}
	if (a.isNull()) 
	{
		return 1;
	}
	return mData == dynamic_cast<const AQLDataBool&>(a).mData ?
		0 : (mData == true ? 1 : -1);
}

//============ AQLDataString =============================
/*!
    @brief default constructor
*/
AQLDataString::AQLDataString(void) : 
AQLPriceDataType(DATA_STRING), mData()
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
AQLDataString::AQLDataString(const AQLDataString& attr)
: AQLPriceDataType(attr), mData()
{
	if (! isNull())
	{
		mData = attr.mData;
	}
}

/*!
    @brief constructor

    @param[in] str string data to be set
*/
AQLDataString::AQLDataString(const char_t*   str)
: AQLPriceDataType(DATA_STRING), mData(str)
{
	setNull(strToData(str, mData));
}

/*!
    @brief constructor

    @param[in] str string data to be set
*/
AQLDataString::AQLDataString(const AQLString& str)
: AQLPriceDataType(DATA_STRING), mData(str)
{
	setNull(strToData(str, mData));
}

/*!
    @brief destructor
*/
AQLDataString::~AQLDataString(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, AQLPriceDataType class) 
	is not copied, and Null is set
*/
AQLPriceDataType*
AQLDataString::clone() const
{
    try {
    	AQLDataString*	pAttr = new AQLDataString(*this);
    	return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}


/*!
    @brief convert string type data of the data into string format

    @return string type of AQLString
    @note if the data is NULL, it returns the string "NULL"
*/
AQLString
AQLDataString::convertToString(void) const
{
	return isNull() ? AQLString(NULL_STR) : 
						AQLString("\"") + mData + AQLString("\"");
}
	
/*!
    @brief set string data from a specific string format

    @param[in] str string data to be set
*/
void
AQLDataString::convertFromString(
	const AQLString& str)
{
	bool ret = strToData(str, mData);
	setNull(ret);
	// notify of the change
	update();
}

/*!
    @brief assignment operator
    
    existing data in the left-hand side is discarded

    @return the object
*/
AQLDataString&
AQLDataString::operator=(const AQLString& str)
{
	convertFromString(str); // method
	return *this;
}

/*!
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
AQLPriceDataType&
AQLDataString::assignment(const AQLPriceDataType& a)
{
	// not copy when a == this
	if (this == &a) return *this;
	if (a.getType() != DATA_STRING) 
	{
		// exception
		AQLString err = "Assignment error for AQLDataString : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	mData=dynamic_cast<const AQLDataString&>(a).mData;
	setNull(a.isNull());
	// notify of the change
	update();
	return *this;
}

/*!
    @brief compare the contents against the other objecet

    @param[in] a the object to be compared

    @retval  0 cotents of object is the same(or both are NULL)
    @retval  1 string code of this object is larger than that of object a
               (also this object is not NULL and object a is NULL)
    @retval -1 string code of this object is smaller than that of object a
               (also this object is NULL and object a is not NULL)
*/
int
AQLDataString::compare(
	const AQLPriceDataType& a) const
{
	if (a.getType() != DATA_STRING) 
	{
		// exception
		AQLString err = "Compare error for AQLDataString : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	if (isNull() && a.isNull())
	{
		return 0;
	}
	if (isNull()) 
	{
		return -1;
	}
	if (a.isNull()) 
	{
		return 1;
	}
	return mData.cmp(dynamic_cast<const AQLDataString&>(a).mData);
}

//============ AQLDataDateTime =============================
/*!
    @brief default constructor
*/
AQLDataDateTime::AQLDataDateTime(void) : AQLPriceDataType(DATA_DATETIME), mData()
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
AQLDataDateTime::AQLDataDateTime(const AQLDataDateTime& attr)
: AQLPriceDataType(attr), mData(attr.mData)
{
}

/*!
    @brief constructor

    @param[in] str string to set date time with format of "YYYYMMDD HH:MM:SS"
*/
AQLDataDateTime::AQLDataDateTime(const char_t*   str)
: AQLPriceDataType(DATA_DATETIME)
{
	AQLString data;
	bool ret = strToData(str, data);
	if (ret || data.size() == 0)
	{
		setNull();
	}
	else
	{
		mData.setDate(data.getCString());
		setNull(false);
	}
}

/*!
    @brief constructor

    @param[in] str date time object to be set given in the format "YYYYMMDD HH:MM:SS"
*/
AQLDataDateTime::AQLDataDateTime(const AQLString& str)
: AQLPriceDataType(DATA_DATETIME)
{
	AQLString data;
	bool ret = strToData(str, data);
	if (ret || data.size() == 0)
	{
		setNull();
	}
	else
	{
		mData.setDate(data.getCString());
		setNull(false);
	}
}

/*!
    @brief constructor

    @param[in] dt date time object to be set
*/
AQLDataDateTime::AQLDataDateTime(const AQLDateTime& dt)
: AQLPriceDataType(DATA_DATETIME), mData(dt)
{
	setNull(false);
}

/*!
    @brief destructor
*/
AQLDataDateTime::~AQLDataDateTime(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, AQLPriceDataType class) 
	is not copied, and Null is set
*/
AQLPriceDataType*
AQLDataDateTime::clone() const
{
    try {
        AQLDataDateTime*	pAttr = new AQLDataDateTime(*this);
    	return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief convert time data of the data into string format"YYYYMMDD HH:MM:SS"

    @return string type of AQLString
    @note if the data is NULL, it returns the string "NULL"
*/
AQLString
AQLDataDateTime::convertToString(void) const
{
	return isNull() ? NULL_STR : mData.stringWithFormat();
}

/*!
    @brief set time data from a specific string format

    @param[in] str string data to be set with format of "YYYYMMDD HH:MM:SS"
*/
void          
AQLDataDateTime::convertFromString(const AQLString& str)
{
	AQLString data;
	bool ret = strToData(str, data);
	// notify of the change
	update();
	if (ret || data.size() == 0)
	{
		setNull();
		mData=AQLDateTime();
	}
	else
	{
		mData.setDate(data.getCString());
		setNull(false);
	}
}

/*!
    @brief set date data

    @param[in] dt date time data to be set
*/
void
AQLDataDateTime::set(const AQLDateTime& dt)
{
	mData = dt;
	setNull(false);
	update();
}

/*!
    @brief assignment operator
    
    existing data in the left-hand side is discarded

    @return the object
*/
AQLDataDateTime&
AQLDataDateTime::operator=(const AQLDateTime& dt)
{
	set(dt); // Method
	return *this;
}

/*!
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
AQLPriceDataType&
AQLDataDateTime::assignment(const AQLPriceDataType& a)
{
	// not copy when a == this
	if (this == &a) return *this;

	if (a.getType() != DATA_DATETIME) 
	{
		// exception
		AQLString err = "Assignment error for AQLDataDateTime : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	mData=dynamic_cast<const AQLDataDateTime&>(a).mData;
	setNull(a.isNull());
	// notify of the change
	update();
	return *this;
}

/*!
    @brief compare the contents against the other objecet

    @param[in] a the object to be compared

    @retval  0 time is the same(or both are NULL)
    @retval  1 time of this object is later than that of object a
               (also this object is not NULL and object a is NULL)
    @retval -1 time of this object is earlier than that of object a
               (also this object is NULL and object a is not NULL)
*/
int
AQLDataDateTime::compare(const AQLPriceDataType& a) const
{
	if (a.getType() != DATA_DATETIME) 
	{
		// exception
		AQLString err = "#Error: Expecting DateTime Data, instead of this data type: ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	if (isNull() && a.isNull())
	{
		return 0;
	}
	if (isNull()) 
	{
		return -1;
	}
	if (a.isNull()) 
	{
		return 1;
	}
	return mData.cmp(dynamic_cast<const AQLDataDateTime&>(a).mData);
}

//============ AQLDataDate =============================
/*!
    @brief default constructor
*/
AQLDataDate::AQLDataDate(void) : AQLPriceDataType(DATA_DATE), mData()
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
AQLDataDate::AQLDataDate(const AQLDataDate& attr)
: AQLPriceDataType(attr), mData(attr.mData)
{
}

/*!
    @brief constructor

    @param[in] str string data to be set and should be expressed by "YYYYMMDD" format
*/
AQLDataDate::AQLDataDate(const char_t*   str)
: AQLPriceDataType(DATA_DATE)
{
	AQLString data;
	bool ret = strToData(str, data);
	if (ret || data.size() == 0)
	{
		setNull();
	}
	else
	{
		mData.setDate(data.getCString());
		setNull(false);
	}
}

/*!
    @brief constructor

    @param[in] str date object to be set given in the format "YYYYMMDD"
*/
AQLDataDate::AQLDataDate(const AQLString& str)
: AQLPriceDataType(DATA_DATE)
{
	AQLString data;
	bool ret = strToData(str, data);
	if (ret || data.size() == 0)
	{
		setNull();
	}
	else
	{
		mData.setDate(data.getCString());
		setNull(false);
	}
}

/*!
    @brief constructor

    @param[in] dt date time object to be set
*/
AQLDataDate::AQLDataDate(const AQLDate& dt)
: AQLPriceDataType(DATA_DATE), mData(dt)
{
	setNull(false);
}

/*!
    @brief destructor
*/
AQLDataDate::~AQLDataDate(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, AQLPriceDataType class) 
	is not copied, and Null is set
*/
AQLPriceDataType*
AQLDataDate::clone() const
{
    try {
    	AQLDataDate*	pAttr = new AQLDataDate(*this);
    	return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief convert date data of the data into string format "YYYYMMDD"

    @return string type of AQLString
    @note if the data is NULL, it returns the string "NULL"
*/
AQLString
AQLDataDate::convertToString(void) const
{
	return isNull() ? NULL_STR : mData.stringWithFormat();
}

/*!
    @brief set date data from a specific string format

    @param[in] str string data to be set with format of "YYYYMMDD"
*/
void          
AQLDataDate::convertFromString(const AQLString& str)
{
	AQLString data;
	bool ret = strToData(str, data);
	// notify of the change
	update();
	if (ret || data.size() == 0)
	{
		mData = AQLDate();
		setNull();
	}
	else
	{
		mData.setDate(data.getCString());
		setNull(false);
	}
}

/*!
    @brief set date data

    @param[in] dt date data to be set
*/
void
AQLDataDate::set(const AQLDate& dt)
{
	mData = dt;
	setNull(false);
	update();
}

/*!
    @brief assignment operator
    
    existing data in the left-hand side is discarded

    @return the object
*/
AQLDataDate&
AQLDataDate::operator=(const AQLDate& dt)
{
	set(dt); // Method
	return *this;
}

/*!
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
AQLPriceDataType&
AQLDataDate::assignment(const AQLPriceDataType& a)
{
	// not copy when a == this
	if (this == &a) return *this;
	if (a.getType() != DATA_DATE) 
	{
		// exception
		AQLString err = "Assignment error for AQLDataDate : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	mData=dynamic_cast<const AQLDataDate&>(a).mData;
	setNull(a.isNull());
	// notify of the change
	update();
	return *this;
}

/*!
    @brief compare the contents against the other objecet

    @param[in] a the object to be compared

    @retval  0 date is the same(or both are NULL)
    @retval  1 date of this object is later than that of object a
               (also this object is not NULL and object a is NULL)
    @retval -1 date of this object is earlier than that of object a
               (also this object is NULL and object a is not NULL)
*/
int
AQLDataDate::compare(const AQLPriceDataType& a) const
{
	if (a.getType() != DATA_DATE) 
	{
		// exception
		AQLString err = "Compare error for AQLDataDate : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	if (isNull() && a.isNull())
	{
		return 0;
	}
	if (isNull()) 
	{
		return -1;
	}
	if (a.isNull()) 
	{
		return 1;
	}
	return mData.cmp(dynamic_cast<const AQLDataDate&>(a).mData);
}

//============ AQLDataInt =============================
/*!
    @brief default constructor
*/
AQLDataInt::AQLDataInt(void) : AQLPriceDataType(DATA_INT), mData(0)
{
}

/*!
    @brief copy constructor

    @param[in] b original object
*/
AQLDataInt::AQLDataInt(const AQLDataInt& b) 
				: AQLPriceDataType(b), mData(b.mData)
{
}

/*!
    @brief constructor

    @param[in] b integer data to be set
*/
AQLDataInt::AQLDataInt(int b) 
				: AQLPriceDataType(DATA_INT), mData(b)
{
	setNull(false);
}

/*!
    @brief destructor
*/
AQLDataInt::~AQLDataInt(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, AQLPriceDataType class) 
	is not copied, and Null is set
*/
AQLPriceDataType*	
AQLDataInt::clone() const
{
    try {
        AQLDataInt*	pAttr = new AQLDataInt(*this);
    	return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief convert integer data of the data into string format

    @return string type of AQLString
    @note if the data is NULL, it returns the string "NULL"
*/
AQLString      
AQLDataInt::convertToString(void) const
{
	return isNull() ? NULL_STR : AQLString(mData);
}

/*!
    @brief set integer data from a specific string format

    @param[in] str string data to be set
*/void          
AQLDataInt::convertFromString(const AQLString& str)
{
	AQLString data;
	bool ret = strToData(str, data);
	// notify of the change
	update();
	if (ret || data.size() == 0) 
	{
		mData = 0;
		setNull(true);
	}
	else
	{
		mData = data.getIntValue(); 
		setNull(false);
	}
}

/*!
    @brief set integer data

    @param[in] b integer data to be set
*/
void 
AQLDataInt::set(int b)
{
	setNull(false);
	mData = b;
	update();
}

/*!
    @brief assignment operator
    
    existing data in the left-hand side is discarded

    @return the object
*/
AQLDataInt&
AQLDataInt::operator=(int b)
{
	set(b); // Method
	return *this;
}

/*!
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
AQLPriceDataType&
AQLDataInt::assignment(const AQLPriceDataType& a)
{
	// not copy when a == this
	if (this == &a) return *this;
	if (a.getType() != DATA_INT) 
	{	// if something is wrong with type check
		// exception
		AQLString err = "Assignment error for AQLDataInt : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	// copy
	mData = dynamic_cast<const AQLDataInt&>(a).mData;
	setNull(a.isNull());
	// notify of the change
	update();
	return *this;
}

/*!
    @brief compare the contents against the other objecet

    @param[in] a the object to be compared

    @return  (integer value of this object) - (integer value of object a)

    @retval  0 integer value is the same(or both are NULL)
    @retval  1 this object is not NULL and object a is NULL
    @retval -1 this object is NULL and object a is not NULL
*/
int
AQLDataInt::compare(const AQLPriceDataType& a) const
{
	if (a.getType() != DATA_INT) 
	{
		// exception
		AQLString err = "Compare error for AQLDataInt : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	if (isNull() && a.isNull()) 
	{
		return 0;
	}
	if (isNull()) 
	{
		return -1;
	}
	if (a.isNull()) 
	{
		return 1;
	}
	return mData - dynamic_cast<const AQLDataInt&>(a).mData;
}

//============ AQLDataDouble =============================
/*!
    @brief default constructor
*/
AQLDataDouble::AQLDataDouble(void) : 
			AQLPriceDataType(DATA_DOUBLE), mPrecision(DOUBLE_PRECISION) 
{
}

/*!
    @brief copy constructor

    @param[in] d original object
*/
AQLDataDouble::AQLDataDouble(const AQLDataDouble& d) : 
			AQLPriceDataType(d), mPrecision(d.mPrecision) 
{ 
	mData = d.mData;
}

/*!
    @brief constructor

    @param[in] d  double data to be set
    @param[in] pre number of decimal places to be set
*/
AQLDataDouble::AQLDataDouble(double d, int pre) : 
			AQLPriceDataType(DATA_DOUBLE), mPrecision(pre) 
{ 
	setNull(false);
	mData = d;
}

/*!
    @brief destructor
*/
AQLDataDouble::~AQLDataDouble(void) 
{ 
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, AQLPriceDataType class) 
	is not copied, and Null is set
*/
AQLPriceDataType*
AQLDataDouble::clone() const
{
    try {
        AQLDataDouble*	pAttr = new AQLDataDouble(*this);
    	return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief convert double data of the data into string format

    @return string type of AQLString
    @note if the data is NULL, it returns the string "NULL"
*/
AQLString
AQLDataDouble::convertToString(void) const
{
     return isNull() ? NULL_STR : AQLString(mData, mPrecision);
}

/*!
    @brief set double data from a specific string format

    @param[in] str string data to be set
*/
void
AQLDataDouble::convertFromString(
	const AQLString& str)
{
	AQLString data;
	bool ret = strToData(str, data);
	// notify of the change
	update();
	if (ret || data.size() == 0) 
	{
		mData = 0.0;
		setNull(true);
	}
	else
	{
		mData = data.getDoubleValue(); 
		setNull(false);
	}
}

/*!
    @brief set double data

    @param[in] d double data to be set
*/
void
AQLDataDouble::set(double d)
{
	mData = d;
	setNull(false);
	update();
}

/*!
    @brief assignment operator
    
    existing data in the left-hand side is discarded

    @return the object
*/
AQLDataDouble&
AQLDataDouble::operator = (double b)
{
	set(b); // Method
	return *this;
}

/*!
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
AQLPriceDataType&
AQLDataDouble::assignment(const AQLPriceDataType& a)
{
	// not copy when a == this
	if (this == &a) return *this;
	if (a.getType() != DATA_DOUBLE)
	{
		// exception
		AQLString err = "Assignment error for AQLDataDouble : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	const AQLDataDouble& flt = dynamic_cast<const AQLDataDouble&>(a);
	mData = flt.mData;
	mPrecision = flt.mPrecision;
	setNull(a.isNull());
	update();
	return *this;
}

/*!
    @brief compare the contents against the other objecet

    @param[in] a the object to be compared

    @retval  0 double value is the same(or both are NULL)
    @retval  1 double value of this object is larger than that of object a
               (also this object is not NULL and object a is NULL)
    @retval -1 double value of this object is smaller than that of object a
               (also this object is NULL and object a is not NULL)
*/
int
AQLDataDouble::compare(const AQLPriceDataType& a) const
{
	if (a.getType() != DATA_DOUBLE) 
	{
		// exception
		AQLString err = "Compare error for AQLDataDouble : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	if (isNull() && a.isNull()) 
	{
		return 0;
	}
	if (isNull()) 
	{
		return -1;
	}
	if (a.isNull()) 
	{
		return 1;
	}

	double ret = mData - dynamic_cast<const AQLDataDouble&>(a).mData;
	if (ret == 0)
	{
		return 0;
	}
	else
	{
		return ret > 0 ? 1 : -1;
	}
}
