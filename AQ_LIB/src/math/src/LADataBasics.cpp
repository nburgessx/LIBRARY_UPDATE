/*! @file
    @brief Implementation of the basic data class.

    Implement the following atrribute classes(LADataBool, LADataInt, LADataDouble, LADataString,
    LADataDate, LADataDateTime)

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LADataBasics.h"

using namespace std;

//============ LADataBool  =============================
/*!
    @brief default constructor
*/
LADataBool::LADataBool(void) : 
LAPriceDataType(DATA_BOOL), mData(true)
{
}

/*!
    @brief copy constructor

    @param[in] b original object
*/
LADataBool::LADataBool(const LADataBool& b) 
				: LAPriceDataType(b), mData(b.mData)
{
}

/*!
    @brief constructor

    @param[in] b boolian data to be set
*/
LADataBool::LADataBool(bool b) 
				: LAPriceDataType(DATA_BOOL), mData(b)
{
	setNull(false);
}

/*!
    @brief destructor
*/
LADataBool::~LADataBool(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

	@note mpHolder which is one of member in coping object (base class, LAPriceDataType class) 
	is not copied, and Null is set


*/
LAPriceDataType*	
LADataBool::clone() const
{
    try {
        LADataBool*	pAttr = new LADataBool(*this);
    	return pAttr;
    }
    catch (bad_alloc & e){
		throw LACoreSystemError(e.what(), __FILE__, __LINE__);
	}
}

/*!
    @brief  // convert boolian data of the data into string format


	@return string type of LAString
    @note if the data is NULL, it returns the string "NULL" and if true/false then, return "TRUE","FALSE"

*/
LAString      
LADataBool::convertToString(void) const
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
LADataBool::convertFromString(const LAString& str)
{
	LAString data;
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
			LAString err = "InvalidData for LADataBool : ";
			err += str;
			throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
		}
	}
}

/*!
    @brief set boolian data

    @param[in] b true or false
*/
void 
LADataBool::set(bool b)
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
LADataBool&
LADataBool::operator=(bool b)
{
	set(b); // Method
	return *this;
}

/*!
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
LAPriceDataType&
LADataBool::assignment(const LAPriceDataType& a)
{
	// not copy when a == this
	if (this == &a) return *this;

	if (a.getType() != DATA_BOOL) 
	{	// if something is wrong with type check
		// exception
		LAString err = "Assignment error for LADataBool : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

	// copy
	mData = dynamic_cast<const LADataBool&>(a).mData;
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
LADataBool::compare(const LAPriceDataType& a) const
{
	if (a.getType() != DATA_BOOL) 
	{
		// exception
		LAString err = "Compare error for LADataBool : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
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
	return mData == dynamic_cast<const LADataBool&>(a).mData ?
		0 : (mData == true ? 1 : -1);
}

//============ LADataString =============================
/*!
    @brief default constructor
*/
LADataString::LADataString(void) : 
LAPriceDataType(DATA_STRING), mData()
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
LADataString::LADataString(const LADataString& attr)
: LAPriceDataType(attr), mData()
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
LADataString::LADataString(const char_t*   str)
: LAPriceDataType(DATA_STRING), mData(str)
{
	setNull(strToData(str, mData));
}

/*!
    @brief constructor

    @param[in] str string data to be set
*/
LADataString::LADataString(const LAString& str)
: LAPriceDataType(DATA_STRING), mData(str)
{
	setNull(strToData(str, mData));
}

/*!
    @brief destructor
*/
LADataString::~LADataString(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, LAPriceDataType class) 
	is not copied, and Null is set
