/*! @file
    @brief Implementation of basic data classes whose types are vector.

    Implement following base Data classes. (LADataBools, LADataInts, LADataDoubles, LADataStrings,
    LADataDates, LADataDateTimes)
	
*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LADataVector.h"

using namespace std;

//============ LADataBools  =============================
/*!
    @brief default constructor
*/
LADataBools::LADataBools(void) : LAPriceDataType(DATA_BOOLS)
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
LADataBools::LADataBools(const LADataBools& attr) : LAPriceDataType(DATA_BOOLS)
{
    setNull(attr.isNull());
    mData = attr.mData;
}

/*!
    @brief constructor

    @param[in] b bool vector data
*/
LADataBools::LADataBools(const BoolVector& b) : LAPriceDataType(DATA_BOOLS)
{
    setNull(b.size() == 0);
    mData = b;
}

/*!
    @brief destructor
*/
LADataBools::~LADataBools(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, LAPriceDataType class) 
	is not copied, and Null is set
*/
LAPriceDataType*
LADataBools::clone() const
{
    try {
        LADataBools*    pAttr = new LADataBools(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief converted to a string separated by delimiter Data array

    @return if it is NULL, return the "NULL" string data.
	           In the case of true / false, return "TRUE", "FALSE" string respectively.
*/
LAString      
LADataBools::convertToString(void) const
{
    LAString ret;
    if (isNull() )
    {
        ret = NULL_STR;
    }
    else
    {
        ret = "";
        vector<bool>::const_iterator it;
        for (it = mData.begin(); it != (mData.end()-1); ++it)
        {
            ret += (*it ? "TRUE" : "FALSE");
            ret += DATA_COLL_DEL;
        }
        ret += (*it ? "TRUE" : "FALSE");
    }
    return ret;
}

/*!
    @brief set boolian vector data from a specific string format

	true and false data is set by selecting "TRUE" or "FALSE" string separated by delimiter(mixed lowercase is OK)
	NULL value is set if no selecting or selecting "NULL" string.
	Other selected strings causes exception.

    @param[in] str "TRUE" or "FALSE"
*/
void          
LADataBools::convertFromString(
    const LAString& str)
{
    update();

    mData.clear();
    if (str == NULL_STR) 
    {
        setNull();
        return;
    }

    vector<LAString> tokens = str.toToken( DATA_COLL_DEL );
    vector<LAString>::iterator it;
    for (it = tokens.begin(); it != tokens.end(); ++it)
    {
        it->toUpper();
        if (*it == "TRUE") 
        {
            mData.push_back(true);
        }
        else if (*it == "FALSE") 
        {
            mData.push_back(false);
        }
        else 
        {
            // exception
            LAString err = "InvalidString for LADataBools : ";
            err += str;
            throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
        }
    }
    if (it != tokens.begin())
    {
        setNull(false);
    }
    else
    {
        setNull();
    }
}

/*!
    @brief set data at the specified position

    @param[in] b data to be set
    @param[in] i element number(starting at 0)
*/
void
LADataBools::set(bool b, int i)
{
    mData[i] = b;
    update();
}

/*!
    @brief set data

    @param[in] b data to be set
*/
void
LADataBools::set(const BoolVector& b)
{
    mData = b;
    if (mData.size() == 0) 
    {
        setNull();
    }
    else
    {
        setNull(false);
    }
    update();
}

/*!
    @brief add data to the end

    @param[in] b data to be added
*/
void
LADataBools::push_back(bool b)
{
    update();
    mData.push_back(b);
    if (isNull()) setNull(false);
}

/*!
    @brief an iterator pointing to the first element

    @return iterator pointing to the first element
*/
BoolVector::const_iterator
LADataBools::begin() const
{
    return mData.begin();
}

/*!
    @brief an iterator pointing to the element at the end

    @return iterator pointing to the element at the end
*/
BoolVector::const_iterator
LADataBools::end() const
{
    return mData.end();
}

/*!
    @brief insert the data
    
    @param[in] it iterator pointing to the element at the insertion (inserted before the element to be specified)
    @param[in] b data to be inserted
*/
void
//LADataBools::insert(BoolVector::const_iterator it, bool b)
LADataBools::insert(BoolVector::iterator it, bool b)
{
    update();
// ysuzuki
//    int i = it - begin();
//    mData.insert(mData.begin()+i, b);
    mData.insert(it, b);
    if (isNull()) setNull(false);
}

/*!
    @brief delete the data for the specified element
    
    @param[in] it iterator pointing to the element to be deleted
*/
void                    
//LADataBools::erase(BoolVector::const_iterator it)
LADataBools::erase(BoolVector::iterator it)
{
    update();
// ysuzuki
//    int i = it - begin();
//    mData.erase(mData.begin() + i);
    mData.erase(it);
    if (mData.size() == 0) setNull();
}

/*!
    @brief clear all data
*/
void
LADataBools::clear()
{
    update();
    setNull();
    mData.clear();
}

/*
    @brief array subscript operator
    
    @param[in] i array subscript

    @return the value at the specified position
*/
bool            
LADataBools::operator[](int i) const
{
    return mData[i];
}

/*
    @brief assignment operator

    @param[in] b right hand side vector data of assignment operator

    @return the object
*/
LADataBools&
LADataBools::operator =(const BoolVector& b)
{
    set(b);
    return *this;
}

/*
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
LAPriceDataType&
LADataBools::assignment(const LAPriceDataType& a)
{
    if (this == &a) return *this;
    if (a.getType() != DATA_BOOLS) 
    {
        // exception
        LAString err = "Assignment error for LADataBools : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    set(dynamic_cast<const LADataBools&>(a).mData);
    setNull(a.isNull());
    return *this;
}

/*
    @brief compare the contents against the other objecet (Magnitude comparison is carried out in the number of elements.)
    
    an exception occurs when different compared to the Data object

    @param[in] a the object to be compared
    
    @retval Number of elements of the object - Number of elements of the object to be compared
*/
int
LADataBools::compare(
    const LAPriceDataType& a) const
{
    if (a.getType() != DATA_BOOLS) 
    {
        LAString err = "Compare error for LADataBools : from ";
        err += LAString(a.getType());
        // exception
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    return mData.size() - 
            dynamic_cast<const LADataBools&>(a).mData.size();
}

//============ LADataStrings  =============================
/*!
    @brief default constructor
*/
LADataStrings::LADataStrings(void) : LAPriceDataType(DATA_STRINGS)
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
LADataStrings::LADataStrings(const LADataStrings& attr) 
: LAPriceDataType(DATA_STRINGS), mData(attr.mData)
{
    if (mData.size() != 0)
        setNull(false);
}

/*!
    @brief constructor

    @param[in] attr LAString vector data
*/
LADataStrings::LADataStrings(const LAStringVector& attr) 
: LAPriceDataType(DATA_STRINGS), mData(attr)
{
    if (mData.size() != 0)
        setNull(false);
}

/*!
    @brief destructor
*/
LADataStrings::~LADataStrings(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, LAPriceDataType class) 
	is not copied, and Null is set
*/
LAPriceDataType*
LADataStrings::clone() const
{
    try {
        LADataStrings*  pAttr = new LADataStrings(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief converted to a string separated by delimiter Data array

    @retval NULL in case of NULL data
	@retval string that it holds otherwise

*/
LAString
LADataStrings::convertToString(void) const
{
    LAString ret;
    if (isNull() )
    {
        ret = NULL_STR;
    }
    else
    {
        ret = "";
        vector<LAString>::const_iterator it;
        for (it = mData.begin(); it != (mData.end()-1); ++it)
        {
            ret += *it;
            ret += DATA_COLL_DEL;
        }
        ret += *it;
    }
    return ret;
}

/*!
    @brief set string vector data from a specific string format

	string data is set into vector if selecting string separated by delimiter

    @param[in] str string separated by a delimiter string
*/
void          
LADataStrings::convertFromString(
    const LAString& str)
{
    vector<LAString> tokens = str.toToken( DATA_COLL_DEL );
    set(tokens);
}

/*!
    @brief set data at the specified position

    @param[in] str data to be set
    @param[in] i element number(starting at 0)
*/
void
LADataStrings::set(const LAString& str, int i)
{
    mData[i] = str;
    update();
}

/*!
    @brief set data

    @param[in] d data to be set
*/
void
LADataStrings::set(const LAStringVector& d)
{
    update();
    mData = d;
    if (mData.size() != 0)
    {
        setNull(false);
    }
    else
    {
        setNull();
    }
}

/*!
    @brief add data to the end

    @param[in] b data to be added
*/
void
LADataStrings::push_back(const LAString& b)
{
    update();
    mData.push_back(b);
    if (isNull()) setNull(false);
}

/*!
    @brief an iterator pointing to the first element

    @return iterator pointing to the first element
*/
LAStringVector::const_iterator
LADataStrings::begin() const
{
    return mData.begin();
}

/*!
    @brief an iterator pointing to the element at the end

    @return iterator pointing to the element at the end
*/
LAStringVector::const_iterator
LADataStrings::end() const
{
    return mData.end();
}

/*!
    @brief insert the data
    
    @param[in] it iterator pointing to the element at the insertion (inserted before the element to be specified)
    @param[in] b data to be inserted
*/
void
//LADataStrings::insert(LAStringVector::const_iterator it, const LAString& b)
LADataStrings::insert(LAStringVector::iterator it, const LAString& b)
{
    update();
// ysuzuki
//    int i = it - begin();
//    mData.insert(mData.begin()+i, b);
    mData.insert(it, b);
    if (isNull()) setNull(false);
}

/*!
    @brief delete the data for the specified element
    
    @param[in] it iterator pointing to the element to be deleted
*/
void                    
//LADataStrings::erase(LAStringVector::const_iterator it)
LADataStrings::erase(LAStringVector::iterator it)
{
    update();
// ysuzuki
//    int i = it - begin();
//    mData.erase(mData.begin() + i);
    mData.erase(it);
    if (mData.size() == 0) setNull();
}

/*!
    @brief clear all data
*/
void
LADataStrings::clear()
{
    update();
    setNull();
    mData.clear();
}

/*
    @brief array subscript operator
    
    @param[in] i array subscript

    @return the value at the specified position
*/
const LAString&         
LADataStrings::operator[](int i) const
{
    return mData[i];
}

/*
    @brief assignment operator

    @param[in] d right hand side vector data of assignment operator

    @return the object
*/
LADataStrings&
LADataStrings::operator=(const LAStringVector& d)
{
    set(d);
    return *this;
}

/*
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
LAPriceDataType&
LADataStrings::assignment(const LAPriceDataType& a)
{
    if (this == &a) return *this;

    if (a.getType() != DATA_STRINGS) 
    {
        // exception
        LAString err = "Assignment error for LADataStrings : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    set(dynamic_cast<const LADataStrings&>(a).mData);
    return *this;
}

/*
    @brief compare the contents against the other objecet (Magnitude comparison is carried out in the number of elements.)
    
    an exception occurs when different compared to the Data object

    @param[in] a the object to be compared
    
    @retval Number of elements of the object - Number of elements of the object to be compared
*/
int
LADataStrings::compare(
    const LAPriceDataType& a) const
{
    if (a.getType() != DATA_STRINGS) 
    {
        // exception
        LAString err = "Compare error for LADataStrings : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    return mData.size() - 
        dynamic_cast<const LADataStrings&>(a).mData.size();
}

//============ LADataDateTimes  =============================
/*!
    @brief default constructor
*/
LADataDateTimes::LADataDateTimes(void) : LAPriceDataType(DATA_DATETIMES)
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
LADataDateTimes::LADataDateTimes(const LADataDateTimes& attr) 
: LAPriceDataType(DATA_DATETIMES), mData(attr.mData)
{
    if (mData.size() != 0)
        setNull(false);
}

/*!
    @brief constructor

    @param[in] attr bool LADateTime vector data
*/
LADataDateTimes::LADataDateTimes(const DateTimeVector& attr) 
: LAPriceDataType(DATA_DATETIMES), mData(attr)
{
    if (mData.size() != 0)
        setNull(false);
}

/*!
    @brief destructor
*/
LADataDateTimes::~LADataDateTimes(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, LAPriceDataType class) 
	is not copied, and Null is set
*/
LAPriceDataType*
LADataDateTimes::clone() const
{
    try {
        LADataDateTimes*    pAttr = new LADataDateTimes(*this);
        return pAttr;   
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief converted to a string separated by delimiter Data array

	@retval NULL in case of NULL data
	@retval string of date time data that it holds otherwise

*/
LAString
LADataDateTimes::convertToString(void) const
{
    LAString ret;
    if (isNull())
    {
        ret = NULL_STR;
    }
    else
    {
        ret = "";
        vector<LADateTime>::const_iterator it;
        for (it = mData.begin(); it != (mData.end()-1); ++it)
        {
            ret += (*it).stringWithFormat();
            ret += DATA_COLL_DEL;
        }
        ret += (*it).stringWithFormat();
    }
    return ret;
}

/*!
    @brief set date vector data from a specific string format

	date time data is set into vector if selecting date time string separated by delimiter

    @param[in] str date time string separated by a delimiter string
*/
void
LADataDateTimes::convertFromString(
    const LAString& str)
{
    update();
	mData.clear();
    if (str == NULL_STR)
    {
        setNull();
        return;
    }

    vector<LAString> tokens = str.toToken( DATA_COLL_DEL );
    vector<LAString>::iterator it;
    for (it = tokens.begin(); it != tokens.end(); ++it)
    {
        try 
        {
            LADateTime d((*it).getCString());
            mData.push_back(d);
        }
        catch(...) 
        {
            // exception
            LAString err = "InvalidString for LADataDates : ";
            err += str;
            throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
        }
    }
    if (it != tokens.begin())
    {
        setNull(false);
    }
    else
    {
        setNull();
    }
}

/*!
    @brief set data

    @param[in] d data to be set
    @param[in] i element number(starting at 0)
*/
void
LADataDateTimes::set(const LADateTime& d, int i)
{
    mData[i] = d;
    update();
}

/*!
    @brief set the specified data

    @param[in] d data to be set
*/
void
LADataDateTimes::set(const DateTimeVector& d)
{
    mData = d;
    update();
    if (mData.size() == 0)
    {
        setNull();
    }
    else
    {
        setNull(false);
    }
}

/*!
    @brief add data to the end

    @param[in] b data to be added
*/
void
LADataDateTimes::push_back(const LADateTime& b)
{
    update();
    mData.push_back(b);
    if (isNull()) setNull(false);
}

/*!
    @brief an iterator pointing to the first element

    @return iterator pointing to the first element
*/
DateTimeVector::const_iterator
LADataDateTimes::begin() const
{
    return mData.begin();
}

/*!
    @brief an iterator pointing to the element at the end

    @return iterator pointing to the element at the end
*/
DateTimeVector::const_iterator
LADataDateTimes::end() const
{
    return mData.end();
}

/*!
    @brief insert the data
    
    @param[in] it iterator pointing to the element at the insertion (inserted before the element to be specified)
    @param[in] b data to be inserted
*/
void                    
//LADataDateTimes::insert(DateTimeVector::const_iterator it, const LADateTime& b)
LADataDateTimes::insert(DateTimeVector::iterator it, const LADateTime& b)
{
    update();
//  ysuzuki
//    int i = it - begin();
//    mData.insert(mData.begin() + i, b);
    mData.insert(it, b);
    if (isNull()) setNull(false);
}

/*!
    @brief delete the data for the specified element
    
    @param[in] it iterator pointing to the element to be deleted
*/
void                    
//LADataDateTimes::erase(DateTimeVector::const_iterator it)
LADataDateTimes::erase(DateTimeVector::iterator it)
{
    update();
// ysuzuki
//    int i = it - begin();
//    mData.erase(mData.begin() + i);
    mData.erase(it);
    if (mData.size() == 0) setNull();
}

/*!
    @brief clear all data
*/
void                    
LADataDateTimes::clear()
{
    update();
    setNull();
    mData.clear();
}

/*
    @brief array subscript operator
    
    @param[in] i array subscript

    @return the value at the specified position
*/
const LADateTime&                   
LADataDateTimes::operator[] (int i) const
{
    return mData[i];
}

/*
    @brief assignment operator

    @param[in] d right hand side vector data of assignment operator

    @return the object
*/
LADataDateTimes&
LADataDateTimes::operator=(const DateTimeVector& d)
{
    set(d);
    return *this;
}

/*
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
LAPriceDataType&
LADataDateTimes::assignment(const LAPriceDataType& a)
{
    if (this == &a) return *this;

    if (a.getType() != DATA_DATETIMES) 
    {
        // exception
        LAString err = "Assignment error for LADataDateTimes : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    set(dynamic_cast<const LADataDateTimes&>(a).mData);
    return *this;
}

/*
    @brief compare the contents against the other objecet (Magnitude comparison is carried out in the number of elements.)
    
    an exception occurs when different compared to the Data object

    @param[in] a the object to be compared
    
    @retval Number of elements of the object - Number of elements of the object to be compared
*/
int
LADataDateTimes::compare(
    const LAPriceDataType& a) const
{
    if (a.getType() != DATA_DATETIMES) 
    {
        // exception
        LAString err = "Compare error for LADataDateTimes : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }

    return mData.size() - 
            dynamic_cast<const LADataDateTimes&>(a).mData.size();
}

//============ LADataDates =============================
/*!
    @brief default constructor
*/
LADataDates::LADataDates(void) : LAPriceDataType(DATA_DATES)
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
LADataDates::LADataDates(const LADataDates& attr) 
: LAPriceDataType(DATA_DATES), mData(attr.mData)
{
    if (mData.size() != 0)
        setNull(false);
}

/*!
    @brief constructor

    @param[in] attr LADate vector data
*/
LADataDates::LADataDates(const DateVector& attr) 
: LAPriceDataType(DATA_DATES), mData(attr)
{
    if (mData.size() != 0)
        setNull(false);
}

/*!
    @brief destructor
*/
LADataDates::~LADataDates(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, LAPriceDataType class) 
	is not copied, and Null is set
*/
LAPriceDataType*
LADataDates::clone() const
{
    try {
        LADataDates*    pAttr = new LADataDates(*this);
        return pAttr;   
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief converted to a string separated by delimiter Data array

	@retval NULL in case of NULL data
	@retval string of date that it holds otherwise

*/
LAString
LADataDates::convertToString(void) const
{
    LAString ret;
    if (isNull() )
    {
        ret = NULL_STR;
    }
    else
    {
        ret = "";
        vector<LADate>::const_iterator it;
        for (it = mData.begin(); it != (mData.end()-1); ++it)
        {
            ret += (*it).stringWithFormat();
            ret += DATA_COLL_DEL;
        }
        ret += (*it).stringWithFormat();
    }
    return ret;
}

/*!
    @brief set date vector data from a specific string format

	date data is set into vector if selecting date string separated by delimiter

	@param[in] str date string separated by a delimiter string
*/
void
LADataDates::convertFromString(
    const LAString& str)
{
    update();
	mData.clear();
    if (str == NULL_STR)
    {
        setNull();
        return;
    }

    vector<LAString> tokens = str.toToken( DATA_COLL_DEL );
    vector<LAString>::iterator it;
    for (it = tokens.begin(); it != tokens.end(); ++it)
    {
        try 
        {
            LADate d((*it).getCString());
            mData.push_back(d);
        }
        catch(...) 
        {
            // exception
            LAString err = "InvalidString for LADataDates : ";
            err += str;
            throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
        }
    }
    if (it != tokens.begin())
    {
        setNull(false);
    }
    else
    {
        setNull();
    }
}

/*!
    @brief set data at the specified position 

    @param[in] d data to be set
    @param[in] i element number(starting at 0)
*/
void
LADataDates::set(const LADate& d, int i)
{
    mData[i] = d;
    update();
}

/*!
    @brief set data

    @param[in] d data to be set
*/
void
LADataDates::set(const DateVector& d)
{
    mData = d;
    update();
    if (mData.size() == 0)
    {
        setNull();
    }
    else
    {
        setNull(false);
    }
}

/*!
    @brief add data to the end

    @param[in] b data to be added
*/
void
LADataDates::push_back(const LADate& b)
{
    update();
    mData.push_back(b);
    if (isNull()) setNull(false);
}

/*!
    @brief an iterator pointing to the first element

    @return iterator pointing to the first element
*/
DateVector::const_iterator
LADataDates::begin() const
{
    return mData.begin();
}

/*!
    @brief an iterator pointing to the element at the end

    @return iterator pointing to the element at the end
*/
DateVector::const_iterator
LADataDates::end() const
{
    return mData.end();
}

/*!
    @brief insert the data
    
    @param[in] it iterator pointing to the element at the insertion (inserted before the element to be specified)
    @param[in] b data to be inserted
*/
void                    
//LADataDates::insert(DateVector::const_iterator it, const LADate& b)
LADataDates::insert(DateVector::iterator it, const LADate& b)
{
    update();
// ysuzuki
//    int i = it - begin();
//    mData.insert(mData.begin()+i, b);
    mData.insert(it, b);
    if (isNull()) setNull(false);
}

/*!
    @brief delete the data for the specified element
    
    @param[in] it iterator pointing to the element to be deleted
*/
void                    
//LADataDates::erase(DateVector::const_iterator it)
LADataDates::erase(DateVector::iterator it)
{
    update();
// ysuzuki
//    int i = it - begin();
//    mData.erase(mData.begin() + i);
    mData.erase(it);
    if (mData.size() == 0) setNull();
}

/*!
    @brief clear all data
*/
void                    
LADataDates::clear()
{
    update();
    setNull();
    mData.clear();
}

/*
    @brief array subscript operator
    
    @param[in] i array subscript

    @return the value at the specified position
*/
const LADate&                   
LADataDates::operator[] (int i) const
{
    return mData[i];
}

/*
    @brief assignment operator

    @param[in] d right hand side vector data of assignment operator

    @return the object
*/
LADataDates&
LADataDates::operator=(const DateVector& d)
{
    set(d);
    return *this;
}

/*
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
LAPriceDataType&
LADataDates::assignment(const LAPriceDataType& a)
{
    if (a.getType() != DATA_DATES) 
    {
        // exception
        LAString err = "Assignment error for LADataDates : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    set(dynamic_cast<const LADataDates&>(a).mData);
    return *this;
}

/*
    @brief compare the contents against the other objecet (Magnitude comparison is carried out in the number of elements.)
    
    an exception occurs when different compared to the Data object

    @param[in] a the object to be compared
    
    @retval Number of elements of the object - Number of elements of the object to be compared
*/
int
LADataDates::compare(
    const LAPriceDataType& a) const
{
    if (a.getType() != DATA_DATES) 
    {
        // exception
        LAString err = "Compare error for LADataDates : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }

    return mData.size() - 
            dynamic_cast<const LADataDates&>(a).mData.size();
}

//============ LADataInts =============================
/*!
    @brief default constructor
*/
LADataInts::LADataInts(void) : LAPriceDataType(DATA_INTS)
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
LADataInts::LADataInts(const LADataInts& attr) 
: LAPriceDataType(DATA_INTS), mData(attr.mData)
{
    if (mData.size() != 0)
        setNull(false);
}

/*!
    @brief constructor

    @param[in] attr valarray data with int type
*/
LADataInts::LADataInts(const IntArray& attr) 
: LAPriceDataType(DATA_INTS), mData(attr)
{
    if (mData.size() != 0)
        setNull(false);
}


/*!
    @brief destructor
*/
LADataInts::~LADataInts(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, LAPriceDataType class) 
	is not copied, and Null is set
*/
LAPriceDataType*
LADataInts::clone() const
{
    try {
        LADataInts* pAttr = new LADataInts(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief converted to a string separated by delimiter Data array

	@retval NULL in case of NULL data
	@retval string of integer data that it holds otherwise

*/
LAString
LADataInts::convertToString(void) const
{
    LAString ret;
    if (isNull() )
    {
        ret = NULL_STR;
    }
    else
    {
        ret = "";
		int i;
        int size = mData.size();
        for (i=0; i< size-1; i++)
        {
            ret += LAString(mData[i]);
            ret += DATA_COLL_DEL;
        }
        ret += LAString(mData[size-1]);
    }
    return ret;
}

/*!
    @brief set integer vector data from a specific string format

	integer data is set into vector if selecting integer string separated by delimiter

    @param[in] str inteter string separated by a delimiter string
*/
void
LADataInts::convertFromString(
    const LAString& str)
{
    update();

    mData.clear();
    if (str == NULL_STR) 
    {
        setNull();
        return;
    }

    vector<LAString> tokens = str.toToken( DATA_COLL_DEL );
    vector<LAString>::iterator it;
    for (it = tokens.begin(); it != tokens.end(); ++it)
    {
        mData.resize(mData.size() + 1, (*it).getIntValue());
    }
    if (it != tokens.begin())
    {
        setNull(false);
    }
    else
    {
        setNull();
    }
}

/*!
    @brief set data at the specified position

    @param[in] d data to be set
    @param[in] i element number(starting at 0)
*/
void
LADataInts::set(int d, int i)
{
    mData[i] = d;
    update();
}

/*!
    @brief set data

    @param[in] d data to be set
*/
void
LADataInts::set(const IntArray& d)
{
    mData = d;
    update();
    if (mData.size() == 0)
    {
        setNull();
    }
    else
    {
        setNull(false);
    }
}


/*!
    @brief resize data. In case of element added, assign a value i

    @param[in] num new size
	@param[in] i value to be assigned in case of element added
*/   
void
LADataInts::resize(unsigned int num, const int i)
{
    update();
	mData.resize(num,i);
}

/*!
    @brief clear all data
*/
void                    
LADataInts::clear()
{
    update();
    setNull();
    mData.clear();
}

/*
    @brief array subscript operator
    
    @param[in] i array subscript

    @return the value at the specified position
*/
int                 
LADataInts::operator[] (int i) const
{
    return mData[i];
}

/*
    @brief assignment operator

    @param[in] d right hand side vector data of assignment operator

    @return the object
*/
LADataInts&
LADataInts::operator=(const IntArray& d)
{
    set(d);
    return *this;
}

/*
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
LAPriceDataType&
LADataInts::assignment(const LAPriceDataType& a)
{
    if (this == &a) return *this;

    if (a.getType() != DATA_INTS)
    {
        // exception
        LAString err = "Assignment error for LADataInts : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    set(dynamic_cast<const LADataInts&>(a).mData);
    return *this;
}

/*
    @brief compare the contents against the other objecet (Magnitude comparison is carried out in the number of elements.)
    
    @param[in] a the object to be compared
    
    @retval Number of elements of the object - Number of elements of the object to be compared
*/
int
LADataInts::compare(
    const LAPriceDataType& a) const
{
    if (a.getType() != DATA_INTS) 
    {
        // exception
        LAString err = "Compare error for LADataInts : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    return mData.size() - 
        dynamic_cast<const LADataInts&>(a).mData.size();
}

//============ LADataDoubles =============================
/*!
    @brief constructor
    
    @param[in] pre number of decimal places
*/
LADataDoubles::LADataDoubles(int pre) : LAPriceDataType(DATA_DOUBLES)
{
    mPre = pre;
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
LADataDoubles::LADataDoubles(const LADataDoubles& attr) 
: LAPriceDataType(DATA_DOUBLES), mData(attr.mData), mPre(attr.mPre)
{
    if (mData.size() != 0)
        setNull(false);
}

/*!
    @brief constructor

    @param[in] attr valarray data with doubl type 
    @param[in] pre number of decimal places
*/
LADataDoubles::LADataDoubles(const DoubleArray& attr, int pre) 
: LAPriceDataType(DATA_DOUBLES), mData(attr), mPre(pre)
{
    if (mData.size() != 0)
        setNull(false);
}

/*!
    @brief destructor
*/
LADataDoubles::~LADataDoubles(void)
{
}

/*!
    @brief deep copy of this object

    @return the copied object

    @note mpHolder which is one of member in coping object (base class, LAPriceDataType class) 
	is not copied, and Null is set
*/
LAPriceDataType*
LADataDoubles::clone() const 
{
    try {
        LADataDoubles*  pAttr = new LADataDoubles(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief converted to a string separated by delimiter Data array

	@retval NULL in case of NULL data
	@retval string of double data that it holds otherwise

*/
LAString
LADataDoubles::convertToString(void) const
{
    LAString result;
    if (isNull() )
    {
        result = NULL_STR;
    }
    else
    {
		// OLD
		// ***

		// result = "";
		// size_t size = mData.size();
		//
		// for ( size_t i = 0; i < size-1; ++i)
        // {
        //     result += LAString(mData[i], mPre);
        //     result += DATA_COLL_DEL;
		// 	
		// 	
        // }
        // result += LAString(mData[size-1], mPre);

		// NEW
		// ***

		result = "";
		size_t size = mData.size();

		for ( size_t i = 0; i < size-1; ++i)
        {
            result += std::to_string(mData[i]) + DATA_COLL_DEL;
        }
        result += std::to_string(mData[size-1]);
    }
	
    return result;
}

/*!
    @brief set double vector data from a specific string format

    double data is set into vector if selecting double string separated by delimiter

    @param[in] str double string separated by a delimiter string
*/
void
LADataDoubles::convertFromString(
    const LAString& str)
{
    update();

	mData.clear();
    if (str == NULL_STR) 
    {
        setNull();
        return;
    }

    vector<LAString> tokens = str.toToken( DATA_COLL_DEL );
    vector<LAString>::iterator it;
    for (it = tokens.begin(); it != tokens.end(); ++it)
    {
        mData.resize(mData.size()+1,(*it).getDoubleValue());
    }
    if (it != tokens.begin())
    {
        setNull(false);
    }
    else
    {
        setNull();
    }
}

/*!
    @brief set data at the specified position

    @param[in] d data to be set
    @param[in] i element number(starting at 0)
*/
void
LADataDoubles::set(double d, int i)
{
    mData[i] = d;
    update();
}

/*!
    @brief set data

    @param[in] d data to be set
*/
void
LADataDoubles::set(const DoubleArray& d)
{
    mData = d;
    update();
    if (mData.size() == 0)
    {
        setNull();
    }
    else
    {
        setNull(false);
    }
}

/*!
    @brief  resize data. In case of element added, assign a value i.

    @param[in] num new size
	@param[in] d value to be assigned in case of element added
*/   
void
LADataDoubles::resize(unsigned int num, const double d)
{
    update();
	mData.resize(num,d);
}

/*!
    @brief clear all data
*/
void                    
LADataDoubles::clear()
{
    update();
    setNull();
    mData.clear();
}

/*
    @briefarray subscript operator
    
    @param[in] i array subscript

    @return the value at the specified position
*/
double                  
LADataDoubles::operator[] (int i) const
{
    return mData[i];
}

/*
    @brief assignment operator

    @param[in] d right hand side vector data of assignment operator

    @return the object
*/
LADataDoubles&
LADataDoubles::operator=(const DoubleArray& d)
{
    set(d);
    return *this;
}

/*
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents

    @return the object
*/
LAPriceDataType&
LADataDoubles::assignment(const LAPriceDataType& a)
{
    if (this == &a) return *this;
    if (a.getType() != DATA_DOUBLES) 
    {
        // exception
        LAString err = "Assignment error for LADataDoubles : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    set(dynamic_cast<const LADataDoubles&>(a).mData);
    mPre = dynamic_cast<const LADataDoubles&>(a).mPre;
    return *this;
}

/*
    @brief compare the contents against the other objecet (Magnitude comparison is carried out in the number of elements.)
    
    an exception occurs when different compared to the Data object

    @param[in] a the object to be compared
    
    @retval Number of elements of the object - Number of elements of the object to be compared
*/
int
LADataDoubles::compare(
    const LAPriceDataType& a) const
{
    if (a.getType() != DATA_DOUBLES) 
    {
        // exception
        LAString err = "Compare error for LADataDoubles : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    return mData.size() - 
                dynamic_cast<const LADataDoubles&>(a).mData.size();
}