*/
LAPriceDataType*
LADataString::clone() const
{
    try {
    	LADataString*	pAttr = new LADataString(*this);
    	return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}


/*!
    @brief convert string type data of the data into string format

    @return string type of LAString
    @note if the data is NULL, it returns the string "NULL"
*/
LAString
LADataString::convertToString(void) const
{
	return isNull() ? LAString(NULL_STR) : 
						LAString("\"") + mData + LAString("\"");
}
	
/*!
    @brief set string data from a specific string format

    @param[in] str string data to be set
*/
void
LADataString::convertFromString(
	const LAString& str)
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
LADataString&
LADataString::operator=(const LAString& str)
{
	convertFromString(str); // method
	return *this;
}

/*!
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
LAPriceDataType&
LADataString::assignment(const LAPriceDataType& a)
{
	// not copy when a == this
	if (this == &a) return *this;
	if (a.getType() != DATA_STRING) 
	{
		// exception
		LAString err = "Assignment error for LADataString : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	mData=dynamic_cast<const LADataString&>(a).mData;
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
LADataString::compare(
	const LAPriceDataType& a) const
{
	if (a.getType() != DATA_STRING) 
	{
		// exception
		LAString err = "Compare error for LADataString : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
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
	return mData.cmp(dynamic_cast<const LADataString&>(a).mData);
}

//============ LADataDateTime =============================
/*!
    @brief default constructor
*/
LADataDateTime::LADataDateTime(void) : LAPriceDataType(DATA_DATETIME), mData()
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
LADataDateTime::LADataDateTime(const LADataDateTime& attr)
: LAPriceDataType(attr), mData(attr.mData)
{
}

/*!
    @brief constructor

    @param[in] str string to set date time with format of "YYYYMMDD HH:MM:SS"
*/
LADataDateTime::LADataDateTime(const char_t*   str)
: LAPriceDataType(DATA_DATETIME)
{
	LAString data;
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
LADataDateTime::LADataDateTime(const LAString& str)
: LAPriceDataType(DATA_DATETIME)
{
	LAString data;
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
LADataDateTime::LADataDateTime(const LADateTime& dt)
: LAPriceDataType(DATA_DATETIME), mData(dt)
{
	setNull(false);
}

/*!
    @brief destructor
*/
LADataDateTime::~LADataDateTime(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, LAPriceDataType class) 
	is not copied, and Null is set
*/
LAPriceDataType*
LADataDateTime::clone() const
{
    try {
        LADataDateTime*	pAttr = new LADataDateTime(*this);
    	return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief convert time data of the data into string format"YYYYMMDD HH:MM:SS"

    @return string type of LAString
    @note if the data is NULL, it returns the string "NULL"
*/
LAString
LADataDateTime::convertToString(void) const
{
	return isNull() ? NULL_STR : mData.stringWithFormat();
}

/*!
    @brief set time data from a specific string format

    @param[in] str string data to be set with format of "YYYYMMDD HH:MM:SS"
*/
void          
LADataDateTime::convertFromString(const LAString& str)
{
	LAString data;
	bool ret = strToData(str, data);
	// notify of the change
	update();
	if (ret || data.size() == 0)
	{
		setNull();
		mData=LADateTime();
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
LADataDateTime::set(const LADateTime& dt)
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
LADataDateTime&
LADataDateTime::operator=(const LADateTime& dt)
{
	set(dt); // Method
	return *this;
}

/*!
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
LAPriceDataType&
LADataDateTime::assignment(const LAPriceDataType& a)
{
	// not copy when a == this
	if (this == &a) return *this;

	if (a.getType() != DATA_DATETIME) 
	{
		// exception
		LAString err = "Assignment error for LADataDateTime : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	mData=dynamic_cast<const LADataDateTime&>(a).mData;
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
LADataDateTime::compare(const LAPriceDataType& a) const
{
	if (a.getType() != DATA_DATETIME) 
	{
		// exception
		LAString err = "#Error: Expecting DateTime Data, instead of this data type: ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
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
	return mData.cmp(dynamic_cast<const LADataDateTime&>(a).mData);
}

//============ LADataDate =============================
/*!
    @brief default constructor
*/
LADataDate::LADataDate(void) : LAPriceDataType(DATA_DATE), mData()
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
LADataDate::LADataDate(const LADataDate& attr)
: LAPriceDataType(attr), mData(attr.mData)
{
}

/*!
    @brief constructor

    @param[in] str string data to be set and should be expressed by "YYYYMMDD" format
*/
LADataDate::LADataDate(const char_t*   str)
: LAPriceDataType(DATA_DATE)
{
	LAString data;
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
LADataDate::LADataDate(const LAString& str)
: LAPriceDataType(DATA_DATE)
{
	LAString data;
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
LADataDate::LADataDate(const LADate& dt)
: LAPriceDataType(DATA_DATE), mData(dt)
{
	setNull(false);
}

/*!
    @brief destructor
*/
LADataDate::~LADataDate(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, LAPriceDataType class) 
	is not copied, and Null is set
*/
LAPriceDataType*
LADataDate::clone() const
{
    try {
    	LADataDate*	pAttr = new LADataDate(*this);
    	return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief convert date data of the data into string format "YYYYMMDD"

    @return string type of LAString
    @note if the data is NULL, it returns the string "NULL"
*/
LAString
LADataDate::convertToString(void) const
{
	return isNull() ? NULL_STR : mData.stringWithFormat();
}

/*!
    @brief set date data from a specific string format

    @param[in] str string data to be set with format of "YYYYMMDD"
*/
void          
LADataDate::convertFromString(const LAString& str)
{
	LAString data;
	bool ret = strToData(str, data);
	// notify of the change
	update();
	if (ret || data.size() == 0)
	{
		mData = LADate();
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
LADataDate::set(const LADate& dt)
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
LADataDate&
LADataDate::operator=(const LADate& dt)
{
	set(dt); // Method
	return *this;
}

/*!
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
LAPriceDataType&
LADataDate::assignment(const LAPriceDataType& a)
{
	// not copy when a == this
	if (this == &a) return *this;
	if (a.getType() != DATA_DATE) 
	{
		// exception
		LAString err = "Assignment error for LADataDate : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	mData=dynamic_cast<const LADataDate&>(a).mData;
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
LADataDate::compare(const LAPriceDataType& a) const
{
	if (a.getType() != DATA_DATE) 
	{
		// exception
		LAString err = "Compare error for LADataDate : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
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
	return mData.cmp(dynamic_cast<const LADataDate&>(a).mData);
}

//============ LADataInt =============================
/*!
    @brief default constructor
*/
LADataInt::LADataInt(void) : LAPriceDataType(DATA_INT), mData(0)
{
}

/*!
    @brief copy constructor

    @param[in] b original object
*/
LADataInt::LADataInt(const LADataInt& b) 
				: LAPriceDataType(b), mData(b.mData)
{
}

/*!
    @brief constructor

    @param[in] b integer data to be set
*/
LADataInt::LADataInt(int b) 
				: LAPriceDataType(DATA_INT), mData(b)
{
	setNull(false);
}

/*!
    @brief destructor
*/
LADataInt::~LADataInt(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, LAPriceDataType class) 
	is not copied, and Null is set
*/
LAPriceDataType*	
LADataInt::clone() const
{
    try {
        LADataInt*	pAttr = new LADataInt(*this);
    	return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief convert integer data of the data into string format

    @return string type of LAString
    @note if the data is NULL, it returns the string "NULL"
*/
LAString      
LADataInt::convertToString(void) const
{
	return isNull() ? NULL_STR : LAString(mData);
}

/*!
    @brief set integer data from a specific string format

    @param[in] str string data to be set
*/void          
LADataInt::convertFromString(const LAString& str)
{
	LAString data;
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
LADataInt::set(int b)
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
LADataInt&
LADataInt::operator=(int b)
{
	set(b); // Method
	return *this;
}

/*!
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
LAPriceDataType&
LADataInt::assignment(const LAPriceDataType& a)
{
	// not copy when a == this
	if (this == &a) return *this;
	if (a.getType() != DATA_INT) 
	{	// if something is wrong with type check
		// exception
		LAString err = "Assignment error for LADataInt : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	// copy
	mData = dynamic_cast<const LADataInt&>(a).mData;
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
LADataInt::compare(const LAPriceDataType& a) const
{
	if (a.getType() != DATA_INT) 
	{
		// exception
		LAString err = "Compare error for LADataInt : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
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
	return mData - dynamic_cast<const LADataInt&>(a).mData;
}

//============ LADataDouble =============================
/*!
    @brief default constructor
*/
LADataDouble::LADataDouble(void) : 
			LAPriceDataType(DATA_DOUBLE), mPrecision(DOUBLE_PRECISION) 
{
}

/*!
    @brief copy constructor

    @param[in] d original object
*/
LADataDouble::LADataDouble(const LADataDouble& d) : 
			LAPriceDataType(d), mPrecision(d.mPrecision) 
{ 
	mData = d.mData;
}

/*!
    @brief constructor

    @param[in] d  double data to be set
    @param[in] pre number of decimal places to be set
*/
LADataDouble::LADataDouble(double d, int pre) : 
			LAPriceDataType(DATA_DOUBLE), mPrecision(pre) 
{ 
	setNull(false);
	mData = d;
}

/*!
    @brief destructor
*/
LADataDouble::~LADataDouble(void) 
{ 
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, LAPriceDataType class) 
	is not copied, and Null is set
*/
LAPriceDataType*
LADataDouble::clone() const
{
    try {
        LADataDouble*	pAttr = new LADataDouble(*this);
    	return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief convert double data of the data into string format

    @return string type of LAString
    @note if the data is NULL, it returns the string "NULL"
*/
LAString
LADataDouble::convertToString(void) const
{
     return isNull() ? NULL_STR : LAString(mData, mPrecision);
}

/*!
    @brief set double data from a specific string format

    @param[in] str string data to be set
*/
void
LADataDouble::convertFromString(
	const LAString& str)
{
	LAString data;
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
LADataDouble::set(double d)
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
LADataDouble&
LADataDouble::operator = (double b)
{
	set(b); // Method
	return *this;
}

/*!
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
LAPriceDataType&
LADataDouble::assignment(const LAPriceDataType& a)
{
	// not copy when a == this
	if (this == &a) return *this;
	if (a.getType() != DATA_DOUBLE)
	{
		// exception
		LAString err = "Assignment error for LADataDouble : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	const LADataDouble& flt = dynamic_cast<const LADataDouble&>(a);
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
LADataDouble::compare(const LAPriceDataType& a) const
{
	if (a.getType() != DATA_DOUBLE) 
	{
		// exception
		LAString err = "Compare error for LADataDouble : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
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

	double ret = mData - dynamic_cast<const LADataDouble&>(a).mData;
	if (ret == 0)
	{
		return 0;
	}
	else
	{
		return ret > 0 ? 1 : -1;
	}
}
